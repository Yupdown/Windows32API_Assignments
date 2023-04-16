#include <Windows.h>
#include <tchar.h>

#include <vector>
#include <random>

#include "3-1_knob.h"
#include "3-1_board.h"

std::default_random_engine dre;
std::uniform_int_distribution<int> uid;

WCHAR szTitle[] = L"Windows32 API Example";
WCHAR szWindowClass[] = L"Windows32 API Class";

RECT screen_rect;

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void    CALLBACK    TimerProc(HWND, UINT, UINT_PTR, DWORD);

Knob* player_knob;
Board board;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
    WNDCLASSEXW wClass;
    wClass.cbSize = sizeof(WNDCLASSEX);
    wClass.style = CS_HREDRAW | CS_VREDRAW;
    wClass.lpfnWndProc = WndProc;
    wClass.cbClsExtra = 0;
    wClass.cbWndExtra = 0;
    wClass.hInstance = hInstance;
    wClass.hIcon = LoadIcon(NULL, IDI_INFORMATION);
    wClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wClass.lpszMenuName = NULL;
    wClass.lpszClassName = szWindowClass;
    wClass.hIconSm = LoadIcon(NULL, IDI_INFORMATION);
    RegisterClassEx(&wClass);

    HWND hWnd = CreateWindow(
        szWindowClass, szTitle,
        WS_OVERLAPPEDWINDOW,
        0, 0, BOARD_SIZE + 16, BOARD_SIZE + 39,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG message;
    while (GetMessage(&message, NULL, 0, 0))
    {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }

    return (int)message.wParam;
}

void Initialize()
{
    player_knob = new Knob(POINT{ 0, 0 }, 0x00FFFFFF, true);
    board.AddKnob(player_knob);

    for (int i = 0; i < 20; ++i)
    {
        Drop* drop = new Drop(POINT{uid(dre) % Board::BOARD_LENGTH, uid(dre) % Board::BOARD_LENGTH }, uid(dre) % 0x01000000);
        board.AddDrop(drop);
    }
}

void PaintScreen(HDC hDC)
{
    board.Draw(hDC);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hDC, mem_hDC;
    HBITMAP mem_bit, old_bit;
    PAINTSTRUCT ps;

    switch (message)
    {
    case WM_CREATE:
        Initialize();
        break;

    case WM_CHAR:
        switch (towupper(wParam))
        {
        case L'S':
            SetTimer(hWnd, 1, 50, TimerProc);
            break;
        case L'+':
            break;
        case L'-':
            break;
        case L'J':
            break;
        case L'T':
            break;
        case L'Q':
            PostQuitMessage(0);
            break;
        }
        break;

    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_UP:
            player_knob->ChangeMoveBehaviour(new MoveBehaviourT2(true, false));
            break;
        case VK_LEFT:
            player_knob->ChangeMoveBehaviour(new MoveBehaviourT1(true, false));
            break;
        case VK_DOWN:
            player_knob->ChangeMoveBehaviour(new MoveBehaviourT2(false, false));
            break;
        case VK_RIGHT:
            player_knob->ChangeMoveBehaviour(new MoveBehaviourT1(false, false));
            break;
        }
        break;

    case WM_SIZE:
        GetClientRect(hWnd, &screen_rect);
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_PAINT:
        hDC = BeginPaint(hWnd, &ps);
        mem_hDC = CreateCompatibleDC(hDC);

        mem_bit = CreateCompatibleBitmap(hDC, screen_rect.right, screen_rect.bottom);
        old_bit = (HBITMAP)SelectObject(mem_hDC, mem_bit);

        PatBlt(mem_hDC, 0, 0, screen_rect.right, screen_rect.bottom, WHITENESS);
        PaintScreen(mem_hDC);
        BitBlt(hDC, 0, 0, screen_rect.right, screen_rect.bottom, mem_hDC, 0, 0, SRCCOPY);

        SelectObject(mem_hDC, old_bit);
        DeleteObject(mem_bit);

        DeleteDC(mem_hDC);
        EndPaint(hWnd, &ps);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

void CALLBACK TimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    board.Update();
    InvalidateRect(hWnd, NULL, false);
}