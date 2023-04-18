#include <Windows.h>
#include <tchar.h>
#include <random>

#include "resource.h"
#include "winapiutil.h"

std::default_random_engine dre;
std::uniform_int_distribution<int> uid;

WCHAR szTitle[] = L"Windows32 API Example";
WCHAR szWindowClass[] = L"Windows32 API Class";

RECT screen_rect;

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

constexpr int SCREEN_SIZE = 960;
constexpr int BOARD_LENGTH = 40;

HBRUSH colors[] =
{
    CreateSolidBrush(0x000000FF),
    CreateSolidBrush(0x0000FF00),
    CreateSolidBrush(0x00FF0000),
    CreateSolidBrush(0x00008080),
    CreateSolidBrush(0x00808000),
    CreateSolidBrush(0x00800080)
};

int board_color = -1;

HPEN select_pen = CreatePen(PS_SOLID, 5, 0);
HPEN dot_pen = CreatePen(PS_DASH, 1, 0);

RECT draw_rects[5];
int draw_colors[5];

RECT drawing_rect;
int drawing_color;

POINT pick_start;
RECT pick_last;

int select_index = -1;
int draw_index = 0;
int pick_index = -1;
int count = 0;

bool state_lbutton;
bool state_rbutton;

bool toggle_border = true;
bool toggle_grid;

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
        0, 0, SCREEN_SIZE + 16, SCREEN_SIZE + 39,
        nullptr,
        LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU1)),
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

#define CELLTOWORLD(x) ((x) * SCREEN_SIZE / BOARD_LENGTH)
#define WORLDTOCELL(x) ((x) * BOARD_LENGTH / SCREEN_SIZE)

void PaintRects(HDC hDC)
{
    for (int idx = 0; idx < 5; ++idx)
    {
        RECT r = draw_rects[idx];
        DrawPolygon(Rectangle, hDC, CELLTOWORLD(r.left), CELLTOWORLD(r.top), CELLTOWORLD(r.right), CELLTOWORLD(r.bottom), NULL, colors[draw_colors[idx]]);
    }
    if (state_lbutton)
    {
        RECT r = drawing_rect;
        DrawPolygon(Rectangle, hDC, CELLTOWORLD(r.left), CELLTOWORLD(r.top), CELLTOWORLD(r.right), CELLTOWORLD(r.bottom), NULL, colors[drawing_color]);
    }
}

void PaintScreen(HDC hDC)
{
    PatBlt(hDC, 0, 0, screen_rect.right, screen_rect.bottom, BLACKNESS);

    for (int row = 0; row < BOARD_LENGTH; ++row)
    {
        for (int col = 0; col < BOARD_LENGTH; ++col)
            DrawPolygon(Rectangle, hDC, CELLTOWORLD(col), CELLTOWORLD(row), CELLTOWORLD(col + 1), CELLTOWORLD(row + 1), toggle_grid ? dot_pen : NULL, board_color < 0 ? NULL : colors[board_color]);
    }

    SetROP2(hDC, R2_BLACK);
    PaintRects(hDC);

    SetROP2(hDC, R2_MERGEPEN);
    PaintRects(hDC);

    SetROP2(hDC, R2_MASKPEN);
    if (select_index >= 0 && toggle_border)
    {
        RECT r = draw_rects[select_index];
        DrawPolygon(Rectangle, hDC, CELLTOWORLD(r.left), CELLTOWORLD(r.top), CELLTOWORLD(r.right), CELLTOWORLD(r.bottom), select_pen, NULL);
    }

    SetROP2(hDC, R2_COPYPEN);
}

int GetIntersectRect(const POINT& pos)
{
    for (int idx = 0; idx < 5; ++idx)
    {
        RECT r = draw_rects[idx];
        if (r.left <= pos.x && r.top <= pos.y && r.right > pos.x && r.bottom > pos.y)
            return idx;
    }
    return -1;
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
    case WM_SIZE:
        GetClientRect(hWnd, &screen_rect);
        InvalidateRect(hWnd, NULL, false);
        break;
        
    case WM_CHAR:
        if (iswdigit(wParam))
        {
            int index = wParam - L'1';
            if (index >= 0 && index < 5)
                select_index = index;
        }
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_KEYDOWN:
        if (select_index >= 0)
        {
            RECT r = draw_rects[select_index];
            POINT delta = { 0, 0 };
            switch (wParam)
            {
            case VK_UP:
                if (r.top > 0)
                    delta = { 0, -1 };
                break;
            case VK_LEFT:
                if (r.left > 0)
                    delta = { -1, 0 };
                break;
            case VK_DOWN:
                if (r.bottom < BOARD_LENGTH)
                    delta = { 0, 1 };
                break;
            case VK_RIGHT:
                if (r.right < BOARD_LENGTH)
                    delta = { 1, 0 };
                break;
            }
            draw_rects[select_index] = { r.left + delta.x, r.top + delta.y, r.right + delta.x, r.bottom + delta.y };
            InvalidateRect(hWnd, NULL, false);
        }
        break;

    case WM_LBUTTONDOWN:
        if (state_lbutton)
            break;
        drawing_rect.left = WORLDTOCELL(x);
        drawing_rect.top = WORLDTOCELL(y);
        drawing_rect.right = WORLDTOCELL(x) + 1;
        drawing_rect.bottom = WORLDTOCELL(y) + 1;
        drawing_color = count++ % 6;
        state_lbutton = true;
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_MOUSEMOVE:
        if (state_lbutton)
        {
            drawing_rect.right = WORLDTOCELL(x) + 1;
            drawing_rect.bottom = WORLDTOCELL(y) + 1;
        }
        if (state_rbutton && pick_index >= 0)
        {
            int dx = WORLDTOCELL(x) - pick_start.x;
            int dy = WORLDTOCELL(y) - pick_start.y;
            draw_rects[pick_index] = { pick_last.left + dx, pick_last.top + dy, pick_last.right + dx, pick_last.bottom + dy };
        }
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_LBUTTONUP:
        if (!state_lbutton)
            break;
        draw_rects[draw_index] = drawing_rect;
        draw_colors[draw_index] = drawing_color;
        draw_index = (draw_index + 1) % 5;
        state_lbutton = false;
        break;

    case WM_RBUTTONDOWN:
        if (state_rbutton)
            break;
        pick_start = POINT{ WORLDTOCELL(x), WORLDTOCELL(y) };
        pick_index = GetIntersectRect(pick_start);
        if (pick_index >= 0)
            pick_last = draw_rects[pick_index];
        state_rbutton = true;
        break;

    case WM_RBUTTONUP:
        if (!state_rbutton)
            break;
        state_rbutton = false;
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case ID_GRID_SOLID:
            toggle_grid = false;
            break;
        case ID_GRID_DOT:
            toggle_grid = true;
            break;
        case ID_COLOR_COLOR1:
            board_color = 0;
            break;
        case ID_COLOR_COLOR2:
            board_color = 1;
            break;
        case ID_COLOR_COLOR3:
            board_color = 2;
            break;
        case ID_COLOR_COLOR4:
            board_color = 3;
            break;
        case ID_COLOR_COLOR5:
            board_color = 4;
            break;
        case ID_COLOR_COLOR6:
            board_color = 5;
            break;
        case ID_BORDER_ON:
            toggle_border = true;
            break;
        case ID_BORDER_OFF:
            toggle_border = false;
            break;
        }
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