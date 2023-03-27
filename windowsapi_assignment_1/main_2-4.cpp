#include <Windows.h>
#include <tchar.h>

WCHAR szTitle[] = L"Windows32 API Example";
WCHAR szWindowClass[] = L"Windows32 API Class";

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

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
        0, 0, 640, 480,
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

TCHAR input_string[128];
TCHAR buffer_string[128];
int length;
int x, y, n, m;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hDC;
    PAINTSTRUCT ps;

    SIZE input_size;

    switch (message)
    {
    case WM_CREATE:
        length = 0;

        CreateCaret(hWnd, NULL, 5, 15);
        ShowCaret(hWnd);
        break;

    case WM_CHAR:
        switch (wParam)
        {
        case VK_RETURN:
            _stscanf_s(input_string, L"%d %d %d %d", &x, &y, &n, &m);
            length = 0;
            input_string[0] = 0;
            break;
        case VK_BACK:
            if (length > 0)
                input_string[--length] = 0;
            break;

        default:
            input_string[length++] = wParam;
            input_string[length] = 0;
            break;
        }
        InvalidateRect(hWnd, NULL, TRUE);
        break;

    case WM_PAINT:
        hDC = BeginPaint(hWnd, &ps);
        for (int b = 0, yp = y; b < m; ++b)
        {
            wsprintf(buffer_string, L"%d * %d = %d", n, b + 1, n * (b + 1));
            TextOut(hDC, x, yp, buffer_string, lstrlen(buffer_string));
            GetTextExtentPoint32(hDC, buffer_string, lstrlen(buffer_string), &input_size);
            yp += input_size.cy;
        }
        GetTextExtentPoint32(hDC, input_string, length, &input_size);
        TextOut(hDC, 0, 0, input_string, length);
        SetCaretPos(input_size.cx, 0);

        EndPaint(hWnd, &ps);
        break;

    case WM_DESTROY:
        HideCaret(hWnd);
        DestroyCaret();
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}