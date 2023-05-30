#include <Windows.h>
#include <tchar.h>
#include <time.h>
#include <vector>
#include <cmath>

#include "resource.h"

HINSTANCE g_hInst;
WCHAR szTitle[] = L"Windows32 API Example";
WCHAR szWindowClass[] = L"Windows32 API Class";

HWND hWnd;
RECT screen_rect;
int draw_mode;
int shape_mode;
int size_of = 5;
int speed = 2;
int stop_time = -1;
COLORREF shape_color;
wchar_t buffer[128];
std::vector<POINT> points;

bool toggle_shape;
bool toggle_reverse;
bool toggle_dialogue_draw;

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

    hWnd = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_DIALOG4), nullptr, (DLGPROC)&DlalogProc);

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
    if (draw_mode != 1)
    {
        bool flag = false;
        POINT* sp = nullptr;
        for (POINT& p : points)
        {
            if (flag)
                LineTo(hDC, p.x, p.y);
            else
            {
                MoveToEx(hDC, p.x, p.y, NULL);
                flag = true;
                sp = &p;
            }
        }

        if (sp != nullptr && draw_mode != 2)
        {
            LineTo(hDC, sp->x, sp->y);

            if (toggle_shape)
            {
                HPEN pen = CreatePen(PS_SOLID, 3, shape_color);
                HPEN old_pen = (HPEN)SelectObject(hDC, pen);

                int t = stop_time < 0 ? clock() : stop_time;
                int n = points.size();
                int idx = (t * speed) / 10 % n;
                POINT p = points[idx];
                (shape_mode ? Rectangle : Ellipse)(hDC, p.x - size_of, p.y - size_of, p.x + size_of, p.y + size_of);

                SelectObject(hDC, old_pen);
                DeleteObject(pen);

                TextOut(hDC, p.x + 10, p.y, buffer, lstrlen(buffer));
            }
        }
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hDC;
    PAINTSTRUCT ps;

    switch (message)
    {
    case WM_CREATE:
        break;

    case WM_SIZE:
        GetClientRect(hWnd, &screen_rect);
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_PAINT:
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

BOOL CALLBACK DlalogProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    HWND hListBox;
    HDC hDC, mem_hDC;
    HBITMAP mem_bit, old_bit;
    PAINTSTRUCT ps;

    int x = LOWORD(lParam);
    int y = HIWORD(lParam);

    switch (iMsg)
    {
    case WM_INITDIALOG:
        CheckRadioButton(hDlg, IDC_RADIO1, IDC_RADIO8, IDC_RADIO1);
        CheckRadioButton(hDlg, IDC_RADIO9, IDC_RADIO10, IDC_RADIO9);
        CheckRadioButton(hDlg, IDC_RADIO12, IDC_RADIO13, IDC_RADIO12);
        SetTimer(hDlg, 1,
            20, NULL);
        break;

    case WM_MOUSEMOVE:
        if (draw_mode == 2)
            points.push_back(POINT{ x, y });
        break;

    case WM_LBUTTONDOWN:
        if (draw_mode)
        {
            draw_mode = 2;
            points.push_back(POINT{ x, y });
        }
        break;

    case WM_LBUTTONUP:
        draw_mode = 0;
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BUTTON1:
            points.clear(); // draw
            draw_mode = 1;
            break;
        case IDC_BUTTON33: // move
            toggle_shape ^= 1;
            break;
        case IDC_BUTTON34: // quit
            PostQuitMessage(0);
            break;

        case IDC_RADIO1:
            size_of = 5;
            break;
        case IDC_RADIO2:
            size_of = 10;
            break;
        case IDC_RADIO8:
            size_of = 15;
            break;
        case IDC_RADIO9:
            shape_mode = 0;
            break;
        case IDC_RADIO10:
            shape_mode = 1;
            break;
        case IDC_RADIO12:
            speed = 2;
            break;
        case IDC_RADIO13:
            speed = 1;
            break;
        }
        break;

    case WM_TIMER:
        InvalidateRect(hWnd, NULL, true);
        break;

    case WM_PAINT:
        hDC = BeginPaint(hWnd, &ps);
        /*mem_hDC = CreateCompatibleDC(hDC);

        mem_bit = CreateCompatibleBitmap(hDC, screen_rect.right, screen_rect.bottom);
        old_bit = (HBITMAP)SelectObject(mem_hDC, mem_bit);

        PatBlt(mem_hDC, 0, 0, screen_rect.right, screen_rect.bottom, WHITENESS);
        PaintScreen(mem_hDC);
        BitBlt(hDC, 0, 0, screen_rect.right, screen_rect.bottom, mem_hDC, 0, 0, SRCCOPY);

        SelectObject(mem_hDC, old_bit);
        DeleteObject(mem_bit);

        DeleteDC(mem_hDC);*/
        PaintScreen(hDC);
        EndPaint(hWnd, &ps);
        break;

    case WM_CLOSE:
        PostQuitMessage(0);
        break;
    }
    return 0;
}