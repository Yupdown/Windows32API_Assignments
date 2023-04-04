#include <Windows.h>
#include <tchar.h>
#include <random>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

HINSTANCE g_hInst;

LPCTSTR lpszClass = L"Windows Class Name";
LPCTSTR lpszWindowName = L"Windows Program 2-1";

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
        WS_OVERLAPPEDWINDOW,
        100, 100, SCREEN_WIDTH, SCREEN_HEIGHT,
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
    wcex.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

    return RegisterClassExW(&wcex);
}

COLORREF solid_brushes[] =
{
    0x00808080, 0x008080FF, 0x0080FF80, 0x0080FFFF,
    0x00FF8080, 0x00FF80FF, 0x00FFFF80, 0x00FFFFFF
};

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hDC;
    static char str_buffer[128];

    static std::default_random_engine dre;
    static std::uniform_int_distribution<int> uid{ 0, 16'777'216 };
    static int x, y, n, count;

    switch (message)
    {
    case WM_CREATE:
        dre.seed(clock());
        x = uid(dre) % 700;
        y = uid(dre) % 500;
        n = uid(dre) % 10;
        count = uid(dre) % 50;
        for (int i = 0; i < count; ++i)
            str_buffer[i] = ('0' + n);
        str_buffer[count] = 0;
        break;
    case WM_PAINT:
        hDC = BeginPaint(hWnd, &ps);
        SetBkColor(hDC, uid(dre));
        SetTextColor(hDC, uid(dre));
        TextOutA(hDC, x, y, str_buffer, count);
        EndPaint(hWnd, &ps);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}