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

ReleaseParser::ReleaseParser(JSON json, size_t handle_count, JSON discid) : m_json(json), m_handle_count(handle_count)
{
	m_release.discid = json_to_string(discid);
}

Release ReleaseParser::parse()
{
	parse_release_info(); // must be first before parse_tracks
	parse_tracks();
	parse_label_and_barcode();
	parse_rg_and_date();
	return m_release;
}

std::string ReleaseParser::json_to_string(JSON& obj)
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

std::string ReleaseParser::set_to_string(const StringSet& set)
{
	return fmt::format("{}", fmt::join(set | std::views::filter([](auto&& str) { return str.length() > 0; }), s_sep));
}

void ReleaseParser::filter_releases(JSON& releases, size_t count, Strings& out)
{
	if (!releases.is_array()) return;

	for (auto&& release : releases)
	{
		const std::string id = json_to_string(release["id"]);
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

void ReleaseParser::parse_artist_credits(JSON& obj, std::string& artist, std::string& artist_sort, Strings& artists, Strings& ids)
{
	auto& artist_credits = obj["artist-credit"];
	if (!artist_credits.is_array()) return;

	for (auto&& artist_credit : artist_credits)
	{
		auto& artist_obj = artist_credit["artist"];

		const std::string name = json_to_string(artist_credit["name"]);
		const std::string joinphrase = json_to_string(artist_credit["joinphrase"]);
		const std::string id = json_to_string(artist_obj["id"]);

		std::string sort_name = json_to_string(artist_obj["sort-name"]);
		if (sort_name.empty()) sort_name = name;

		artist += name + joinphrase;
		artist_sort += sort_name + joinphrase;
		artists.emplace_back(name);
		ids.emplace_back(id);
	}
}

void ReleaseParser::parse_label_and_barcode()
{
	m_release.barcode = json_to_string(m_json["barcode"]);

	auto& label_infos = m_json["label-info"];
	if (label_infos.is_array() && label_infos.size() > 0)
	{
		StringSet labels, catalogs;

		for (auto&& label_info : label_infos)
		{
			auto& label = label_info["label"];
			if (label.is_object())
			{
				labels.insert(json_to_string(label["name"]));
			}

			catalogs.insert(json_to_string(label_info["catalog-number"]));
		}

		m_release.catalog = set_to_string(catalogs);
		m_release.label = set_to_string(labels);
	}
}

void ReleaseParser::parse_relations(JSON& obj, Strings& composers, Performers& performers)
{
	auto& relations = obj["relations"];
	if (relations.is_array())
	{
		for (auto&& relation : relations)
		{
			auto& artist_obj = relation["artist"];
			if (artist_obj.is_object())
			{
				const std::string artist = json_to_string(artist_obj["name"]);
				const std::string type = json_to_string(relation["type"]);

				if (type == "composer")
				{
					composers.emplace_back(artist);
				}
				else if (type == "instrument" || type == "vocal")
				{
					auto& attributes = relation["attributes"];
					if (attributes.is_array() && attributes.size() > 0)
					{
						auto view = attributes | std::views::transform([](auto&& attribute) { return json_to_string(attribute); });

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
	parse_artist_credits(m_json, m_release.album_artist, m_release.album_artist_sort, dummy, m_release.albumartistids);

	m_release.albumid = json_to_string(m_json["id"]);
	m_release.status = json_to_string(m_json["status"]);
	m_release.title = json_to_string(m_json["title"]);
	m_release.country = json_to_string(m_json["country"]);

#if 0
	if (m_release.country != "GB")
	{
		auto& release_events = m_json["release-events"];
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
						const auto it = std::ranges::find_if(codes, [](auto&& code) { return json_to_string(code) == "GB"; });
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
	m_release.date = json_to_string(m_json["date"]);

	auto& rg = m_json["release-group"];
	if (rg.is_object())
	{
		m_release.original_release_date = json_to_string(rg["first-release-date"]);
		m_release.releasegroupid = json_to_string(rg["id"]);
		m_release.primary_type = json_to_string(rg["primary-type"]);

		auto& secondary_types = rg["secondary-types"];
		if (secondary_types.is_array())
		{
			auto view = secondary_types | std::views::transform([](auto&& secondary_type) { return json_to_string(secondary_type); });
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
	auto& medias = m_json["media"];
	if (!medias.is_array()) return;

	const size_t release_totaltracks = std::accumulate(medias.begin(), medias.end(), size_t{ 0 }, [](size_t t, JSON j) { return t + j["tracks"].size(); });
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
				const auto it = std::ranges::find_if(discs, [=](auto&& disc) { return m_release.discid == json_to_string(disc["id"]); });
				if (it == discs.end()) continue;
			}

			if (!complete) m_release.partial_lookup_matches++;

			const std::string format = json_to_string(media["format"]);
			const std::string subtitle = json_to_string(media["title"]);
			const size_t discnumber = media["position"].get<size_t>();
			const size_t totaltracks = tracks.size();

			for (auto&& track : tracks)
			{
				Performers performers;
				Track t;
				parse_artist_credits(track, t.artist, t.artist_sort, t.artists, t.artistids);
				parse_relations(m_json, t.composers, performers);
				if (m_release.album_artist != t.artist) m_release.is_various = true;

				t.discnumber = discnumber;
				t.media = format;
				t.subtitle = subtitle;
				t.title = json_to_string(track["title"]);
				t.releasetrackid = json_to_string(track["id"]);
				t.tracknumber = track["position"].get<size_t>();
				t.totaltracks = totaltracks;

				auto& recording = track["recording"];
				if (recording.is_object())
				{
					t.trackid = json_to_string(recording["id"]);

					auto& isrcs = recording["isrcs"];
					if (isrcs.is_array())
					{
						auto view = isrcs | std::views::transform([](auto&& isrc) { return json_to_string(isrc); });
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
