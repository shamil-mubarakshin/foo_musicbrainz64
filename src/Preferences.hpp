#pragma once

namespace prefs
{
	namespace bools
	{
		extern cfg_bool custom_server;
		extern cfg_bool write_standard_tags;
		extern cfg_bool short_date;
		extern cfg_bool ascii_punctuation;
		extern cfg_bool write_original_date;
		extern cfg_bool write_artists;
		extern cfg_bool write_artist_sort;
		extern cfg_bool write_performer;
		extern cfg_bool write_composer;
		extern cfg_bool write_ids;
		extern cfg_bool write_releasetype;
		extern cfg_bool write_releasestatus;
		extern cfg_bool write_releasecountry;
		extern cfg_bool write_label_info;
		extern cfg_bool write_media;
		extern cfg_bool write_isrc;
	}

	namespace strings
	{
		extern cfg_string server;
	}

	std::string get_server();
}
