#include <Windows.h>
#include <tchar.h>
#include <cmath>

#include "resource.h"
#include "winapiutil.h"

struct Brick
{
    RECT rect;
    HBRUSH color_fill;
    int break_time;
};

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
        0, 0, 640 + 16, 480 + 39,
        nullptr,
        LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU2)),
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
Brick bricks[5][10];

HBRUSH brick_fillcolors[]
{
    CreateSolidBrush(0x00FFFF00),
    CreateSolidBrush(0x00FF00FF),
    CreateSolidBrush(0x0000FFFF),
};
HBRUSH current_fillcolor = NULL;

POINT ball_position = { 320, 400 };
int ball_velocity_magnitude;
POINT ball_velocity_direction;
int ball_radius = 10;
int bricks_rows = 3;

RECT bar_rect;
bool mouse_down;
bool ball_state;
bool update_enabled;
LONG current_time;
TCHAR text_count[128];

void UpdateBarRect(LONG position_x)
{
    bar_rect = { position_x - 60, rect_board.bottom - 60, position_x + 60, rect_board.bottom - 40 };
}

void Initialize()
{
    for (int i = 0; i < 5; ++i)
    {
        for (int j = 0; j < 10; ++j)
        {
            bricks[i][j].break_time = -1;
            bricks[i][j].color_fill = CreateSolidBrush(0x00ABCDEF * (i * 10 + j) / 50);
            bricks[i][j].rect = {};
        }
    }
    ball_velocity_magnitude = 5;
    ball_velocity_direction = { 1, -1 };
    UpdateBarRect((rect_board.left + rect_board.right) / 2);
    ball_state = false;
    update_enabled = true;
}

inline RECT CircleRect(const POINT& position, int radius)
{
    return RECT{ position.x - radius, position.y - radius, position.x + radius, position.y + radius };
}

void PaintScreen(HDC hDC)
{
    Rectangle(hDC, rect_board.left, rect_board.top, rect_board.right, rect_board.bottom);
    Rectangle(hDC, bar_rect.left, bar_rect.top, bar_rect.right, bar_rect.bottom);
    for (int i = 0; i < bricks_rows; ++i)
    {
        for (int j = 0; j < 10; ++j)
        {
            bool broken = bricks[i][j].break_time >= 0;
            if (broken && current_time - bricks[i][j].break_time > 1000)
                continue;
            int offset = broken ? 15 * (current_time - bricks[i][j].break_time) / 1000 : 0;
            DrawPolygon(Rectangle, hDC,
                bricks[i][j].rect.left + offset,
                bricks[i][j].rect.top + offset,
                bricks[i][j].rect.right - offset,
                bricks[i][j].rect.bottom - offset,
                NULL, broken ? bricks[i][j].color_fill : current_fillcolor);
        }
    }
    Ellipse(hDC, ball_position.x - ball_radius, ball_position.y - ball_radius, ball_position.x + ball_radius, ball_position.y + ball_radius);

    if (!update_enabled)
        TextOut(hDC, 1, 1, text_count, lstrlen(text_count));
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hDC, mem_hDC;
    HBITMAP mem_bit, old_bit;
    PAINTSTRUCT ps;

    switch (message)
    {
    case WM_CREATE:
        Initialize();
        SetTimer(hWnd, 1, 10, TimerProc);
        break;

    case WM_SIZE:
        GetClientRect(hWnd, &screen_rect);
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_CHAR:
        switch (towupper(wParam))
        {
        case 'S':
            ball_state = true;
            break;
        case 'P':
            update_enabled = !update_enabled;
            if (!update_enabled)
            {
                int c1 = 0;
                int c2 = 0;
                for (int i = 0; i < bricks_rows; ++i)
                {
                    for (int j = 0; j < 10; ++j)
                    {
                        if (bricks[i][j].break_time >= 0)
                            (current_time - bricks[i][j].break_time > 1000 ? c2 : c1) += 1;
                    }
                }
                wsprintf(text_count, L"Breaking bricks : %d, Broken bricks : %d", c1, c2);
            }
            break;
        case 'N':
            Initialize();
            break;
        case 'Q':
            PostQuitMessage(0);
            break;
        case '+':
            if (ball_velocity_magnitude < 10)
                ball_velocity_magnitude += 1;
            break;
        case '-':
            if (ball_velocity_magnitude > 1)
                ball_velocity_magnitude -= 1;
            break;
        }
        break;

    case WM_LBUTTONDOWN:
        mouse_down = true;
        UpdateBarRect(LOWORD(lParam));
        break;

    case WM_MOUSEMOVE:
        if (mouse_down)
            UpdateBarRect(LOWORD(lParam));
        break;

    case WM_LBUTTONUP:
        mouse_down = false;
        UpdateBarRect((rect_board.left + rect_board.right) / 2);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case ID_GAME_START:
            ball_state = true;
            break;
        case ID_GAME_RESET:
            Initialize();
            break;
        case ID_GAME_END:
            PostQuitMessage(0);
            break;
        case ID_SPEED_FAST:
            ball_velocity_magnitude = 8;
            break;
        case ID_SPEED_MEDIUM:
            ball_velocity_magnitude = 5;
            break;
        case ID_SPEED_SLOW:
            ball_velocity_magnitude = 2;
            break;
        case ID_COLOR_CYAN:
            current_fillcolor = brick_fillcolors[0];
            break;
        case ID_COLOR_MAGENTA:
            current_fillcolor = brick_fillcolors[1];
            break;
        case ID_COLOR_YELLOW:
            current_fillcolor = brick_fillcolors[2];
            break;
        case ID_SIZE_SMALL:
            ball_radius = 10;
            break;
        case ID_SIZE_BIG:
            ball_radius = 20;
            break;
        case ID_BRICKS_3:
            bricks_rows = 3;
            break;
        case ID_BRICKS_4:
            bricks_rows = 4;
            break;
        case ID_BRICKS_5:
            bricks_rows = 5;
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
    current_time = dwTime;

    if (ball_state)
    {
        if (update_enabled)
        {
            ball_position.x += ball_velocity_direction.x * ball_velocity_magnitude;
            ball_position.y += ball_velocity_direction.y * ball_velocity_magnitude;
        }
        if (ball_position.x - ball_radius < rect_board.left)
        {
            ball_position.x = (rect_board.left + ball_radius) * 2 - ball_position.x;
            ball_velocity_direction.x = -ball_velocity_direction.x;
        }
        if (ball_position.y - ball_radius < rect_board.top)
        {
            ball_position.y = (rect_board.top + ball_radius) * 2 - ball_position.y;
            ball_velocity_direction.y = -ball_velocity_direction.y;
        }
        if (ball_position.x + ball_radius > rect_board.right)
        {
            ball_position.x = (rect_board.right - ball_radius) * 2 - ball_position.x;
            ball_velocity_direction.x = -ball_velocity_direction.x;
        }
        if (ball_position.y + ball_radius > rect_board.bottom)
            ball_state = false;
    }
    else if (update_enabled)
    {
        ball_position.x = (bar_rect.left + bar_rect.right) / 2;
        ball_position.y = bar_rect.top - ball_radius;
    }

    bool flag = false;
    for (int i = 0; i < bricks_rows; ++i)
    {
        for (int j = 0; j < 10; ++j)
        {
            if (bricks[i][j].break_time >= 0)
                continue;

            int offset_x = (sin(0.005 * dwTime + i) + 1.0) * 20.0;
            bricks[i][j].rect.left = j * 60 + offset_x;
            bricks[i][j].rect.top = i * 30 + 30;
            bricks[i][j].rect.right = (j + 1) * 60 + offset_x;
            bricks[i][j].rect.bottom = (i + 1) * 30 + 30;

            if (!flag)
            {
                if (flag = ProcessAABBCollision(CircleRect(ball_position, ball_radius), bricks[i][j].rect, ball_position, ball_velocity_direction))
                    bricks[i][j].break_time = dwTime;
            }
        }
    }
    ProcessAABBCollision(CircleRect(ball_position, ball_radius), bar_rect, ball_position, ball_velocity_direction);
    InvalidateRect(hWnd, NULL, false);
}