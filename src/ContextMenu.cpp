#include "stdafx.hpp"
#include "Query.hpp"
#include "RequestThread.hpp"
#include "TOC.hpp"
#include "DialogAlbumID.hpp"
#include "DialogArtistAlbum.hpp"

struct ContextItem
{
	const GUID* guid;
	const std::string name;
};

static const std::vector<ContextItem> context_items =
{
	{ &guids::context_get_by_toc, "Get tags by TOC" },
	{ &guids::context_get_by_artist_album, "Get tags by artist & album" },
	{ &guids::context_get_by_albumid, "Get tags by MusicBrainz Album ID" },
	{ &guids::context_add_toc, "Add TOC to MusicBrainz" }
};

class ContextMenu : public contextmenu_item_simple
{
public:
	GUID get_item_guid(uint32_t index) final
	{
		if (index >= context_items.size()) FB2K_BugCheck();

		return *context_items[index].guid;
	}

	GUID get_parent() final
	{
		return guids::context_group;
	}

	bool context_get_display(uint32_t index, metadb_handle_list_cref handles, pfc::string_base& out, uint32_t&, const GUID&) final
	{
		if (index >= context_items.size()) FB2K_BugCheck();

		get_item_name(index, out);

		// Always display search/albumid options
		if (index == 1 || index == 2) return true; 

		// TOC options are conditional based on selection properties
		if (handles.get_count() > 99) return false;
		auto recs = metadb_v2::get()->queryMultiSimple(handles);
		if (index == 0) return check_samplerate(recs);
		if (index == 3) return check_cdda(recs);
		return false;
	}

	bool get_item_description(uint32_t index, pfc::string_base& out) final
	{
		if (index >= context_items.size()) FB2K_BugCheck();

		out = context_items[index].name;
		return true;
	}

	uint32_t get_num_items() final
	{
		return static_cast<uint32_t>(context_items.size());
	}

	void context_command(uint32_t index, metadb_handle_list_cref handles, const GUID&) final
	{
		if (index >= context_items.size()) FB2K_BugCheck();

		auto recs = metadb_v2::get()->queryMultiSimple(handles);
		if (index == 0) get_by_toc(recs, handles);
		else if (index == 1) get_by_artist_album(recs, handles);
		else if (index == 2) get_by_albumid(recs, handles);
		else if (index == 3) add_toc(recs);
	}

	void get_item_name(uint32_t index, pfc::string_base& out) final
	{
		if (index >= context_items.size()) FB2K_BugCheck();
		out = context_items[index].name;
	}

private:
	bool check_cdda(const pfc::array_t<metadb_v2::rec_t>& recs)
	{
		for (auto&& rec : recs)
		{
			if (rec.info.is_empty()) return false;
			if (rec.info->info().is_encoding_lossy()) return false;
			if (rec.info->info().info_get_int("samplerate") != 44100) return false;
			if (rec.info->info().info_get_length_samples() % 588 != 0) return false;
		}
		return true;
	}

	bool check_samplerate(const pfc::array_t<metadb_v2::rec_t>& recs)
	{
		if (recs[0].info.is_empty()) return false;

		const int64_t samplerate = recs[0].info->info().info_get_int("samplerate");
		if (samplerate != 44100 && samplerate != 48000) return false;
		const uint32_t bits_per_sector = samplerate == 48000 ? 640U : 588U;

		for (auto&& rec : recs)
		{
			if (rec.info.is_empty()) return false;
			if (rec.info->info().info_get_int("samplerate") != samplerate) return false;
			if (rec.info->info().info_get_length_samples() % bits_per_sector != 0) return false;
		}
		return true;
	}

	void add_toc(const pfc::array_t<metadb_v2::rec_t>& recs)
	{
		if (recs.get_count() > 99 || !check_cdda(recs)) return;

		const std::string url = TOC(recs).get_url();
		ShellExecuteW(nullptr, L"open", pfc::wideFromUTF8(url.c_str()), nullptr, nullptr, SW_SHOW);
	}

	void get_by_albumid(const pfc::array_t<metadb_v2::rec_t>& recs, metadb_handle_list_cref handles)
	{
		const HWND hwnd = core_api::get_main_window();
		bool first = true;
		std::string albumid;

		for (auto&& rec : recs)
		{
			if (rec.info.is_empty())
			{
				albumid.clear();
				break;
			}

			const char* current_albumid = rec.info->info().meta_get("MUSICBRAINZ_ALBUMID", 0);
			if (current_albumid == nullptr) current_albumid = rec.info->info().meta_get("MUSICBRAINZ ALBUM ID", 0);

			if (!is_uuid(current_albumid))
			{
				albumid.clear();
				break;
			}

			if (first)
			{
				albumid = current_albumid;
				first = false;
			}
			else if (albumid != current_albumid)
			{
				albumid.clear();
				break;
			}
		}

		modal_dialog_scope scope;
		if (scope.can_create())
		{
			scope.initialize(hwnd);
			CDialogAlbumID dlg(albumid);
			if (dlg.DoModal(hwnd) == IDOK)
			{
				auto query = std::make_unique<Query>("release", dlg.m_albumid);
				query->add_param("inc", Query::s_inc_release);

				auto cb = fb2k::service_new<RequestThread>(RequestThread::Type::AlbumID, std::move(query), handles);
				threaded_process::get()->run_modeless(cb, threaded_process::flag_show_delayed, hwnd, "Querying data from MusicBrainz");
			}
		}
	}

	void get_by_artist_album(const pfc::array_t<metadb_v2::rec_t>& recs, metadb_handle_list_cref handles)
	{
		const HWND hwnd = core_api::get_main_window();
		const bool single = handles.get_count() == 1;
		bool first = true;
		std::string artist, album;

		for (auto&& rec : recs)
		{
			if (rec.info.is_empty())
			{
				artist.clear();
				album.clear();
				break;
			}

			const char* current_artist = rec.info->info().meta_get("ALBUM ARTIST", 0);
			if (current_artist == nullptr) current_artist = rec.info->info().meta_get("ARTIST", 0);

			const char* current_album = rec.info->info().meta_get("ALBUM", 0);
			if (current_album == nullptr && first && single) current_album = rec.info->info().meta_get("TITLE", 0);

			if (current_artist == nullptr || current_album == nullptr)
			{
				artist.clear();
				album.clear();
				break;
			}
			else
			{
				if (first)
				{
					artist = current_artist;
					album = current_album;
					first = false;
				}
				else if (artist != current_artist || album != current_album)
				{
					artist.clear();
					album.clear();
					break;
				}
			}
		}

		modal_dialog_scope scope;
		if (scope.can_create())
		{
			scope.initialize(hwnd);
			CDialogArtistAlbum dlg(artist, album);
			if (dlg.DoModal(hwnd) == IDOK)
			{
				const std::string search = fmt::format("artist:\"{}\" AND release:\"{}\"", dlg.m_artist, dlg.m_album);

				pfc::string8 encoded_search;
				pfc::urlEncode(encoded_search, search.c_str());

				auto query = std::make_unique<Query>("release");
				query->add_param("query", encoded_search);
				query->add_param("limit", "100");

				static constexpr uint32_t flags = threaded_process::flag_show_progress | threaded_process::flag_show_abort | threaded_process::flag_show_delayed;
				auto cb = fb2k::service_new<RequestThread>(RequestThread::Type::Search, std::move(query), handles);
				threaded_process::get()->run_modeless(cb, flags, hwnd, "Querying data from MusicBrainz");
			}
		}
	}

	void get_by_toc(const pfc::array_t<metadb_v2::rec_t>& recs, metadb_handle_list_cref handles)
	{
		if (recs.get_count() > 99 || !check_samplerate(recs)) return;

		const HWND hwnd = core_api::get_main_window();
		const std::string discid = TOC(recs).get_discid();

		auto query = std::make_unique<Query>("discid", discid);
		query->add_param("cdstubs", "no");
		query->add_param("inc", Query::s_inc_discid);

		auto cb = fb2k::service_new<RequestThread>(RequestThread::Type::DiscID, std::move(query), handles);
		threaded_process::get()->run_modeless(cb, threaded_process::flag_show_delayed, hwnd, "Querying data from MusicBrainz");
	}
};

static contextmenu_group_popup_factory g_context_group(guids::context_group, contextmenu_groups::root, Component::name.data(), 0);
FB2K_SERVICE_FACTORY(ContextMenu);
