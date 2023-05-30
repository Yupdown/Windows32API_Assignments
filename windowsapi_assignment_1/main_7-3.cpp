#include <Windows.h>
#include <tchar.h>
#include <math.h>
#include <time.h>

HINSTANCE g_hInst;
WCHAR szTitle[] = L"Windows32 API Example";
WCHAR szWindowClass[] = L"Windows32 API Class";

RECT screen_rect;

struct Entity
{
    float px;
    float py;
    float vx;
    float vy;
    float sx;
    float sy;
    bool ground;
};

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    CldProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
    g_hInst = hInstance;

    WNDCLASSEXW wClass;
    wClass.cbSize = sizeof(WNDCLASSEX);
    wClass.style = CS_HREDRAW | CS_VREDRAW;
    wClass.lpfnWndProc = WndProc;
    wClass.cbClsExtra = 0;
    wClass.cbWndExtra = 0;
    wClass.hInstance = hInstance;
    wClass.hIcon = LoadIcon(NULL, IDI_INFORMATION);
    wClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wClass.hbrBackground = (HBRUSH)CreateSolidBrush(0x00A0A0A0);
    wClass.lpszMenuName = NULL;
    wClass.lpszClassName = szWindowClass;
    wClass.hIconSm = LoadIcon(NULL, IDI_INFORMATION);
    RegisterClassEx(&wClass);

    wClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wClass.lpfnWndProc = CldProc;
    wClass.lpszMenuName = L"Child";
    wClass.lpszClassName = L"ChildClass";

    RegisterClassEx(&wClass);

    HWND hWnd = CreateWindow(
        szWindowClass, szTitle,
        WS_OVERLAPPEDWINDOW,
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

#define MAP_LENGTH 20

double eTime = 0.0;
HWND hList;
int show_index = 0;
Entity player;

HBITMAP player_bitmap;
HBITMAP tile_bitmaps[6];
HBITMAP backgrounds[10];
LPCWSTR backgrounds_path[10] =
{
    L"backgrounds/wp2966843.bmp",
    L"backgrounds/wp3788126.bmp",
    L"backgrounds/wp3788128.bmp",
    L"backgrounds/wp3788129.bmp",
    L"backgrounds/wp3788133.bmp",
    L"backgrounds/wp3788135.bmp",
    L"backgrounds/wp3788139.bmp",
    L"backgrounds/wp3788141.bmp",
    L"backgrounds/wp3788160.bmp",
    L"backgrounds/wp3788163.bmp"
};

HBITMAP t_backgrounds[10];
HBITMAP t_tilemap[10][MAP_LENGTH][MAP_LENGTH];
int current_stile = -1;

bool mode_test = false;
int sequence_length = 7000;

int DrawPosition(int width)
{
    if (!mode_test)
        return show_index * width;
    int v = player.px * width / MAP_LENGTH - width / 2;
    v = max(v, 0);
    v = min(v, width * 9);
    return v;
}

void PaintScreen(HDC hDC)
{
    int sx = screen_rect.right;
    int sy = screen_rect.bottom;

    HDC bitmap_dc = CreateCompatibleDC(hDC);

    for (int n = 0; n < 10; ++n)
    {
        int offset = n * sx - DrawPosition(sx);

        if (t_backgrounds[n] != NULL)
        {
            SelectObject(bitmap_dc, t_backgrounds[n]);
            StretchBlt(hDC, offset, 0, screen_rect.right, screen_rect.bottom, bitmap_dc, 0, 0, 700, 700, SRCCOPY);
        }

        for (int i = 0; i < MAP_LENGTH; ++i)
        {
            for (int j = 0; j < MAP_LENGTH; ++j)
            {
                if (t_tilemap[n][i][j] == NULL)
                    continue;

                SelectObject(bitmap_dc, t_tilemap[n][i][j]);
                TransparentBlt(hDC, i * sx / MAP_LENGTH + offset, j * sy / MAP_LENGTH, sx / MAP_LENGTH + 1, sy / MAP_LENGTH + 1, bitmap_dc, 0, 0, 8, 8, 0x00FF00FF);
            }
        }
    }

    if (!mode_test)
    {
        wchar_t buffer[128];
        wsprintf(buffer, L"SECTION #%02d", show_index);

        SetROP2(hDC, R2_MASKPEN);
        for (int i = 0; i < MAP_LENGTH; ++i)
        {
            for (int j = 0; j < MAP_LENGTH; ++j)
                Rectangle(hDC, i * sx / MAP_LENGTH, j * sy / MAP_LENGTH, (i + 1) * sx / MAP_LENGTH + 1, (j + 1) * sy / MAP_LENGTH + 1);
        }
        SetROP2(hDC, R2_COPYPEN);
        TextOut(hDC, 1, 1, buffer, lstrlen(buffer));
    }
    
    SelectObject(bitmap_dc, player_bitmap);
    TransparentBlt(hDC, (player.px - player.sx * 0.5f) * sx / MAP_LENGTH - DrawPosition(sx), (MAP_LENGTH - player.py - player.sy) * sy / MAP_LENGTH, player.sx * sx / MAP_LENGTH, player.sy * sy / MAP_LENGTH, bitmap_dc, 0, 0, 16, 24, 0x00FF00FF);
    DeleteDC(bitmap_dc);
}

void UpdateWorld()
{
    if (player.ground && GetAsyncKeyState(VK_UP) & 0x8000)
    {
        player.ground = false;
        player.vy = 0.5f;
    }
    if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
        player.vx = 0.2f;
    else if (GetAsyncKeyState(VK_LEFT) & 0x8000)
        player.vx = -0.2f;
    else
        player.vx *= 0.9f;

    if (!player.ground)
        player.vy = player.vy - 0.02f;

    float px = player.px + player.vx;
    float py = player.py + player.vy;

    if (py < 0.0f)
    {
        py = 0.0f;
        player.vy = 0.0f;
        player.ground = true;
    }
    else if (px - player.sx * 0.5f < 0.0f)
    {
        px = player.sx * 0.5f;
        player.vx = 0.0;
    }
    else if (px + player.sx * 0.5f >= MAP_LENGTH * 10)
    {
        px = MAP_LENGTH * 10 - player.sx * 0.5f;
        player.vx = 0.0;
    }
    else
    {
        int fx = floor(player.px);
        int fy = floor(player.py);
        int tx = floor(px);
        int ty = floor(py);

        HBITMAP fp = t_tilemap[fx / MAP_LENGTH][fx % MAP_LENGTH][MAP_LENGTH - 1 - fy];
        HBITMAP tp = t_tilemap[tx / MAP_LENGTH][tx % MAP_LENGTH][MAP_LENGTH - 1 - ty];

        if (player.vy < 0.0f)
        {
            if (fp == NULL && tp != NULL)
            {
                py = ty + 1;
                player.vy = 0.0f;
                player.ground = true;
            }
        }
        if (tp == NULL)
            player.ground = false;
    }

    player.px = px;
    player.py = py;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HBITMAP temp_bit;
    HWND temp_hwnd;

    HDC hDC, mem_hDC;
    HBITMAP mem_bit, old_bit;
    PAINTSTRUCT ps;

    switch (message)
    {
    case WM_CREATE:
        CreateWindow(L"ChildClass", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER, 70, 10, 900, 900, hWnd, (HMENU)0, g_hInst, NULL);

        hList = CreateWindow(L"listbox", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_NOTIFY, 1000, 110, 240, 480, hWnd, (HMENU)101, g_hInst, NULL);

        temp_hwnd = CreateWindow(L"button", L"Left", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_BITMAP, 10, 855, 55, 55, hWnd, (HMENU)401, g_hInst, NULL);
        temp_bit = (HBITMAP)LoadImage(g_hInst, L"Arrow2.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
        SendMessage(temp_hwnd, BM_SETIMAGE, 0, (LPARAM)temp_bit);

        temp_hwnd = CreateWindow(L"button", L"Right", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_BITMAP, 975, 855, 55, 55, hWnd, (HMENU)402, g_hInst, NULL);
        temp_bit = (HBITMAP)LoadImage(g_hInst, L"Arrow1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
        SendMessage(temp_hwnd, BM_SETIMAGE, 0, (LPARAM)temp_bit);

        player_bitmap = (HBITMAP)LoadImage(g_hInst, L"Terraria_player.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
        tile_bitmaps[0] = (HBITMAP)LoadImage(g_hInst, L"tiles/Tiles_0000.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
        tile_bitmaps[1] = (HBITMAP)LoadImage(g_hInst, L"tiles/Tiles_0001.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
        tile_bitmaps[2] = (HBITMAP)LoadImage(g_hInst, L"tiles/Tiles_0002.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
        tile_bitmaps[3] = (HBITMAP)LoadImage(g_hInst, L"tiles/Tiles_0003.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
        tile_bitmaps[4] = (HBITMAP)LoadImage(g_hInst, L"tiles/Tiles_0004.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
        tile_bitmaps[5] = (HBITMAP)LoadImage(g_hInst, L"tiles/Tiles_0005.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);

        temp_hwnd = CreateWindow(L"button", L"", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_BITMAP, 1000, 10, 40, 40, hWnd, (HMENU)501, g_hInst, NULL);
        SendMessage(temp_hwnd, BM_SETIMAGE, 0, (LPARAM)tile_bitmaps[0]);
        temp_hwnd = CreateWindow(L"button", L"", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_BITMAP, 1050, 10, 40, 40, hWnd, (HMENU)502, g_hInst, NULL);
        SendMessage(temp_hwnd, BM_SETIMAGE, 0, (LPARAM)tile_bitmaps[1]);
        temp_hwnd = CreateWindow(L"button", L"", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_BITMAP, 1100, 10, 40, 40, hWnd, (HMENU)503, g_hInst, NULL);
        SendMessage(temp_hwnd, BM_SETIMAGE, 0, (LPARAM)tile_bitmaps[2]);
        temp_hwnd = CreateWindow(L"button", L"", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_BITMAP, 1000, 60, 40, 40, hWnd, (HMENU)504, g_hInst, NULL);
        SendMessage(temp_hwnd, BM_SETIMAGE, 0, (LPARAM)tile_bitmaps[3]);
        temp_hwnd = CreateWindow(L"button", L"", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_BITMAP, 1050, 60, 40, 40, hWnd, (HMENU)505, g_hInst, NULL);
        SendMessage(temp_hwnd, BM_SETIMAGE, 0, (LPARAM)tile_bitmaps[4]);
        temp_hwnd = CreateWindow(L"button", L"", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_BITMAP, 1100, 60, 40, 40, hWnd, (HMENU)506, g_hInst, NULL);
        SendMessage(temp_hwnd, BM_SETIMAGE, 0, (LPARAM)tile_bitmaps[5]);

        CreateWindow(L"button", L"Del", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 1150, 60, 40, 40, hWnd, (HMENU)507, g_hInst, NULL);
        CreateWindow(L"button", L"Test", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 1000, 590, 240, 40, hWnd, (HMENU)508, g_hInst, NULL);

        for (int i = 0; i < 10; ++i)
        {
            backgrounds[i] = (HBITMAP)LoadImage(g_hInst, backgrounds_path[i], IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
            SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)backgrounds_path[i]);
        }

        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case 101:
            if (HIWORD(wParam) == LBN_DBLCLK)
            {
                int index = SendMessage(hList, LB_GETCURSEL, 0, 0);
                t_backgrounds[show_index] = backgrounds[index];
            }
            break;
        case 401:
            show_index = (show_index + 9) % 10;
            break;
        case 402:
            show_index = (show_index + 1) % 10;
            break;
        case 501:
            current_stile = 0;
            break;
        case 502:
            current_stile = 1;
            break;
        case 503:
            current_stile = 2;
            break;
        case 504:
            current_stile = 3;
            break;
        case 505:
            current_stile = 4;
            break;
        case 506:
            current_stile = 5;
            break;
        case 507:
            current_stile = -1;
            break;
        case 508:
            mode_test = true;
            EnableWindow(GetDlgItem(hWnd, 508), false);
            player.px = 1.0f;
            player.sx = 2.0f;
            player.sy = 3.0f;
            break;
        }
        break;

    case WM_PAINT:
        hDC = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

void PlaceTile(LONG sx, LONG sy, int tile)
{
    int tx = sx * MAP_LENGTH / screen_rect.right;
    int ty = sy * MAP_LENGTH / screen_rect.bottom;

    if (tx < 0 || tx >= MAP_LENGTH || ty < 0 || ty >= MAP_LENGTH)
        return;

    if (tile < 0)
        t_tilemap[show_index][tx][ty] = NULL;
    else
        t_tilemap[show_index][tx][ty] = tile_bitmaps[tile];
}

LRESULT CALLBACK CldProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static bool lbutton_down = false;

    HDC hDC, mem_hDC;
    HBITMAP mem_bit, old_bit;
    PAINTSTRUCT ps;

    switch (message)
    {
    case WM_CREATE:
        SetTimer(hWnd, 1, 0, NULL);
        break;

    case WM_SIZE:
        GetClientRect(hWnd, &screen_rect);
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_TIMER:
        UpdateWorld();
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_LBUTTONDOWN:
        lbutton_down = true;
        PlaceTile(LOWORD(lParam), HIWORD(lParam), current_stile);
        break;

    case WM_LBUTTONUP:
        lbutton_down = false;
        break;

    case WM_MOUSEMOVE:
        if (lbutton_down)
            PlaceTile(LOWORD(lParam), HIWORD(lParam), current_stile);
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