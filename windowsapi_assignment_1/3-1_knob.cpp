#include "3-1_knob.h"
#include "winapiutil.h"

MoveBehaviourFollow::MoveBehaviourFollow(Knob* _target)
{
    target = _target;
    target_lastposition = target->GetPosition();
}

void MoveBehaviourFollow::Move(POINT& position, const Board& board)
{
    target_lastposition = target->GetPosition();
    position = target_lastposition;
}

Knob::Knob(POINT _pos, COLORREF _col, bool _player)
{
    position.x = _pos.x;
    position.y = _pos.y;
    fill_color = CreateSolidBrush(_col);

    move_behaviour = new MoveBehaviourT1(false, false);
    front_knob = nullptr;
    back_knob = nullptr;
    player = _player;
}

Knob::~Knob()
{
    if (move_behaviour != nullptr)
        delete move_behaviour;
}

void Knob::Update(const Board& board)
{
    if (back_knob != nullptr)
    {
        back_knob->Update(board);
        back_knob->position = position;
    }

    if (IsHead() && move_behaviour != nullptr)
        move_behaviour->Move(position, board);
}

void Knob::Draw(HDC hDC, const Board& board) const
{
    RECT r = board.GetCellRect(position);
    DrawPolygon(Ellipse, hDC, r.left, r.top, r.right, r.bottom, NULL, fill_color);
}

bool Knob::IsHead() const
{
    return front_knob == nullptr;
}

Knob* Knob::Head()
{
    return (front_knob != nullptr ? front_knob->Head() : this);
}

POINT Knob::GetPosition() const
{
    return position;
}

void Knob::ChangeMoveBehaviour(MoveBehaviour* new_behaviour)
{
    if (move_behaviour != nullptr)
        delete move_behaviour;
    move_behaviour = new_behaviour;
}

void Knob::OnCollisionKnob(Knob& other, Board& board)
{
    if (front_knob != nullptr)
        return;

    if (!other.Head()->player)
        ConnectKnob(other);
}

void Knob::OnCollisionDrop(Drop& other, Board& board)
{
    static int round_robin = 0;

    if (!player)
        return;

    board.DeleteDrop(&other);
    Knob* new_knob = new Knob(other.GetPosition(), other.GetColor(), false);

    switch (round_robin++ % 4)
    {
    case 0:
        new_knob->ChangeMoveBehaviour(new MoveBehaviourT1(false, false));
        break;
    case 1:
        new_knob->ChangeMoveBehaviour(new MoveBehaviourT2(false, false));
        break;
    case 2:
        new_knob->ChangeMoveBehaviour(new MoveBehaviourT3());
        break;
    }

    board.AddKnob(new_knob);
}

bool Knob::ConnectKnob(Knob& other)
{
    Knob* head = other.Head();
    if (Head() == head)
        return false;

    Knob* tail = this;
    while (tail->back_knob != nullptr)
        tail = tail->back_knob;

    tail->back_knob = head;
    head->front_knob = tail;
    head->ChangeMoveBehaviour(new MoveBehaviourFollow(tail));

    return true;
}

Drop::Drop(POINT pos, COLORREF col)
{
    position = pos;
    color = col;
    fill_color = CreateSolidBrush(col);
}

void Drop::Draw(HDC hDC, const Board& board) const
{
    RECT r = board.GetCellRect(position);
    DrawPolygon(Rectangle, hDC, r.left + 10, r.top + 10, r.right - 10, r.bottom - 10, NULL, fill_color);
}

POINT Drop::GetPosition() const
{
    return position;
}

COLORREF Drop::GetColor() const
{
    return color;
}
