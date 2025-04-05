#pragma once

#include <telnet_logger.h>

#define LOG(x) TelnetLogger::log(x, false)         // Log without new line
#define LOGL(x) TelnetLogger::log(x, true)         // Log with new line
#define LOGND(x) TelnetLogger::log(x, true, false) // No database log
#define LOGF(...) TelnetLogger::logf(__VA_ARGS__)  // Log with format
