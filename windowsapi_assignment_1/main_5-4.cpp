#include <windows.h>
#include <tchar.h>
#include <math.h>

#include "winapiutil.h"

WCHAR szTitle[] = L"Windows32 API Example";
WCHAR szWindowClass[] = L"Windows32 API Class";

HINSTANCE instance;
RECT screen_rect;

HBITMAP bitmap;
SIZE bitmap_size;

SIZE player_size = {128, 144};

POINT positions[6];
int direction_indexes[6];

POINT trace_points[100];
int trace_start;

POINT exp_position;
POINT target_position;

int n_players = 1;

POINT& player_position = positions[0];
int& player_direction_index = direction_indexes[0];

POINT directions[] =
{
    {10, 0},
    {0, 10},
    {-10, 0},
    {0, -10}
};

int vx = 20;
int vy = -20;

HBRUSH brush_shadow = CreateSolidBrush(0x0808080);

unsigned long e_time;
unsigned long jump_time = 20UL;
unsigned long scale_time = 100UL;
long exp_time = 0;

bool scale_type;
bool move_type;
bool move_target;
float offset = 0.0f;

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

inline float Lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

void PaintScreen(HDC hDC)
{
    HDC bitmap_dc = CreateCompatibleDC(hDC);
    for (int i = 0; i < n_players; ++i)
    {
        SelectObject(bitmap_dc, bitmap);
        int y = (20 - jump_time) * jump_time;
        int ax = 3;
        if (y < 0)
        {
            y = 0;
            ax = e_time / 2 % 10;
        }
        POINT p = positions[i];
        if (move_type)
            p = trace_points[(99 + trace_start - i * 10) % 100];
        RECT r;
        r.left = p.x - offset;
        r.top = p.y - y - offset;
        r.right = player_size.cx + offset * 2;
        r.bottom = player_size.cy + offset * 2;

        DrawPolygon(Ellipse, hDC, p.x + 32, p.y + player_size.cy - 24, p.x + 96, p.y + player_size.cy, NULL, brush_shadow);
        TransparentBlt(hDC, r.left, r.top, r.right, r.bottom, bitmap_dc, 32 * ax, (5 - direction_indexes[i]) % 4 * 36, 32, 36, 0x00FF00FF);
    }

    if (exp_time > 0)
        TransparentBlt(hDC, exp_position.x, exp_position.y, player_size.cx, player_size.cy, bitmap_dc, 0, exp_time / 2 % 4 * 36, 32, 36, 0x00FF00FF);

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
        SetTimer(hWnd, 1, 15, NULL);
        bitmap = (HBITMAP)LoadImage(instance, L"Isaac.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
        BITMAP bInst;
        GetObject(bitmap, sizeof(BITMAP), &bInst);
        bitmap_size = SIZE{ bInst.bmWidth, bInst.bmHeight };
        break;

    case WM_TIMER:
        e_time += 1UL;
        jump_time += 1UL;
        scale_time += 1UL;
        exp_time -= 1;

        if (move_type)
        {
            int nx = player_position.x + vx;
            int ny = player_position.y + vy;

            if (nx < 0)
            {
                nx = 0;
                vx = -vx;
            }
            if (nx + player_size.cx > screen_rect.right)
            {
                nx = screen_rect.right - player_size.cx;
                vx = -vx;
            }
            if (ny < 0)
            {
                ny = 0;
                vy = -vy;
            }
            if (ny + player_size.cy > screen_rect.bottom)
            {
                ny = screen_rect.bottom - player_size.cy;
                vy = -vy;
            }

            trace_points[trace_start] = { nx, ny };
            trace_start = (trace_start + 1) % 100;

            player_position.x = nx;
            player_position.y = ny;
        }
        else
        {
            for (int i = 0; i < n_players; ++i)
            {
                if (i == 0 && move_target)
                {
                    int dx = target_position.x - positions[i].x;
                    int dy = target_position.y - positions[y].y;
                    double dist = sqrt(dx * dx + dy * dy);

                    if (dist < 10.0)
                    {
                        positions[i] = target_position;
                        move_target = false;
                    }
                    else
                    {
                        positions[i].x += dx * 20 / dist;
                        positions[i].y += dy * 20 / dist;
                    }
                }
                else
                {
                    int nx = positions[i].x + directions[direction_indexes[i]].x;
                    int ny = positions[i].y + directions[direction_indexes[i]].y;

                    bool flag = false;
                    if (nx < 0)
                    {
                        nx = 0;
                        flag |= true;
                    }
                    if (nx + player_size.cx > screen_rect.right)
                    {
                        nx = screen_rect.right - player_size.cx;
                        flag |= true;
                    }
                    if (ny < 0)
                    {
                        ny = 0;
                        flag |= true;
                    }
                    if (ny + player_size.cy > screen_rect.bottom)
                    {
                        ny = screen_rect.bottom - player_size.cy;
                        flag |= true;
                    }

                    if (flag)
                        direction_indexes[i] = (direction_indexes[i] + 1) % 4;

                    positions[i].x = nx;
                    positions[i].y = ny;
                }
            }
        }
        {
            float target_offset = 0.0f;
            if (scale_time < 50UL)
                target_offset = scale_type ? 40.0f : -40.0f;

            offset = Lerp(offset, target_offset, 0.1f);
        }
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_SIZE:
        GetClientRect(hWnd, &screen_rect);
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_CHAR:
        switch (towupper(wParam))
        {
        case L'J':
            jump_time = 0UL;
            break;
        case L'E':
            scale_time = 0UL;
            scale_type = true;
            break;
        case L'S':
            scale_time = 0UL;
            scale_type = false;
            break;
        case L'T':
            if (n_players < 6)
            {
                positions[n_players] = player_position;
                direction_indexes[n_players] = (player_direction_index + 1) % 4;
                n_players += 1;
            }
            break;
        case L'A':
            move_type = !move_type;
            if (move_type)
            {
                for (int i = 0; i < 100; ++i)
                    trace_points[i] = player_position;
                trace_start = 0;
            }
            break;
        case L'R':
            vx = -vx;
            vy = -vy;
            break;
        case L'Q':
            PostQuitMessage(0);
            break;
        }
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_UP:
            for (int i = 0; i < n_players; ++i)
                direction_indexes[i] = 3;
            break;
        case VK_LEFT:
            for (int i = 0; i < n_players; ++i)
                direction_indexes[i] = 2;
            break;
        case VK_DOWN:
            for (int i = 0; i < n_players; ++i)
                direction_indexes[i] = 1;
            break;
        case VK_RIGHT:
            for (int i = 0; i < n_players; ++i)
                direction_indexes[i] = 0;
            break;
        }
        InvalidateRect(hWnd, NULL, false);
        break;
    
    case WM_LBUTTONDOWN:
    {
        bool flag = true;
        for (int i = 0; i < n_players; ++i)
        {
            if (x >= positions[i].x && x <= positions[i].x + player_size.cx && y >= positions[i].y && y <= positions[i].y + player_size.cy)
            {
                exp_position = positions[i];
                positions[i] = { 100, 100 };
                exp_time = 30;
                flag &= false;
            }
        }
        if (flag)
        {
            target_position = { x, y };
            move_target = true;
        }
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