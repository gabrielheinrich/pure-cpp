#pragma once

#include <cstdio>
#include <pure/type_class.hpp>

namespace pure {
	struct file_stream : implements<Type_Class::File_Stream> {
		std::FILE* m_file;

		file_stream (std::FILE* file) noexcept : m_file {file} {}
		file_stream (const file_stream&) = delete;
		file_stream (file_stream&& other) : m_file {other.m_file} { other.m_file = nullptr; }

		~file_stream () { if (m_file) fclose (m_file); }
		file_stream& operator= (const file_stream&) = delete;
		file_stream& operator= (file_stream&&) = delete;

		operator std::FILE* () const noexcept { return m_file; }
	};
}
