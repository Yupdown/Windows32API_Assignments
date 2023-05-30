#include <Windows.h>
#include <tchar.h>
#include <time.h>
#include <cmath>

#include "resource.h"

HINSTANCE g_hInst;
WCHAR szTitle[] = L"Windows32 API Example";
WCHAR szWindowClass[] = L"Windows32 API Class";

RECT screen_rect;
int draw_mode;
int color_mask;

bool move_x;
bool move_y;
bool move_circle;

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL    CALLBACK    DlalogProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
    g_hInst = hInstance;

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

inline float Fraction(float x)
{
    return x - floor(x);
}

void ParameterFunction(float t, float a, float b, float& x, float& y)
{
    switch (draw_mode)
    {
    case 0:
        x = t;
        y = sin((t + a) * 3.1415f);
        break;
    case 1:
        x = t;
        y = 1.0f - 4.0f * abs(0.5f - Fraction((t + a) * 0.5f + 0.25f));
        break;
    case 2:
        x = sin((t + a) * 3.1415f) + t * 0.5f;
        y = cos((t + a) * 3.1415f);
        break;
    case 3:
        x = t;
        y = Fraction((t + a) * 0.5f) < 0.5f ? 1.0f : -1.0f;
        break;
    case 4:
        x = t;
        y = 0.0f;
        for (int n = 0; n < 20; ++n)
            y += pow(0.5f, n) * cos(pow(5.0f, n) * (t + a) * 3.1415f);
        break;
    default:
        break;
    }

    y *= sin(b) * 0.3f + 0.7f;
}

POINT WorldToScreen(float x, float y)
{
    return POINT {
        static_cast<long>(x * 100) + screen_rect.right / 2,
        -static_cast<long>(y * 100) + screen_rect.bottom / 2
    };
}

void PaintScreen(HDC hDC)
{
    MoveToEx(hDC, 0, screen_rect.bottom / 2, NULL);
    LineTo(hDC, screen_rect.right, screen_rect.bottom / 2);

    MoveToEx(hDC, screen_rect.right / 2, 0, NULL);
    LineTo(hDC, screen_rect.right / 2, screen_rect.bottom);

    COLORREF color = 0;

    if (color_mask & 1)
        color |= 0x000000FF;
    if (color_mask & 2)
        color |= 0x0000FF00;
    if (color_mask & 4)
        color |= 0x00FF0000;
    if (color_mask & 8)
        color ^= 0x00FFFFFF;

    HPEN pen = CreatePen(PS_SOLID, 3, color);
    HPEN old_pen = (HPEN)SelectObject(hDC, pen);

    float x, y;
    float time = clock() / 1000.0f;
    float a = move_x ? time : 0;
    float b = move_y ? time : 0;

    bool flag = false;
    for (float t = -10.0f; t < 10.0f; t += 0.0125f)
    {
        ParameterFunction(t, a, b, x, y);
        POINT pt = WorldToScreen(x, y);

        if (flag)
            LineTo(hDC, pt.x, pt.y);
        else
        {
            MoveToEx(hDC, pt.x, pt.y, NULL);
            flag |= true;
        }
    }

    SelectObject(hDC, old_pen);
    DeleteObject(pen);

    if (move_circle)
    {
        float tp = Fraction(time / 5.0f) * 16.0f - 8.0f;
        ParameterFunction(tp, a, b, x, y);
        POINT pt = WorldToScreen(x, y);
        Rectangle(hDC, pt.x - 10, pt.y - 10, pt.x + 10, pt.y + 10);
    }
}

HWND hDialogWnd;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hDC, mem_hDC;
    HBITMAP mem_bit, old_bit;
    PAINTSTRUCT ps;

    switch (message)
    {
    case WM_CREATE:
        hDialogWnd = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, (DLGPROC)&DlalogProc);
        CheckRadioButton(hDialogWnd, IDC_RADIO3, IDC_RADIO6, IDC_RADIO3);
        ShowWindow(hDialogWnd, SW_SHOW);
        SetTimer(hWnd, 1, 0, NULL);
        break;

    case WM_SIZE:
        GetClientRect(hWnd, &screen_rect);
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_TIMER:
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

BOOL CALLBACK DlalogProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    switch (iMsg)
    {
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BUTTON1:
            move_x = true;
            break;
        case IDC_BUTTON2:
            move_y = true;
            break;
        case IDC_BUTTON3:
            move_x = false;
            move_y = false;
            break;
        case IDC_BUTTON4:
            move_x = false;
            move_y = false;
            move_circle = false;
            draw_mode = 0;
            color_mask = 0;
            CheckRadioButton(hDialogWnd, IDC_RADIO3, IDC_RADIO6, IDC_RADIO3);
            break;
        case IDC_BUTTON5:
            move_circle = true;
            break;

        case IDC_RADIO3:
            draw_mode = 0;
            break;
        case IDC_RADIO4:
            draw_mode = 1;
            break;
        case IDC_RADIO5:
            draw_mode = 2;
            break;
        case IDC_RADIO6:
            draw_mode = 3;
            break;
        case IDC_RADIO7:
            draw_mode = 4;
            break;

        case IDC_CHECK2:
            color_mask ^= 1;
            break;
        case IDC_CHECK3:
            color_mask ^= 2;
            break;
        case IDC_CHECK4:
            color_mask ^= 4;
            break;
        case IDC_CHECK5:
            color_mask ^= 8;
            break;
        }
        break;
    }
    return 0;
}