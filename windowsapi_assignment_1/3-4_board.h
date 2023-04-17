#pragma once

#include <Windows.h>
#include "3-4_tile.h"
#include "3-4_rectris.h"

class Rectris;
class Board
{
public:
    static constexpr int BOARD_COLUMNS = 6;
    static constexpr int BOARD_ROWS = 12;
    static constexpr int CELL_SIZE = 40;

private:
    Tile tile_map[BOARD_COLUMNS][BOARD_ROWS];
    Rectris* current_rectris;

public:
    void Initialize();
    void Draw(HDC hDC);
    RECT GetCellRect(const POINT& position) const;
    void ResetRectris();

    void MoveRectris(const POINT& delta);
    void RotateRectris();
    void SetTile(const POINT& position, Tile new_tile);
    Tile TileMap(const POINT& position) const;
    void SimulateTileMap();
};