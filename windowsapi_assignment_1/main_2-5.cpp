#include <Windows.h>
#include <tchar.h>

WCHAR szTitle[] = L"Windows32 API Example";
WCHAR szWindowClass[] = L"Windows32 API Class";

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void ProcessString(WPARAM command, LPWSTR string, int& line, int& length);

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
        0, 0, 800, 600,
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

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hDC;
    PAINTSTRUCT ps;

    static TCHAR input_string[32];
    static RECT invalidate_rect;
    static SIZE input_size;
    static int line, length;

    switch (message)
    {
    case WM_CREATE:
        CreateCaret(hWnd, NULL, 5, 15);
        ShowCaret(hWnd);
        break;

    case WM_CHAR:
        ProcessString(wParam, input_string, line, length);
        HideCaret(hWnd);
        InvalidateRect(hWnd, NULL, FALSE);
        break;

    case WM_PAINT:
        hDC = BeginPaint(hWnd, &ps);

        GetTextExtentPoint32(hDC, input_string, length, &input_size);
        TextOut(hDC, 0, line * 16, input_string, length);
        TextOut(hDC, input_size.cx, line * 16, L"  ", 2);

        EndPaint(hWnd, &ps);

        ShowCaret(hWnd);
        SetCaretPos(input_size.cx, line * 16);
        break;

    case WM_DESTROY:
        HideCaret(hWnd);
        DestroyCaret();

        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

void ProcessString(WPARAM command, LPWSTR string, int& line, int& length)
{
    switch (command)
    {
    case VK_RETURN:
        line = (line + 1) % 10;
        length = 0;
        string[0] = 0;
        break;

    case VK_BACK:
        if (length > 0)
            string[--length] = 0;
        break;

    default:
        if (length >= 30)
            ProcessString(VK_RETURN, string, line, length);
        string[length++] = command;
        string[length] = 0;
        break;
    }
}
