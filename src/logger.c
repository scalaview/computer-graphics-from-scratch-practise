#include "logger.h"
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

void console_write(const char *message, ...)
{

    char out_message[32000];
    memset(out_message, 0, sizeof(out_message));

    __builtin_va_list arg_ptr;
    va_start(arg_ptr, message);

    i32 written = vsnprintf(out_message, 32000, message, arg_ptr);
    out_message[written] = 0;

    va_end(arg_ptr);

    OutputDebugStringA(out_message);
}