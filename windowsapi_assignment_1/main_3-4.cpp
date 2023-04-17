#include <Windows.h>
#include <tchar.h>
#include <concepts>
#include <type_traits>
#include "winapiutil.h"

WCHAR szTitle[] = L"Windows32 API Example";
WCHAR szWindowClass[] = L"Windows32 API Class";

RECT screen_rect;

HBRUSH color_brushes[] =
{
    CreateSolidBrush(0x00B92D2D),
    CreateSolidBrush(0x0042B925),
    CreateSolidBrush(0x002380B9)
};

struct Tile
{
    int color_index;
};

class Board
{
public:
    static constexpr int BOARD_COLUMNS = 6;
    static constexpr int BOARD_ROWS = 12;
    static constexpr int CELL_SIZE = 40;

private:
    Tile tile_map[BOARD_COLUMNS][BOARD_ROWS];

public:
    void Draw(HDC hDC)
    {
        for (int i = 0; i < BOARD_COLUMNS; ++i)
        {
            for (int j = 0; j < BOARD_ROWS; ++j)
            {
                RECT r = GetCellRect(POINT{ i, j });
                Rectangle(hDC, r.left, r.top, r.right, r.bottom);
            }
        }
    }
    
    RECT GetCellRect(const POINT& position) const
    {
        return RECT{ position.x * CELL_SIZE, position.y * CELL_SIZE, (position.x + 1) * CELL_SIZE, (position.y + 1) * CELL_SIZE };
    }



    void ApplyTileMap()
    {

    }
};

Board game_board;

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
        0, 0, Board::BOARD_COLUMNS * Board::CELL_SIZE + 16, Board::BOARD_ROWS * Board::CELL_SIZE + 39,
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
    game_board.Draw(hDC);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hDC, mem_hDC;
    HBITMAP mem_bit, old_bit;
    PAINTSTRUCT ps;

    switch (message)
    {
    case WM_CREATE:
        SetTimer(hWnd, 1, 500, TimerProc);
        break;

    case WM_SIZE:
        GetClientRect(hWnd, &screen_rect);
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_KEYDOWN:
        switch (wParam)
        {
        case L'\n':
            break;
        case VK_UP:
            break;
        case VK_LEFT:
            break;
        case VK_DOWN:
            break;
        case VK_RIGHT:
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

void CALLBACK TimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{

}