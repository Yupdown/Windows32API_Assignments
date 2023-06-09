#include <Windows.h>
#include <tchar.h>

#include <vector>
#include <sstream>

#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)

WCHAR szTitle[] = L"Windows32 API Example";
WCHAR szWindowClass[] = L"Windows32 API Class";

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

void InsertChar(LPWSTR string, int& x, int& y, TCHAR c);
void RemoveChar(LPWSTR string, int& x, int& y);
void RemoveLine(int y);
void AddNumber(int v);

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

#define MAX_LENGTH 80

TCHAR input_string[10][128], last_string[10][128];
SIZE input_size;
int x, y;
bool toggle_uppercase, toggle_insert, toggle_password, toggle_process;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hDC;
    PAINTSTRUCT ps;

    switch (message)
    {
    case WM_CREATE:
        CreateCaret(hWnd, NULL, 5, 15);
        ShowCaret(hWnd);
        break;

    case WM_CHAR:
        if (wParam == VK_RETURN || wParam == VK_BACK || wParam == VK_TAB || wParam == VK_ESCAPE || wParam == 43 || wParam == 45)
            break;
        InsertChar(input_string[y], x, y, wParam);
        break;

    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_RETURN:
            x = 0;
            y = (y + 1) % 10;
            break;

        case VK_BACK:
            if (x > 0)
            {
                x = x - 1;
                RemoveChar(input_string[y], x, y);
            }
            else if (y > 0)
            {
                y = y - 1;
                x = lstrlen(input_string[y]);
            }
            break;

        case VK_ESCAPE:
            x = 0;
            y = 0;
            memset(input_string, 0, sizeof(input_string));
            break;

        case VK_TAB:
            for (int cnt = 0; cnt < 4; ++cnt)
                InsertChar(input_string[y], x, y, L' ');
            break;

        case VK_LEFT:
            if (x > 0)
                x = x - 1;
            else if (y > 0)
            {
                y = y - 1;
                x = lstrlen(input_string[y]);
            }
            break;

        case VK_UP:
            y = max(y - 1, 0);
            x = min(x, lstrlen(input_string[y]));
            break;

        case VK_RIGHT:
            if (x < lstrlen(input_string[y]))
                x = x + 1;
            else if (y < 9)
            {
                y = y + 1;
                x = 0;
            }
            break;

        case VK_DOWN:
            y = min(y + 1, 9);
            x = min(x, lstrlen(input_string[y]));
            break;

        case VK_F1:
            toggle_uppercase = !toggle_uppercase;
            break;

        case VK_F2:
            toggle_password = !toggle_password;
            break;

        case VK_F3:
            for (int i = 9; i > 0; --i)
                memcpy(input_string[i], input_string[i - 1], MAX_LENGTH * sizeof(TCHAR));
            RemoveLine(0);
            x = min(x, lstrlen(input_string[y]));
            break;

        case VK_F4:
            RemoveLine(y);
            x = 0;
            break;

        case VK_DELETE:
            if (input_string[y][0] != 0)
            {
                int sx = 0;
                for (int i = 0; i < x; ++i)
                {
                    if (input_string[y][i] == L' ')
                        sx = i + 1;
                }
                x = sx;
                while (input_string[y][x] != 0 && input_string[y][x] != L' ')
                    RemoveChar(input_string[y], x, y);
            }
            break;

        case VK_HOME:
            x = 0;
            break;

        case VK_END:
            x = lstrlen(input_string[y]);
            break;

        case VK_PRIOR:
            if (toggle_process)
            {
                memcpy(input_string, last_string, sizeof(input_string));
                x = min(x, lstrlen(input_string[y]));
            }
            else
            {
                memcpy(last_string, input_string, sizeof(input_string));
                for (int i = 0; i < 10; ++i)
                {
                    std::wstring buffer;
                    std::vector<std::wstring> words;
                    std::wstringstream sstream{ input_string[i] };

                    while (sstream >> buffer)
                        words.emplace_back(buffer);

                    buffer.clear();
                    for (auto& word : words)
                        buffer.append(word);

                    RemoveLine(i);
                    lstrcpyn(input_string[i], buffer.c_str(), MAX_LENGTH);
                }
                x = min(x, lstrlen(input_string[y]));
            }
            toggle_process = !toggle_process;
            break;

        case VK_NEXT:
            for (int i = 0; i < 10; ++i)
            {
                std::wstring buffer;
                std::vector<std::wstring> words;
                std::wstringstream sstream{ input_string[i] };

                while (sstream >> buffer)
                    words.emplace_back(buffer);

                buffer.clear();
                for (auto& word : words)
                {
                    buffer.push_back(L'(');
                    buffer.append(word);
                    buffer.push_back(L')');
                }

                RemoveLine(i);
                lstrcpyn(input_string[i], buffer.c_str(), MAX_LENGTH);
            }
            x = min(x, lstrlen(input_string[y]));
            break;

        case VK_INSERT:
            toggle_insert = !toggle_insert;
            break;

        case 107:
        case 187:
            AddNumber(1);
            break;

        case 109:
        case 189:
            AddNumber(-1);
            break;
        }
        HideCaret(hWnd);
        InvalidateRect(hWnd, NULL, TRUE);
        break;

    case WM_PAINT:
        hDC = BeginPaint(hWnd, &ps);

        for (int i = 0; i < 10; ++i)
            TextOut(hDC, 0, i * 16, input_string[i], lstrlen(input_string[i]));
        GetTextExtentPoint32(hDC, input_string[y], x, &input_size);

        EndPaint(hWnd, &ps);

        ShowCaret(hWnd);
        SetCaretPos(input_size.cx, y * 16);
        break;

    case WM_DESTROY:
        HideCaret(hWnd);
        DestroyCaret();

        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

void InsertChar(LPWSTR string, int& x, int& y, TCHAR c)
{
    if (!toggle_insert)
    {
        for (int i = min(lstrlen(input_string[y]), MAX_LENGTH - 1), c = 0; i > x; --i)
            input_string[y][i] = input_string[y][i - 1];
    }

    if (toggle_uppercase)
        c = towupper(c);
    if (toggle_password)
        c = '*';
    input_string[y][x++] = c;

    if (x >= MAX_LENGTH)
    {
        y = (y + 1) % 10;
        x = 0;
    }
}

void RemoveChar(LPWSTR string, int& x, int& y)
{
    for (int i = x; input_string[y][i] != 0; ++i)
        input_string[y][i] = input_string[y][i + 1];
}

void RemoveLine(int y)
{
    memset(input_string[y], 0, MAX_LENGTH * sizeof(TCHAR));
}

void AddNumber(int v)
{
    for (int i = 0; i < 10; ++i)
    {
        for (int j = 0; j < MAX_LENGTH; ++j)
        {
            if (iswdigit(input_string[i][j]))
                input_string[i][j] = (input_string[i][j] - 38 + v) % 10 + 48;
        }
    }
}