#include <Windows.h>
#include <tchar.h>

HINSTANCE g_hInst;

LPCTSTR lpszClass = L"Windows Class Name";
LPCTSTR lpszWindowName = L"Windows Program 2";

ATOM                MyRegisterClass(HINSTANCE hInstance);
// BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrecInstance, LPSTR lpszCmdParam, int nCmdShow)
{
    HWND hWnd;
    MSG message;
    WNDCLASSEX WndClass;

    MyRegisterClass(hInstance);

    hWnd = CreateWindow(
        lpszClass,
        lpszWindowName,
        WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL,
        100, 50, 800, 600,
        NULL,
        (HMENU)NULL,
        hInstance,
        NULL
    );
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    while (GetMessage(&message, nullptr, 0, 0))
    {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }

    return message.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = (WNDPROC)WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_HAND);
    wcex.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = lpszClass;
    wcex.hIconSm = LoadIcon(nullptr, IDI_QUESTION);

    return RegisterClassExW(&wcex);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hDC;

    switch (message)
    {
    case WM_CREATE:
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