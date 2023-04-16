#pragma once

#include <Windows.h>
#include <vector>

#define BOARD_SIZE 1080

class Knob;
class Drop;

class Board
{
private:
    std::vector<Knob*> knobs;
    std::vector<Drop*> drops;

public:
    static const int BOARD_LENGTH = 20;

public:
    Board();
    void AddKnob(Knob* knob);
    void AddDrop(Drop* drop);
    void DeleteDrop(Drop* drop);
    void Update();
    void Draw(HDC hDC) const;
    RECT GetCellRect(const POINT& position) const;
};