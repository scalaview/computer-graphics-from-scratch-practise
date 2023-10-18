#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "define_types.h"
#include "canvas.h"
#include "logger.h"
#include "model.h"
#include <stdlib.h>

#define assert(c)           \
    do                      \
    {                       \
        if (!(c))           \
            __debugbreak(); \
    } while (0)

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

void draw_pixel(i32 x, i32 y, u32 color);
i32 window_width = 616;
i32 window_height = 616;

canvas canvas_ctx = {
    .put_pixel = draw_pixel,
    .width = 605,
    .height = 605};
void *memory;

void draw_pixel(i32 x, i32 y, u32 color)
{
    if (x < 0 || x >= canvas_ctx.width || y < 0 || y >= canvas_ctx.height)
        return;
    u32 *pixel = (u32 *)memory;
    pixel += y * canvas_ctx.width + x;
    *pixel = color;
}

void clear_screen(u32 color)
{
    u32 *pixel = (u32 *)memory;
    for (i32 i = 0; i < canvas_ctx.width * canvas_ctx.height; ++i)
    {
        *pixel++ = color;
    }
}

LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    switch (message)
    {
    case WM_DESTROY:
    {
        PostQuitMessage(0);
    }
    break;
    case WM_KEYDOWN:
    {
        switch (wparam)
        {
        case 'O':
        {
            DestroyWindow(window);
        }
        break;
        }
    }
    break;
    default:
    {
        return DefWindowProcW(window, message, wparam, lparam);
    }
    }

    return 0;
}

i32 WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmd, i32 cmd_show)
{

    DEBUG("create a window.");
    WNDCLASSW window_class = {
        .lpszClassName = L"raster_window_class",
        .lpfnWndProc = window_proc,
        .hInstance = instance,
        .hCursor = LoadCursor(NULL, IDC_CROSS),
    };
    ATOM atom = RegisterClassW(&window_class);
    assert(atom && "Failed to register a window");

    HWND window = CreateWindowW(window_class.lpszClassName, L"Raster", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, window_width, window_height, NULL, NULL, instance, NULL);
    assert(window && "Failed to create a window");

    ShowWindow(window, cmd_show);

    // Allocate memory.

    RECT rect;
    GetClientRect(window, &rect);
    canvas_ctx.width = rect.right - rect.left;
    canvas_ctx.height = rect.bottom - rect.top;
    canvas_ctx.line_point_result = malloc(sizeof(i32) * max(canvas_ctx.width, canvas_ctx.height));
    canvas_ctx.depth_buffer = malloc(sizeof(f64) * canvas_ctx.width * canvas_ctx.height);

    initialize_models();
    initialize_instances();
    prepare_memory_buffer(canvas_ctx);

    memory = VirtualAlloc(0, canvas_ctx.width * canvas_ctx.height * 4, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    BITMAPINFO bitmap_info;
    bitmap_info.bmiHeader.biSize = sizeof(bitmap_info.bmiHeader);
    bitmap_info.bmiHeader.biWidth = canvas_ctx.width;
    bitmap_info.bmiHeader.biHeight = -canvas_ctx.height;
    bitmap_info.bmiHeader.biPlanes = 1;
    bitmap_info.bmiHeader.biBitCount = 32;
    bitmap_info.bmiHeader.biCompression = BI_RGB;

    HDC DeviceContext = GetDC(window);
    extern const color backgroud_color;

    while (true)
    {
        MSG message;
        if (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
        {
            if (message.message == WM_QUIT)
                break;
            TranslateMessage(&message);
            DispatchMessage(&message);
            continue;
        }

        clear_screen(backgroud_color.argb);

        render_frame(canvas_ctx);
        // Display image.

        StretchDIBits(DeviceContext, 0, 0, canvas_ctx.width, canvas_ctx.height, 0, 0, canvas_ctx.width, canvas_ctx.height, memory, &bitmap_info, DIB_RGB_COLORS, SRCCOPY);
    }

    free(canvas_ctx.line_point_result);
    free(canvas_ctx.depth_buffer);
    free_memory_buffer();
    free_instances();
    free_models();
    return 0;
}