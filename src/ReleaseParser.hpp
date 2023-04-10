#pragma once

class ReleaseParser
{
public:
	ReleaseParser(JSON json, size_t handle_count, std::string_view discid = "");

	static void filter_releases(JSON& releases, size_t count, Strings& out);

	Release parse();

private:
	static bool check_array(JSON& arr);
	static std::string json_to_string(JSON& obj);
	static std::string set_to_string(const StringSet& set);

	void parse_artist_credits(JSON& obj, std::string& artist, std::string& artist_sort, Strings& artists, Strings& ids);
	void parse_label_and_barcode();
	void parse_relations(JSON& obj, Strings& composers, Performers& performers);
	void parse_release_info();
	void parse_rg_and_date();
	void parse_tracks();

	static constexpr fmt::string_view s_sep = "; ";

	JSON m_json;
	Release m_release;
	size_t m_handle_count{};
};
