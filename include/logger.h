#pragma once

#include <telnet_logger.h>
#include <database.h>

class Logger
{
private:
    static Database *database;

    Logger() {} // Private constructor to prevent instantiation

public:
    static void setup(Database *db)
    {
        database = db;
    }

    static void log(const String &message, bool newLine = true, bool noDatabase = false)
    {
        TelnetLogger::log(message, newLine);

        if (!noDatabase && database != nullptr)
            database->saveLogEntry(message, newLine);
    }

    static void logf(const char *format, ...)
    {
        va_list args;
        va_start(args, format);

        const String message = TelnetLogger::logf(format, args);

        va_end(args);

        database->saveLogEntry(message);
    }

    static void logfnd(const char *format, ...)
    {
        va_list args;
        va_start(args, format);

        TelnetLogger::logf(format, args);

        va_end(args);
    }
};

Database *Logger::database = nullptr;

#define LOG(x) Logger::log(x, false)            // Log without new line
#define LOGL(x) Logger::log(x, true)            // Log with new line
#define LOGND(x) Logger::log(x, true, false)    // No database log
#define LOGF(...) Logger::logf(__VA_ARGS__)     // Log with format
#define LOGFND(...) Logger::logfnd(__VA_ARGS__) // Log with format and no database
