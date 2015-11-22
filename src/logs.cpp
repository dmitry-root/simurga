#include "logs.hpp"
#include <syslog.h>

LogManager& LogManager::instance()
{
	static LogManager mgr;
	return mgr;
}

LogManager::LogManager()
{
	::openlog("simurga", 0, LOG_USER);
}

LogManager::~LogManager()
{
	::closelog();
}

void LogManager::log(int level, const char* format, ::va_list ap)
{
	::vsyslog(level, format, ap);
}

void err(const char* format, ...)
{
	va_list ap;
	va_start(ap, format);
	LogManager::instance().log(LOG_ERR, format, ap);
	va_end(ap);
}

void warn(const char* format, ...)
{
	va_list ap;
	va_start(ap, format);
	LogManager::instance().log(LOG_WARNING, format, ap);
	va_end(ap);
}

void info(const char* format, ...)
{
	va_list ap;
	va_start(ap, format);
	LogManager::instance().log(LOG_INFO, format, ap);
	va_end(ap);
}
