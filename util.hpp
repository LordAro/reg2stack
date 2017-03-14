#ifndef UTIL_HPP
#define UTIL_HPP

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

std::vector<std::string> split_words(const std::string &line);

template<typename... Args>
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

/**
 * Checks if a bit in a value is set.
 *
 * This function checks if a bit inside a value is set or not.
 * The \a y value specific the position of the bit, started at the
 * LSB and count from \c 0.
 *
 * @param x The value to check
 * @param y The position of the bit to check, started from the LSB
 * @pre y < sizeof(T) * 8
 * @return True if the bit is set, false else.
 */
template <typename T>
static inline bool HasBit(const T x, const uint8_t y)
{
	return (x & (static_cast<T>(1U) << y)) != 0;
}

/**
 * Set a bit in a variable.
 *
 * This function sets a bit in a variable. The variable is changed
 * and the value is also returned. Parameter y defines the bit and
 * starts at the LSB with 0.
 *
 * @param x The variable to set a bit
 * @param y The bit position to set
 * @pre y < sizeof(T) * 8
 * @return The new value of the old value with the bit set
 */
template <typename T>
T SetBit(T &x, const uint8_t y)
{
	return x = static_cast<T>(x | (static_cast<T>(1U) << y));
}

/**
 * Clears a bit in a variable.
 *
 * This function clears a bit in a variable. The variable is
 * changed and the value is also returned. Parameter y defines the bit
 * to clear and starts at the LSB with 0.
 *
 * @param x The variable to clear the bit
 * @param y The bit position to clear
 * @pre y < sizeof(T) * 8
 * @return The new value of the old value with the bit cleared
 */
template <typename T>
T ClrBit(T &x, const uint8_t y)
{
	return x = static_cast<T>(x & ~(static_cast<T>(1U) << y));
}

#endif /* UTIL_HPP */
