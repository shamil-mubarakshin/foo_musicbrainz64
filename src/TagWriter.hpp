#pragma once

class TagWriter
{
public:
	TagWriter(metadb_handle_list_cref handles, const Release& release);

	void write();

private:
	pfc::string8 trim(std::string_view str);
	void set(file_info& info, std::string_view name, size_t value);
	void set(file_info& info, std::string_view name, std::string_view value);
	void set_values(file_info& info, std::string_view name, const Strings& values);

	Release m_release;
	metadb_handle_list m_handles;
};
