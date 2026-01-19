// gui.c
// last updated: 19/01/2026 <d/m/y>
#include <windows.h>
#include <uxtheme.h>
#include <dwmapi.h>
#include <commctrl.h>
#include <stdio.h>
#include "../c_header_files/gui.h"
#include "../c_header_files/core64.h"
#include "../config.h"
#ifdef _MSC_VER
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "uxtheme.lib")
#endif
#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 600
#define ID_BTN_PROCESS 101
#define ID_EDIT_INPUT 102
#define ID_EDIT_OUTPUT 103
#define ID_EDIT_PASS 104
#define ID_BTN_BROWSE_IN 105
#define ID_BTN_BROWSE_OUT 106
#define ID_CHK_SILENT 107
#define ID_LBL_STATUS 108
HWND hEditInput, hEditOutput, hEditPass, hStatus;
HFONT hFont;
#define COL_BG RGB(30, 30, 30)
#define COL_TEXT RGB(220, 220, 220)
#define COL_INPUT_BG RGB(50, 50, 50)
#define COL_BTN_BG RGB(60, 60, 60)
HBRUSH hBrushBg, hBrushInput;
void EnforceDarkMode(HWND hwnd)
{
    BOOL value = TRUE;
    DwmSetWindowAttribute(hwnd, 20, &value, sizeof(value)); // DWMWA_USE_IMMERSIVE_DARK_MODE = 20
}
void ProcessAction(HWND hwnd)
{
    (void)hwnd;
    char input[MAX_PATH], output[MAX_PATH], pass[256];
    GetWindowTextA(hEditInput, input, MAX_PATH);
    GetWindowTextA(hEditOutput, output, MAX_PATH);
    GetWindowTextA(hEditPass, pass, 256);
    if (strlen(input) == 0)
    {
        SetWindowTextA(hStatus, "[INFO] Please select an input file.");
        return;
    }
    if (strstr(input, ".bxe"))
    {
        SetWindowTextA(hStatus, "[INFO] Decrypting and running...");
        const char *p = (strlen(pass) > 0) ? pass : DEFAULT_PASSWORD;
        (strlen(pass) > 0) ? pass : DEFAULT_PASSWORD;
        if (decrypt_and_run(input, p) == 0)
        {
            SetWindowTextA(hStatus, "[INFO] Execution finished.");
        }
        else
        {
            SetWindowTextA(hStatus, "[INFO] Error executing / decrypting.");
        }
    }
    else
    {
        if (strlen(output) == 0)
        {
            snprintf(output, MAX_PATH, "%s.bxe", input);
            char *ext = strstr(output, ".exe");
            if (ext)
                strcpy(ext, ".bxe");
            SetWindowTextA(hEditOutput, output);
        }
        SetWindowTextA(hStatus, "[INFO] Encrypting...");
        const char *p = (strlen(pass) > 0) ? pass : DEFAULT_PASSWORD;
        (strlen(pass) > 0) ? pass : DEFAULT_PASSWORD;
        if (encrypt_and_binaryify(input, output, p) == 0)
        {
            SetWindowTextA(hStatus, "[INFO] Encryption successful.");
        }
        else
        {
            SetWindowTextA(hStatus, "[INFO] Error encrypting.");
        }
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        EnforceDarkMode(hwnd);
        hBrushBg = CreateSolidBrush(COL_BG);
        hBrushInput = CreateSolidBrush(COL_INPUT_BG);
        hFont = CreateFont(18, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
        int y = 20;
        CreateWindow("STATIC", "Input file:", WS_VISIBLE | WS_CHILD | SS_SIMPLE, 20, y, 100, 20, hwnd, NULL, NULL, NULL);
        hEditInput = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 20, y + 25, 450, 25, hwnd, (HMENU)ID_EDIT_INPUT, NULL, NULL);
        CreateWindow("BUTTON", "...", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_FLAT, 480, y + 25, 30, 25, hwnd, (HMENU)ID_BTN_BROWSE_IN, NULL, NULL);
        y += 70;
        CreateWindow("STATIC", "Output file:", WS_VISIBLE | WS_CHILD | SS_SIMPLE, 20, y, 100, 20, hwnd, NULL, NULL, NULL);
        hEditOutput = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 20, y + 25, 450, 25, hwnd, (HMENU)ID_EDIT_OUTPUT, NULL, NULL);
        y += 70;
        CreateWindow("STATIC", "Password (empty for default):", WS_VISIBLE | WS_CHILD | SS_SIMPLE, 20, y, 200, 20, hwnd, NULL, NULL, NULL);
        hEditPass = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_PASSWORD | ES_AUTOHSCROLL, 20, y + 25, 490, 25, hwnd, (HMENU)ID_EDIT_PASS, NULL, NULL);
        y += 70;
        CreateWindow("BUTTON", "Process", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | BS_FLAT, 20, y + 20, 200, 40, hwnd, (HMENU)ID_BTN_PROCESS, NULL, NULL);
        hStatus = CreateWindow("STATIC", "[INFO] Ready.", WS_VISIBLE | WS_CHILD | SS_SIMPLE, 20, y + 80, 500, 20, hwnd, (HMENU)ID_LBL_STATUS, NULL, NULL);
        EnumChildWindows(hwnd, (WNDENUMPROC)(void *)SendMessageA, (LPARAM)hFont);
        SendMessage(hEditInput, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hEditOutput, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hEditPass, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hStatus, WM_SETFONT, (WPARAM)hFont, TRUE);
        break;
    case WM_CTLCOLORSTATIC:
    {
        HDC hdc = (HDC)wParam;
        SetTextColor(hdc, COL_TEXT);
        SetBkColor(hdc, COL_BG);
        return (LRESULT)hBrushBg;
    }
    case WM_CTLCOLOREDIT:
    {
        HDC hdc = (HDC)wParam;
        SetTextColor(hdc, COL_TEXT);
        SetBkColor(hdc, COL_INPUT_BG);
        return (LRESULT)hBrushInput;
    }
    case WM_CTLCOLORBTN:
    {
        return (LRESULT)hBrushBg;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == ID_BTN_PROCESS)
        {
            ProcessAction(hwnd);
        }
        if (LOWORD(wParam) == ID_BTN_BROWSE_IN)
        {
            char file[MAX_PATH] = {0};
            OPENFILENAMEA ofn = {0};
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hwnd;
            ofn.lpstrFile = file;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrFilter = "All files\0*.*\0Executable\0*.exe\0BXE Files\0*.bxe\0";
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
            if (GetOpenFileNameA(&ofn))
            {
                SetWindowTextA(hEditInput, file);
            }
        }
        break;
    case WM_DESTROY:
        DeleteObject(hBrushBg);
        DeleteObject(hBrushInput);
        DeleteObject(hFont);
        PostQuitMessage(0);
        return 0;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        FillRect(hdc, &ps.rcPaint, hBrushBg);
        EndPaint(hwnd, &ps);
        return 0;
    }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
void c_gui_run()
{
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "binf";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    RegisterClassA(&wc);
    HWND hwnd = CreateWindowA("binf", "binf - GUI ver. coz why not", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, NULL, wc.hInstance, NULL);
    ShowWindow(hwnd, SW_SHOW);
    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

// end