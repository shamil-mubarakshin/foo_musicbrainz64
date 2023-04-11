#pragma once

using namespace std::literals::string_view_literals;

using Strings = std::vector<std::string>;
using StringSet = std::set<std::string>;
using Performers = std::map<std::string, StringSet>;

struct Track
{
	Strings artists;
	Strings artistids;
	Strings composers;
	Strings isrcs;
	Strings performers;
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

static constexpr std::array primary_types =
{
	"(None)"sv,
	"Album"sv,
	"Single"sv,
	"EP"sv,
	"Broadcast"sv,
	"Other"sv,
};

static constexpr std::array release_statuses =
{
	"(None)"sv,
	"Official"sv,
	"Promotion"sv,
	"Bootleg"sv,
	"Pseudo-Release"sv,
	"Withdrawn"sv,
	"Cancelled"sv,
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

static int get_index(std::ranges::sized_range auto& arr, std::string_view str)
{
	const auto it = std::ranges::find_if(arr, [str](auto&& elem) { return stricmp_utf8_ex(elem.data(), elem.length(), str.data(), str.length()) == 0; });
	if (it == arr.end()) return 0;
	return static_cast<int>(std::ranges::distance(arr.begin(), it));
}

static void set_window_text(HWND hwnd, std::string_view str)
{
	SetWindowTextW(hwnd, pfc::wideFromUTF8(str.data()));
}
