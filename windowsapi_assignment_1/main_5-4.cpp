#include <windows.h>
#include <tchar.h>

#include "winapiutil.h"

WCHAR szTitle[] = L"Windows32 API Example";
WCHAR szWindowClass[] = L"Windows32 API Class";

HINSTANCE instance;
RECT screen_rect;

HBITMAP bitmap;
SIZE bitmap_size;

POINT player_position;
SIZE player_size = {128, 144};
int direction_index;

POINT directions[] =
{
    {10, 0},
    {0, 10},
    {-10, 0},
    {0, -10}
};

HBRUSH brush_shadow = CreateSolidBrush(0x0808080);

unsigned long e_time;
unsigned long jump_time = 20UL;

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
    instance = hInstance;

    WNDCLASSEXW wClass;
    wClass.cbSize = sizeof(WNDCLASSEX);
    wClass.style = CS_HREDRAW | CS_VREDRAW;
    wClass.lpfnWndProc = WndProc;
    wClass.cbClsExtra = 0;
    wClass.cbWndExtra = 0;
    wClass.hInstance = hInstance;
    wClass.hIcon = LoadIcon(NULL, IDI_INFORMATION);
    wClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wClass.lpszMenuName = NULL;
    wClass.lpszClassName = szWindowClass;
    wClass.hIconSm = LoadIcon(NULL, IDI_INFORMATION);
    RegisterClassEx(&wClass);

    HWND hWnd = CreateWindow(
        szWindowClass, szTitle,
        WS_OVERLAPPEDWINDOW,
        0, 0, 1280, 960,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG message;
    while (GetMessage(&message, NULL, 0, 0))
    {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }

    return (int)message.wParam;
}

void PaintScreen(HDC hDC)
{
    HDC bitmap_dc = CreateCompatibleDC(hDC);
    DrawPolygon(Ellipse, hDC, player_position.x + 32, player_position.y + player_size.cy - 24, player_position.x + 96, player_position.y + player_size.cy, NULL, brush_shadow);
    SelectObject(bitmap_dc, bitmap);
    int y = (20 - jump_time) * jump_time;
    if (y < 0)
        y = 0;
    TransparentBlt(hDC, player_position.x, player_position.y - y, player_size.cx, player_size.cy,
        bitmap_dc, 32 * (e_time / 2 % 10), (5 - direction_index) % 4 * 36, 32, 36, 0x00FF00FF);
    DeleteDC(bitmap_dc);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hDC, mem_hDC;
    HBITMAP mem_bit, old_bit;
    PAINTSTRUCT ps;

    switch (message)
    {
    case WM_CREATE:
        SetTimer(hWnd, 1, 15, NULL);
        bitmap = (HBITMAP)LoadImage(instance, L"Isaac.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
        BITMAP bInst;
        GetObject(bitmap, sizeof(BITMAP), &bInst);
        bitmap_size = SIZE{ bInst.bmWidth, bInst.bmHeight };
        break;

    case WM_TIMER:
        e_time += 1UL;
        jump_time += 1UL;
        {
            int nx = player_position.x + directions[direction_index].x;
            int ny = player_position.y + directions[direction_index].y;

            bool flag = false;
            if (nx < 0)
            {
                nx = 0;
                flag |= true;
            }
            if (nx + player_size.cx > screen_rect.right)
            {
                nx = screen_rect.right - player_size.cx;
                flag |= true;
            }
            if (ny < 0)
            {
                ny = 0;
                flag |= true;
            }
            if (ny + player_size.cy > screen_rect.bottom)
            {
                ny = screen_rect.bottom - player_size.cy;
                flag |= true;
            }

            if (flag)
                direction_index = (direction_index + 1) % 4;

            player_position.x = nx;
            player_position.y = ny;
        }
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_SIZE:
        GetClientRect(hWnd, &screen_rect);
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_CHAR:
        switch (towupper(wParam))
        {
        case L'J':
            jump_time = 0UL;
            break;
        case L'E':
            break;
        case L'S':
            break;
        case L'T':
            break;
        case L'A':
            break;
        case L'R':
            break;
        case L'Q':
            PostQuitMessage(0);
            break;
        }
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_UP:
            direction_index = 3;
            break;
        case VK_LEFT:
            direction_index = 2;
            break;
        case VK_DOWN:
            direction_index = 1;
            break;
        case VK_RIGHT:
            direction_index = 0;
            break;
        }
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_PAINT:
        hDC = BeginPaint(hWnd, &ps);
        mem_hDC = CreateCompatibleDC(hDC);

        mem_bit = CreateCompatibleBitmap(hDC, screen_rect.right, screen_rect.bottom);
        old_bit = (HBITMAP)SelectObject(mem_hDC, mem_bit);

        PatBlt(mem_hDC, 0, 0, screen_rect.right, screen_rect.bottom, WHITENESS);
        PaintScreen(mem_hDC);
        BitBlt(hDC, 0, 0, screen_rect.right, screen_rect.bottom, mem_hDC, 0, 0, SRCCOPY);

        SelectObject(mem_hDC, old_bit);
        DeleteObject(mem_bit);

        DeleteDC(mem_hDC);
        EndPaint(hWnd, &ps);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}