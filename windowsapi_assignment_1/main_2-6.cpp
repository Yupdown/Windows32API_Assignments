#include <Windows.h>
#include <random>
#include <tchar.h>

WCHAR szTitle[] = L"Windows32 API Example";
WCHAR szWindowClass[] = L"Windows32 API Class";

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void PaintPattern(HDC, int, int, int, int, int, int);

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

    static std::default_random_engine dre;
    static std::uniform_int_distribution<int> uid;

    static int x, y, seed;

    switch (message)
    {
    case WM_CREATE:
        dre.seed(clock());
        x = uid(dre) % 7 + 4;
        y = uid(dre) % 9 + 2;
        seed = uid(dre);
        break;

    case WM_PAINT:
        dre.seed(seed);
        hDC = BeginPaint(hWnd, &ps);
        for (int i = 0; i < y; ++i)
        {
            for (int j = 0; j < x; ++j)
            {
                SetBkColor(hDC, uid(dre) % 0x01000000);
                SetTextColor(hDC, uid(dre) % 0x01000000);
                PaintPattern(hDC, uid(dre) % 6 + 1, 800 * j / x, 600 * i / y, 800 / x, 600 / y, 15);
            }
        }
        EndPaint(hWnd, &ps);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

void PaintPattern(HDC hDC, int pattern, int x, int y, int w, int h, int n)
{
    SIZE char_size;
    GetTextExtentPoint32(hDC, L"#", 1, &char_size);

    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < n; ++j)
        {
            bool flag = false;
            int ip = ip = i * 2 >= n ? i + 1 : i;
            int jp = j * 2 >= n ? j + 1 : j;

            switch (pattern)
            {
            case 1:
                flag = i == j || n - i == j + 1;
                break;
            case 2:
                flag = i <= j && n - i >= j + 1;
                break;
            case 3:
                flag = (i * 2 + 1 - n) + abs(j * 2 + 1 - n) <= n;
                break;
            case 4:
                flag = (jp <= i) ^ (n - jp <= i);
                break;
            case 5:
                flag = (j >= ip) ^ (n - j <= ip);
                break;
            case 6:
                flag = j * 3 / n != 1;
                break;
            }

            if (flag)
                TextOut(hDC, x + (w - char_size.cx) * j / (n - 1), y + (h - char_size.cy) * i / (n - 1), L"#", 1);
        }
    }
}