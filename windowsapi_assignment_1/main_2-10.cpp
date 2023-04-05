#include <Windows.h>
#include <tchar.h>
#include <random>

#define SIZEOF_BOARD 40
#define CELL_SIZE 20

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
        0, 0, SIZEOF_BOARD * CELL_SIZE + 16, SIZEOF_BOARD * CELL_SIZE + 38,
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

std::default_random_engine dre;
std::uniform_int_distribution<int> uid;

class Player;

class Tile
{
public:
    bool wall = false;
    int color;
    virtual void OnPlayer(Player& target) = 0;
};

Tile* tiles[SIZEOF_BOARD][SIZEOF_BOARD];

class Player
{
public:
    int x, y;
    int shape;
    int color;
    int scale;

    Player()
    {
        shape = 1;
        color = 3;
        scale = CELL_SIZE - 4;
    }

    bool Move(int dx, int dy)
    {
        int nx = (SIZEOF_BOARD + x + dx) % SIZEOF_BOARD;
        int ny = (SIZEOF_BOARD + y + dy) % SIZEOF_BOARD;
        if (tiles[ny][nx] != nullptr && tiles[ny][nx]->wall)
            return false;
        x = nx;
        y = ny;
        return true;
    }
};

class TileChangeColor : public Tile
{
public:
    TileChangeColor()
    {
        color = uid(dre) % 6 + 4;
    }

    void OnPlayer(Player& target) override
    {
        target.color = color;
    }
};

class TileChangeShape : public Tile
{
public:
    TileChangeShape()
    {
        color = 1;
    }

    void OnPlayer(Player& target) override
    {
        int value = uid(dre) % (4 - 1);
        target.shape = value < target.shape ? value : value + 1;
    }
};

class TileChangeScale : public Tile
{
public:
    TileChangeScale()
    {
        color = 2;
    }

    void OnPlayer(Player& target) override
    {
        int value = uid(dre) % (CELL_SIZE - 5) + 6;
        target.scale = value;
    }
};

class TileWall : public Tile
{
public:
    TileWall()
    {
        wall = true;
        color = 0;
    }

    void OnPlayer(Player& target) override
    {
        
    }
};

void (*shapes[])(HDC, int, int, int, int) =
{
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

LPCWSTR messages[] =
{
    L"",
    L"CANNOT MOVE UNAUTHORIZED PLAYER",
    L"CANNOT MOVE PLAYER INTO WALL"
};
LPCWSTR current_message = messages[0];

HBRUSH brushes[] =
{
    CreateSolidBrush(0x00000000),
    CreateSolidBrush(0x00808080),
    CreateSolidBrush(0x00C0C0C0),
    CreateSolidBrush(0x00FFFFFF),
    CreateSolidBrush(0x00FF0000),
    CreateSolidBrush(0x00FFFF00),
    CreateSolidBrush(0x0000FF00),
    CreateSolidBrush(0x0000FFFF),
    CreateSolidBrush(0x000000FF),
    CreateSolidBrush(0x00FF00FF)
};

Player players[2];
int turn = 0;

void Initialize()
{
    players[0] = Player();
    players[1] = Player();
    players[0].x = 0;
    players[0].y = 0;
    players[1].x = SIZEOF_BOARD - 1;
    players[1].y = SIZEOF_BOARD - 1;

    for (int i = 0; i < 10; ++i)
        tiles[uid(dre) % SIZEOF_BOARD][uid(dre) % SIZEOF_BOARD] = new TileWall();
    for (int i = 0; i < 10; ++i)
        tiles[uid(dre) % SIZEOF_BOARD][uid(dre) % SIZEOF_BOARD] = new TileChangeColor();
    for (int i = 0; i < 10; ++i)
        tiles[uid(dre) % SIZEOF_BOARD][uid(dre) % SIZEOF_BOARD] = new TileChangeShape();
    for (int i = 0; i < 10; ++i)
        tiles[uid(dre) % SIZEOF_BOARD][uid(dre) % SIZEOF_BOARD] = new TileChangeScale();
}

void RequestMovePlayer(int index, int dx, int dy)
{
    if (turn != index)
    {
        current_message = messages[1];
        return;
    }
    if (!players[index].Move(dx, dy))
    {
        current_message = messages[2];
        return;
    }
    current_message = messages[0];
    Tile* tile = tiles[players[index].y][players[index].x];
    if (tile != nullptr)
        tile->OnPlayer(players[index]);
    turn = (turn + 1) % 2;
}

void Draw(HDC hDC)
{
    for (int i = 0; i < SIZEOF_BOARD; ++i)
    {
        for (int j = 0; j < SIZEOF_BOARD; ++j)
        {
            HBRUSH old_brush = (HBRUSH)SelectObject(hDC, brushes[tiles[i][j] != nullptr ? tiles[i][j]->color : 3]);
            Rectangle(hDC, j * CELL_SIZE, i * CELL_SIZE, (j + 1) * CELL_SIZE, (i + 1) * CELL_SIZE);
            SelectObject(hDC, old_brush);
        }
    }

    for (int i = 0; i < 2; ++i)
    {
        Player player = players[i];

        HBRUSH old_brush = (HBRUSH)SelectObject(hDC, brushes[player.color]);
        int offset = (CELL_SIZE - player.scale) / 2;
        shapes[player.shape](
            hDC,
            player.x * CELL_SIZE + offset,
            player.y * CELL_SIZE + offset,
            (player.x + 1) * CELL_SIZE - offset,
            (player.y + 1) * CELL_SIZE - offset
            );
        SelectObject(hDC, old_brush);
    }

    TextOut(hDC, 0, 0, current_message, lstrlen(current_message));
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hDC, mem_hDC;
    HBITMAP mem_bit, old_bit;
    static RECT screen_rect;

    switch (message)
    {
    case WM_CREATE:
        dre.seed(time(NULL));
        Initialize();
        break;

    case WM_SIZE:
        GetClientRect(hWnd, &screen_rect);
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_KEYDOWN:
        switch (toupper(wParam))
        {
        case 'W':
            RequestMovePlayer(0, 0, -1);
            break;
        case 'A':
            RequestMovePlayer(0, -1, 0);
            break;
        case 'S':
            RequestMovePlayer(0, 0, 1);
            break;
        case 'D':
            RequestMovePlayer(0, 1, 0);
            break;
        case 'R':
            for (int i = 0; i < SIZEOF_BOARD; ++i)
            {
                for (int j = 0; j < SIZEOF_BOARD; ++j)
                {
                    if (tiles[i][j] != nullptr)
                    {
                        delete tiles[i][j];
                        tiles[i][j] = nullptr;
                    }
                }
            }
            Initialize();
            break;
        case 'Q':
            PostQuitMessage(0);
            break;
        case VK_UP:
            RequestMovePlayer(1, 0, -1);
            break;
        case VK_LEFT:
            RequestMovePlayer(1, -1, 0);
            break;
        case VK_DOWN:
            RequestMovePlayer(1, 0, 1);
            break;
        case VK_RIGHT:
            RequestMovePlayer(1, 1, 0);
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
        Draw(mem_hDC);
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