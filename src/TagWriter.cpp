#include "stdafx.hpp"
#include "TagWriter.hpp"

TagWriter::TagWriter(metadb_handle_list_cref handles, const Release& release, size_t offset) : m_handles(handles), m_release(release), m_offset(offset) {}

pfc::string8 TagWriter::trim(wil::zstring_view str)
{
	return pfc::string8(str).trim(' ');
}

void TagWriter::set(file_info& info, wil::zstring_view name, size_t value)
{
	info.meta_set(name.data(), pfc::format_uint(value));
}

void TagWriter::set(file_info& info, wil::zstring_view name, wil::zstring_view value)
{
	const pfc::string8 trimmed = trim(value);
	if (trimmed.get_length()) info.meta_set(name.data(), trimmed);
	else info.meta_remove_field(name.data());
}

void TagWriter::set_values(file_info& info, wil::zstring_view name, const Strings& values)
{
	info.meta_remove_field(name.data());
	if (values.empty()) return;

	auto transform = [this](auto&& str) { return trim(str); };
	for (auto&& value : values | std::views::transform(transform))
	{
		if (value.get_length())
		{
			info.meta_add(name.data(), value);
		}
	}
}

void TagWriter::write()
{
	const size_t count = m_handles.get_count();
	std::string subtitle;

	for (size_t i = 0; i < count; ++i)
	{
		file_info_impl info = m_handles[i]->get_info_ref()->info();
		auto& track = m_release.tracks[i + m_offset];

		if (prefs::bools::write_standard_tags)
		{
			if (m_release.is_various)
			{
				set(info, "ALBUM ARTIST", m_release.album_artist);

				if (prefs::bools::write_artist_sort)
				{
					set(info, "ALBUMARTISTSORT", m_release.album_artist_sort);
				}
			}

			set(info, "ALBUM", m_release.title);
			set(info, "ARTIST", track.artist);
			set(info, "TITLE", track.title);
			set(info, "TRACKNUMBER", track.tracknumber);
			set(info, "TOTALTRACKS", track.totaltracks);
			set(info, "DATE", m_release.date);

			if (m_release.totaldiscs > 1)
			{
				if (track.tracknumber == 1) subtitle = track.subtitle;

				set(info, "DISCSUBTITLE", subtitle);
				set(info, "DISCNUMBER", track.discnumber);
				set(info, "TOTALDISCS", m_release.totaldiscs);
			}
		}

		if (prefs::bools::write_artist_sort)
		{
			set(info, "ARTISTSORT", track.artist_sort);
		}

		if (prefs::bools::write_artists)
		{
			set_values(info, "ARTISTS", track.artists);
		}

		if (prefs::bools::write_original_date || m_release.date != m_release.original_release_date)
		{
			set(info, "ORIGINAL RELEASE DATE", m_release.original_release_date);
		}

		if (prefs::bools::write_releasetype)
		{
			Strings types = split_string(m_release.secondary_types, ",");
			if (get_primary_type_index(m_release.primary_type) > 0)
			{
				types.insert(types.begin(), m_release.primary_type);
			}

			set_values(info, "RELEASETYPE", types);
		}

		if (prefs::bools::write_releasestatus && get_status_index(m_release.status) > 0)
		{
			set(info, "RELEASESTATUS", m_release.status);
		}

		if (prefs::bools::write_label_info)
		{
			set(info, "LABEL", m_release.label);
			set(info, "CATALOGNUMBER", m_release.catalog);
			set(info, "BARCODE", m_release.barcode);
		}

		if (prefs::bools::write_ids)
		{
			if (m_release.is_various) set_values(info, "MUSICBRAINZ_ALBUMARTISTID", m_release.albumartistids);
			set_values(info, "MUSICBRAINZ_ARTISTID", track.artistids);
			set(info, "MUSICBRAINZ_DISCID", m_release.discid);
			set(info, "MUSICBRAINZ_ALBUMID", m_release.albumid);
			set(info, "MUSICBRAINZ_RELEASEGROUPID", m_release.releasegroupid);
			set(info, "MUSICBRAINZ_RELEASETRACKID", track.releasetrackid);
			set(info, "MUSICBRAINZ_TRACKID", track.trackid);
		}

		if (prefs::bools::write_releasecountry)
		{
			set(info, "RELEASECOUNTRY", m_release.country);
		}

		if (prefs::bools::write_media)
		{
			set(info, "MEDIA", track.media);
		}

		if (prefs::bools::write_isrc)
		{
			set_values(info, "ISRC", track.isrcs);
		}

		if (prefs::bools::write_performer)
		{
			set_values(info, "PERFORMER", m_release.performers);
		}

		if (prefs::bools::write_composer)
		{
			set_values(info, "COMPOSER", m_release.composers);
		}

		m_infos.emplace_back(info);
	}

	metadb_io_v2::get()->update_info_async_simple(
		m_handles,
		pfc::ptr_list_const_array_t<const file_info, file_info_impl*>(m_infos.data(), m_infos.size()),
		core_api::get_main_window(),
		metadb_io_v2::op_flag_delay_ui,
		nullptr
	);
}
