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
int shape = 0;
int size = 5;
int speed = 5;
int colors[3];
HWND hChecks[3];
HWND hEdit;
HWND hList;

double get_radius(int shape, double t)
{
    double tp, l;

    switch (shape)
    {
    case 0:
        tp = t + 1.57079632;
        l = 2.09439510;
        return 0.5 / sin(2.61799388 - (tp - floor(tp / l) * l));
    case 1:
        tp = t + 0.78539816;
        l = 1.57079632;
        return 0.70710678 / sin(2.35619449 - (tp - floor(tp / l) * l));
    case 2:
        return 1.0;
    case 3:
        return sin(t * 2.0);
    }
    return 0.0;
}

void PaintScreen(HDC hDC)
{
    int sx = screen_rect.right;
    int sy = screen_rect.bottom;

    MoveToEx(hDC, 0, sy / 2, NULL);
    LineTo(hDC, sx, sy / 2);

    MoveToEx(hDC, sx / 2, 0, NULL);
    LineTo(hDC, sx / 2, sy);

    double r0 = 250.0;
    for (int i = 0; i <= 360; ++i)
    {
        double t = 0.017453292 * i;
        double r = get_radius(shape, t) * r0;
        int dx = cos(t) * r;
        int dy = sin(t) * r;

        if (i == 0)
            MoveToEx(hDC, sx / 2 + dx, sy / 2 + dy, NULL);
        LineTo(hDC, sx / 2 + dx, sy / 2 + dy);
    }

    double t = eTime * 0.001;
    double r = get_radius(shape, t) * r0;
    int dx = cos(t) * r;
    int dy = sin(t) * r;

    COLORREF col = 0;
    for (int i = 0; i < 3; ++i)
        col |= colors[i] ? (0xFF << i * 8) : 0;

    HPEN pen = CreatePen(PS_SOLID, 1, col);
    HPEN old_pen = (HPEN)SelectObject(hDC, pen);
    Rectangle(hDC, sx / 2 + dx - size, sy / 2 + dy - size, sx / 2 + dx + size, sy / 2 + dy + size);
    SelectObject(hDC, old_pen);
    DeleteObject(pen);
}

void SendChatMessage(HWND hWnd)
{
    wchar_t wstr[128];
    GetDlgItemText(hWnd, 400, wstr, 128);
    SetDlgItemText(hWnd, 400, L"");
    SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)wstr);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hDC, mem_hDC;
    HBITMAP mem_bit, old_bit;
    PAINTSTRUCT ps;

    switch (message)
    {
    case WM_CREATE:
        CreateWindow(L"ChildClass", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_THICKFRAME, 10, 10, 900, 900, hWnd, (HMENU)0, g_hInst, NULL);

        CreateWindow(L"button", L"Triangle", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP, 960, 40, 160, 30, hWnd, (HMENU)101, g_hInst, NULL);
        CreateWindow(L"button", L"Rectangle", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, 960, 70, 160, 30, hWnd, (HMENU)102, g_hInst, NULL);
        CreateWindow(L"button", L"Ellipse", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, 960, 100, 160, 30, hWnd, (HMENU)103, g_hInst, NULL);
        CreateWindow(L"button", L"Flower", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, 960, 130, 160, 30, hWnd, (HMENU)104, g_hInst, NULL);
        CreateWindow(L"button", L"Shape", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 960, 10, 160, 150, hWnd, (HMENU)100, g_hInst, NULL);

        CreateWindow(L"button", L"Small", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP, 960, 190, 160, 30, hWnd, (HMENU)201, g_hInst, NULL);
        CreateWindow(L"button", L"Medium", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, 960, 220, 160, 30, hWnd, (HMENU)202, g_hInst, NULL);
        CreateWindow(L"button", L"Large", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, 960, 250, 160, 30, hWnd, (HMENU)203, g_hInst, NULL);
        CreateWindow(L"button", L"Size", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 960, 160, 160, 120, hWnd, (HMENU)200, g_hInst, NULL);

        hChecks[0] = CreateWindow(L"button", L"Red", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_GROUP, 960, 340, 160, 30, hWnd, (HMENU)301, g_hInst, NULL);
        hChecks[1] = CreateWindow(L"button", L"Green", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 960, 370, 160, 30, hWnd, (HMENU)302, g_hInst, NULL);
        hChecks[2] = CreateWindow(L"button", L"Blue", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 960, 400, 160, 30, hWnd, (HMENU)303, g_hInst, NULL);
        CreateWindow(L"button", L"Color", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 960, 310, 160, 120, hWnd, (HMENU)300, g_hInst, NULL);

        hEdit = CreateWindow(L"edit", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 960, 470, 160, 30, hWnd, (HMENU)400, g_hInst, NULL);
        CreateWindow(L"button", L"Send", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 960, 510, 160, 30, hWnd, (HMENU)401, g_hInst, NULL);

        CreateWindow(L"button", L"Move", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 960, 550, 75, 30, hWnd, (HMENU)501, g_hInst, NULL);
        CreateWindow(L"button", L"+", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 1045, 550, 75, 30, hWnd, (HMENU)502, g_hInst, NULL);
        CreateWindow(L"button", L"-", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 960, 590, 75, 30, hWnd, (HMENU)503, g_hInst, NULL);
        CreateWindow(L"button", L"Stop", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 1045, 590, 75, 30, hWnd, (HMENU)504, g_hInst, NULL);

        CheckRadioButton(hWnd, 101, 104, 101);
        CheckRadioButton(hWnd, 201, 203, 201);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case 101:
            shape = 0;
            break;
        case 102:
            shape = 1;
            break;
        case 103:
            shape = 2;
            break;
        case 104:
            shape = 3;
            break;
        case 201:
            size = 5;
            break;
        case 202:
            size = 10;
            break;
        case 203:
            size = 20;
            break;
        case 301:
            colors[0] = SendMessage(hChecks[0], BM_GETCHECK, 0, 0) == BST_CHECKED;
            break;
        case 302:
            colors[1] = SendMessage(hChecks[1], BM_GETCHECK, 0, 0) == BST_CHECKED;
            break;
        case 303:
            colors[2] = SendMessage(hChecks[2], BM_GETCHECK, 0, 0) == BST_CHECKED;
            break;
        case 401:
            SendChatMessage(hWnd);
            break;
        case 501:
            speed = 5;
            break;
        case 502:
            speed += 1;
            break;
        case 503:
            speed -= 1;
            break;
        case 504:
            speed = 0;
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
        hList = CreateWindow(L"listbox", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_NOTIFY, 0, 0, 200, 400, hWnd, (HMENU)101, g_hInst, NULL);
        SetTimer(hWnd, 1, 0, NULL);
        break;

    case WM_SIZE:
        GetClientRect(hWnd, &screen_rect);
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_TIMER:
        eTime += speed;
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