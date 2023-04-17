#pragma once

#include <Windows.h>

const HBRUSH color_brushes[] =
{
    CreateSolidBrush(0x001A1ACC),
    CreateSolidBrush(0x001ACC8C),
    CreateSolidBrush(0x00E86910),
    CreateSolidBrush(0x00E82390)
};

class Tile
{
public:
    bool is_fill;
    int color_index;

public:
    Tile()
    {
        is_fill = false;
        color_index = -1;
    }
};