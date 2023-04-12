#include <Windows.h>
#include <tchar.h>
#include <cmath>

#include "winapiutil.h"

WCHAR szTitle[] = L"Windows32 API Example";
WCHAR szWindowClass[] = L"Windows32 API Class";

RECT screen_rect;

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void    CALLBACK    TimerProc(HWND, UINT, UINT_PTR, DWORD);

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

RECT rect_board = { 0, 0, 640, 480 };
RECT bricks[3][10];
POINT ball_position = { 320, 400 };
POINT ball_velocity = { -5, -5 };
int ball_radius = 10;

void PaintScreen(HDC hDC)
{
    Rectangle(hDC, rect_board.left, rect_board.top, rect_board.right, rect_board.bottom);
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 10; ++j)
            Rectangle(hDC, bricks[i][j].left, bricks[i][j].top, bricks[i][j].right, bricks[i][j].bottom);
    }

    Ellipse(hDC, ball_position.x - ball_radius, ball_position.y - ball_radius, ball_position.x + ball_radius, ball_position.y + ball_radius);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hDC, mem_hDC;
    HBITMAP mem_bit, old_bit;
    PAINTSTRUCT ps;

    switch (message)
    {
    case WM_CREATE:
        SetTimer(hWnd, 1, 1000 / 72, TimerProc);
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

void CALLBACK TimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{  
    ball_position.x += ball_velocity.x;
    ball_position.y += ball_velocity.y;

    if (ball_position.x - ball_radius < rect_board.left)
    {
        ball_position.x = rect_board.left * 2 + ball_radius - ball_position.x;
        ball_velocity.x = -ball_velocity.x;
    }
    if (ball_position.y - ball_radius < rect_board.top)
    {
        ball_position.y = rect_board.top * 2 + ball_radius - ball_position.y;
        ball_velocity.y = -ball_velocity.y;
    }
    if (ball_position.x + ball_radius > rect_board.right)
    {
        ball_position.x = rect_board.right * 2 - ball_radius - ball_position.x;
        ball_velocity.x = -ball_velocity.x;
    }
    if (ball_position.y + ball_radius > rect_board.bottom)
    {
        ball_position.y = rect_board.bottom * 2 - ball_radius - ball_position.y;
        ball_velocity.y = -ball_velocity.y;
    }

    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 10; ++j)
        {
            int offset_x = (sin(0.005 * dwTime + i) + 1.0) * 20.0;
            bricks[i][j].left = j * 60 + offset_x;
            bricks[i][j].top = i * 30 + 30;
            bricks[i][j].right = (j + 1) * 60 + offset_x;
            bricks[i][j].bottom = (i + 1) * 30 + 30;
        }
    }

    InvalidateRect(hWnd, NULL, false);
}