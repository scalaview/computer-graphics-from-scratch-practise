#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include "define_types.h"
#include "canvas.h"
#include "logger.h"

#define assert(c)           \
    do                      \
    {                       \
        if (!(c))           \
            __debugbreak(); \
    } while (0)

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

void draw_pixel(i32 x, i32 y, u32 color);

sphere spheres[4] = {
    {{0, -1, 3},
     1,
     {.alpha = 255, .r = 255, .g = 0, .b = 0},
     500},
    {{2, 0, 4},
     1,
     {.alpha = 255, .r = 0, .g = 0, .b = 255},
     500},
    {{-2, 0, 4},
     1,
     {.alpha = 255, .r = 0, .g = 255, .b = 0},
     10},
    {{0, -5001, 0},
     5000,
     {.alpha = 255, .r = 255, .g = 255, .b = 0},
     1000}};

light lights[3] = {
    {.type = LIGHT_AMBIENT,
     .intensity = 0.2f},
    {.type = LIGHT_POINT,
     .intensity = 0.6f,
     .position = (vec3){2, 1, 0}},
    {.type = LIGHT_DIRECTIONAL,
     .intensity = 0.2f,
     .direction = (vec3){1, 4, 4}}};

canvas canvas_ctx = {
    .put_pixel = draw_pixel,
    .width = 0,
    .height = 0,
    .sphere_size = 4,
    .spheres = &spheres,
    .lights = &lights,
    .light_size = 3};
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

static void draw_rect(i32 x, i32 y, i32 width, i32 height, u32 color)
{
    for (u32 i = 0; i < width; i++)
    {
        for (u32 j = 0; j < height; j++)
        {
            draw_pixel(x + i, y + j, color);
        }
    }
}

i32 WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmd, i32 cmd_show)
{

    DEBUG("create a window.");
    WNDCLASSW window_class = {
        .lpszClassName = L"ray_tracer_window_class",
        .lpfnWndProc = window_proc,
        .hInstance = instance,
        .hCursor = LoadCursor(NULL, IDC_CROSS),
    };
    ATOM atom = RegisterClassW(&window_class);
    assert(atom && "Failed to register a window");

    HWND window = CreateWindowW(window_class.lpszClassName, L"Ray Tracer", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 600, 600, NULL, NULL, instance, NULL);
    assert(window && "Failed to create a window");

    ShowWindow(window, cmd_show);

    // Allocate memory.

    RECT rect;
    GetClientRect(window, &rect);
    canvas_ctx.width = rect.right - rect.left;
    canvas_ctx.height = rect.bottom - rect.top;

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

    return 0;
}
