#include <Windows.h>
#include <tchar.h>
#include <math.h>
#include <random>

#include "winapiutil.h"

WCHAR szTitle[] = L"Windows32 API Example";
WCHAR szWindowClass[] = L"Windows32 API Class";

RECT screen_rect;
HINSTANCE instance;
HBITMAP bitmap;
HBRUSH highlight_pen = CreateSolidBrush(0x000000FF);
SIZE bitmap_size;
SIZE board_size = { 720, 720 };

int interpolation;
int shift[4];

bool toggle_full;
bool toggle_mode;
bool toggle_invert;
int select_index;

unsigned long t;

int BOARD_ROWS = 3;
int BOARD_COLUMNS = 3;

bool state_lbutton;
bool state_rbutton;

POINT pick_start;
POINT pick_last;
int pick_index = -1;

class Fragment
{
public:
    RECT target;
    POINT position;

public:
    void Render(HDC hDC, HDC bitmap_DC, bool flag)
    {
        int targetx = target.right - target.left;
        int targety = target.bottom - target.top;
        int cell_width = board_size.cx / BOARD_COLUMNS;
        int cell_height = board_size.cy / BOARD_ROWS;
        int offset = flag ? 10 : 0;
        StretchBlt(hDC, position.x + offset, position.y + offset, cell_width - offset * 2, cell_height - offset * 2, bitmap_DC, target.left, target.top, targetx, targety, SRCCOPY);
    }

    bool IntersectPoint(const POINT& point)
    {
        int cell_width = board_size.cx / BOARD_COLUMNS;
        int cell_height = board_size.cy / BOARD_ROWS;

        if (point.x < position.x || point.x > position.x + cell_width)
            return false;
        if (point.y < position.y || point.y > position.y + cell_width)
            return false;
        return true;
    }
};

Fragment fragments[24];
int board[5][5];

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

void Initialize()
{
    for (int i = 0; i < BOARD_ROWS * BOARD_COLUMNS - 1; ++i)
    {
        int row = i / BOARD_COLUMNS;
        int col = i % BOARD_COLUMNS;

        fragments[i].position.x = col * board_size.cx / BOARD_COLUMNS;
        fragments[i].position.y = row * board_size.cy / BOARD_ROWS;

        fragments[i].target = {
            col * bitmap_size.cx / BOARD_COLUMNS,
            row * bitmap_size.cy / BOARD_ROWS,
            (col + 1)* bitmap_size.cx / BOARD_COLUMNS,
            (row + 1) * bitmap_size.cy / BOARD_ROWS,
        };
    }
}

int GetIntersectFragment(const POINT& point)
{
    for (int i = 0; i < BOARD_ROWS * BOARD_COLUMNS - 1; ++i)
    {
        if (fragments[i].IntersectPoint(point))
            return i;
    }
    return -1;
}

void PaintScreen(HDC hDC)
{
    HDC bitmap_dc = CreateCompatibleDC(hDC);
    SelectObject(bitmap_dc, bitmap);
    for (int i = 0; i < BOARD_ROWS * BOARD_COLUMNS - 1; ++i)
        fragments[i].Render(hDC, bitmap_dc, i == pick_index);
    DeleteDC(bitmap_dc);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hDC, mem_hDC;
    HBITMAP mem_bit, old_bit;
    PAINTSTRUCT ps;

    int x = LOWORD(lParam);
    int y = HIWORD(lParam);

    switch (message)
    {
    case WM_CREATE:
        SetTimer(hWnd, 1, 10, NULL);
        bitmap = (HBITMAP)LoadImage(instance, L"Lenna.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
        BITMAP bInst;
        GetObject(bitmap, sizeof(BITMAP), &bInst);
        bitmap_size = SIZE{ bInst.bmWidth, bInst.bmHeight };
        Initialize();
        break;

    case WM_TIMER:
        t += 1UL;
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_CHAR:
        switch (towupper(wParam))
        {
        case L'S':
            break;
        case L'F':
            toggle_mode = !toggle_mode;
            break;
        case L'Q':
            PostQuitMessage(0);
            break;
        case L'V':
            break;
        case L'H':
            break;
        }
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_MOUSEMOVE:
        if (state_rbutton && pick_index >= 0)
        {
            int dx = x - pick_start.x;
            int dy = y - pick_start.y;
            fragments[pick_index].position = { pick_last.x + dx, pick_last.y + dy };
        }
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_LBUTTONDOWN:
        if (state_rbutton)
            break;
        pick_start = { x, y };
        pick_index = GetIntersectFragment(pick_start);
        if (pick_index >= 0)
            pick_last = fragments[pick_index].position;
        state_rbutton = true;
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_LBUTTONUP:
        if (!state_rbutton)
            break;
        pick_index = -1;
        state_rbutton = false;
        InvalidateRect(hWnd, NULL, false);
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