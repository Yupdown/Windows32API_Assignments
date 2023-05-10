#include <Windows.h>
#include <tchar.h>
#include <math.h>
#include <random>

#include "winapiutil.h"
#include "resource.h"

std::default_random_engine dre;
std::uniform_int_distribution<int> uid;

WCHAR szTitle[] = L"Windows32 API Example";
WCHAR szWindowClass[] = L"Windows32 API Class";

RECT screen_rect;
HINSTANCE instance;
SIZE bitmap_size;
SIZE board_size = { 720, 720 };

HBITMAP bitmaps[2];
int selected_bitmap;

int interpolation;
int shift[4];

int game_mode;

bool toggle_mode;
bool toggle_preview;
bool toggle_invert;
int select_index;

unsigned long t;

int BOARD_ROWS = 3;
int BOARD_COLUMNS = 3;

int fragment_size;

bool state_lbutton;
bool state_rbutton;

POINT pick_start;
POINT pick_last;

int pick_index = -1;

POINT board_to_world(const POINT& board)
{
    POINT p;
    p.x = board.x * board_size.cx / BOARD_COLUMNS;
    p.y = board.y * board_size.cy / BOARD_ROWS;
    return p;
}

class Fragment
{
public:
    RECT target;
    POINT position;
    POINT board_position;

public:
    void Render(HDC hDC, HDC bitmap_DC, bool flag)
    {
        int targetx = target.right - target.left;
        int targety = target.bottom - target.top;
        int cell_width = board_size.cx / BOARD_COLUMNS;
        int cell_height = board_size.cy / BOARD_ROWS;
        int offset = flag ? 10 : 0;
        StretchBlt(hDC, position.x + offset, position.y + offset, cell_width - offset * 2, cell_height - offset * 2, bitmap_DC, target.left, target.top, targetx, targety, toggle_invert ? SRCINVERT : SRCCOPY);
    }

    bool IntersectPoint(const POINT& point)
    {
        int cell_width = board_size.cx / BOARD_COLUMNS;
        int cell_height = board_size.cy / BOARD_ROWS;

        if (point.x < position.x || point.x > position.x + cell_width)
            return false;
        if (point.y < position.y || point.y > position.y + cell_height)
            return false;
        return true;
    }

    bool IsCanMove(const POINT& blank)
    {
        return abs(blank.x - board_position.x) + abs(blank.y - board_position.y) <= 1;
    }
};

Fragment fragments[24];
POINT blank_position;

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
        LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU5)),
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

void Initialize(int mode)
{
    game_mode = mode;
    fragment_size = mode ? BOARD_ROWS * BOARD_COLUMNS : BOARD_ROWS * BOARD_COLUMNS - 1;

    for (int i = 0; i < fragment_size; ++i)
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

        fragments[i].board_position = { col, row };
    }

    blank_position.x = BOARD_COLUMNS - 1;
    blank_position.y = BOARD_ROWS - 1;

    toggle_invert = false;
    toggle_mode = false;
    toggle_preview = false;
}

int GetIntersectFragment(const POINT& point)
{
    for (int i = fragment_size - 1; i >= 0; --i)
    {
        if (fragments[i].IntersectPoint(point) && (game_mode == 1 || fragments[i].IsCanMove(blank_position)))
            return i;
    }
    return -1;
}

void PaintScreen(HDC hDC)
{
    HDC bitmap_dc = CreateCompatibleDC(hDC);
    SelectObject(bitmap_dc, bitmaps[selected_bitmap]);
    if (toggle_preview)
        BitBlt(hDC, 0, 0, bitmap_size.cx, bitmap_size.cy, bitmap_dc, 0, 0, toggle_invert ? SRCINVERT : SRCCOPY);
    else
    {
        for (int i = 0; i < fragment_size; ++i)
        {
            if (pick_index != i)
                fragments[i].Render(hDC, bitmap_dc, false);
        }
        if (pick_index >= 0)
            fragments[pick_index].Render(hDC, bitmap_dc, true);
    }
    DeleteDC(bitmap_dc);
}

void StartGame()
{
    if (toggle_mode)
        return;
    for (int i = 1; i < fragment_size; ++i)
        std::swap(fragments[i].target, fragments[uid(dre) % i].target);
    toggle_mode = true;
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
        bitmaps[0] = (HBITMAP)LoadImage(instance, L"Lenna.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
        bitmaps[1] = (HBITMAP)LoadImage(instance, L"Cornell.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
        BITMAP bInst;
        GetObject(bitmaps[selected_bitmap], sizeof(BITMAP), &bInst);
        bitmap_size = SIZE{ bInst.bmWidth, bInst.bmHeight };
        Initialize(0);
        break;

    case WM_TIMER:
        t += 1UL;
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_CHAR:
        switch (towupper(wParam))
        {
        case L'S':
            StartGame();
            break;
        case L'F':
            toggle_preview = !toggle_preview;
            break;
        case L'Q':
            PostQuitMessage(0);
            break;
        case L'V':
            if (game_mode)
                BOARD_ROWS = max(BOARD_COLUMNS, BOARD_ROWS);
            BOARD_COLUMNS = 1;
            Initialize(1);
            StartGame();
            break;
        case L'H':
            if (game_mode)
                BOARD_COLUMNS = max(BOARD_COLUMNS, BOARD_ROWS);
            BOARD_ROWS = 1;
            Initialize(1);
            StartGame();
            break;
        }
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_MOUSEMOVE:
        if (state_rbutton && pick_index >= 0)
        {
            int dx = x - pick_start.x;
            int dy = y - pick_start.y;

            POINT bp = fragments[pick_index].board_position;

            int xf = min(bp.x, blank_position.x);
            int xt = max(bp.x, blank_position.x);
            int yf = min(bp.y, blank_position.y);
            int yt = max(bp.y, blank_position.y);
            if (game_mode == 1)
            {
                xf = 0;
                xt = BOARD_COLUMNS - 1;
                yf = 0;
                yt = BOARD_ROWS - 1;
            }

            POINT pf = board_to_world({ xf, yf });
            POINT pt = board_to_world({ xt, yt });

            POINT p = { pick_last.x + dx, pick_last.y + dy };
            p.x = min(max(p.x, pf.x), pt.x);
            p.y = min(max(p.y, pf.y), pt.y);
            fragments[pick_index].position = p;
        }
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_LBUTTONDOWN:
        if (state_rbutton || !toggle_mode)
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
        if (pick_index >= 0)
        {
            int cell_width = board_size.cx / BOARD_COLUMNS;
            int cell_height = board_size.cy / BOARD_ROWS;

            POINT fp = board_to_world(fragments[pick_index].board_position);
            POINT tp = fragments[pick_index].position;

            if (game_mode == 0)
            {
                if (abs(tp.x - fp.x) * 2 > cell_width || abs(tp.y - fp.y) * 2 > cell_height)
                {
                    fragments[pick_index].position = board_to_world(blank_position);
                    std::swap(blank_position, fragments[pick_index].board_position);
                }
                else
                    fragments[pick_index].position = fp;
            }
            else
            {
                int bx = (tp.x * 2 / cell_width + 1) / 2;
                int by = (tp.y * 2 / cell_height + 1) / 2;

                for (int i = 0; i < fragment_size; ++i)
                {
                    if (bx == fragments[i].board_position.x && fragments[i].board_position.y == by)
                        std::swap(fragments[pick_index].target, fragments[i].target);
                }

                fragments[pick_index].position = fp;
            }
        }
        pick_index = -1;
        state_rbutton = false;
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_SIZE:
        GetClientRect(hWnd, &screen_rect);
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case ID_BITMAP_TYPE1:
            selected_bitmap = 0;
            break;
        case ID_BITMAP_TYPE2:
            selected_bitmap = 1;
            break;

        case ID_SIZE_3X3:
            BOARD_ROWS = 3;
            BOARD_COLUMNS = 3;
            Initialize(0);
            break;
        case ID_SIZE_4X4:
            BOARD_ROWS = 4;
            BOARD_COLUMNS = 4;
            Initialize(0);
            break;
        case ID_SIZE_5X5:
            BOARD_ROWS = 5;
            BOARD_COLUMNS = 5;
            Initialize(0);
            break;
        case ID_GAME_START40056:
            StartGame();
            break;
        case ID_GAME_PREVIEW:
            toggle_preview = !toggle_preview;
            break;
        case ID_GAME_INVERT:
            toggle_invert = !toggle_invert;
            break;
        case ID_GAME_QUIT40059:
            PostQuitMessage(0);
            break;
        }
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