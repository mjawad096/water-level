#pragma once

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
        if (database == nullptr)
        {
            TelnetLogger::log("Database not initialized. Please call Logger::setup() first.");
            return;
        }

        database->log(message, newLine, noDatabase);
    }

    static void logf(const char *format, ...)
    {
        va_list args;
        va_start(args, format);

        char buffer[128];
        vsnprintf(buffer, sizeof(buffer), format, args);

        va_end(args);

        log(buffer, true);
    }

    static void logfnd(const char *format, ...)
    {
        va_list args;
        va_start(args, format);

        char buffer[128];
        vsnprintf(buffer, sizeof(buffer), format, args);

        va_end(args);

        log(buffer, true, true);
    }

    static void logd(const String &message)
    {
        if (database == nullptr)
        {
            TelnetLogger::log("Database not initialized. Please call Logger::setup() first.");
            return;
        }

        database->logd(message);
    }

    static void logdf(const char *format, ...)
    {
        va_list args;
        va_start(args, format);

        char buffer[128];
        vsnprintf(buffer, sizeof(buffer), format, args);

        va_end(args);

        logd(buffer);
    }
};

Database *Logger::database = nullptr;

#define LOG(x) Logger::log(x, false)         // Log without new line
#define LOGND(x) Logger::log(x, false, true) // Log without new line && No database log

#define LOGL(x) Logger::log(x)               // Log with new line
#define LOGLND(x) Logger::log(x, true, true) // Log with new line && No database log

#define LOGF(...) Logger::logf(__VA_ARGS__)     // Log with format
#define LOGFND(...) Logger::logfnd(__VA_ARGS__) // Log with format and no database

#define LOGD(x) Logger::logd(x)               // Log with debug prefix
#define LOGDF(...) Logger::logdf(__VA_ARGS__) // Log with debug prefix and format