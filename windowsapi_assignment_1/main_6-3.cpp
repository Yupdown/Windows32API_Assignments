#include <Windows.h>
#include <tchar.h>
#include <time.h>
#include <vector>
#include <cmath>

#include "resource.h"

HINSTANCE g_hInst;
WCHAR szTitle[] = L"Windows32 API Example";
WCHAR szWindowClass[] = L"Windows32 API Class";

HWND hDialogWnd;
RECT screen_rect;
int draw_mode;
int shape_mode;
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
                int idx = toggle_reverse ? n - t / 10 % n - 1 : t / 10 % n;
                POINT p = points[idx];
                (shape_mode ? Rectangle : Ellipse)(hDC, p.x - 10, p.y - 10, p.x + 10, p.y + 10);

                SelectObject(hDC, old_pen);
                DeleteObject(pen);

                TextOut(hDC, p.x + 10, p.y, buffer, lstrlen(buffer));
            }
        }
    }
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
        hDialogWnd = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_DIALOG3), hWnd, (DLGPROC)&DlalogProc);
        SetTimer(hWnd, 1, 0, NULL);
        break;

    case WM_SIZE:
        GetClientRect(hWnd, &screen_rect);
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_TIMER:
        InvalidateRect(hWnd, NULL, false);
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
        else
            ShowWindow(hDialogWnd, SW_SHOW);
        break;

    case WM_LBUTTONUP:
        draw_mode = 0;
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

BOOL CALLBACK DlalogProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    HWND hListBox;

    HDC hDC;
    PAINTSTRUCT ps;

    switch (iMsg)
    {
    case WM_INITDIALOG:
        CheckRadioButton(hDlg, IDC_RADIO1, IDC_RADIO2, IDC_RADIO2);
        CheckRadioButton(hDlg, IDC_RADIO3, IDC_RADIO5, IDC_RADIO5);
        hListBox = GetDlgItem(hDlg, IDC_LIST1);
        SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)L"Tech University Korea");
        SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)L"Major of Game Engineering");
        SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)L"2022180003 Kim Doyup");
        SetTimer(hDlg, 1, 0, NULL);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_LIST1:
            switch (HIWORD(wParam))
            {
            case LBN_SELCHANGE:
            {
                hListBox = GetDlgItem(hDlg, IDC_LIST1);
                int i = SendMessage(hListBox, LB_GETCURSEL, 0, 0);
                SendMessage(hListBox, LB_GETTEXT, i, (LPARAM)buffer);
            }
            break;
            }
            break;

        case IDC_BUTTON1:
            points.clear();
            draw_mode = 1;
            break;
        case IDC_BUTTON29:
            toggle_shape ^= 1;
            break;
        case IDC_BUTTON6:
            toggle_reverse ^= 1;
            break;
        case IDC_BUTTON30:
            if (stop_time < 0)
                stop_time = clock();
            else
                stop_time = -1;
            break;
        case IDC_BUTTON31:
            points.clear();
            draw_mode = 0;
            toggle_shape = false;
            toggle_reverse = false;
            stop_time = -1;
            buffer[0] = 0;
            break;
        case IDC_BUTTON32:
            toggle_dialogue_draw ^= 1;
            break;

        case IDC_RADIO2:
            shape_mode = 0;
            break;
        case IDC_RADIO1:
            shape_mode = 1;
            break;

            break;
        case IDC_RADIO5:
            shape_color = 0x000000FF;
            break;
        case IDC_RADIO4:
            shape_color = 0x0000FF00;
            break;
        case IDC_RADIO3:
            shape_color = 0x00FF0000;
            break;

        case IDC_BUTTON8:
            PostQuitMessage(0);
            break;
        }
        break;

    case WM_TIMER:
        InvalidateRect(hDlg, NULL, true);
        break;

    case WM_PAINT:
        hDC = BeginPaint(hDlg, &ps);
        if (toggle_dialogue_draw)
        {
            POINT p;
            p.x = (sin(clock() / 1000.0f) + 1.0f) * 160 + 70;
            p.y = 240;
            (shape_mode ? Rectangle : Ellipse)(hDC, p.x - 10, p.y - 10, p.x + 10, p.y + 10);
        }
        EndPaint(hDlg, &ps);
        break;

    case WM_CLOSE:
        EndDialog(hDialogWnd, 0);
        break;
    }
    return 0;
}