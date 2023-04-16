#include <Windows.h>
#include <tchar.h>

#include <vector>
#include <random>

#include "vector2d.h"
#include "winapiutil.h"

std::default_random_engine dre;
std::uniform_int_distribution<int> uid;

WCHAR szTitle[] = L"Windows32 API Example";
WCHAR szWindowClass[] = L"Windows32 API Class";

RECT screen_rect;

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void    CALLBACK    TimerProc(HWND, UINT, UINT_PTR, DWORD);

constexpr SIZE SCREEN_SIZE = { 1600, 1200 };
constexpr LONG LANE_NUMBER = 2;
constexpr LONG LANE_WIDTH = 60;

constexpr RECT CrossRect()
{
    constexpr int OFFSET = 20;
    return RECT
    {
        SCREEN_SIZE.cx / 2 - LANE_WIDTH * LANE_NUMBER - OFFSET,
        SCREEN_SIZE.cy / 2 - LANE_WIDTH * LANE_NUMBER - OFFSET,
        SCREEN_SIZE.cx / 2 + LANE_WIDTH * LANE_NUMBER + OFFSET,
        SCREEN_SIZE.cy / 2 + LANE_WIDTH * LANE_NUMBER + OFFSET
    };
}

class Traffic;
std::vector<Traffic> traffics;

HBRUSH debug_brush_collision = CreateHatchBrush(HS_DIAGCROSS, 0x000000FF);
HBRUSH debug_brush = CreateHatchBrush(HS_DIAGCROSS, 0x00C0C0C0);
bool toggle_debug;
bool toggle_auto;
int base_speed = 10;

class Signal
{
private:
    const int ticks[4] = { 50, 300, 50, 300 };
    int state;
    int timer;

    const int BULB_RADIUS = 20;
    const int BULB_OFFSET = 40;

    int highlight;

public:
    Signal()
    {
        ChangeState(0);
        highlight = -1;
    }

    int GetState() const
    {
        return state;
    }

    void ChangeState(int new_state)
    {
        state = new_state;
        timer = ticks[new_state];
    }

    void Draw(HDC hDC, const SIZE& screen_size)
    {
        static HBRUSH brush_black =         CreateSolidBrush(0x00101010);
        static HBRUSH brush_red =           CreateSolidBrush(0x000000FF);
        static HBRUSH brush_yellow =        CreateSolidBrush(0x0000FFFF);
        static HBRUSH brush_green =         CreateSolidBrush(0x0000FF00);
        static HBRUSH brush_red_off =       CreateSolidBrush(0x00000080);
        static HBRUSH brush_yellow_off =    CreateSolidBrush(0x00008080);
        static HBRUSH brush_green_off =     CreateSolidBrush(0x00008000);
        static HPEN pen_highLight =         CreatePen(PS_SOLID, 3, 0x00FFFFFF);

        POINT center = POINT{ screen_size.cx / 2, screen_size.cy / 2 };

        DrawPolygon(Rectangle, hDC, center.x - BULB_OFFSET - BULB_RADIUS - 10, center.y - BULB_RADIUS - 10, center.x + BULB_OFFSET + BULB_RADIUS + 10, center.y + BULB_RADIUS + 10, NULL, brush_black);
        DrawPolygon(Ellipse, hDC, 
            center.x - BULB_RADIUS - BULB_OFFSET, 
            center.y - BULB_RADIUS, 
            center.x + BULB_RADIUS - BULB_OFFSET, 
            center.y + BULB_RADIUS, 
            highlight == 0 ? pen_highLight : NULL,
            state == 1 ? brush_red : brush_red_off
        );
        DrawPolygon(Ellipse, hDC, 
            center.x - BULB_RADIUS, 
            center.y - BULB_RADIUS, 
            center.x + BULB_RADIUS, 
            center.y + BULB_RADIUS,
            highlight == 1 ? pen_highLight : NULL,
            state % 2 == 0 ? brush_yellow : brush_yellow_off
        );
        DrawPolygon(Ellipse, hDC, 
            center.x - BULB_RADIUS + BULB_OFFSET, 
            center.y - BULB_RADIUS, 
            center.x + BULB_RADIUS + BULB_OFFSET, 
            center.y + BULB_RADIUS,
            highlight == 2 ? pen_highLight : NULL,
            state == 3 ? brush_green : brush_green_off);

        static TCHAR text[16];
        if (toggle_auto)
        {
            wsprintf(text, L"AUTO (T: %d)", timer);
            TextOut(hDC, 1, 1, text, lstrlen(text));
        }
    }

    void HandleMouse(LONG x, LONG y, bool press, const SIZE& screen_size)
    {
        POINT center = POINT{ screen_size.cx / 2, screen_size.cy / 2 };

        POINT bulb_positions[] = { center.x - BULB_OFFSET, center.y, center.x, center.y, center.x + BULB_OFFSET, center.y };
        highlight = -1;
        for (int i = 0; i < 3 && highlight < 0; ++i)
        {
            LONG dx = x - bulb_positions[i].x;
            LONG dy = y - bulb_positions[i].y;

            if (dx * dx + dy * dy <= BULB_RADIUS * BULB_RADIUS)
                highlight = i;
        }

        if (press)
        {
            switch (highlight)
            {
            case 0:
                ChangeState(1);
                break;
            case 1:
                ChangeState((state + 1) % 4 / 2 * 2);
                break;
            case 2:
                ChangeState(3);
                break;
            }
        }
    }

    void Update()
    {
        timer -= 1;
        if (timer <= 0)
            ChangeState((state + 1) % 4);
    }
};

Signal signal;

class Traffic
{
private:
    Vector2d position;
    Vector2d velocity_direction;
    double velocity_magnitude;
    double speed_max;
    SIZE rect_size;
    bool collision;

public:
    Traffic(LONG dir, LONG lane, const SIZE& screen_size)
    {
        LONG length = 80 + uid(dre) % 3 * 40;
        position = { screen_size.cx / 2.0, screen_size.cy / 2.0 };
        rect_size = { 0, 0 };
        switch (dir)
        {
        case 0:
            position.x += (lane * 2 + 1) * LANE_WIDTH / 2;
            position.y = 0;
            rect_size = { 50, length };
            velocity_direction = { 0.0, -1.0 };
            break;
        case 1:
            position.x = 0;
            position.y -= (lane * 2 + 1) * LANE_WIDTH / 2;
            rect_size = { length, 50 };
            velocity_direction = { -1.0, 0.0 };
            break;
        case 2:
            position.x -= (lane * 2 + 1) * LANE_WIDTH / 2;
            position.y = 0;
            rect_size = { 50, length };
            velocity_direction = { 0.0, 1.0 };
            break;
        case 3:
            position.x = 0;
            position.y += (lane * 2 + 1) * LANE_WIDTH / 2;
            rect_size = { length, 50 };
            velocity_direction = { 1.0, 0.0 };
            break;
        }
        velocity_magnitude = 0;
        speed_max = uid(dre) % 100 / 30.0;
        collision = false;
    }

    RECT Rect() const
    {
        LONG left = position.x - rect_size.cx / 2;
        LONG top = position.y - rect_size.cy / 2;
        LONG right = position.x + rect_size.cx / 2;
        LONG bottom = position.y + rect_size.cy / 2;
        return RECT{ left, top, right, bottom };
    }

    RECT DetectRect() const
    {
        RECT r = Rect();
        LONG left;
        LONG top;
        LONG right;
        LONG bottom;
        
        double delta = velocity_magnitude * velocity_magnitude * 5.0 / 3.0 + 10.0;

        if (velocity_direction.x > 0.0)
        {
            left = r.right;
            right = r.right + delta;
        }
        else if (velocity_direction.x < 0.0)
        {
            left = r.left - delta;
            right = r.left;
        }
        else
        {
            left = r.left;
            right = r.right;
        }
        if (velocity_direction.y > 0.0)
        {
            top = r.bottom;
            bottom = r.bottom + delta;
        }
        else if (velocity_direction.y < 0.0)
        {
            top = r.top - delta;
            bottom = r.top;
        }
        else
        {
            top = r.top;
            bottom = r.bottom;
        }
        return RECT{ left, top, right, bottom };
    }

    void DebugDraw(HDC hDC, const SIZE& screen_size)
    {
        RECT rd = DetectRect();
        FillRect(hDC, &rd, collision ? debug_brush_collision : debug_brush);
    }

    void Draw(HDC hDC, const SIZE& screen_size)
    {
        RECT r = Rect();
        Rectangle(hDC, r.left, r.top, r.right, r.bottom);
        Rectangle(hDC, r.left - screen_size.cx, r.top, r.right - screen_size.cx, r.bottom);
        Rectangle(hDC, r.left + screen_size.cx, r.top, r.right + screen_size.cx, r.bottom);
        Rectangle(hDC, r.left, r.top - screen_size.cy, r.right, r.bottom - screen_size.cy);
        Rectangle(hDC, r.left, r.top + screen_size.cy, r.right, r.bottom + screen_size.cy);
    }

    void Update(const SIZE& screen_size)
    {
        RECT t_rect, d_rect = DetectRect();
        bool flag = false;
        for (Traffic& traffic : traffics)
        {
            RECT rr = Rect();
            RECT r = traffic.Rect();
            //if (IntersectRect(&t_rect, &rr, &r))
            //    velocity_magnitude = 0.0;
            flag |= IntersectRect(&t_rect, &d_rect, &r);
            if (!(signal.GetState() % 2) || (signal.GetState() / 2 ? velocity_direction.x : velocity_direction.y) != 0.0)
            {
                RECT cr = CrossRect();
                flag |= IntersectRect(&t_rect, &d_rect, &cr) && !IntersectRect(&t_rect, &rr, &cr);
            }
        }
        collision = flag;
        velocity_magnitude = collision ? max(velocity_magnitude - 0.3, 0.0) : min(velocity_magnitude + 0.1, base_speed + speed_max);

        double sx = (position.x + velocity_direction.x * velocity_magnitude) / screen_size.cx;
        double sy = (position.y + velocity_direction.y * velocity_magnitude) / screen_size.cy;
        position.x = (sx - floor(sx)) * screen_size.cx;
        position.y = (sy - floor(sy)) * screen_size.cy;
    }
};

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
        0, 0, SCREEN_SIZE.cx + 16, SCREEN_SIZE.cy + 39,
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
    static HBRUSH road_brush = CreateSolidBrush(0x00202020);
    Rectangle(hDC, 0, 0, SCREEN_SIZE.cx, SCREEN_SIZE.cy);

    RECT cr = CrossRect();
    RECT ccr = RECT
    {
        SCREEN_SIZE.cx / 2 - LANE_WIDTH * LANE_NUMBER,
        SCREEN_SIZE.cy / 2 - LANE_WIDTH * LANE_NUMBER,
        SCREEN_SIZE.cx / 2 + LANE_WIDTH * LANE_NUMBER,
        SCREEN_SIZE.cy / 2 + LANE_WIDTH * LANE_NUMBER
    };

    RECT r = RECT{ ccr.left, 0, ccr.right, SCREEN_SIZE.cy };
    FillRect(hDC, &r, road_brush);
    r = RECT{ 0, ccr.top, SCREEN_SIZE.cx, ccr.bottom };
    FillRect(hDC, &r, road_brush);

    Rectangle(hDC, ccr.left, cr.top, SCREEN_SIZE.cx / 2, ccr.top);
    Rectangle(hDC, cr.left, SCREEN_SIZE.cy / 2, ccr.left, ccr.bottom);
    Rectangle(hDC, SCREEN_SIZE.cx / 2, cr.bottom, ccr.right, ccr.bottom);
    Rectangle(hDC, ccr.right, ccr.top, cr.right, SCREEN_SIZE.cy / 2);

    // Rectangle(hDC, cr.left, cr.top, cr.right, cr.bottom);

    if (toggle_debug)
    {
        for (Traffic& traffic : traffics)
            traffic.DebugDraw(hDC, SCREEN_SIZE);
    }
    for (Traffic& traffic : traffics)
        traffic.Draw(hDC, SCREEN_SIZE);
    signal.Draw(hDC, SCREEN_SIZE);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hDC, mem_hDC;
    HBITMAP mem_bit, old_bit;
    PAINTSTRUCT ps;

    switch (message)
    {
    case WM_CREATE:
        for (int i = 0; i < 20; ++i)
            traffics.push_back(Traffic(i % 4, i / 4 % 2, SCREEN_SIZE));
        SetTimer(hWnd, 1, 1000 / 72, TimerProc);
        break;

    case WM_CHAR:
        switch (towupper(wParam))
        {
        case '+':
            base_speed = min(base_speed + 1, 10);
            break;
        case '-':
            base_speed = max(base_speed - 1, 0);
            break;
        case 'A':
            toggle_auto = !toggle_auto;
            break;
        case 'Q':
            PostQuitMessage(0);
            break;
        case 'S':
            toggle_debug = !toggle_debug;
            break;
        }
        break;

    case WM_MOUSEMOVE:
        signal.HandleMouse(LOWORD(lParam), HIWORD(lParam), false, SCREEN_SIZE);
        break;

    case WM_LBUTTONDOWN:
        signal.HandleMouse(LOWORD(lParam), HIWORD(lParam), true, SCREEN_SIZE);
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

void CALLBACK TimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    if (toggle_auto)
        signal.Update();
    for (Traffic& traffic : traffics)
        traffic.Update(SCREEN_SIZE);

    InvalidateRect(hWnd, NULL, false);
}