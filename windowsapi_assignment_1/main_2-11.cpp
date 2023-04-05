#include <Windows.h>
#include <tchar.h>
#include <random>

WCHAR szTitle[] = L"Windows32 API Example";
WCHAR szWindowClass[] = L"Windows32 API Class";

std::default_random_engine dre;
std::uniform_int_distribution<int> uid;

RECT screen_rect;
WCHAR command_buffer[128];
size_t command_size;

struct DrawFormat
{
    int mode;

    int shape;
    int thickness;

    RECT draw_rect;

    COLORREF color_edge;
    COLORREF color_fill;
};

DrawFormat current_format, last_format;
bool undo_state = false;

void (*shapes[])(HDC, int, int, int, int) =
{
    [](HDC hDC, int x0, int y0, int x1, int y1)
    {
        MoveToEx(hDC, x0, y0, NULL);
        LineTo(hDC, x1, y1);
    },
    [](HDC hDC, int x0, int y0, int x1, int y1)
    {
        Rectangle(hDC, x0, y0, x1, y1);
    },
    [](HDC hDC, int x0, int y0, int x1, int y1)
    {
        Ellipse(hDC, x0, y0, x1, y1);
    },
    [](HDC hDC, int x0, int y0, int x1, int y1)
    {
        const POINT apt[] = {x0, y0, x1, y0, x0, y1, x1, y1};
        Polygon(hDC, apt, 4);
    },
    [](HDC hDC, int x0, int y0, int x1, int y1)
    {
        const POINT apt[] = {(x0 + x1) / 2, y0, x0, y1, x1, y1};
        Polygon(hDC, apt, 3);
    },
};

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

void PaintScreen(HDC hDC)
{
    SIZE input_size;

    DrawFormat f = current_format;

    if (f.mode == 1)
    {
        for (int i = 0; i < 5; ++i)
        {
            HPEN pen = CreatePen(PS_SOLID, f.thickness, uid(dre) % 0x01000000);
            HBRUSH brush = CreateSolidBrush(uid(dre) % 0x01000000);

            HPEN old_pen = (HPEN)SelectObject(hDC, pen);
            HBRUSH old_brush = (HBRUSH)SelectObject(hDC, brush);

            shapes[i](hDC, f.draw_rect.left, f.draw_rect.top, f.draw_rect.right, f.draw_rect.bottom);
            SelectObject(hDC, old_pen);
            SelectObject(hDC, old_brush);

            DeleteObject(pen);
            DeleteObject(brush);
        }
    }
    else
    {
        HPEN pen = CreatePen(PS_SOLID, f.thickness, f.color_edge);
        HBRUSH brush = CreateSolidBrush(f.color_fill);

        HPEN old_pen = (HPEN)SelectObject(hDC, pen);
        HBRUSH old_brush = (HBRUSH)SelectObject(hDC, brush);

        shapes[f.shape](hDC, f.draw_rect.left, f.draw_rect.top, f.draw_rect.right, f.draw_rect.bottom);
        SelectObject(hDC, old_pen);
        SelectObject(hDC, old_brush);

        DeleteObject(pen);
        DeleteObject(brush);
    }

    Rectangle(hDC, 0, screen_rect.bottom - 24, screen_rect.right, screen_rect.bottom);
    GetTextExtentPoint32(hDC, command_buffer, command_size, &input_size);
    TextOut(hDC, 0, screen_rect.bottom - 22, command_buffer, command_size);
    SetCaretPos(input_size.cx, screen_rect.bottom - 22);
}

void ValidateMessage()
{
    DrawFormat f = current_format;
    if (f.draw_rect.left == f.draw_rect.right && f.draw_rect.top == f.draw_rect.bottom)
        MessageBox(NULL, L"No shape to draw", L"Error", MB_OK);
}

void Translate(int dx, int dy)
{
    current_format.draw_rect.left += dx;
    current_format.draw_rect.top += dy;
    current_format.draw_rect.right += dx;
    current_format.draw_rect.bottom += dy;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hDC, mem_hDC;
    HBITMAP mem_bit, old_bit;
    PAINTSTRUCT ps;

    switch (message)
    {
    case WM_CREATE:
        CreateCaret(hWnd, NULL, 5, 15);
        ShowCaret(hWnd);
        break;

    case WM_SIZE:
        GetClientRect(hWnd, &screen_rect);
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_CHAR:
        switch (wParam)
        {
        case VK_ESCAPE:
            command_size = 0;
            command_buffer[0] = 0;
            break;

        case VK_RETURN:
            DrawFormat f;
            _stscanf_s(command_buffer, L"%d %d %d %d %d %d", &f.shape, &f.draw_rect.left, &f.draw_rect.top, &f.draw_rect.right, &f.draw_rect.bottom, &f.thickness);
            --f.shape;
            f.color_edge = uid(dre) % 0x01000000;
            f.color_fill = uid(dre) % 0x01000000;
            last_format = current_format;
            current_format = f;

            command_size = 0;
            command_buffer[0] = 0;
            break;

        case VK_BACK:
            if (command_size > 0)
                command_buffer[--command_size] = 0;
            break;

        case '+':
            ValidateMessage();
            if (current_format.thickness < 10)
                current_format.thickness += 1;
            else
            {
                current_format.draw_rect.right += 10;
                current_format.draw_rect.bottom += 10;
            }
            break;
        case '-':
            ValidateMessage();
            if (current_format.thickness > 1)
                current_format.thickness -= 1;
            else
            {
                current_format.draw_rect.right -= 10;
                current_format.draw_rect.bottom -= 10;
            }
            break;
        case 'q':
            ValidateMessage();
            current_format.color_edge = uid(dre) % 0x01000000;
            break;
        case 'e':
            ValidateMessage();
            current_format.color_fill = uid(dre) % 0x01000000;
            break;
        case 'p':
            if (!undo_state)
                std::swap(current_format, last_format);
            undo_state = true;
            break;
        case 'n':
            if (undo_state)
                std::swap(current_format, last_format);
            undo_state = false;
            break;
        case 'a':
            ValidateMessage();
            f.mode = 1;
            f.draw_rect = current_format.draw_rect;
            f.thickness = current_format.thickness;
            last_format = current_format;
            current_format = f;
            break;

        default:
            command_buffer[command_size++] = wParam;
            command_buffer[command_size] = 0;
            break;
        }
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_UP:
            Translate(0, -10);
            break;
        case VK_LEFT:
            Translate(-10, 0);
            break;
        case VK_DOWN:
            Translate(0, 10);
            break;
        case VK_RIGHT:
            Translate(10, 0);
            break;
        }
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