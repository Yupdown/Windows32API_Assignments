#include <Windows.h>
#include <tchar.h>

#include "winapiutil.h"

WCHAR szTitle[] = L"Windows32 API Example";
WCHAR szWindowClass[] = L"Windows32 API Class";

HINSTANCE instance;
RECT screen_rect;
HBITMAP bitmap_back;
HBITMAP bitmap[2];
SIZE bitmap_size;

struct GameObject
{
    POINT pos;
    int vel;
    int anim;
    int state;
};

GameObject objects[30];

unsigned long ut = 0UL;

bool state_lbutton;
bool state_rbutton;
bool state_select;

RECT box_rect;

POINT pick_start;
RECT pick_last;

HPEN highlight_pen = CreatePen(PS_SOLID, 3, 0x00FF0000);

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
    instance = hInstance;

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
        WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZE | WS_CAPTION,
        0, 0, 1280, 960,
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
    HDC bitmap_dc = CreateCompatibleDC(hDC);
    SelectObject(bitmap_dc, bitmap_back);
    StretchBlt(hDC, 0, 0, screen_rect.right, screen_rect.bottom, bitmap_dc, 0, 0, 720, 540, SRCCOPY);

    for (int i = 0; i < 30; ++i)
    {
        int animtick = ut + objects[i].anim;
        int animpos = 0;

        POINT pos = objects[i].pos;

        if (objects[i].state == 0)
        {
            SelectObject(bitmap_dc, bitmap[0]);
            animpos = (animtick / 2 % 8) * 100;
        }
        else
        {
            SelectObject(bitmap_dc, bitmap[1]);
            animpos = (animtick / 2 % 11) * 100;
            pos.x += box_rect.left;
            pos.y += box_rect.top;
        }

        TransparentBlt(hDC, pos.x, pos.y, 100, 100, bitmap_dc, animpos, 0, 100, 100, 0x00FF00FF);
    }

    SetROP2(hDC, R2_MASKPEN);
    DrawPolygon(Rectangle, hDC, box_rect.left, box_rect.top, box_rect.right, box_rect.bottom, highlight_pen, NULL);

    SetROP2(hDC, R2_COPYPEN);

    DeleteDC(bitmap_dc);
}

GameObject ResetObject()
{
    GameObject inst;
    inst.pos.x = rand() % (1280 - 100);
    inst.pos.y = -100;
    inst.vel = rand() % 10;
    inst.anim = rand();
    inst.state = 0;
    return inst;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hDC, mem_hDC;
    HBITMAP mem_bit, old_bit;
    PAINTSTRUCT ps;

    int x = LOWORD(lParam);
    int y = HIWORD(lParam);

    switch (message)
    {
    case WM_CREATE:
        SetTimer(hWnd, 1, 10, NULL);
        bitmap_back = (HBITMAP)LoadImage(instance, L"Background.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
        bitmap[0] = (HBITMAP)LoadImage(instance, L"Peppino.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
        bitmap[1] = (HBITMAP)LoadImage(instance, L"Peppino_idle.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
        BITMAP bInst;
        GetObject(bitmap, sizeof(BITMAP), &bInst);
        bitmap_size = SIZE{ bInst.bmWidth, bInst.bmHeight };
        for (int i = 0; i < 30; ++i)
            objects[i] = ResetObject();
        break;

    case WM_SIZE:
        GetClientRect(hWnd, &screen_rect);
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_TIMER:
        ut += 1UL;
        for (int i = 0; i < 30; ++i)
        {
            if (objects[i].state == 0)
            {
                int x = objects[i].pos.x;
                int y = objects[i].pos.y;
                int yp = y + objects[i].vel;

                if (x >= box_rect.left && x + 100 <= box_rect.right && y + 100 <= box_rect.bottom && yp + 100 > box_rect.bottom)
                {
                    yp = box_rect.bottom - 100;
                    objects[i].vel = 0;
                    objects[i].state = 1;
                    objects[i].pos.x = objects[i].pos.x - box_rect.left;
                }
                else
                {
                    objects[i].pos.y = yp;
                    objects[i].vel += 1;
                }
            }
            if (objects[i].state == 1)
                objects[i].pos.y = box_rect.bottom - box_rect.top - 100;

            if (objects[i].pos.y >= screen_rect.bottom)
                objects[i] = ResetObject();
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
    
    case WM_CHAR:
        switch (towupper(wParam))
        {
        case L'P':
            for (int i = 0; i < 30; ++i)
                objects[i] = ResetObject();
            break;
        case L'D':
            for (int i = 0; i < 30; ++i)
            {
                if (objects[i].state == 0)
                    continue;
                objects[i].pos.x += box_rect.left;
                objects[i].pos.y += box_rect.top;
                objects[i].state = 0;
            }
            box_rect = { 0, 0, 0, 0 };
            break;
        case L'R':
            for (int i = 0; i < 30; ++i)
                objects[i] = ResetObject();
            box_rect = { 0, 0, 0, 0 };
            break;
        }
        break;

    case WM_MOUSEMOVE:
        if (state_lbutton)
        {
            box_rect.right = x;
            box_rect.bottom = y;
        }
        if (state_rbutton && state_select)
        {
            int dx = x - pick_start.x;
            int dy = y - pick_start.y;
            box_rect = { pick_last.left + dx, pick_last.top + dy, pick_last.right + dx, pick_last.bottom + dy };
        }
        break;

    case WM_LBUTTONDOWN:
        if (state_lbutton)
            break;
        for (int i = 0; i < 30; ++i)
        {
            if (objects[i].state == 0)
                continue;
            objects[i].pos.x += box_rect.left;
            objects[i].pos.y += box_rect.top;
            objects[i].state = 0;
        }
        box_rect = { x, y, x, y };

        state_lbutton = true;
        break;

    case WM_LBUTTONUP:
        if (!state_lbutton)
            break;

        box_rect.right = x;
        box_rect.bottom = y;
        {
            RECT r;
            r.left = min(box_rect.left, box_rect.right);
            r.top = min(box_rect.top, box_rect.bottom);
            r.right = max(box_rect.left, box_rect.right);
            r.bottom = max(box_rect.top, box_rect.bottom);

            box_rect = r;
        }

        state_lbutton = false;
        break;

    case WM_RBUTTONDOWN:
        if (state_rbutton)
            break;

        pick_start = { x, y };

        if (x > box_rect.left && x < box_rect.right &&
            y > box_rect.top && y < box_rect.bottom)
        {
            state_select = true;
            pick_last = box_rect;
        }
        else
            state_select = false;

        state_rbutton = true;
        break;

    case WM_RBUTTONUP:
        if (!state_rbutton)
            break;

        state_rbutton = false;
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}