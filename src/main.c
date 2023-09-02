#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <stdbool.h>

#define assert(c)           \
    do                      \
    {                       \
        if (!(c))           \
            __debugbreak(); \
    } while (0)

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

typedef uint32_t u32;

int client_width;
int client_height;
void *memory;

void draw_pixel(int x, int y, u32 color)
{
    u32 *pixel = (u32 *)memory;
    pixel += y * client_width + x;
    *pixel = color;
}

void clear_screen(u32 color)
{
    u32 *pixel = (u32 *)memory;
    for (int i = 0; i < client_width * client_height; ++i)
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

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmd, int cmd_show)
{

    // Create a window.

    WNDCLASSW window_class = {
        .lpszClassName = L"ray_tracer_window_class",
        .lpfnWndProc = window_proc,
        .hInstance = instance,
        .hCursor = LoadCursor(NULL, IDC_CROSS),
    };
    ATOM atom = RegisterClassW(&window_class);
    assert(atom && "Failed to register a window");

    HWND window = CreateWindowW(window_class.lpszClassName, L"Ray Tracer", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, instance, NULL);
    assert(window && "Failed to create a window");

    ShowWindow(window, cmd_show);

    // Allocate memory.

    RECT rect;
    GetClientRect(window, &rect);
    client_width = rect.right - rect.left;
    client_height = rect.bottom - rect.top;

    memory = VirtualAlloc(0, client_width * client_height * 4, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    BITMAPINFO bitmap_info;
    bitmap_info.bmiHeader.biSize = sizeof(bitmap_info.bmiHeader);
    bitmap_info.bmiHeader.biWidth = client_width;
    bitmap_info.bmiHeader.biHeight = -client_height;
    bitmap_info.bmiHeader.biPlanes = 1;
    bitmap_info.bmiHeader.biBitCount = 32;
    bitmap_info.bmiHeader.biCompression = BI_RGB;

    HDC DeviceContext = GetDC(window);

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

        clear_screen(0xfffffff);

        draw_pixel(100, 100, 0xffffff);

        // Display image.

        StretchDIBits(DeviceContext, 0, 0, client_width, client_height, 0, 0, client_width, client_height, memory, &bitmap_info, DIB_RGB_COLORS, SRCCOPY);
    }

    return 0;
}
