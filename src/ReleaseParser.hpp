#pragma once

class ReleaseParser
{
public:
	ReleaseParser(JSON obj, size_t handle_count, JSON discid = JSON());

	static void filter_releases(JSON& releases, size_t count, Strings& out);

	Release parse();

private:
	static std::string to_str(JSON& obj);

	void parse_artist_credits(JSON& obj, std::string& artist, std::string& artist_sort, Strings& artists, Strings& ids);
	void parse_label_and_barcode();
	void parse_relations(JSON& obj, Strings& composers, Performers& performers);
	void parse_release_info();
	void parse_rg_and_date();
	void parse_tracks();

	static constexpr fmt::string_view s_sep = "; ";

	JSON m_obj;
	Release m_release;
	size_t m_handle_count{};
};
