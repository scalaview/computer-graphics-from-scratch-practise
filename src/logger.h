#pragma once

#include "define_types.h"

void console_write(const char *message);

#define DEBUG(msg) console_write(msg);