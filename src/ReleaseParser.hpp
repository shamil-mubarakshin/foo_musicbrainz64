#pragma once

class ReleaseParser
{
public:
	ReleaseParser(json obj, size_t handle_count, json discid = json());

	static std::string to_str(json j);
	static void filter_releases(json releases, size_t count, Strings& out);
	static void get_artist_info(json j, std::string& artist, std::string& artist_sort, Strings& artists, Strings& ids);

	Release parse();

private:
	void parse_label_and_barcode();
	void parse_relations();
	void parse_release_info();
	void parse_rg_and_date();
	void parse_tracks();

	static constexpr fmt::string_view s_sep = ", ";

	Release m_release;
	json m_obj;
	size_t m_handle_count{};
};
