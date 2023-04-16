#include "3-1_knob.h"
#include "3-1_board.h"

Board::Board()
{

}

void Board::AddKnob(Knob* knob)
{
    knobs.push_back(knob);
}

void Board::AddDrop(Drop* drop)
{
    drops.push_back(drop);
}

void Board::DeleteDrop(Drop* drop)
{
    drops.erase(std::find(drops.begin(), drops.end(), drop));
}

void Board::Update()
{
    for (size_t index1 = knobs.size(); index1 > 0; --index1)
    {
        Knob* knob = knobs[index1 - 1];
        if (!knob->IsHead())
            continue;

        knob->Update(*this);

        for (size_t index2 = knobs.size(); index2 > 0; --index2)
        {
            if (index1 == index2)
                continue;

            Knob* knob_other = knobs[index2 - 1];
            POINT p1 = knob_other->GetPosition();
            POINT p2 = knob->GetPosition();
            if (p1.x == p2.x && p1.y == p2.y)
                knob->OnCollisionKnob(*knob_other, *this);
        }

        for (size_t index2 = drops.size(); index2 > 0; --index2)
        {
            Drop* drop = drops[index2 - 1];
            POINT p1 = drop->GetPosition();
            POINT p2 = knob->GetPosition();
            if (p1.x == p2.x && p1.y == p2.y)
                knob->OnCollisionDrop(*drop, *this);
        }
    }
}

void Board::Draw(HDC hDC) const
{
    int cell_size = BOARD_SIZE / BOARD_LENGTH;
    for (int i = 0; i < BOARD_LENGTH; ++i)
    {
        for (int j = 0; j < BOARD_LENGTH; ++j)
            Rectangle(hDC, j * cell_size, i * cell_size, (j + 1) * cell_size, (i + 1) * cell_size);
    }

    for (const Drop* drop : drops)
        drop->Draw(hDC, *this);
    for (const Knob* knob : knobs)
        knob->Draw(hDC, *this);
}

RECT Board::GetCellRect(const POINT& position) const
{
    int cell_size = BOARD_SIZE / BOARD_LENGTH;
    return RECT{ position.x * cell_size, position.y * cell_size, (position.x + 1) * cell_size, (position.y + 1) * cell_size };
}