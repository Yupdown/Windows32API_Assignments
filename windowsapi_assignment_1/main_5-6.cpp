#include <windows.h>		//--- 윈도우 헤더 파일
#include <tchar.h>
#include <memory.h>

#include <random>
#include <vector>

int constexpr BOARD_WIDTH = 20;
int constexpr CELL_SIZE = 40;

std::default_random_engine dre;
std::uniform_int_distribution<int> uid;

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"Window Class Name";
LPCTSTR lpszWindowName = L"2023 Window Programming Exam 2022180003_김도엽";

POINT direction[] =
{
	{10, 0},
	{0, 10},
	{-10, 0},
	{0, -10}
};

HBITMAP bitmaps[4];
RECT screen_rect;

struct Enemy
{
	bool state;
	POINT pos;
	int direction;
	unsigned long d_time;
};

struct Player
{
	POINT pos;
	int direction;
};

struct Bullet
{
	POINT pos;
	POINT move_dir;
};

bool obstacle[BOARD_WIDTH][BOARD_WIDTH];
SIZE player_size = { 96, 108 };

Player player;
std::vector<Enemy> enemies;
std::vector<Bullet> bullets;

bool dragging;
POINT drag_cellpos = { -1, -1 };

unsigned long e_time;

void DrawPolygon(HDC hDC, int left, int top, int right, int bottom, HBRUSH brush, BOOL(_stdcall* callback)(HDC, int, int, int, int))
{
	HBRUSH old_brush = NULL;
	if (brush != NULL)
		old_brush = (HBRUSH)SelectObject(hDC, brush);
	callback(hDC, left, top, right, bottom);
	if (brush != NULL)
		SelectObject(hDC, old_brush);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

int  WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_  LPSTR lpszCmdParam, _In_  int nCmdShow)
{
	HWND hWnd;
	MSG Message;
	WNDCLASSEX WndClass;
	g_hInst = hInstance;

	WndClass.cbSize = sizeof(WndClass);
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = (WNDPROC)WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = hInstance;
	WndClass.hIcon = LoadIcon(NULL, IDI_QUESTION);
	WndClass.hCursor = LoadCursor(NULL, IDI_APPLICATION);
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = lpszClass;
	WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	RegisterClassEx(&WndClass);

	//--- 크기 변경 가능 (기존 (1024, 768))
	hWnd = CreateWindow(lpszClass, lpszWindowName, WS_OVERLAPPEDWINDOW, 0, 0, CELL_SIZE * BOARD_WIDTH + 16, CELL_SIZE * BOARD_WIDTH + 39, NULL, (HMENU)NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	while (GetMessage(&Message, 0, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	return Message.wParam;
}

POINT WorldToCell(const POINT& p)
{
	return { p.x / CELL_SIZE, p.y / CELL_SIZE };
}

POINT CellToWorld(const POINT& p)
{
	return { p.x * CELL_SIZE, p.y * CELL_SIZE };
}

RECT CellToWorld(const RECT& r)
{
	return { r.left * CELL_SIZE, r.top * CELL_SIZE, r.right * CELL_SIZE, r.bottom * CELL_SIZE };
}

void Initialize(HWND hWnd)
{
	player.direction = 0;
	player.pos = { 0, 0 };

	enemies.clear();
	bullets.clear();

	for (int c = 0; c < 5; ++c)
	{
		int v0 = uid(dre) % (screen_rect.right - 64);
		for (int i = 0; i < 3; ++i)
		{
			int v1 = (i + 1) * 50 + (c + 1) * 100;
			int v2 = v0;

			Enemy e;
			e.pos.x = c & 1 ? v1 : v2;
			e.pos.y = c & 1 ? v2 : v1;
			e.direction = (c + 1) % 4;
			e.state = false;

			enemies.push_back(e);
		}
	}

	memset(obstacle, 0, sizeof(obstacle));
}

void DrawScreen(HDC hDC)
{
	HDC bitmap_dc = CreateCompatibleDC(hDC);
	SelectObject(bitmap_dc, bitmaps[3]);

	RECT r;
	for (int x = 0; x < BOARD_WIDTH; ++x)
	{
		for (int y = 0; y < BOARD_WIDTH; ++y)
		{
			r = CellToWorld(RECT{ x, y, x + 1, y + 1 });
			DrawPolygon(hDC, r.left, r.top, r.right + 1, r.bottom + 1, NULL, Rectangle);

			if (obstacle[x][y])
				StretchBlt(hDC, r.left, r.top, r.right - r.left, r.bottom - r.top, bitmap_dc, 0, 0, 16, 16, SRCCOPY);
		}
	}
	r = CellToWorld(RECT{ drag_cellpos.x, drag_cellpos.y, drag_cellpos.x + 1, drag_cellpos.y + 1 });
	StretchBlt(hDC, r.left, r.top, r.right - r.left, r.bottom - r.top, bitmap_dc, 0, 0, 16, 16, SRCCOPY);

	for (Enemy& e : enemies)
	{
		if (e.state)
		{
			int ax = (e_time - e.d_time) / 2;
			if (ax < 26)
			{
				SelectObject(bitmap_dc, bitmaps[2]);
				TransparentBlt(hDC, e.pos.x, e.pos.y - 66, 84, 132, bitmap_dc, 42 * ax, 0, 42, 66, 0x00FF00FF);
			}
		}
		else
		{
			SelectObject(bitmap_dc, bitmaps[1]);
			TransparentBlt(hDC, e.pos.x, e.pos.y, 64, 64, bitmap_dc, 32 * (e_time / 2 % 8), e.direction / 2 * 32, 32, 32, 0x00FF00FF);
		}
	}

	SelectObject(bitmap_dc, bitmaps[0]);
	TransparentBlt(hDC, player.pos.x, player.pos.y, player_size.cx, player_size.cy, bitmap_dc, 32 * (e_time / 2 % 10), (5 - player.direction) % 4 * 36, 32, 36, 0x00FF00FF);

	for (const auto& b : bullets)
		DrawPolygon(hDC, b.pos.x - 8, b.pos.y - 8, b.pos.x + 8, b.pos.y + 8, NULL, Ellipse);

	DeleteDC(bitmap_dc);
}

void ShootPlayer()
{
	Bullet new_bullet;
	new_bullet.pos = player.pos;
	new_bullet.pos.x += player_size.cx / 2;
	new_bullet.pos.y += player_size.cy / 2;
	new_bullet.move_dir.x = direction[player.direction].x * 2;
	new_bullet.move_dir.y = direction[player.direction].y * 2;

	bullets.push_back(new_bullet);
}

void MovePlayer()
{
	int nx = player.pos.x + direction[player.direction].x;
	int ny = player.pos.y + direction[player.direction].y;

	bool flag = false;
	if (nx < 0)
	{
		nx = 0;
		flag |= true;
	}
	if (nx + player_size.cx > screen_rect.right)
	{
		nx = screen_rect.right - player_size.cx;
		flag |= true;
	}
	if (ny < 0)
	{
		ny = 0;
		flag |= true;
	}
	if (ny + player_size.cy > screen_rect.bottom)
	{
		ny = screen_rect.bottom - player_size.cy;
		flag |= true;
	}

	if (flag)
		player.direction = (player.direction + 1) % 4;

	POINT cp = WorldToCell({ nx + player_size.cx / 2, ny + player_size.cy });
	if (obstacle[cp.x][cp.y])
	{
		nx = player.pos.x;
		ny = player.pos.y;
		player.direction = (player.direction + 2) % 4;
	}

	player.pos.x = nx;
	player.pos.y = ny;
}

void MoveEnemies()
{
	for (Enemy& e : enemies)
	{
		if (e.state)
			continue;

		int nx = e.pos.x + direction[e.direction].x;
		int ny = e.pos.y + direction[e.direction].y;

		bool flag = false;
		if (nx < 0)
		{
			nx = 0;
			flag |= true;
		}
		if (nx + 64 > screen_rect.right)
		{
			nx = screen_rect.right - 64;
			flag |= true;
		}
		if (ny < 0)
		{
			ny = 0;
			flag |= true;
		}
		if (ny + 64 > screen_rect.bottom)
		{
			ny = screen_rect.bottom - 64;
			flag |= true;
		}

		if (flag)
			e.direction = (e.direction + 2) % 4;

		POINT cp = WorldToCell({ nx + 32, ny + 64 });
		if (obstacle[cp.x][cp.y])
		{
			nx = e.pos.x;
			ny = e.pos.y;
			e.direction = (e.direction + 2) % 4;
		}

		e.pos.x = nx;
		e.pos.y = ny;
	}
}

void SetObstacle(int x, int y)
{
	POINT p = POINT{ x, y };
	p = WorldToCell(p);
	obstacle[p.x][p.y] = true;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HBITMAP bitmap, old_bitmap;
	HDC	hDC, mem_hDC;

	static bool lbutton_down = false;
	static bool rbutton_down = false;

	switch (iMessage) {

	case WM_CREATE:
		bitmaps[0] = (HBITMAP)LoadImage(g_hInst, L"Isaac.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		bitmaps[1] = (HBITMAP)LoadImage(g_hInst, L"Mob.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		bitmaps[2] = (HBITMAP)LoadImage(g_hInst, L"Explosion.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		bitmaps[3] = (HBITMAP)LoadImage(g_hInst, L"Tile.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);

		Initialize(hWnd);
		SetTimer(hWnd, 1, 0, NULL);
		break;

	case WM_SIZE:
		GetClientRect(hWnd, &screen_rect);
		InvalidateRect(hWnd, NULL, false);
		break;

	case WM_MOUSEMOVE:
		if (lbutton_down)
			SetObstacle(LOWORD(lParam), HIWORD(lParam));
		if (rbutton_down && dragging)
			drag_cellpos = WorldToCell({ LOWORD(lParam), HIWORD(lParam) });
		break;

	case WM_LBUTTONDOWN:
		if (lbutton_down)
			break;
		SetObstacle(LOWORD(lParam), HIWORD(lParam));
		lbutton_down = true;
		break;

	case WM_LBUTTONUP:
		if (!lbutton_down)
			break;
		lbutton_down = false;
		break;

	case WM_RBUTTONDOWN:
		if (rbutton_down)
			break;

		POINT cp = WorldToCell({ LOWORD(lParam), HIWORD(lParam) });
		if (obstacle[cp.x][cp.y])
		{
			obstacle[cp.x][cp.y] = false;
			dragging = true;
			drag_cellpos = cp;
		}
		rbutton_down = true;
		break;

	case WM_RBUTTONUP:
		if (!rbutton_down)
			break;
		if (dragging)
		{
			obstacle[drag_cellpos.x][drag_cellpos.y] = true;
			dragging = false;
		}
		rbutton_down = false;
		break;
	case WM_CHAR:
		switch (towupper(wParam))
		{
		case L'S':
			Initialize(hWnd);
			break;
		case 'Q':
			PostQuitMessage(0);
			break;
		}
		InvalidateRect(hWnd, NULL, false);
		break;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_UP:
			player.direction = 3;
			break;
		case VK_LEFT:
			player.direction = 2;
			break;
		case VK_DOWN:
			player.direction = 1;
			break;
		case VK_RIGHT:
			player.direction = 0;
			break;
		case VK_SPACE:
			ShootPlayer();
			break;
		}
		break;

	case WM_TIMER:
		e_time += 1;

		MovePlayer();
		MoveEnemies();

		for (auto& b : bullets)
		{
			b.pos.x += b.move_dir.x;
			b.pos.y += b.move_dir.y;

			POINT c = WorldToCell(b.pos);
			if (c.x < 0 || c.x >= BOARD_WIDTH || c.y < 0 || c.y >= BOARD_WIDTH)
				continue;
			if (obstacle[c.x][c.y])
			{
				obstacle[c.x][c.y] = false;
				b.pos = { -100, -100 };
			}
			for (auto& e : enemies)
			{
				if (e.state)
					continue;

				if (b.pos.x > e.pos.x && b.pos.x < e.pos.x + 64 && b.pos.y > e.pos.y && b.pos.y < e.pos.y + 64)
				{
					e.state = true;
					e.d_time = e_time;
					b.pos = { -100, -100 };
					break;
				}
			}
		}
		InvalidateRect(hWnd, NULL, false);
		break;

	case WM_PAINT:
		hDC = BeginPaint(hWnd, &ps);
		mem_hDC = CreateCompatibleDC(hDC);

		bitmap = CreateCompatibleBitmap(hDC, BOARD_WIDTH * CELL_SIZE, BOARD_WIDTH * CELL_SIZE);
		old_bitmap = (HBITMAP)SelectObject(mem_hDC, bitmap);

		PatBlt(mem_hDC, 0, 0, BOARD_WIDTH * CELL_SIZE, BOARD_WIDTH * CELL_SIZE, WHITENESS);
		DrawScreen(mem_hDC);

		BitBlt(hDC, 0, 0, BOARD_WIDTH * CELL_SIZE, BOARD_WIDTH * CELL_SIZE, mem_hDC, 0, 0, SRCCOPY);

		SelectObject(mem_hDC, old_bitmap);

		DeleteObject(bitmap);
		DeleteDC(mem_hDC);

		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return (DefWindowProc(hWnd, iMessage, wParam, lParam));
}
