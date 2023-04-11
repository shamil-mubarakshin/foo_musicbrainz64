#include "stdafx.hpp"
#include "RequestThread.hpp"
#include "DialogTagger.hpp"
#include "Query.hpp"
#include "ReleaseParser.hpp"

RequestThread::RequestThread(Type type, std::unique_ptr<Query> query, metadb_handle_list_cref handles)
	: m_type(type)
	, m_query(std::move(query))
	, m_handles(handles) {}

void RequestThread::on_done(HWND, bool was_aborted)
{
	if (was_aborted || m_failed) return;

	if (m_releases.empty())
	{
		popup_message::g_show("No matching results were found.", Component::name.data());
		return;
	}

	std::ranges::sort(m_releases, [](const Release& one, const Release& two) -> bool
		{
			return stricmp_utf8(one.date.c_str(), two.date.c_str()) < 0;
		});
	fb2k::newDialog<CDialogTagger>(m_releases, m_handles);
}

void RequestThread::run(threaded_process_status& status, abort_callback& abort)
{
	const size_t handle_count = m_handles.get_count();

	auto j = m_query->lookup(abort);
	if (!j.is_object())
	{
		m_failed = true;
		return;
	}

	switch (m_type)
	{
	case Type::DiscID:
		{
			auto& releases = j["releases"];
			if (!releases.is_array() || !j["id"].is_string()) return;
			const std::string discid = j["id"].get<std::string>();

			for (auto&& release : releases)
			{
				auto r = ReleaseParser(release, handle_count, discid).parse();
				if (r.tracks.size())
				{
					m_releases.emplace_back(r);
				}
			}
		}
		break;
	case Type::Search:
		{
			auto& releases = j["releases"];
			Strings ids;
			ReleaseParser::filter_releases(releases, handle_count, ids);
			const size_t count = ids.size();

			for (const size_t i : std::views::iota(0U, count))
			{
				abort.check();
				status.set_progress(i + 1, count);
				status.set_title(fmt::format("Fetching {} of {}", i + 1, count).c_str());
				Sleep(800);

				auto query = Query("release", ids[i]);
				query.add_param("inc", Query::s_inc_release);

				auto j2 = query.lookup(abort);
				if (!j2.is_object())
				{
					m_failed = true;
					return;
				}

				auto r = ReleaseParser(j2, handle_count).parse();
				if (r.tracks.size())
				{
					m_releases.emplace_back(r);
				}
			}
		}
		break;
	case Type::AlbumID:
		{
			auto r = ReleaseParser(j, handle_count).parse();
			if (r.tracks.size())
			{
				m_releases.emplace_back(r);
			}
		}
		break;
	}
}
