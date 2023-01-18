#pragma once

class ReleaseParser
{
public:
	ReleaseParser(json obj, size_t handle_count, json discid = json());

	static void filter_releases(json& releases, size_t count, Strings& out);

	Release parse();

private:
	static std::string to_str(json& obj);

	void parse_artist_credits(json& obj, std::string& artist, std::string& artist_sort, Strings& artists, Strings& ids);
	void parse_label_and_barcode();
	void parse_relations(json& obj, Strings& composers, Performers& performers);
	void parse_release_info();
	void parse_rg_and_date();
	void parse_tracks();

	static constexpr fmt::string_view s_sep = "; ";

	Release m_release;
	json m_obj;
	size_t m_handle_count{};
};
