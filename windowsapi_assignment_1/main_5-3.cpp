#include <Windows.h>
#include <tchar.h>
#include <math.h>
#include <random>

#include "winapiutil.h"

std::default_random_engine dre;
std::uniform_int_distribution<int> uid;

WCHAR szTitle[] = L"Windows32 API Example";
WCHAR szWindowClass[] = L"Windows32 API Class";

RECT screen_rect;
HINSTANCE instance;

HBITMAP bitmaps[2];
int bitmap_selected;
HPEN highlight_pen = CreatePen(PS_SOLID, 3, 0x000000FF);

SIZE bitmap_size;

unsigned long t;

bool state_lbutton;
bool state_rbutton;

bool toggle_fullscreen;
bool flag_copy;

class Magnifier
{
public:
    RECT r_src;
    RECT r_dst;
    RECT r_temp;
    POINT velocity = { 5, -5 };
    int level = 10;
    int move = 0;
    bool flip_x;
    bool flip_y;

public:
    void Move(int dx, int dy)
    {
        dx = max(-r_dst.left, dx);
        dy = max(-r_dst.top, dy);
        dx = min(-r_dst.right + bitmap_size.cx, dx);
        dy = min(-r_dst.bottom + bitmap_size.cy, dy);

        r_dst.left += dx;
        r_dst.top += dy;
        r_dst.right += dx;
        r_dst.bottom += dy;

        r_src = r_dst;
    }

    void ChangeMode(int new_mode)
    {
        if (new_mode == move)
            new_mode = 0;

        if (move == 2)
            r_dst = r_temp;

        r_temp = r_dst;
        move = new_mode;
    }

    void ChangeSize(int ds)
    {
        r_dst.left -= ds;
        r_dst.top -= ds;
        r_dst.right += ds;
        r_dst.bottom += ds;

        if (r_dst.right - r_dst.left < 10)
        {
            int center = (r_dst.right + r_dst.left) / 2;
            r_dst.left = center - 5;
            r_dst.right = center + 5;
        }
        if (r_dst.bottom - r_dst.top < 10)
        {
            int center = (r_dst.top + r_dst.bottom) / 2;
            r_dst.top = center - 5;
            r_dst.bottom = center + 5;
        }
    }

    void Render(HDC hDC, HDC bitmap_DC, bool full = false)
    {
        int dst_w = (r_src.right - r_src.left) * level / 10;
        int dst_h = (r_src.bottom - r_src.top) * level / 10;
        DrawPolygon(Rectangle, hDC, r_dst.left, r_dst.top, r_dst.right, r_dst.bottom, highlight_pen, NULL);
        if (full)
        {
            StretchBlt(hDC, 0, 0, bitmap_size.cx, bitmap_size.cy,
                bitmap_DC, r_src.left + (r_src.right - r_src.left - dst_w) / 2, r_src.top + (r_src.bottom - r_src.top - dst_h) / 2, dst_w, dst_h,
                SRCCOPY);
        }
        else
        {
            int x = r_src.left + (r_src.right - r_src.left - dst_w) / 2;
            int y = r_src.top + (r_src.bottom - r_src.top - dst_h) / 2;
            int w = dst_w;
            int h = dst_h;
            if (flip_x)
            {
                x += w;
                w = -w;
            }
            if (flip_y)
            {
                y += h;
                h = -h;
            }

            StretchBlt(hDC, r_dst.left, r_dst.top, r_dst.right - r_dst.left, r_dst.bottom - r_dst.top,
                bitmap_DC, x, y, w, h,
                SRCCOPY);
        }
    }

    void Update()
    {
        switch (move)
        {
        case 1:
        {
            int px = r_dst.left;
            int py = r_dst.top;
            int pw = r_dst.right - r_dst.left;
            int ph = r_dst.bottom - r_dst.top;

            px += velocity.x;
            py += velocity.y;

            if (px < 0)
            {
                px = -px;
                velocity.x = -velocity.x;
            }
            if (py < 0)
            {
                py = -py;
                velocity.y = -velocity.y;
            }
            if (px + pw > bitmap_size.cx)
            {
                px = (bitmap_size.cx - pw) * 2 - px;
                velocity.x = -velocity.x;
            }
            if (py + ph > bitmap_size.cy)
            {
                py = (bitmap_size.cy - ph) * 2 - py;
                velocity.y = -velocity.y;
            }

            r_dst.left = px;
            r_dst.top = py;
            r_dst.right = px + pw;
            r_dst.bottom = py + ph;
        }
            break;

        case 2:
        {
            int ds = round(sin(0.1 * t) * 100);
            r_dst.left = r_temp.left - ds;
            r_dst.top = r_temp.top - ds;
            r_dst.right = r_temp.right + ds;
            r_dst.bottom = r_temp.bottom + ds;
        }
            break;
        }
    }
};

Magnifier magnifier_clipboard;
Magnifier magnifiers[6];
Magnifier& magnifier_main = magnifiers[0];
int magnifier_size;

POINT pick_start;
RECT pick_last;

int pick_index = -1;

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

void ValidateMagnifiers(const Magnifier& src)
{
    for (int idx = 1; idx < magnifier_size; ++idx)
        magnifiers[idx].r_src = src.r_src;
}

void PaintScreen(HDC hDC)
{
    HDC bitmap_dc = CreateCompatibleDC(hDC);
    SelectObject(bitmap_dc, bitmaps[bitmap_selected]);
    BitBlt(hDC, 0, 0, bitmap_size.cx, bitmap_size.cy, bitmap_dc, 0, 0, SRCCOPY);
    for (int i = 0; i < magnifier_size; ++i)
        magnifiers[i].Render(hDC, bitmap_dc);
    if (toggle_fullscreen)
        magnifier_clipboard.Render(hDC, bitmap_dc, true);
    DeleteDC(bitmap_dc);
}

void Initialize()
{
    toggle_fullscreen = false;
    flag_copy = false;
    magnifier_size = 0;
    magnifier_main.move = 0;
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
        GetObject(bitmaps[0], sizeof(BITMAP), &bInst);
        bitmap_size = SIZE{ bInst.bmWidth, bInst.bmHeight };
        break;

    case WM_TIMER:
        t += 1UL;
        magnifier_main.Update();
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_CHAR:
        switch (towupper(wParam))
        {
        case L'1':
            bitmap_selected = 0;
            break;
        case L'2':
            bitmap_selected = 1;
            break;
        case L'E':
            magnifier_main.level -= 1;
            if (magnifier_main.level <= 0)
                magnifier_main.level = 10;
            break;
        case L'S':
            magnifier_main.level += 1;
            if (magnifier_main.level >= 20)
                magnifier_main.level = 10;
            break;
        case L'0':
            magnifier_main.level = 10;
            break;
        case L'C':
            magnifier_clipboard = magnifier_main;
            flag_copy = true;
            break;
        case L'P':
            if (flag_copy)
            {
                if (magnifier_size <= 5)
                    magnifier_size += 1;
                int index = magnifier_size - 1;
                RECT r = magnifier_clipboard.r_dst;
                POINT pos = POINT{ uid(dre) % (bitmap_size.cx - r.right + r.left), uid(dre) % (bitmap_size.cy - r.bottom + r.top) };

                magnifiers[index] = magnifier_clipboard;
                magnifiers[index].r_dst.right = pos.x + r.right - r.left;
                magnifiers[index].r_dst.bottom = pos.y + r.bottom - r.top;
                magnifiers[index].r_dst.left = pos.x;
                magnifiers[index].r_dst.top = pos.y;
            }
            break;
        case L'F':
            if (flag_copy)
                toggle_fullscreen = !toggle_fullscreen;
            break;
        case L'R':
            Initialize();
            break;
        case L'M':
            magnifier_main.ChangeSize(-5);
            break;
        case L'N':
            magnifier_main.ChangeSize(5);
            break;
        case L'H':
            for (int idx = 1; idx < magnifier_size; ++idx)
                magnifiers[idx].flip_x = !magnifiers[idx].flip_x;
            break;
        case L'V':
            for (int idx = 1; idx < magnifier_size; ++idx)
                magnifiers[idx].flip_y = !magnifiers[idx].flip_y;
            break;
        case L'X':
        {
            int dx = isupper(wParam) ? 10 : -10;
            magnifier_main.r_dst.left -= dx;
            magnifier_main.r_dst.right += dx;
        }
            break;
        case L'Y':
        {
            int dy = isupper(wParam) ? 10 : -10;
            magnifier_main.r_dst.top -= dy;
            magnifier_main.r_dst.bottom += dy;
        }
            break;
        case L'A':
            magnifier_main.ChangeMode(1);
            break;
        case L'B':
            magnifier_main.ChangeMode(2);
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
            magnifier_main.Move(0, -10);
            ValidateMagnifiers(magnifier_main);
            break;
        case VK_LEFT:
            magnifier_main.Move(-10, 0);
            ValidateMagnifiers(magnifier_main);
            break;
        case VK_DOWN:
            magnifier_main.Move(0, 10);
            ValidateMagnifiers(magnifier_main);
            break;
        case VK_RIGHT:
            magnifier_main.Move(10, 0);
            ValidateMagnifiers(magnifier_main);
            break;
        }
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_MOUSEMOVE:
        if (state_lbutton)
        {
            magnifier_main.r_dst.right = x;
            magnifier_main.r_dst.bottom = y;
            magnifier_main.r_src = magnifier_main.r_dst;
        }
        if (state_rbutton && pick_index >= 0)
        {
            int dx = x - pick_start.x;
            int dy = y - pick_start.y;
            magnifiers[pick_index].r_dst = { pick_last.left + dx, pick_last.top + dy, pick_last.right + dx, pick_last.bottom + dy };
            magnifiers[pick_index].r_src = magnifiers[pick_index].r_dst;
            ValidateMagnifiers(magnifier_main);
        }
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_LBUTTONDOWN:
        if (state_lbutton)
            break;
        magnifier_main.r_dst = { x, y, x, y };
        magnifier_main.r_src = magnifier_main.r_dst;
        magnifier_main.level = 10;

        if (magnifier_size <= 0)
            magnifier_size = 1;

        state_lbutton = true;
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_LBUTTONUP:
        if (!state_lbutton)
            break;

        magnifier_main.r_dst.right = x;
        magnifier_main.r_dst.bottom = y;
        {
            RECT r;
            r.left = min(magnifier_main.r_dst.left, magnifier_main.r_dst.right);
            r.top = min(magnifier_main.r_dst.top, magnifier_main.r_dst.bottom);
            r.right = max(magnifier_main.r_dst.left, magnifier_main.r_dst.right);
            r.bottom = max(magnifier_main.r_dst.top, magnifier_main.r_dst.bottom);

            magnifier_main.r_dst = r;
            magnifier_main.r_src = r;
        }

        state_lbutton = false;
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_RBUTTONDOWN:
        if (state_rbutton)
            break;

        pick_start = { x, y };
        pick_index = -1;
        for (int i = 0; i < magnifier_size; ++i)
        {
            if (x > magnifiers[i].r_dst.left && x < magnifiers[i].r_dst.right &&
                y > magnifiers[i].r_dst.top && y < magnifiers[i].r_dst.bottom)
                pick_index = i;
        }
        if (pick_index >= 0)
            pick_last = magnifiers[pick_index].r_dst;

        state_rbutton = true;
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_RBUTTONUP:
        if (!state_rbutton)
            break;

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