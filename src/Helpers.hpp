#pragma once

using Performers = std::map<std::string, Strings>;

struct Track
{
	Performers performers;
	Strings artists;
	Strings artistids;
	Strings composers;
	Strings isrcs;
	size_t discnumber{};
	size_t tracknumber{};
	size_t totaltracks{};
	std::string artist;
	std::string artist_sort;
	std::string media;
	std::string releasetrackid;
	std::string subtitle;
	std::string title;
	std::string trackid;
};

using Tracks = std::vector<Track>;

struct Release
{
	Strings albumartistids;
	Tracks tracks;
	bool is_various{};
	size_t partial_lookup_matches{};
	size_t totaldiscs{};
	std::string album_artist;
	std::string album_artist_sort;
	std::string albumid;
	std::string barcode;
	std::string catalog;
	std::string country;
	std::string date;
	std::string discid;
	std::string label;
	std::string original_release_date;
	std::string primary_type;
	std::string secondary_types;
	std::string releasegroupid;
	std::string status;
	std::string title;
};

using Releases = std::vector<Release>;

static constexpr std::array<wil::zstring_view, 6> primary_types =
{
	"(None)",
	"Album",
	"Single",
	"EP",
	"Broadcast",
	"Other",
};

static constexpr std::array<wil::zstring_view, 7> release_statuses =
{
	"(None)",
	"Official",
	"Promotion",
	"Bootleg",
	"Pseudo-Release",
	"Withdrawn",
	"Cancelled",
};

static Strings split_string(std::string_view text, std::string_view delims)
{
	auto view = text | std::views::split(delims);
	return std::ranges::to<Strings>(view);
}

static bool is_uuid(const char* mbid)
{
	if (mbid == nullptr) return false;
	std::regex rx("^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$");
	return std::regex_search(mbid, rx);
}

static int get_primary_type_index(wil::zstring_view str)
{
	const auto it = std::ranges::find_if(primary_types, [str](auto&& elem) { return stricmp_utf8(elem.data(), str.data()) == 0; });
	if (it == primary_types.end()) return 0;
	return static_cast<int>(std::ranges::distance(primary_types.begin(), it));
}

static int get_status_index(wil::zstring_view str)
{
	const auto it = std::ranges::find_if(release_statuses, [str](auto&& elem) { return stricmp_utf8(elem.data(), str.data()) == 0; });
	if (it == release_statuses.end()) return 0;
	return static_cast<int>(std::ranges::distance(release_statuses.begin(), it));
}

static std::string get_primary_type_str(size_t idx)
{
	return primary_types[idx].data();
}

static std::string get_status_str(size_t idx)
{
	return release_statuses[idx].data();
}

static void set_window_text(HWND hwnd, wil::zstring_view str)
{
	SetWindowTextW(hwnd, pfc::wideFromUTF8(str.data()));
}
