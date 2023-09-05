#pragma once

#include "define_types.h"
#include <stdarg.h>

void console_write(const char *message, ...);

#define DEBUG(msg, ...) console_write(msg, ##__VA_ARGS__);