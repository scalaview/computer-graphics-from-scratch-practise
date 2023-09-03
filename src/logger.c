#include "logger.h"
#include <windows.h>

void console_write(const char *message)
{
    OutputDebugStringA(message);
}