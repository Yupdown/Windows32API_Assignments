#include "3-4_board.h"
#include "winapiutil.h"

#include <random>

void Board::Initialize()
{
    current_rectris = nullptr;
    ResetRectris();
}

void Board::Draw(HDC hDC)
{
    for (int i = 0; i < BOARD_COLUMNS; ++i)
    {
        for (int j = 0; j < BOARD_ROWS; ++j)
        {
            RECT r = GetCellRect(POINT{ i, j });
            DrawPolygon(Rectangle, hDC, r.left, r.top, r.right, r.bottom, NULL, color_brushes[tile_map[i][j].color_index]);
        }
    }

    if (current_rectris != nullptr)
        current_rectris->Draw(hDC, *this);
}

RECT Board::GetCellRect(const POINT& position) const
{
    return RECT{
        position.x * CELL_SIZE,
        (BOARD_ROWS - 1 - position.y) * CELL_SIZE,
        (position.x + 1) * CELL_SIZE,
        (BOARD_ROWS - position.y) * CELL_SIZE
    };
}

void Board::ResetRectris()
{
    static std::default_random_engine dre;
    static std::uniform_int_distribution uid;

    if (current_rectris != nullptr)
        delete current_rectris;
    current_rectris = new Rectris(POINT{ uid(dre) % (BOARD_COLUMNS - Rectris::MAX_SIZE), BOARD_ROWS - 2 }, uid(dre) % 6, uid(dre) % 4);
}

void Board::MoveRectris(const POINT& delta)
{
    if (current_rectris == nullptr)
        return;

    bool flag = current_rectris->Move(delta, *this);
    if (!flag)
    {
        ResetRectris();
        SimulateTileMap();
    }
}

void Board::RotateRectris()
{
    if (current_rectris == nullptr)
        return;
    current_rectris->Rotate(*this);
}

void Board::SetTile(const POINT& position, Tile new_tile)
{
    tile_map[position.x][position.y] = new_tile;
}

Tile Board::TileMap(const POINT& position) const
{
    return tile_map[position.x][position.y];
}

void Board::SimulateTileMap()
{
    for (int x = 0; x < BOARD_COLUMNS; ++x)
    {
        for (int y = 0, fy = 0; y < BOARD_ROWS; ++y)
        {
            if (tile_map[x][y].is_fill)
            {
                Tile temp = tile_map[x][y];
                tile_map[x][y].is_fill = false;
                tile_map[x][y].color_index = -1;
                tile_map[x][fy++] = temp;
            }
        }
    }

    bool flag = false;
    for (int x = 0; x < BOARD_COLUMNS; ++x)
    {
        for (int y = 1, c = 1; y <= BOARD_ROWS; ++y)
        {
            if (y != BOARD_ROWS && tile_map[x][y].color_index == tile_map[x][y - 1].color_index)
                c++;
            else
            {
                if (c >= 3 && tile_map[x][y - 1].is_fill)
                {
                    flag |= true;
                    for (int k = 0; k < c; ++k)
                    {
                        tile_map[x][y - k - 1].is_fill = false;
                        tile_map[x][y - k - 1].color_index = -1;
                    }
                }
                c = 1;
            }
        }
    }
    for (int y = 0; y < BOARD_ROWS; ++y)
    {
        for (int x = 1, c = 1; x <= BOARD_COLUMNS; ++x)
        {
            if (x != BOARD_COLUMNS && tile_map[x][y].color_index == tile_map[x - 1][y].color_index)
                c++;
            else
            {
                if (c >= 3 && tile_map[x - 1][y].is_fill)
                {
                    flag |= true;
                    for (int k = 0; k < c; ++k)
                    {
                        tile_map[x - k - 1][y].is_fill = false;
                        tile_map[x - k - 1][y].color_index = -1;
                    }
                }
                c = 1;
            }
        }
    }

    if (flag)
        SimulateTileMap();
}
