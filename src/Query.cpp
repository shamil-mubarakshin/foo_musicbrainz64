#include "stdafx.hpp"
#include "Query.hpp"

Query::Query(std::string_view what, std::string_view id)
{
	if (id.length())
	{
		m_url = fmt::format("{}/ws/2/{}/{}?fmt=json", prefs::get_server(), what, id);
	}
	else
	{
		m_url = fmt::format("{}/ws/2/{}?fmt=json", prefs::get_server(), what);
	}
}

JSON Query::lookup(abort_callback& abort)
{
	pfc::string8 buffer;
	auto request = http_client::get()->create_request("GET");
	request->add_header("User-Agent", fmt::format("foo_musicbrainz64/{}", Component::version).c_str());

	try
	{
		// FB2K_console_formatter() << m_url;
		auto response = request->run_ex(m_url.c_str(), abort);
		response->read_string_raw(buffer, abort);
		auto j = JSON::parse(buffer.get_ptr(), nullptr, false);

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
	return JSON();
}

void Query::add_param(std::string_view name, std::string_view value)
{
	m_url += fmt::format("&{}={}", name, value);
}
