#pragma once

class Query;

class RequestThread : public threaded_process_callback
{
public:
	enum class Type
	{
		DiscID,
		Search,
		AlbumID
	};

	RequestThread(Type type, std::unique_ptr<Query> q, metadb_handle_list_cref handles);

	void on_done(HWND hwnd, bool was_aborted) final;
	void run(threaded_process_status& status, abort_callback& abort) final;

private:
	Releases m_releases;
	Type m_type;
	bool m_failed{};
	metadb_handle_list m_handles;
	std::unique_ptr<Query> m_query;
};
