#include "stdafx.hpp"
#include "Query.hpp"

Query::Query(wil::zstring_view entity, wil::zstring_view id)
{
	url = fmt::format("{}/ws/2/{}", prefs::get_server(), entity);
	if (id.length() > 0) url += fmt::format("/{}", id);
	url += "?fmt=json";
}

json Query::lookup(abort_callback& abort)
{
	pfc::string8 buffer;
	auto request = http_client::get()->create_request("GET");
	request->add_header("User-Agent", fmt::format("foo_musicbrainz64/{}", Component::version).c_str());

	try
	{
		// FB2K_console_formatter() << url;
		auto response = request->run_ex(url.c_str(), abort);
		response->read_string_raw(buffer, abort);
		json j = json::parse(buffer.get_ptr(), nullptr, false);

		if (j.is_object())
		{
			return j;
		}

		http_reply::ptr ptr;
		if (response->cast(ptr))
		{
			ptr->get_status(buffer);
		}
	}
	catch (const std::exception& e)
	{
		buffer = e.what();
	}

	popup_message::g_show(buffer, Component::name.data());
	return json();
}

void Query::add_param(wil::zstring_view param, wil::zstring_view value)
{
	url += fmt::format("&{}={}", param, value);
}
