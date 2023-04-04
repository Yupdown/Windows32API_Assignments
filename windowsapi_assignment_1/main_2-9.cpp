#include <Windows.h>
#include <tchar.h>
#include <random>

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
        0, 0, 320, 360,
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

void (*shapes[])(HDC, int, int, int, int) =
{
    [](HDC hDC, int x0, int y0, int x1, int y1)
    {
        Pie(hDC, x0, y0, x1, y1, x0, (y0 + y1) / 2, (x0 + x1) / 2, y0);
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
    [](HDC hDC, int x0, int y0, int x1, int y1)
    {
        const POINT apt[] = {
            (x0 + x1) / 2, y0,
            x1, y0 + (y1 - y0) * 2 / 5,
            x0 + (x1 - x0) * 4 / 5, y1,
            x0 + (x1 - x0) / 5, y1,
            x0, y0 + (y1 - y0) * 2 / 5
        };
        Polygon(hDC, apt, 5);
    },
};

std::default_random_engine dre;
std::uniform_int_distribution<int> uid;

RECT region_center = { 100, 100, 200, 200 };
RECT regions[4] =
{
    100, 0, 200, 100,
    100, 200, 200, 300,
    0, 100, 100, 200,
    200, 100, 300, 200,
};
HBRUSH solid_brushes[6];
int region_to_shape[4] = { 0, 1, 2, 3 };
int shape_to_region[4];
int selected_shape = 0;
int colored_region = 0;
int mode = 0;
bool keydown = false;

void SelectShape(int index, int new_mode)
{
    selected_shape = index;
    mode = new_mode;
    if (mode == 1)
    {
        DeleteObject(solid_brushes[5]);
        solid_brushes[5] = CreateSolidBrush(uid(dre) % 0x01000000);
        colored_region = shape_to_region[index];
    }
}

void Draw(HDC hDC)
{
    Rectangle(hDC, region_center.left, region_center.top, region_center.right, region_center.bottom);
    for (int i = 0; i < 4; ++i)
    {
        HBRUSH old_brush = (HBRUSH)SelectObject(hDC, solid_brushes[mode == 1 && i == colored_region ? 5 : i]);
        RECT r = regions[i];
        shapes[region_to_shape[i]](hDC, r.left + 10, r.top + 10, r.right - 10, r.bottom - 10);
        SelectObject(hDC, old_brush);
    }
    int margin = mode == 2 ? 30 : 10;
    int brush_index = 0;
    switch (mode)
    {
    case 0:
        brush_index = 4;
        break;
    case 1:
        brush_index = 5;
        break;
    case 2:
        brush_index = shape_to_region[selected_shape];
        break;
    }
    HBRUSH old_brush = (HBRUSH)SelectObject(hDC, solid_brushes[brush_index]);
    shapes[selected_shape](
        hDC,
        region_center.left + margin,
        region_center.top + margin,
        region_center.right - margin,
        region_center.bottom - margin
        );
    SelectObject(hDC, old_brush);
}

void QuitApplication()
{
    for (int i = 0; i < 6; ++i)
        DeleteObject(solid_brushes[i]);
    PostQuitMessage(0);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hDC;
    PAINTSTRUCT ps;

    switch (message)
    {
    case WM_CREATE:
        dre.seed(time(NULL));
        for (int i = 0; i < 4; ++i)
            std::swap(region_to_shape[i], region_to_shape[uid(dre) % 4]);
        for (int i = 0; i < 4; ++i)
            shape_to_region[region_to_shape[i]] = i;
        for (int i = 0; i < 6; ++i)
            solid_brushes[i] = CreateSolidBrush(uid(dre) % 0x01000000);
        selected_shape = uid(dre) % 4;
        break;

    case WM_KEYDOWN:
        if (keydown)
            break;
        keydown = true;
        switch (toupper(wParam))
        {
        case 'E':
            SelectShape(0, 1);
            break;
        case 'S':
            SelectShape(1, 1);
            break;
        case 'T':
            SelectShape(2, 1);
            break;
        case 'P':
            SelectShape(3, 1);
            break;
        case 'Q':
            QuitApplication();
            break;
        case VK_UP:
            SelectShape(region_to_shape[0], 2);
            break;
        case VK_DOWN:
            SelectShape(region_to_shape[1], 2);
            break;
        case VK_LEFT:
            SelectShape(region_to_shape[2], 2);
            break;
        case VK_RIGHT:
            SelectShape(region_to_shape[3], 2);
            break;
        }
        InvalidateRect(hWnd, NULL, true);
        break;

    case WM_KEYUP:
        mode = 0;
        keydown = false;
        InvalidateRect(hWnd, NULL, true);
        break;

    case WM_PAINT:
        hDC = BeginPaint(hWnd, &ps);
        Draw(hDC);
        EndPaint(hWnd, &ps);
        break;

    case WM_DESTROY:
        QuitApplication();
        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}