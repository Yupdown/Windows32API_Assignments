#include <Windows.h>
#include <tchar.h>
#include <time.h>

#include <random>
#include <vector>

HINSTANCE g_hInst;
WCHAR szTitle[] = L"Windows32 API Example";
WCHAR szWindowClass[] = L"Windows32 API Class";

std::default_random_engine dre;
std::uniform_int_distribution<int> uid;

HBITMAP bitmaps[5];

RECT screen_rect;
unsigned long u_time;

struct Player
{
    POINT pos;
    POINT vel;
    float offset;
    long jump_time;
};

struct Enemy
{
    POINT pos;
    POINT vel;
};

struct Bullet
{
    POINT pos;
    POINT vel;
};

struct Effect
{
    POINT pos;
    unsigned long e_time;
};

Player player;
std::vector<Enemy> enemies;
std::vector<Bullet> bullets;
std::vector<Effect> effects;

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

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
    wClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wClass.lpszMenuName = NULL;
    wClass.lpszClassName = szWindowClass;
    wClass.hIconSm = LoadIcon(NULL, IDI_INFORMATION);
    RegisterClassEx(&wClass);

    HWND hWnd = CreateWindow(
        szWindowClass, szTitle,
        WS_OVERLAPPEDWINDOW,
        0, 0, 960, 960,
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
    player.pos = POINT{ 480, 720 };
    player.vel = POINT{ 0, 0 };

    for (int i = 0; i < 5; ++i)
    {
        Enemy enemy;
        enemy.pos = POINT{ 240 + uid(dre) % 480, uid(dre) % 480 };
        enemy.vel = POINT{ 0, 12 + uid(dre) % 3 };
        enemies.push_back(enemy);
    }
}

void AddForce(int dx, int dy)
{
    player.vel.x = min(player.vel.x + dx, 20);
    player.vel.y = min(player.vel.y + dy, 20);
}

inline float Lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

void AddBullet(POINT pos)
{
    Bullet bullet;
    bullet.pos = pos;
    bullet.vel = POINT{ 0, -20 - uid(dre) % 10 };
    bullets.push_back(bullet);
}

void AddEffect(POINT pos)
{
    Effect effect;
    effect.pos = pos;
    effect.e_time = u_time;
    effects.push_back(effect);
}

void PaintScreen(HDC hDC)
{
    HDC bitmap_dc = CreateCompatibleDC(hDC);
    SelectObject(bitmap_dc, bitmaps[1]);
    StretchBlt(hDC, 0, u_time / 2 % 960, 960, 960, bitmap_dc, 0, 0, 192, 192, SRCCOPY);
    StretchBlt(hDC, 0, u_time / 2 % 960 - 960, 960, 960, bitmap_dc, 0, 0, 192, 192, SRCCOPY);

    SelectObject(bitmap_dc, bitmaps[3]);
    for (Enemy& enemy : enemies)
        TransparentBlt(hDC, enemy.pos.x - 30, enemy.pos.y - 30, 65, 60, bitmap_dc, (u_time / 100 % 4) * 26, 0, 26, 24, 0x00FF00FF);

    for (Bullet& bullet : bullets)
        Rectangle(hDC, bullet.pos.x - 4, bullet.pos.y, bullet.pos.x + 4, bullet.pos.y + 40);

    SelectObject(bitmap_dc, bitmaps[4]);
    for (Effect& effect : effects)
    {
        long ax = (u_time - effect.e_time) / 50;
        if (ax < 26)
            TransparentBlt(hDC, effect.pos.x - 52, effect.pos.y - 120, 105, 165, bitmap_dc, ax * 42, 0, 42, 66, 0x00FF00FF);
    }

    SelectObject(bitmap_dc, bitmaps[0]);
    TransparentBlt(hDC, player.pos.x - 80 - player.offset, player.pos.y - 80 - player.offset, 160 + player.offset * 2, 160 + player.offset * 2, bitmap_dc, (u_time / 100 % 5) * 64, 0, 64, 64, 0x00FF00FF);

    SelectObject(bitmap_dc, bitmaps[2]);
    TransparentBlt(hDC, 0, u_time * 1 % 960, 960, 960, bitmap_dc, 0, 0, 192, 192, 0x00FF00FF);
    TransparentBlt(hDC, 0, u_time * 1 % 960 - 960, 960, 960, bitmap_dc, 0, 0, 192, 192, 0x00FF00FF);
    DeleteDC(bitmap_dc);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hDC, mem_hDC;
    HBITMAP mem_bit, old_bit;
    PAINTSTRUCT ps;

    switch (message)
    {
    case WM_CREATE:
        bitmaps[0] = (HBITMAP)LoadImage(g_hInst, L"Raven.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
        bitmaps[1] = (HBITMAP)LoadImage(g_hInst, L"apebackground0.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
        bitmaps[2] = (HBITMAP)LoadImage(g_hInst, L"apebackground1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
        bitmaps[3] = (HBITMAP)LoadImage(g_hInst, L"Enemy.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
        bitmaps[4] = (HBITMAP)LoadImage(g_hInst, L"Explosion.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
        Initialize();
        SetTimer(hWnd, 1, 0, NULL);
        break;

    case WM_SIZE:
        GetClientRect(hWnd, &screen_rect);
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_TIMER:
        u_time = clock();

        player.pos.x += player.vel.x;
        player.pos.y += player.vel.y;

        player.offset = Lerp(player.offset, player.jump_time > 0 ? 100.0f : 0.0f, 0.1f);
        player.jump_time -= 1;

        {
            bool flag0 = false;
            for (Enemy& enemy : enemies)
            {
                enemy.pos.x += enemy.vel.x;
                enemy.pos.y += enemy.vel.y;

                if (enemy.pos.y >= 1000)
                {
                    enemy.pos = POINT{ 240 + uid(dre) % 480, -20 };
                    enemy.vel = POINT{ 0, 12 + uid(dre) % 3 };
                }

                int dx = enemy.pos.x - player.pos.x;
                int dy = enemy.pos.y - player.pos.y;

                if (dx * dx + dy * dy < 1000)
                    flag0 |= true;

                bool flag1 = false;
                for (Bullet& bullet : bullets)
                {
                    if (bullet.pos.y < 0)
                        continue;

                    int dx = enemy.pos.x - bullet.pos.x;
                    int dy = enemy.pos.y - bullet.pos.y;

                    if (dx * dx + dy * dy < 1000)
                    {
                        flag1 |= true;
                        bullet.pos.y = -100;
                    }
                }

                if (flag1)
                {
                    AddEffect(enemy.pos);

                    enemy.pos = POINT{ 240 + uid(dre) % 480, -20 };
                    enemy.vel = POINT{ 0, 12 + uid(dre) % 3 };
                }
            }

            if (player.jump_time <= 0 && flag0)
            {
                AddEffect(player.pos);

                player.pos = POINT{ 480, 720 };
                player.vel = POINT{ 0, 0 };
                player.offset = 0.0f;
                player.jump_time = 100;
            }
        }

        for (Bullet& bullet : bullets)
        {
            bullet.pos.x += bullet.vel.x;
            bullet.pos.y += bullet.vel.y;
        }

        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_UP:
            player.jump_time = 100;
            break;
        case VK_LEFT:
            player.vel.x = -10;
            break;
        case VK_RIGHT:
            player.vel.x = 10;
            break;
        case VK_SPACE:
            AddBullet(player.pos);
            break;
        }
        break;
    case WM_KEYUP:
        switch (wParam)
        {
        case VK_LEFT:
        case VK_RIGHT:
            player.vel.x = 0;
            break;
        }
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