#include "Logger.h"

#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <iomanip>
#include <iostream>

Logger gLogger;

Logger::Logger()
{
    _logBuf.fill('0');
}

void Logger::Log(LogLevel level, const char *position, const char *fmt, ...)
{
    static char buffer[512];
    _logBuf.str("");

    switch (level)
    {
    case LogLevel::INFO:
        _logBuf << "[INFO / ";
        break;
    case LogLevel::WARNING:
        _logBuf << "[WARNING / ";
        break;
    case LogLevel::ERROR:
        _logBuf << "[ERROR / ";
        break;
    default:
        break;
    }

    PrintTime();

    _logBuf << "] <" << position << "> ";

    std::va_list args;
    va_start(args, fmt);
    std::vsprintf(buffer, fmt, args);
    va_end(args);

    _logBuf << buffer << '\n';

    std::string logMsg = _logBuf.str();
    std::cout << logMsg << std::flush;
}

void Logger::PrintTime()
{
    auto now = std::chrono::system_clock::now();
    auto tt = std::chrono::system_clock::to_time_t(now);
    auto pTime = std::localtime(&tt);

    _logBuf << std::setw(2) << pTime->tm_hour << ':' << std::setw(2) << pTime->tm_min << ':' << std::setw(2) << pTime->tm_sec;
}
