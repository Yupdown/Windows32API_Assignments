#define _CRT_NON_CONFORMING_SWPRINTFS
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <tchar.h>
#include <time.h>
#include <string>
#include <cmath>

#include "resource.h"

HWND hWnd;
HINSTANCE g_hInst;

WCHAR szTitle[] = L"Windows32 API Example";
WCHAR szWindowClass[] = L"Windows32 API Class";

RECT screen_rect;

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL    CALLBACK    DlalogProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
    g_hInst = hInstance;

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

    hWnd = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_DIALOG2), nullptr, (DLGPROC)&DlalogProc);

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

int display_mode;
char opcode;
int operand_input;
double operand_dst;
double operand_src;
bool toggle_binary;

WCHAR operand_dst_buff[128];

void UpdateDisplay()
{
    WCHAR operand_src_buff[128];
    if (toggle_binary)
    {
        operand_src_buff[32] = 0;
        for (int i = 0, v = operand_src; i < 32; ++i)
            operand_src_buff[i] = (v << i) & (1 << 31) ? '1' : '0';
    }
    else
        swprintf(operand_src_buff, L"%.2lf", operand_src);

    if (display_mode == 1)
    {
        SetDlgItemText(hWnd, IDC_EDIT1, operand_src_buff);
        SetDlgItemText(hWnd, IDC_EDIT2, operand_dst_buff);
    }
    else
    {
        SetDlgItemText(hWnd, IDC_EDIT1, operand_src_buff);
        swprintf(operand_dst_buff, L"%.2lf%c", operand_dst, opcode);
        SetDlgItemText(hWnd, IDC_EDIT2, operand_dst_buff);
    }
}

void Clear()
{
    display_mode = 0;
    opcode = 0;
    operand_dst = 0;
    operand_src = 0;
    operand_dst_buff[0] = 0;
}

void AppendOperand(int digit)
{
    operand_src = operand_src * 10 + digit % 10;
    display_mode = 0;
}

void Operate()
{
    double result = operand_src;

    switch (opcode)
    {
    case '+':
        result = operand_dst + operand_src;
        break;
    case '-':
        result = operand_dst - operand_src;
        break;
    case '*':
        result = operand_dst * operand_src;
        break;
    case '/':
        result = operand_dst / operand_src;
        break;
    }

    swprintf(operand_dst_buff, L"%.2lf%c%.2lf=", operand_dst, opcode, operand_src);

    opcode = 0;
    operand_dst = 0;
    operand_src = result;
    display_mode = 1;
}

void SetOperator(char new_opcode)
{
    if (operand_src != 0)
    {
        Operate();
        operand_dst = operand_src;
    }
    opcode = new_opcode;
    operand_src = 0;
    display_mode = 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hDC, mem_hDC;
    HBITMAP mem_bit, old_bit;
    PAINTSTRUCT ps;

    switch (message)
    {
    case WM_CREATE:
        break;

    case WM_SIZE:
        GetClientRect(hWnd, &screen_rect);
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_TIMER:
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

BOOL CALLBACK DlalogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_SHOWWINDOW:
        UpdateDisplay();
        break;

    case WM_COMMAND:

        switch (LOWORD(wParam))
        {
        case IDC_EDIT1:
            if (HIWORD(wParam) == EN_CHANGE && !toggle_binary)
            {
                BOOL state;
                int value = GetDlgItemInt(hWnd, IDC_EDIT1, &state, true);
                
                if (state)
                    operand_src = value;
            }
            break;
        case IDC_BUTTON2:
            SetOperator('+');
            UpdateDisplay();
            break;
        case IDC_BUTTON6:
            AppendOperand(7);
            UpdateDisplay();
            break;
        case IDC_BUTTON7:
            AppendOperand(8);
            UpdateDisplay();
            break;
        case IDC_BUTTON8:
            AppendOperand(9);
            UpdateDisplay();
            break;
        case IDC_BUTTON9:
            SetOperator('-');
            UpdateDisplay();
            break;
        case IDC_BUTTON10:
            AppendOperand(4);
            UpdateDisplay();
            break;
        case IDC_BUTTON11:
            AppendOperand(5);
            UpdateDisplay();
            break;
        case IDC_BUTTON12:
            AppendOperand(6);
            UpdateDisplay();
            break;
        case IDC_BUTTON13:
            SetOperator('*');
            UpdateDisplay();
            break;
        case IDC_BUTTON14:
            AppendOperand(1);
            UpdateDisplay();
            break;
        case IDC_BUTTON15:
            AppendOperand(2);
            UpdateDisplay();
            break;
        case IDC_BUTTON16:
            AppendOperand(3);
            UpdateDisplay();
            break;
        case IDC_BUTTON17:
            SetOperator('/');
            UpdateDisplay();
            break;
        case IDC_BUTTON18:
            std::swap(operand_dst, operand_src);
            UpdateDisplay();
            break;
        case IDC_BUTTON19:
            AppendOperand(0);
            UpdateDisplay();
            break;
        case IDC_BUTTON20:
            operand_src = 0;
            UpdateDisplay();
            break;
        case IDC_BUTTON21:
            Operate();
            UpdateDisplay();
            break;
        case IDC_BUTTON22:
            SetOperator('/');
            operand_src = operand_dst;
            operand_dst = 1.0;
            Operate();
            UpdateDisplay();
            break;
        case IDC_BUTTON23:
            toggle_binary ^= 1;
            UpdateDisplay();
            break;
        case IDC_BUTTON24:
            Clear();
            UpdateDisplay();
            break;
        case IDC_BUTTON25:
            PostQuitMessage(0);
            break;
        case IDC_BUTTON26:
            SetOperator(0);
            operand_src = sqrt(operand_dst);
            Operate();
            UpdateDisplay();
            break;
        case IDC_BUTTON27:
            operand_dst = operand_src;
            SetOperator('*');
            operand_src = 10;
            Operate();
            UpdateDisplay();
            break;
        case IDC_BUTTON28:
            operand_src /= 10;
            UpdateDisplay();
            break;
        }
        break;

    case WM_CLOSE:
        PostQuitMessage(0);
        break;
    }
    return 0;
}