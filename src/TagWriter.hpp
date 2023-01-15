#pragma once

class TagWriter
{
public:
	TagWriter(metadb_handle_list_cref handles, const Release& release, size_t offset);

	void write();

private:
	pfc::string8 trim(wil::zstring_view str);
	void set(file_info& info, wil::zstring_view name, size_t value);
	void set(file_info& info, wil::zstring_view name, wil::zstring_view value);
	void set_values(file_info& info, wil::zstring_view name, const Strings& values);

	Release m_release;
	metadb_handle_list m_handles;
	size_t m_offset{};
	std::vector<file_info_impl> m_infos;
};
