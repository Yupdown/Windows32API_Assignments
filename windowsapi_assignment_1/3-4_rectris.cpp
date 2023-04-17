#include "3-4_rectris.h"
#include "winapiutil.h"

Rectris::Rectris(POINT pos, int type, int color_index)
{
	position = pos;
	rotation = type % 2;

	for (int x = 0; x < MAX_SIZE; ++x)
	{
		for (int y = 0; y < MAX_SIZE; ++y)
		{
			if (RECTRIS_SHAPES[type / 2][x][y] == '1')
			{
				tile_map[x][y].is_fill = true;
				tile_map[x][y].color_index = (x + y + color_index) % 4;
			}
			else
			{
				tile_map[x][y].is_fill = false;
				tile_map[x][y].color_index = -1;
			}
		}
	}
}

void Rectris::Draw(HDC hDC, const Board& board)
{
	for (int x = 0; x < MAX_SIZE; ++x)
	{
		for (int y = 0; y < MAX_SIZE; ++y)
		{
			if (!tile_map[x][y].is_fill)
				continue;

			POINT p = LocalToWorld(POINT{ x, y });
			RECT r = board.GetCellRect(p);
			DrawPolygon(Rectangle, hDC, r.left, r.top, r.right, r.bottom, NULL, color_brushes[tile_map[x][y].color_index]);
		}
	}
}

void Rectris::Rotate(Board& board)
{
	POINT old_position = position;
	int old_rotation = rotation;
	rotation = (rotation + 1) % 4;

	if (!Collision(board))
		return;

	POINT p[] =
	{
		position.x - 1, position.y,
		position.x + 1, position.y,
		position.x, position.y - 1,
		position.x, position.y + 1,
	};

	for (int i = 0; i < 4; ++i)
	{
		position = p[i];
		if (!Collision(board))
			return;
	}

	position = old_position;
	rotation = old_rotation;
}

bool Rectris::Move(const POINT& delta, Board& board)
{
	POINT old_position = position;
	position.x += delta.x;
	position.y += delta.y;

	if (Collision(board))
	{
		position = old_position;
		if (delta.y < 0)
		{
			ApplyToBoard(board);
			return false;
		}
	}
	return true;
}

bool Rectris::Collision(Board& board)
{
	for (int x = 0; x < MAX_SIZE; ++x)
	{
		for (int y = 0; y < MAX_SIZE; ++y)
		{
			if (!tile_map[x][y].is_fill)
				continue;

			POINT p = LocalToWorld(POINT{ x, y });

			if (p.x < 0 || p.y < 0 || p.x >= Board::BOARD_COLUMNS)
				return true;
			if (p.y >= Board::BOARD_ROWS)
				continue;
			if (board.TileMap(p).is_fill)
				return true;
		}
	}
	return false;
}

void Rectris::ApplyToBoard(Board& board)
{
	for (int x = 0; x < MAX_SIZE; ++x)
	{
		for (int y = 0; y < MAX_SIZE; ++y)
		{
			if (!tile_map[x][y].is_fill)
				continue;

			POINT p = LocalToWorld(POINT{ x, y });
			board.SetTile(p, tile_map[x][y]);
		}
	}
}

POINT Rectris::LocalToWorld(const POINT& local)
{
	int nx = position.x;
	int ny = position.y;

	switch (rotation)
	{
	case 0:
		nx += local.x;
		ny += local.y;
		break;
	case 1:
		nx += local.y;
		ny += local.x;
		break;
	case 2:
		nx += MAX_SIZE - 1 - local.x;
		ny += local.y;
		break;
	case 3:
		nx += local.y;
		ny += MAX_SIZE - 1 - local.x;
		break;
	}

	return POINT{ nx, ny };
}