/**
 * This file contains common includes and definitions for Deephome Agent Daemon.
 */

#pragma once

#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <inttypes.h>


/// Mixin to make class non-copyable
class NonCopyable
{
public:
	NonCopyable() {}
	~NonCopyable() {}

protected:
	NonCopyable(const NonCopyable&) {}
	NonCopyable& operator= (const NonCopyable&) { return *this; }
};

std::string get_data_dir();

std::string trim(const std::string& s);

template<typename T>
inline T from_string(const std::string& s)
{
	std::istringstream ss(s);
	T result = T();
	ss >> result;
	return result;
}

template<typename T>
std::string to_string(const T& value)
{
	std::ostringstream ss;
	ss << value;
	return ss.str();
}

