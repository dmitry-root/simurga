/**
 * Logging support.
 */

#pragma once

#include "common.hpp"
#include <stdarg.h>

class LogManager : public NonCopyable
{
public:
	static LogManager& instance();
	~LogManager();

	void log(int level, const char* format, ::va_list ap);

private:
    LogManager();
};

void err(const char* format, ...);
void warn(const char* format, ...);
void info(const char* format, ...);

