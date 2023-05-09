#include <Windows.h>
#include <tchar.h>

#include <random>
#include "winapiutil.h"

std::default_random_engine dre;
std::uniform_int_distribution<int> uid;

WCHAR szTitle[] = L"Windows32 API Example";
WCHAR szWindowClass[] = L"Windows32 API Class";

RECT screen_rect;

constexpr size_t SIZE_ARRAY = 128;
int sort_array[SIZE_ARRAY];

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

int i = 0;
int j = 0;

int swapA = -1;
int swapB = -1;

HBRUSH brush_swap = CreateSolidBrush(0x000000FF);

void PaintScreen(HDC hDC)
{
    for (int idx = 0; idx < SIZE_ARRAY; ++idx)
    {
        int left = idx * screen_rect.right / SIZE_ARRAY;
        int right = (idx + 1) * screen_rect.right / SIZE_ARRAY;
        DrawPolygon(Rectangle, hDC, left, screen_rect.bottom - sort_array[idx] * screen_rect.bottom / SIZE_ARRAY, right, screen_rect.bottom, NULL, 
            idx == swapA || idx == swapB ? brush_swap : NULL);
    }
}

void Initialize()
{
    i = 0;
    j = 0;
    swapA = -1;
    swapB = -1;

    for (int idx = 0; idx < SIZE_ARRAY; ++idx)
        sort_array[idx] = idx + 1;
    for (int idx = 0; idx < SIZE_ARRAY; ++idx)
        std::swap(sort_array[idx], sort_array[uid(dre) % SIZE_ARRAY]);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hDC, mem_hDC;
    HBITMAP mem_bit, old_bit;
    PAINTSTRUCT ps;
    int jmin;

    switch (message)
    {
    case WM_CREATE:
        Initialize();
        SetTimer(hWnd, 1, 20, NULL);
        break;

    case WM_SIZE:
        GetClientRect(hWnd, &screen_rect);
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_TIMER:
    {
        /*
        bool flag = true;

        while (flag)
        {
            if (i >= SIZE_ARRAY)
                break;

            if (sort_array[j] > sort_array[j + 1])
            {
                swapA = j;
                swapB = j + 1;
                flag = false;
            }

            if (++j + 1 >= SIZE_ARRAY)
            {
                ++i;
                j = 0;
            }
        }
        */
    }

        jmin = i;
        j = i;
        while (j < SIZE_ARRAY)
        {
            jmin = sort_array[j] < sort_array[jmin] ? j : jmin;
            ++j;
        }
        swapA = i;
        swapB = jmin;
        ++i;
        std::swap(sort_array[swapA], sort_array[swapB]);
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_CHAR:
        if (towupper(wParam) == L'R')
            Initialize();
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