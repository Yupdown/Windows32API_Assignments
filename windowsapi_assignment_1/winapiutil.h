#pragma once

#include <Windows.h>

inline void DrawPolygon(BOOL(*call_polygon)(HDC, int, int, int, int), HDC hdc, int left, int top, int right, int bottom, HPEN pen_border, HBRUSH brush_fill)
{
	HBRUSH brush_old = NULL;
	HPEN pen_old = NULL;

	if (brush_fill != NULL)
		brush_old = (HBRUSH)SelectObject(hdc, brush_fill);
	if (pen_border != NULL)
		pen_old = (HPEN)SelectObject(hdc, pen_border);

	call_polygon(hdc, left, top, right, bottom);

	if (brush_fill != NULL)
		SelectObject(hdc, brush_old);
	if (pen_border != NULL)
		SelectObject(hdc, pen_old);
}

inline void DrawPolygon(BOOL(*call_polygon)(HDC, int, int, int, int), HDC hdc, int left, int top, int right, int bottom, int border_width, COLORREF border_color, COLORREF fill_color)
{
	HBRUSH brush_fill = CreateSolidBrush(fill_color);
	HPEN pen_border = CreatePen(PS_SOLID, border_width, border_color);

	DrawPolygon(call_polygon, hdc, left, top, right, bottom, pen_border, brush_fill);

	DeleteObject(brush_fill);
	DeleteObject(pen_border);
}

inline bool ProcessAABBCollision(const RECT& lhs, const RECT& rhs, POINT& position, POINT& velocity)
{
	RECT temp;
	if (!IntersectRect(&temp, &lhs, &rhs))
		return false;

	LONG dx = temp.right - temp.left;
	LONG dy = temp.bottom - temp.top;

	if (dx < dy)
	{
		velocity.x = -velocity.x;
		position.x += dx * (velocity.x / abs(velocity.x));
	}
	else
	{
		velocity.y = -velocity.y;
		position.y += dy * (velocity.y / abs(velocity.y));
	}
	return true;
}