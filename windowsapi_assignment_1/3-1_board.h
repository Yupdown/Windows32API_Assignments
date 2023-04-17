#pragma once

#include <Windows.h>
#include <vector>

#define BOARD_SIZE 960

class Knob;
class Drop;

class Board
{
public:
    static const int BOARD_LENGTH = 40;

private:
    Knob* player;
    std::vector<Knob*> knobs;
    std::vector<Drop*> drops;
    POINT highlight_point;
    bool obstacle[BOARD_LENGTH][BOARD_LENGTH];
    HBRUSH obstacle_color_fill;
    HBRUSH highlight_fill;

public:
    Board();
    void SetPlayer(Knob* knob);
    void AddKnob(Knob* knob);
    void AddDrop(Drop* drop);
    void DeleteDrop(Drop* drop);
    void Update();
    void Draw(HDC hDC) const;
    RECT GetCellRect(const POINT& position) const;
    bool ObstructTile(const POINT& position) const;
    void HandleMouse(LONG x, LONG y, int mouse_state);
};