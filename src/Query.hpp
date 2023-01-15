#pragma once

class Query
{
public:
	Query(wil::zstring_view entity, wil::zstring_view id = "");

	json lookup(abort_callback& abort);
	void add_param(wil::zstring_view param, wil::zstring_view value);

	static constexpr wil::zstring_view s_inc_discid = "artists+labels+recordings+release-groups+artist-credits+isrcs";
	static constexpr wil::zstring_view s_inc_release = "artists+labels+recordings+release-groups+artist-credits+isrcs+artist-rels+release-rels+recording-rels+recording-level-rels";

private:
	std::string url;
};
