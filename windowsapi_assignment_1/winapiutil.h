#pragma once
#include <Windows.h>

void DrawPolygon(BOOL(*call_polygon)(HDC, int, int, int, int), HDC hdc, int left, int top, int right, int bottom, HPEN pen_border, HBRUSH brush_fill)
{
	HBRUSH brush_old = (HBRUSH)SelectObject(hdc, brush_fill);
	HPEN pen_old = (HPEN)SelectObject(hdc, pen_border);

	call_polygon(hdc, left, top, right, bottom);

	SelectObject(hdc, brush_old);
	SelectObject(hdc, pen_old);
}

void DrawPolygon(BOOL(*call_polygon)(HDC, int, int, int, int), HDC hdc, int left, int top, int right, int bottom, int border_width, COLORREF border_color, COLORREF fill_color)
{
	HBRUSH brush_fill = CreateSolidBrush(fill_color);
	HPEN pen_border = CreatePen(PS_SOLID, border_width, border_color);

	DrawPolygon(call_polygon, hdc, left, top, right, bottom, pen_border, brush_fill);

	DeleteObject(brush_fill);
	DeleteObject(pen_border);
}