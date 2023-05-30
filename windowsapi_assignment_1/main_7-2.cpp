#include <Windows.h>
#include <tchar.h>
#include <math.h>
#include <time.h>

HINSTANCE g_hInst;
WCHAR szTitle[] = L"Windows32 API Example";
WCHAR szWindowClass[] = L"Windows32 API Class";

RECT screen_rect;

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    CldProc(HWND, UINT, WPARAM, LPARAM);

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
    wClass.hbrBackground = (HBRUSH)CreateSolidBrush(0x00A0A0A0);
    wClass.lpszMenuName = NULL;
    wClass.lpszClassName = szWindowClass;
    wClass.hIconSm = LoadIcon(NULL, IDI_INFORMATION);
    RegisterClassEx(&wClass);

    wClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wClass.lpfnWndProc = CldProc;
    wClass.lpszMenuName = L"Child";
    wClass.lpszClassName = L"ChildClass";

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

double eTime = 0.0;
HWND hList;
HWND hList_panel;
int show_index = 0;

HBITMAP backgrounds[10];
LPCWSTR backgrounds_path[10] =
{
    L"backgrounds/wp2966843.bmp",
    L"backgrounds/wp3788126.bmp",
    L"backgrounds/wp3788128.bmp",
    L"backgrounds/wp3788129.bmp",
    L"backgrounds/wp3788133.bmp",
    L"backgrounds/wp3788135.bmp",
    L"backgrounds/wp3788139.bmp",
    L"backgrounds/wp3788141.bmp",
    L"backgrounds/wp3788160.bmp",
    L"backgrounds/wp3788163.bmp"
};

HDC canvas_dc;
HBITMAP hCanvas;
int bitmap_select[10];

bool sequence = false;
bool animate = false;
int draw_position = 0;
int sequence_length = 700;

void PaintScreen(HDC hDC)
{
    HDC bitmap_DC = CreateCompatibleDC(hDC);

    SelectObject(bitmap_DC, hCanvas);

    if (sequence)
    {
        StretchBlt(hDC, 0, 0, screen_rect.right, screen_rect.bottom, bitmap_DC, draw_position, 0, 700, 700, SRCCOPY);
        StretchBlt(hDC, 0, 0, screen_rect.right, screen_rect.bottom, bitmap_DC, draw_position - sequence_length, 0, 700, 700, SRCCOPY);
    }
    else
        StretchBlt(hDC, 0, 0, screen_rect.right, screen_rect.bottom, bitmap_DC, show_index * 700, 0, 700, 700, SRCCOPY);

    DeleteDC(bitmap_DC);
}

void AttachBackground(int index)
{
    bitmap_select[show_index] = index;

    HDC hDC = CreateCompatibleDC(canvas_dc);
    HDC hDC_src = CreateCompatibleDC(canvas_dc);

    HBITMAP old_bitmap = (HBITMAP)SelectObject(hDC_src, backgrounds[index]);

    SelectObject(hDC, hCanvas);
    BitBlt(hDC, 700 * show_index, 0, 700, 700, hDC_src, 0, 0, SRCCOPY);

    SelectObject(hDC_src, old_bitmap);
    DeleteDC(hDC_src);
    DeleteDC(hDC);

    SendMessage(hList_panel, LB_RESETCONTENT, 0, 0);
    for (int i = 0; i < 10; ++i)
    {
        if (bitmap_select[i] < 0)
            continue;
        wchar_t buffer[128];
        wsprintf(buffer, L"Section #%02d : %s", i, backgrounds_path[bitmap_select[i]]);
        SendMessage(hList_panel, LB_ADDSTRING, 0, (LPARAM)buffer);
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HBITMAP temp_bit;
    HWND temp_hwnd;

    HDC hDC, mem_hDC;
    HBITMAP mem_bit, old_bit;
    PAINTSTRUCT ps;

    switch (message)
    {
    case WM_CREATE:
        CreateWindow(L"ChildClass", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER, 70, 10, 900, 900, hWnd, (HMENU)0, g_hInst, NULL);

        hList = CreateWindow(L"listbox", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_NOTIFY, 1000, 60, 240, 480, hWnd, (HMENU)101, g_hInst, NULL);

        temp_hwnd = CreateWindow(L"button", L"Left", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_BITMAP, 10, 855, 55, 55, hWnd, (HMENU)401, g_hInst, NULL);
        temp_bit = (HBITMAP)LoadImage(g_hInst, L"Arrow2.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
        SendMessage(temp_hwnd, BM_SETIMAGE, 0, (LPARAM)temp_bit);

        temp_hwnd = CreateWindow(L"button", L"Right", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_BITMAP, 975, 855, 55, 55, hWnd, (HMENU)402, g_hInst, NULL);
        temp_bit = (HBITMAP)LoadImage(g_hInst, L"Arrow1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
        SendMessage(temp_hwnd, BM_SETIMAGE, 0, (LPARAM)temp_bit);

        CreateWindow(L"button", L"Select", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 1000, 550, 240, 40, hWnd, (HMENU)501, g_hInst, NULL);
        CreateWindow(L"button", L"Move", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 1000, 600, 240, 40, hWnd, (HMENU)502, g_hInst, NULL);
        EnableWindow(GetDlgItem(hWnd, 502), false);
        CreateWindow(L"button", L"Stop", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 1000, 650, 240, 40, hWnd, (HMENU)503, g_hInst, NULL);
        CreateWindow(L"button", L"Sequence", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 1000, 700, 240, 40, hWnd, (HMENU)504, g_hInst, NULL);

        for (int i = 0; i < 10; ++i)
        {
            backgrounds[i] = (HBITMAP)LoadImage(g_hInst, backgrounds_path[i], IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
            SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)backgrounds_path[i]);
            bitmap_select[i] = -1;
        }
        SendMessage(hList, LB_SETCURSEL, 0, 0);

        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case 401:
            show_index = (show_index + 9) % 10;
            break;
        case 402:
            show_index = (show_index + 1) % 10;
            break;
        case 501:
            AttachBackground(SendMessage(hList, LB_GETCURSEL, 0, 0));
            break;
        case 502:
            animate = true;
            break;
        case 503:
            animate = false;
            break;
        case 504:
            sequence = true;
            EnableWindow(GetDlgItem(hWnd, 501), false);
            EnableWindow(GetDlgItem(hWnd, 502), true);

            hDC = CreateCompatibleDC(canvas_dc);
            SelectObject(hDC, hCanvas);
            int l = 0;
            for (int i = 0; i < 10; ++i)
            {
                if (bitmap_select[i] < 0)
                    continue;
                if (i != l)
                    BitBlt(hDC, 700 * l, 0, 700, 700, hDC, 700 * i, 0, SRCCOPY);
                ++l;
            }
            sequence_length = 700 * max(l, 1);
            DeleteDC(hDC);
            break;
        }
        break;

    case WM_PAINT:
        hDC = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT CALLBACK CldProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hDC, mem_hDC;
    HBITMAP mem_bit, old_bit;
    PAINTSTRUCT ps;

    switch (message)
    {
    case WM_CREATE:
        canvas_dc = GetDC(NULL);
        hCanvas = CreateCompatibleBitmap(canvas_dc, 7000, 700);

        hList_panel = CreateWindow(L"listbox", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_THICKFRAME | LBS_NOTIFY, 0, 0, 400, 100, hWnd, (HMENU)101, g_hInst, NULL);
        SetTimer(hWnd, 1, 0, NULL);
        break;

    case WM_SIZE:
        GetClientRect(hWnd, &screen_rect);
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_TIMER:
        if (animate)
            draw_position = (draw_position + 10) % sequence_length;
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