#include <Windows.h>
#include <tchar.h>

#include <math.h>
#include <vector>

#include "resource.h"
#include "winapiutil.h"

WCHAR szTitle[] = L"Fourier Series";
WCHAR szWindowClass[] = L"Windows32 API Class";

RECT screen_rect;
unsigned long time = 0UL;

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

HPEN pen = CreatePen(PS_SOLID, 1, 0);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
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
        0, 0, 1280, 1280,
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

constexpr int NUM_ITERATION = 8;
constexpr int SIZE_BUFFER = 640;

double x_buffer[SIZE_BUFFER];
double y_buffer[SIZE_BUFFER];

void PaintScreen(HDC hDC)
{
    HPEN o_pen = (HPEN)SelectObject(hDC, pen);

    SetROP2(hDC, R2_MASKPEN);
    double x[NUM_ITERATION + 1] = { 240.0 };
    double y[NUM_ITERATION + 1] = { 240.0 };
    double r[NUM_ITERATION];

    for (int i = 0; i < NUM_ITERATION; ++i)
    {
        int n = i * 2 + 1;
        double t = 0.025 * time * n;
        /*int n = i + 1;
        double t = -0.025 * time * n;*/
        r[i] = 120.0 / n;
        x[i + 1] = cos(t) * (i & 1 ? r[i] : -r[i]) + x[i];
        y[i + 1] = sin(t) * r[i] + y[i];
    }
    for (int i = 0; i < NUM_ITERATION; ++i)
    {
        Ellipse(hDC, x[i] - r[i], y[i] - r[i], x[i] + r[i], y[i] + r[i]);
        MoveToEx(hDC, x[i], y[i], NULL);
        LineTo(hDC, x[i + 1], y[i + 1]);
    }

    x_buffer[time % SIZE_BUFFER] = y[NUM_ITERATION];
    y_buffer[time % SIZE_BUFFER] = x[NUM_ITERATION];

    for (int i = 0; i < SIZE_BUFFER - 1; ++i)
    {
        int ip0 = (SIZE_BUFFER - i + time) % SIZE_BUFFER;
        int ip1 = (SIZE_BUFFER - (i + 1) + time) % SIZE_BUFFER;
        int x0 = 480 + i * 2;
        int x1 = 480 + (i + 1) * 2;

        MoveToEx(hDC, x0, x_buffer[ip0], NULL);
        LineTo(hDC, x1, x_buffer[ip1]);
        MoveToEx(hDC, y_buffer[ip0], x0, NULL);
        LineTo(hDC, y_buffer[ip1], x1);
    }

    MoveToEx(hDC, 0, y[NUM_ITERATION], NULL);
    LineTo(hDC, 480, y[NUM_ITERATION]);
    MoveToEx(hDC, x[NUM_ITERATION], 0, NULL);
    LineTo(hDC, x[NUM_ITERATION], 480);
    SetROP2(hDC, R2_COPYPEN);

    SelectObject(hDC, o_pen);
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
        break;

    case WM_SIZE:
        GetClientRect(hWnd, &screen_rect);
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_TIMER:
        time += 1UL;
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