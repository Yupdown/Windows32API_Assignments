#include "3-1_knob.h"
#include "winapiutil.h"

Knob::Knob(POINT _pos, COLORREF _col, bool _player)
{
    position.x = _pos.x;
    position.y = _pos.y;
    fill_color = CreateSolidBrush(_col);

    move_behaviour = nullptr;
    front_knob = nullptr;
    back_knob = nullptr;
    player = _player;
    effect_timer = 0;
}

Knob::~Knob()
{
    if (move_behaviour != nullptr)
        delete move_behaviour;
}

void Knob::Update(const Board& board)
{
    effect_timer -= 1;

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
    int offset = (effect_timer > 0 ? 10 : 0);
    RECT r = board.GetCellRect(position);
    DrawPolygon(Ellipse, hDC, r.left - offset, r.top - offset, r.right + offset, r.bottom + offset, NULL, fill_color);
}

bool Knob::IsHead() const
{
    return front_knob == nullptr;
}

Knob* Knob::Head()
{
    return (front_knob != nullptr ? front_knob->Head() : this);
}

Knob* Knob::Tail()
{
    return (back_knob != nullptr ? back_knob->Tail() : this);
}

POINT Knob::GetPosition() const
{
    return position;
}

void Knob::ChangeMoveBehaviour(MoveBehaviour* new_behaviour)
{
    /*if (move_behaviour != nullptr)
        delete move_behaviour;*/
    move_behaviour = new_behaviour;
}

void Knob::OnCollisionKnob(Knob& other, Board& board)
{
    if (front_knob != nullptr)
        return;

    if (!other.Head()->player)
        ConnectKnob(other);
    else
        other.ConnectKnob(*this);
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
    Knob* tail = Tail();

    tail->back_knob = head;
    head->front_knob = tail;

    return true;
}

void Knob::Detach()
{
    if (front_knob != nullptr)
        front_knob->back_knob = nullptr;
    front_knob = nullptr;
}

void Knob::OnMouse(int state)
{
    switch (state)
    {
    case 1:
        if (player)
            effect_timer = 5;
        else
            Detach();
        break;
    case 2:
        break;
    }
}

void Knob::Jump()
{
    Knob* knob = Head();
    while (knob != nullptr)
    {
        std::swap(knob->position.x, knob->position.y);
        knob = knob->back_knob;
    }
}

void Knob::SwapHeadTail()
{
    Knob* head = Head();
    Knob* tail = Tail();

    while (head != tail)
    {
        std::swap(head->position, tail->position);
        if (head->back_knob == tail)
            break;
        head = head->back_knob;
        tail = tail->front_knob;
    }
}

MoveBehaviourTarget::MoveBehaviourTarget(POINT pos, Knob* p, MoveBehaviour* behaviour)
{
    target = pos;
    player = p;
    last_behaviour = behaviour;
}

void MoveBehaviourTarget::Move(POINT& position, const Board& board)
{
    LONG dx = target.x - position.x;
    LONG dy = target.y - position.y;

    if (dx != 0)
        position.x += dx / abs(dx);
    if (dy != 0)
        position.y += dy / abs(dy);

    if (position.x == target.x && position.y == target.y)
        player->ChangeMoveBehaviour(last_behaviour);
}

void Knob::MoveToTarget(POINT target)
{
    MoveBehaviour* new_behaviour = new MoveBehaviourTarget(target, this, move_behaviour);
    ChangeMoveBehaviour(new_behaviour);
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
    DrawPolygon(Rectangle, hDC, r.left + 4, r.top + 4, r.right - 4, r.bottom - 4, NULL, fill_color);
}

POINT Drop::GetPosition() const
{
    return position;
}

COLORREF Drop::GetColor() const
{
    return color;
}
