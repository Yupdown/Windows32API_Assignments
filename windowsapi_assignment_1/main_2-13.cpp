#include <Windows.h>
#include <tchar.h>
#include <random>

WCHAR szTitle[] = L"Windows32 API Example";
WCHAR szWindowClass[] = L"Windows32 API Class";

std::default_random_engine dre;
std::uniform_int_distribution<int> uid;

RECT screen_rect;

LPCWSTR word;
LPCWSTR words[] =
{
    L"WHITE",
    L"BLACK",
    L"YELLOW",
    L"GREEN",
    L"MAGENTA",
    L"BROWN",
    L"ORANGE",
    L"LIGHTGRAY",
    L"OLIVE",
    L"PURPLE"
};
int count;
bool alpha_input[26];
TCHAR entered_char;

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

void Initialize()
{
    word = words[uid(dre) % 10];
    count = 0;
    entered_char = 0;

    memset(alpha_input, false, sizeof(alpha_input));
}

void EnterAlphabet(TCHAR c)
{
    c = towupper(c);
    int index = c - L'A';

    if (alpha_input[index] == true)
        return;
    alpha_input[index] = true;
    entered_char = c;

    for (const TCHAR* ptr = word; *ptr != 0; ++ptr)
    {
        if (*ptr == c)
            return;
    }

    count += 1;
}

void PaintScreen(HDC hDC)
{
    TCHAR ch_text[2]{};
    COLORREF color = 0x00010101 * 0xFF * (5 - count) / 5;
    HBRUSH brush = CreateSolidBrush(color);

    int length = lstrlen(word);
    for (int i = 0; i < length; ++i)
    {
        RECT r = { 50 + i * 20, 50, 50 + (i + 1) * 20, 70 };
        HBRUSH old_brush = (HBRUSH)SelectObject(hDC, brush);
        Rectangle(hDC, r.left, r.top, r.right, r.bottom);
        SelectObject(hDC, old_brush);

        if (iswalpha(word[i]) && alpha_input[word[i] - L'A'])
        {
            *ch_text = word[i];
            COLORREF old_color = SetBkColor(hDC, color);
            DrawText(hDC, ch_text, 1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            SetBkColor(hDC, old_color);
        }
    }
    DeleteObject(brush);

    brush = CreateSolidBrush(0);

    for (int i = 0; i < 26; ++i)
    {
        int x = i % 13;
        int y = i / 13;
        RECT r = { 50 + x * 20, 300 + y * 20, 50 + (x + 1) * 20, 300 + (y + 1) * 20 };

        if (alpha_input[i])
        {
            HBRUSH old_brush = (HBRUSH)SelectObject(hDC, brush);
            Rectangle(hDC, r.left, r.top, r.right, r.bottom);
            SelectObject(hDC, old_brush);
        }
        else
        {
            Rectangle(hDC, r.left, r.top, r.right, r.bottom);
            *ch_text = L'A' + i;
            DrawText(hDC, ch_text, 1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
    }

    RECT r = { 130, 180, 160, 200 };
    if (entered_char != 0)
    {
        *ch_text = entered_char;
        DrawText(hDC, ch_text, 1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    MoveToEx(hDC, r.left, r.bottom, NULL);
    LineTo(hDC, r.right, r.bottom);

    switch (count)
    {
    case 5:
        MoveToEx(hDC, 500, 220, NULL);
        LineTo(hDC, 470, 280);
        MoveToEx(hDC, 500, 220, NULL);
        LineTo(hDC, 530, 280);
    case 4:
        MoveToEx(hDC, 500, 150, NULL);
        LineTo(hDC, 530, 210);
    case 3:
        MoveToEx(hDC, 500, 150, NULL);
        LineTo(hDC, 470, 210);
    case 2:
        MoveToEx(hDC, 500, 140, NULL);
        LineTo(hDC, 500, 220);
    case 1:
        MoveToEx(hDC, 500, 50, NULL);
        LineTo(hDC, 500, 100);
        Ellipse(hDC, 480, 100, 520, 140);
    case 0:
        Rectangle(hDC, 400, 300, 600, 350);
        MoveToEx(hDC, 420, 300, NULL);
        LineTo(hDC, 420, 50);
        LineTo(hDC, 580, 50);
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hDC, mem_hDC;
    HBITMAP mem_bit, old_bit;
    PAINTSTRUCT ps;

    switch (message)
    {
    case WM_CREATE:
        Initialize();
        break;

    case WM_CHAR:
        if (iswalpha(wParam) && count < 5)
            EnterAlphabet(wParam);
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_HOME:
            Initialize();
            break
                ;
        case VK_END:
            PostQuitMessage(0);
            break;
        }
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_SIZE:
        GetClientRect(hWnd, &screen_rect);
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