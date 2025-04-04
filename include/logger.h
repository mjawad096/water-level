#pragma once

#include <telnet_logger.h>

#define LOG(x) TelnetLogger::log(x, false)
#define LOGL(x) TelnetLogger::log(x, true)
#define LOGF(...) TelnetLogger::logf(__VA_ARGS__)
