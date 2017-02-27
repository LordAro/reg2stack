#ifndef UTIL_HPP
#define UTIL_HPP

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

std::vector<std::string> split_words(const std::string &line);
template<typename ... Args>

std::string string_format(const std::string& format, Args... args)
{
	size_t size = std::snprintf(nullptr, 0, format.c_str(), args...) + 1; // Extra space for '\0'
	auto buf = std::make_unique<char[]>(size);
	std::snprintf(buf.get(), size, format.c_str(), args...);
	return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

inline bool is_hex(const std::string &s)
{
	return std::all_of(s.begin(), s.end(), ::isxdigit);
}

#endif /* UTIL_HPP */
