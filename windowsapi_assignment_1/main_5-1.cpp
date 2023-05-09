#include <Windows.h>
#include <tchar.h>
#include <math.h>

#include "winapiutil.h"

WCHAR szTitle[] = L"Windows32 API Example";
WCHAR szWindowClass[] = L"Windows32 API Class";

RECT screen_rect;
HINSTANCE instance;
HBITMAP bitmap;
HBRUSH highlight_pen = CreateSolidBrush(0x000000FF);
SIZE bitmap_size;
POINT pivot;

int interpolation;
int shift[4];

bool toggle_full;
bool toggle_mode;
bool toggle_invert;
int select_index;

unsigned long t;

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

POINT CenterPoint()
{
    POINT p;
    p.x = pivot.x * interpolation / 10;
    p.y = pivot.y * interpolation / 10;
    return p;
}

void PaintScreen(HDC hDC)
{
    HDC bitmap_dc = CreateCompatibleDC(hDC);
    SelectObject(bitmap_dc, bitmap);

    POINT center;
    if (toggle_mode)
        center = POINT{ (long)((cos(0.1 * t) + 1.0) * pivot.x * 0.5), (long)((-sin(0.1 * t) + 1.0) * pivot.y * 0.5) };
    else
        center = CenterPoint();

    RECT r[4] =
    {
        0, 0, center.x, center.y,
        center.x, 0, pivot.x, center.y,
        0, center.y, center.x, pivot.y,
        center.x, center.y, pivot.x, pivot.y
    };

    for (int i = 0; i < 4; ++i)
    {
        int sub_center = r[i].left + (r[i].right - r[i].left) * shift[i] / 10;
        int sub_size = bitmap_size.cx * shift[i] / 10;
        StretchBlt(
            hDC, r[i].left, r[i].top, sub_center - r[i].left, r[i].bottom - r[i].top,
            bitmap_dc, bitmap_size.cx - sub_size, 0, sub_size, bitmap_size.cy,
            toggle_invert ? SRCINVERT : SRCCOPY);
        StretchBlt(
            hDC, sub_center, r[i].top, r[i].right - sub_center, r[i].bottom - r[i].top,
            bitmap_dc, 0, 0, bitmap_size.cx - sub_size, bitmap_size.cy,
            toggle_invert ? SRCINVERT : SRCCOPY);
    }
    if (select_index >= 0)
        FrameRect(hDC, &r[select_index], highlight_pen);
    DeleteDC(bitmap_dc);
}

void Reset()
{
    pivot = POINT{ bitmap_size.cx, bitmap_size.cy };
    toggle_full = false;
    toggle_mode = false;
    toggle_invert = false;
    interpolation = 10;
    select_index = -1;
    for (int i = 0; i < 4; ++i)
        shift[i] = 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hDC, mem_hDC;
    HBITMAP mem_bit, old_bit;
    PAINTSTRUCT ps;

    switch (message)
    {
    case WM_CREATE:
        SetTimer(hWnd, 1, 10, NULL);
        bitmap = (HBITMAP)LoadImage(instance, L"Lenna.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
        BITMAP bInst;
        GetObject(bitmap, sizeof(BITMAP), &bInst);
        bitmap_size = SIZE{ bInst.bmWidth, bInst.bmHeight };
        Reset();
        break;

    case WM_TIMER:
        t += 1UL;
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_CHAR:
        switch (towupper(wParam))
        {
        case L'A':
            toggle_full = !toggle_full;
            if (toggle_full)
                pivot = POINT{ screen_rect.right, screen_rect.bottom };
            else
                pivot = POINT{ bitmap_size.cx, bitmap_size.cy };
            break;
        case L'Y':
            toggle_mode = !toggle_mode;
            break;
        case L'R':
            toggle_invert = !toggle_invert;
            break;
        case L'+':
            interpolation = min(interpolation + 1, 10);
            break;
        case L'-':
            interpolation = max(interpolation - 1, 0);
            break;
        case L'P':
            for (int i = 0; i < 4; ++i)
                shift[i] = (9 + shift[i]) % 10;
            break;
        case L'S':
            Reset();
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
        case VK_LEFT:
        case VK_RIGHT:
            if (select_index >= 0)
                shift[select_index] = (shift[select_index] + (wParam == VK_RIGHT ? 11 : 9)) % 10;
            break;
        }
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_LBUTTONDOWN:
    {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        if (x < 0 || x > pivot.x || y < 0 || y > pivot.y)
            select_index = -1;
        else
        {
            POINT c = CenterPoint();
            if (y < c.y)
                select_index = x > c.x;
            else
                select_index = 2 + (x > c.x);
        }
        break;
    }
    case WM_SIZE:
        GetClientRect(hWnd, &screen_rect);
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