#pragma once

#include <Windows.h>
#include "3-4_board.h"
#include "3-4_tile.h"

class Board;
class Rectris
{
public:
    static constexpr int MAX_SIZE = 3;

private:
    Tile tile_map[MAX_SIZE][MAX_SIZE];
    POINT position;
    int rotation;

public:
    Rectris(POINT pos, int type, int color_index);
    void Draw(HDC hDC, const Board& board);
    void Rotate(Board& board);
    bool Move(const POINT& delta, Board& board);
    bool Collision(Board& board);
    void ApplyToBoard(Board& board);
    POINT LocalToWorld(const POINT& local);
};

constexpr char RECTRIS_SHAPES[][Rectris::MAX_SIZE][Rectris::MAX_SIZE + 1] =
{
    "000",
    "010",
    "000",

    "010",
    "010",
    "000",

    "010",
    "010",
    "010"
};