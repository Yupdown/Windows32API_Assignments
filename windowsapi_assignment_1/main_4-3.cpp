#include <Windows.h>
#include <tchar.h>

#include <math.h>
#include <vector>
#include <random>

#include "winapiutil.h"

std::default_random_engine dre;
std::uniform_int_distribution<int> uid;

struct Circle
{
    POINT pos;
    double radius;
    double theta;
    double theta_delta;
    HBRUSH color_fill;
};

std::vector<Circle> circles;

WCHAR szTitle[] = L"Windows32 API Example";
WCHAR szWindowClass[] = L"Windows32 API Class";

RECT screen_rect;

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

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
        0, 0, 640, 480,
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

void PaintCircle(HDC hDC, const Circle& c)
{
    DrawPolygon(Ellipse, hDC, c.pos.x - c.radius, c.pos.y - c.radius, c.pos.x + c.radius, c.pos.y + c.radius, NULL, c.color_fill);
    for (int i = 0; i < 8; ++i)
    {
        double cost = c.radius * cos(c.theta + (double)i * 0.785398) + c.pos.x;
        double sint = c.radius * sin(c.theta + (double)i * 0.785398) + c.pos.y;
        MoveToEx(hDC, c.pos.x, c.pos.y, NULL);
        LineTo(hDC, (int)cost, (int)sint);
    }
    double cost = (c.radius + 8.0) * cos(c.theta) + c.pos.x;
    double sint = (c.radius + 8.0) * sin(c.theta) + c.pos.y;

    Ellipse(hDC, cost - 8, sint - 8, cost + 8, sint + 8);
}

void PaintScreen(HDC hDC)
{
    for (const auto& iter : circles)
        PaintCircle(hDC, iter);
}

Circle CreateRandomCircle()
{
    Circle c;
    c.pos = { uid(dre) % 640, uid(dre) % 480 };
    c.radius = 30.0 + uid(dre) % 30;
    c.theta = 0.0;
    c.theta_delta = 0.01 * (uid(dre) % 10 - 5);
    c.color_fill = CreateSolidBrush(uid(dre) % 0x01000000);
    return c;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hDC, mem_hDC;
    HBITMAP mem_bit, old_bit;
    PAINTSTRUCT ps;

    switch (message)
    {
    case WM_CREATE:
        for (int i = 0; i < 10; ++i)
            circles.push_back(CreateRandomCircle());
        SetTimer(hWnd, 1, 10, NULL);
        break;

    case WM_TIMER:
        for (auto& iter : circles)
            iter.theta += iter.theta_delta;
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_LBUTTONDOWN:
        Circle c = CreateRandomCircle();
        c.pos = { LOWORD(lParam), HIWORD(lParam) };
        circles.push_back(c);
        break;

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