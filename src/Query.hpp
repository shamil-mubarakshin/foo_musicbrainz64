#pragma once

class Query
{
public:
	Query(std::string_view what, std::string_view id = "");

	JSON lookup(abort_callback& abort);
	void add_param(std::string_view name, std::string_view value);

	static constexpr std::string_view s_inc_discid = "artists+labels+recordings+release-groups+artist-credits+isrcs";
	static constexpr std::string_view s_inc_release = "recording-level-rels+artist-credits+release-rels+aliases+release-groups+recordings+artists+labels+recording-rels+artist-rels+work-level-rels+media+isrcs+discids+work-rels";

private:
	std::string m_url;
};
