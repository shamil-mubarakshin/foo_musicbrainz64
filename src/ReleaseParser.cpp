#include "stdafx.hpp"
#include "ReleaseParser.hpp"

struct AsciiReplacement
{
	std::string what, with;
};

static const std::vector<AsciiReplacement> ascii_replacements =
{
	{ u8"…", "..." },
	{ u8"‘", "'" },
	{ u8"’", "'" },
	{ u8"‚", "'" },
	{ u8"“", "\"" },
	{ u8"”", "\"" },
	{ u8"„", "\"" },
	{ u8"′", "'" },
	{ u8"″", "\"" },
	{ u8"‹", "<" },
	{ u8"›", ">" },
	{ u8"«", "\"" },
	{ u8"»", "\"" },
	{ u8"‐", "-" },
	{ u8"‒", "-" },
	{ u8"–", "-" },
	{ u8"−", "-" },
	{ u8"—", "-" },
	{ u8"―", "-" },
};

ReleaseParser::ReleaseParser(json obj, size_t handle_count, json discid) : m_obj(obj), m_handle_count(handle_count)
{
	m_release.discid = to_str(discid);
}

Release ReleaseParser::parse()
{
	parse_release_info(); // must be first before parse_tracks
	parse_tracks();
	parse_label_and_barcode();
	parse_rg_and_date();
	return m_release;
}

std::string ReleaseParser::to_str(json& obj)
{
	if (obj.is_null()) return "";
	if (!obj.is_string()) return obj.dump();

	pfc::string8 str = obj.get<std::string>();
	if (prefs::bools::ascii_punctuation)
	{
		for (const auto& [what, with] : ascii_replacements)
		{
			str = str.replace(what.c_str(), with.c_str());
		}
	}
	return str.get_ptr();
}

void ReleaseParser::filter_releases(json& releases, size_t count, Strings& out)
{
	if (!releases.is_array()) return;

	for (auto&& release : releases)
	{
		const std::string id = to_str(release["id"]);
		auto& release_track_count = release["track-count"];

		if (release_track_count.is_number_unsigned() && release_track_count.get<size_t>() == count)
		{
			out.emplace_back(id);
		}
		else
		{
			auto& medias = release["media"];
			if (medias.is_array())
			{
				for (auto&& media : medias)
				{
					auto& media_track_count = media["track-count"];
					if (media_track_count.is_number_unsigned() && media_track_count.get<size_t>() == count)
					{
						out.emplace_back(id);
						break;
					}
				}
			}
		}
	}
}

void ReleaseParser::parse_artist_credits(json& obj, std::string& artist, std::string& artist_sort, Strings& artists, Strings& ids)
{
	auto& artist_credits = obj["artist-credit"];
	if (!artist_credits.is_array()) return;

	for (auto&& artist_credit : artist_credits)
	{
		auto& artist_obj = artist_credit["artist"];

		const std::string name = to_str(artist_credit["name"]);
		const std::string joinphrase = to_str(artist_credit["joinphrase"]);
		const std::string id = to_str(artist_obj["id"]);
			
		std::string sort_name = to_str(artist_obj["sort-name"]);
		if (sort_name.empty()) sort_name = name;

		artist += name + joinphrase;
		artist_sort += sort_name + joinphrase;
		artists.emplace_back(name);
		ids.emplace_back(id);
	}
}

void ReleaseParser::parse_label_and_barcode()
{
	m_release.barcode = to_str(m_obj["barcode"]);

	auto& label_infos = m_obj["label-info"];
	if (label_infos.is_array() && label_infos.size() > 0)
	{
		StringSet labels, catalogs;

		for (auto&& label_info : label_infos)
		{
			auto& label = label_info["label"];
			if (label.is_object())
			{
				labels.insert(to_str(label["name"]));
			}

			catalogs.insert(to_str(label_info["catalog-number"]));
		}

		m_release.catalog = fmt::format("{}", fmt::join(catalogs | std::views::filter([](auto&& str) { return str.length() > 0; }), s_sep));
		m_release.label = fmt::format("{}", fmt::join(labels | std::views::filter([](auto&& str) { return str.length() > 0; }), s_sep));
	}
}

void ReleaseParser::parse_relations(json& obj, Strings& composers, Performers& performers)
{
	auto& relations = obj["relations"];
	if (relations.is_array())
	{
		for (auto&& relation : relations)
		{
			auto& artist_obj = relation["artist"];
			if (artist_obj.is_object())
			{
				const std::string artist = to_str(artist_obj["name"]);
				const std::string type = to_str(relation["type"]);

				if (type == "composer")
				{
					composers.emplace_back(artist);
				}
				else if (type == "instrument" || type == "vocal")
				{
					auto& attributes = relation["attributes"];
					if (attributes.is_array() && attributes.size() > 0)
					{
						auto view = attributes | std::views::transform([](auto&& attribute) { return to_str(attribute); });

						const auto it = performers.find(artist);
						if (it == performers.end())
						{
							performers[artist] = std::ranges::to<StringSet>(view);
						}
						else
						{
							std::ranges::copy(view, std::inserter(it->second, it->second.end()));
						}
					}
				}
			}
		}
	}
}

void ReleaseParser::parse_release_info()
{
	Strings dummy;
	parse_artist_credits(m_obj, m_release.album_artist, m_release.album_artist_sort, dummy, m_release.albumartistids);

	m_release.albumid = to_str(m_obj["id"]);
	m_release.status = to_str(m_obj["status"]);
	m_release.title = to_str(m_obj["title"]);
	m_release.country = to_str(m_obj["country"]);

#if 0
	if (m_release.country != "GB")
	{
		auto& release_events = m_obj["release-events"];
		if (release_events.is_array())
		{
			for (auto&& release_event : release_events)
			{
				auto& area = release_event["area"];
				if (area.is_object())
				{
					auto& codes = area["iso-3166-1-codes"];
					if (codes.is_array())
					{
						const auto it = std::ranges::find_if(codes, [](auto&& code) { return to_str(code) == "GB"; });
						if (it != codes.end())
						{
							m_release.country = "GB";
							break;
						}
					}
				}
			}
		}
	}
#endif
}

void ReleaseParser::parse_rg_and_date()
{
	m_release.date = to_str(m_obj["date"]);

	auto& rg = m_obj["release-group"];
	if (rg.is_object())
	{
		m_release.original_release_date = to_str(rg["first-release-date"]);
		m_release.releasegroupid = to_str(rg["id"]);
		m_release.primary_type = to_str(rg["primary-type"]);

		auto& secondary_types = rg["secondary-types"];
		if (secondary_types.is_array())
		{
			auto view = secondary_types | std::views::transform([](auto&& secondary_type) { return to_str(secondary_type); });
			m_release.secondary_types = fmt::format("{}", fmt::join(view, s_sep));
		}
	}

	if (prefs::bools::short_date)
	{
		if (m_release.date.length() > 4)
		{
			m_release.date.resize(4);
		}

		if (m_release.original_release_date.length() > 4)
		{
			m_release.original_release_date.resize(4);
		}
	}
}

void ReleaseParser::parse_tracks()
{
	auto& medias = m_obj["media"];
	if (!medias.is_array()) return;

	const size_t release_totaltracks = std::accumulate(medias.begin(), medias.end(), size_t{ 0 }, [](size_t t, json j) { return t + j["tracks"].size(); });
	const bool complete = release_totaltracks == m_handle_count;
	m_release.totaldiscs = medias.size();

	for (auto&& media : medias)
	{
		auto& tracks = media["tracks"];
		if (tracks.is_array() && (complete || tracks.size() == m_handle_count))
		{
			if (m_release.discid.length())
			{
				auto& discs = media["discs"];
				if (!discs.is_array()) continue;
				const auto it = std::ranges::find_if(discs, [=](auto&& disc) { return m_release.discid == to_str(disc["id"]); });
				if (it == discs.end()) continue;
			}

			if (!complete) m_release.partial_lookup_matches++;

			const std::string format = to_str(media["format"]);
			const std::string subtitle = to_str(media["title"]);
			const size_t discnumber = media["position"].get<size_t>();
			const size_t totaltracks = tracks.size();

			for (auto&& track : tracks)
			{
				Performers performers;
				Track t;
				parse_artist_credits(track, t.artist, t.artist_sort, t.artists, t.artistids);
				parse_relations(m_obj, t.composers, performers);
				if (m_release.album_artist != t.artist) m_release.is_various = true;

				t.discnumber = discnumber;
				t.media = format;
				t.subtitle = subtitle;
				t.title = to_str(track["title"]);
				t.releasetrackid = to_str(track["id"]);
				t.tracknumber = track["position"].get<size_t>();
				t.totaltracks = totaltracks;

				auto& recording = track["recording"];
				if (recording.is_object())
				{
					t.trackid = to_str(recording["id"]);

					auto& isrcs = recording["isrcs"];
					if (isrcs.is_array())
					{
						auto view = isrcs | std::views::transform([](auto&& isrc) { return to_str(isrc); });
						t.isrcs = std::ranges::to<Strings>(view);
					}

					parse_relations(recording, t.composers, performers);
				}

				for (const auto& [performer, what] : performers)
				{
					const std::string str = fmt::format("{} ({})", performer, fmt::join(what, ", "));
					t.performers.emplace_back(str);
				}

				m_release.tracks.emplace_back(t);
			}
		}
	}
}
