#include <Windows.h>
#include <tchar.h>
#include <time.h>
#include <cmath>

HINSTANCE g_hInst;
WCHAR szTitle[] = L"Windows32 API Example";
WCHAR szWindowClass[] = L"Windows32 API Class";

RECT screen_rect;

int draw_mode;

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

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
        y = sin((t + a) * 3.1415);
        break;
    case 1:
        x = t;
        y = 1.0f - 4.0f * abs(0.5f - Fraction((t + a) * 0.5f + 0.25f));
        break;
    case 2:
        x = sin(t * 3.1415) + t * 0.5f;
        y = cos((t + a) * 3.1415);
        break;
    case 3:
        x = t;
        y = Fraction((t + a) * 0.5f) < 0.5f ? 1.0f : -1.0f;
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
    draw_mode = 2;

    MoveToEx(hDC, 0, screen_rect.bottom / 2, NULL);
    LineTo(hDC, screen_rect.right, screen_rect.bottom / 2);

    MoveToEx(hDC, screen_rect.right / 2, 0, NULL);
    LineTo(hDC, screen_rect.right / 2, screen_rect.bottom);

    float x, y;
    float lx, ly;

    bool flag = false;
    for (float t = -10.0f; t < 10.0f; t += 0.0125f)
    {
        float time = clock() / 1000.0f;

        ParameterFunction(t, time, time, x, y);
        POINT pt = WorldToScreen(x, y);

        if (flag)
            LineTo(hDC, pt.x, pt.y);
        else
        {
            MoveToEx(hDC, pt.x, pt.y, NULL);
            flag |= true;
        }
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hDC, mem_hDC;
    HBITMAP mem_bit, old_bit;
    PAINTSTRUCT ps;

    switch (message)
    {
    case WM_CREATE:
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

// DLGPROC