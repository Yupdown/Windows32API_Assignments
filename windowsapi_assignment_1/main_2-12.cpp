#include <Windows.h>
#include <tchar.h>
#include <random>

WCHAR szTitle[] = L"Windows32 API Example";
WCHAR szWindowClass[] = L"Windows32 API Class";

std::default_random_engine dre;
std::uniform_int_distribution<int> uid;

#define BOARD_SIZE 600

RECT screen_rect;

void (*shapes[])(HDC, int, int, int, int) =
{
    [](HDC hDC, int x0, int y0, int x1, int y1)
    {
        Ellipse(hDC, x0, y0, x1, y1);
    },
    [](HDC hDC, int x0, int y0, int x1, int y1)
    {
        const POINT apt[] = {(x0 + x1) / 2, y0, x0, y1, x1, y1};
        Polygon(hDC, apt, 3);
    },
    [](HDC hDC, int x0, int y0, int x1, int y1)
    {
        Rectangle(hDC, x0, y0, x1, y1);
    }
};

class PolygonFormat
{
public:
    int row;
    int col;
    int shape;
    int scale;
    COLORREF color;
};
COLORREF shape_colors[3];

PolygonFormat* polygons[10];
int polygon_size = 0;
int selected_polygon = 0;
int board_length = 40;
bool draw_mode = false;

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
        0, 0, BOARD_SIZE + 16, BOARD_SIZE + 38,
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

void ChangeBoardLength(int length)
{
    board_length = length;

    for (int i = 0; i < polygon_size; ++i)
    {
        polygons[i]->row %= length;
        polygons[i]->col %= length;
    }
}

void RemovePolygon(int index)
{
    if (index >= polygon_size)
        return;

    delete polygons[index];
    for (int i = index; i < polygon_size - 1; ++i)
        polygons[i] = polygons[i + 1];

    polygon_size -= 1;
    if ((selected_polygon > index || selected_polygon >= polygon_size) && selected_polygon > 0)
        selected_polygon -= 1;
}

void AddPolygon(int shape)
{
    if (polygon_size >= 10)
        RemovePolygon(0);

    PolygonFormat* new_polygon = new PolygonFormat();
    new_polygon->row = uid(dre) % board_length;
    new_polygon->col = uid(dre) % board_length;
    new_polygon->scale = 5;
    new_polygon->shape = shape;
    new_polygon->color = uid(dre) % 0x01000000;

    polygons[polygon_size] = new_polygon;
    polygon_size += 1;
}

void MovePolygon(int index, int dx, int dy)
{
    if (index >= polygon_size)
        return;

    PolygonFormat* pf = polygons[selected_polygon];
    pf->col = (board_length + pf->col + dx) % board_length;
    pf->row = (board_length + pf->row + dy) % board_length;
}

void PaintScreen(HDC hDC)
{
    int cell_size = BOARD_SIZE / board_length;
    for (int i = 0; i < board_length; ++i)
    {
        for (int j = 0; j < board_length; ++j)
            Rectangle(hDC, j * cell_size, i * cell_size, (j + 1) * cell_size, (i + 1) * cell_size);
    }

    PolygonFormat* spf = polygons[selected_polygon];
    if (selected_polygon < polygon_size)
    {
        HPEN select_pen = CreatePen(PS_SOLID, 3, 0x000000FF);
        HPEN old_pen = (HPEN)SelectObject(hDC, select_pen);

        Ellipse(hDC, spf->col * cell_size, spf->row * cell_size, (spf->col + 1) * cell_size, (spf->row + 1) * cell_size);

        SelectObject(hDC, old_pen);
        DeleteObject(old_pen);
    }

    for (int i = 0; i < polygon_size; ++i)
    {
        PolygonFormat pf = *polygons[i];
        if (spf != nullptr && pf.row == spf->row && pf.col == spf->col && i != selected_polygon)
            continue;

        HBRUSH select_pen = CreateSolidBrush(draw_mode ? shape_colors[pf.shape] : pf.color);
        HBRUSH old_pen = (HBRUSH)SelectObject(hDC, select_pen);
        int offset = (cell_size - cell_size * pf.scale / 5) / 2;
        shapes[pf.shape](
            hDC,
            pf.col * cell_size + offset,
            pf.row * cell_size + offset,
            (pf.col + 1) * cell_size - offset,
            (pf.row + 1) * cell_size - offset
            );
        SelectObject(hDC, old_pen);
        DeleteObject(old_pen);
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hDC, mem_hDC;
    HBITMAP mem_bit, old_bit;
    PAINTSTRUCT ps;

    switch (message)
    {
    case WM_SIZE:
        GetClientRect(hWnd, &screen_rect);
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_CHAR:
        switch (wParam)
        {
        case 's':
            ChangeBoardLength(30);
            break;

        case 'm':
            ChangeBoardLength(40);
            break;

        case 'l':
            ChangeBoardLength(50);
            break;

        case 'e':
            AddPolygon(0);
            break;

        case 't':
            AddPolygon(1);
            break;

        case 'r':
            AddPolygon(2);
            break;

        case 'c':
            if (!draw_mode)
            {
                for (int i = 0; i < 3; ++i)
                    shape_colors[i] = uid(dre) % 0x01000000;
            }
            draw_mode = !draw_mode;
            break;

        case 'd':
            RemovePolygon(selected_polygon);
            break;

        case '+':
            if (selected_polygon < polygon_size && polygons[selected_polygon]->scale < 5)
                polygons[selected_polygon]->scale += 1;
            break;

        case '-':
            if (selected_polygon < polygon_size && polygons[selected_polygon]->scale > 1)
                polygons[selected_polygon]->scale -= 1;
            break;

        case 'p':
            while (polygon_size > 0)
                RemovePolygon(polygon_size - 1);
            break;

        case 'q':
            PostQuitMessage(0);
            break;

        default:
            if (isdigit(wParam))
            {
                int index = wParam - '0';
                if (index < polygon_size)
                    selected_polygon = index;
            }
            break;
        }
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_UP:
            MovePolygon(selected_polygon, 0, -1);
            break;
        case VK_LEFT:
            MovePolygon(selected_polygon, -1, 0);
            break;
        case VK_DOWN:
            MovePolygon(selected_polygon, 0, 1);
            break;
        case VK_RIGHT:
            MovePolygon(selected_polygon, 1, 0);
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