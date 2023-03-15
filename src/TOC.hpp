#pragma once

class TOC
{
public:
	TOC(const pfc::array_t<metadb_v2::rec_t>& recs);

	std::string get_discid();
	std::string get_url();

private:
	void calc_pregap(const file_info& info);

	size_t m_count{};
	std::vector<uint32_t> m_tracks;
};
