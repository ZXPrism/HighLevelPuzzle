#pragma once

#include <sstream>

enum class LogLevel
{
    INFO,
    WARNING,
    ERROR
};

class Logger
{
public:
    Logger();

    void Log(LogLevel level, const char *position, const char *fmt, ...);

private:
    void PrintTime();

private:
    std::stringstream _logBuf;
};

extern Logger gLogger;

#ifdef MY_DEBUG
#define LOG_INFO(...) gLogger.Log(LogLevel::INFO, __FUNCTION__, __VA_ARGS__)
#define LOG_WARNING(...) gLogger.Log(LogLevel::WARNING, __FUNCTION__, __VA_ARGS__)
#define LOG_ERROR(...) gLogger.Log(LogLevel::ERROR, __FUNCTION__, __VA_ARGS__)
#else
#define LOG_INFO(...)
#define LOG_WARNING(...)
#define LOG_ERROR(...)
#endif
