#pragma once

#include <Windows.h>
#include "3-1_board.h"

class MoveBehaviour
{
public:
    virtual void Move(POINT& position, const Board& board) = 0;
};

class MoveBehaviourT1 : public MoveBehaviour
{
private:
    bool state1;
    bool state2;

public:
    MoveBehaviourT1(bool left, bool up)
    {
        state1 = left;
        state2 = up;
    }

    void Move(POINT& position, const Board& board) override
    {
        int nx = position.x + (state1 ? -1 : 1);
        if (board.ObstructTile(POINT{ nx, position.y }))
        {
            int ny = position.y + (state2 ? -1 : 1);
            if (board.ObstructTile(POINT{ position.x, ny }))
            {
                ny = position.y + (state2 ? 1 : -1);
                state2 = !state2;
            }
            position.y = ny;
            state1 = !state1;
        }
        else
            position.x = nx;
    }
};

class MoveBehaviourT2 : public MoveBehaviour
{
private:
    bool state1;
    bool state2;

public:
    MoveBehaviourT2(bool up, bool left)
    {
        state1 = up;
        state2 = left;
    }

    void Move(POINT& position, const Board& board) override
    {
        int ny = position.y + (state1 ? -1 : 1);
        if (board.ObstructTile(POINT{ position.x, ny }))
        {
            int nx = position.x + (state2 ? -1 : 1);
            if (board.ObstructTile(POINT{ nx, position.y }))
            {
                nx = position.x + (state2 ? 1 : -1);
                state2 = !state2;
            }
            position.x = nx;
            state1 = !state1;
        }
        else
            position.y = ny;
    }
};

class MoveBehaviourT3 : public MoveBehaviour
{
private:
    bool state1;
    bool state2;

public:
    void Move(POINT& position, const Board& board) override
    {
        int nx = position.x + (state1 ? -1 : 1);
        int ny = position.y + (state2 ? -1 : 1);

        if (board.ObstructTile(POINT{ nx, ny }))
        {
            bool c1 = board.ObstructTile(POINT{ nx, position.y });
            bool c2 = board.ObstructTile(POINT{ position.x, ny });
            if (c1 == c2 || c1)
            {
                nx = position.x + (state1 ? 1 : -1);
                state1 = !state1;
            }
            if (c1 == c2 || c2)
            {
                ny = position.y + (state2 ? 1 : -1);
                state2 = !state2;
            }
        }

        position.x = nx;
        position.y = ny;
    }
};

class MoveBehaviourTarget : public MoveBehaviour
{
private:
    POINT target;
    Knob* player;
    MoveBehaviour* last_behaviour;

public:
    MoveBehaviourTarget(POINT pos, Knob* p, MoveBehaviour* behaviour);
    void Move(POINT& position, const Board& board) override;
};

class Drop
{
private:
    POINT position;
    COLORREF color;
    HBRUSH fill_color;

public:
    Drop(POINT pos, COLORREF col);
    void Draw(HDC hDC, const Board& board) const;
    POINT GetPosition() const;
    COLORREF GetColor() const;
};

class Knob
{
private:
    POINT position;
    HBRUSH fill_color;
    MoveBehaviour* move_behaviour;
    Knob* front_knob;
    Knob* back_knob;
    bool player;
    int effect_timer;

public:
    Knob(POINT _pos, COLORREF _col, bool _player);
    ~Knob();
    void Update(const Board& board);
    void Draw(HDC hDC, const Board& board) const;
    bool IsHead() const;
    Knob* Head();
    Knob* Tail();
    POINT GetPosition() const;
    void ChangeMoveBehaviour(MoveBehaviour* new_behaviour);
    void OnCollisionKnob(Knob& other, Board& board);
    void OnCollisionDrop(Drop& other, Board& board);
    bool ConnectKnob(Knob& other);
    void Detach();
    void OnMouse(int state);
    void SwapHeadTail();
    void Jump();
    void MoveToTarget(POINT target);
};