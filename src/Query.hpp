#pragma once

class Query
{
public:
	Query(wil::zstring_view entity, wil::zstring_view id = "");

	json lookup(abort_callback& abort);
	void add_param(wil::zstring_view param, wil::zstring_view value);

	static constexpr wil::zstring_view s_inc_discid = "artists+labels+recordings+release-groups+artist-credits+isrcs";
	static constexpr wil::zstring_view s_inc_release = "recording-level-rels+artist-credits+release-rels+aliases+release-groups+recordings+artists+labels+recording-rels+artist-rels+work-level-rels+media+isrcs+discids+work-rels";

private:
	std::string url;
};
