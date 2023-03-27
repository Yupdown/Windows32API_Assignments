#include <Windows.h>
#include <tchar.h>
#include <random>

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
        100, 100, 800, 600,
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

POINT points[] = { 0, 0, 350, 200, 120, 540, 380, 180 };
POINT center = { 400, 300 };

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hDC;
    TCHAR str_buffer[128];

    static std::default_random_engine dre;
    static std::uniform_int_distribution<int> wuid{ 0, 800 };
    static std::uniform_int_distribution<int> huid{ 0, 600 };

    switch (message)
    {
    case WM_CREATE:
        dre.seed(clock());
        for (int i = 0; i < 4; ++i)
            points[i] = { wuid(dre), huid(dre) };
        break;
    case WM_PAINT:
        hDC = BeginPaint(hWnd, &ps);
        for (int i = 0; i < 4; ++i)
        {
            const wchar_t* wc = L"th";
            switch (i)
            {
            case 0:
                wc = L"st";
                break;
            case 1:
                wc = L"nd";
                break;
            case 2:
                wc = L"rd";
                break;
            }
            wsprintf(str_buffer, L"%d%s sentence: (%d, %d)", i + 1, wc, points[i].x, points[i].y);
            TextOut(hDC, points[i].x, points[i].y, str_buffer, lstrlen(str_buffer));
        }
        wsprintf(str_buffer, L"center (%d, %d)", center.x, center.y);
        TextOut(hDC, center.x, center.y, str_buffer, lstrlen(str_buffer));
        EndPaint(hWnd, &ps);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}