// gui.c
// last updated: 22/01/2026 <d/m/y>
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
#define WINDOW_WIDTH 680
#define WINDOW_HEIGHT 550
#define ID_BTN_PROCESS 101
#define ID_EDIT_INPUT 102
#define ID_EDIT_OUTPUT 103
#define ID_EDIT_PASS 104
#define ID_BTN_BROWSE_IN 105
#define ID_BTN_BROWSE_OUT 106
#define ID_EDIT_TIME_COST 108
#define ID_EDIT_MEM_COST 109
#define ID_EDIT_PARALLEL 110
#define ID_CHK_USE_CUSTOM 111
#define ID_PROGRESS 112
HWND hEditInput, hEditOutput, hEditPass;
HWND hEditTimeCost, hEditMemCost, hEditParallel;
HWND hChkCustom, hProgress;
HFONT hFont, hFontBold;
#define COL_BG RGB(25, 25, 28)
#define COL_TEXT RGB(225, 225, 230)
#define COL_INPUT_BG RGB(45, 45, 50)
#define COL_INPUT_BORDER RGB(70, 70, 75)
#define COL_BTN_BG RGB(55, 55, 62)
#define COL_BTN_HOVER RGB(70, 70, 80)
#define COL_ACCENT RGB(90, 140, 210)
#define COL_ERROR RGB(220, 80, 80)
#define COL_SUCCESS RGB(80, 180, 120)
struct MsgBoxParams
{
    const char *text;
    const char *title;
    UINT type;
    int result;
};
LRESULT CALLBACK MsgBoxProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static HFONT hFont;
    static HBRUSH hBrushBg;
    static HBRUSH hBrushBtn;
    switch (msg)
    {
    case WM_CREATE:
    {
        CREATESTRUCT *cs = (CREATESTRUCT *)lParam;
        struct MsgBoxParams *params = (struct MsgBoxParams *)cs->lpCreateParams;
        SetWindowTextA(hwnd, params->title);
        hBrushBg = CreateSolidBrush(COL_BG);
        hBrushBtn = CreateSolidBrush(COL_BTN_BG);
        hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
        HWND hText = CreateWindow("STATIC", params->text, WS_VISIBLE | WS_CHILD | SS_LEFT, 20, 20, 160, 60, hwnd, NULL, NULL, NULL);
        SendMessage(hText, WM_SETFONT, (WPARAM)hFont, TRUE);
        int btnY = 100;
        if (params->type & MB_YESNO)
        {
            HWND hBtnYes = CreateWindow("BUTTON", "Yes", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 30, btnY, 60, 30, hwnd, (HMENU)IDYES, NULL, NULL);
            SendMessage(hBtnYes, WM_SETFONT, (WPARAM)hFont, TRUE);
            HWND hBtnNo = CreateWindow("BUTTON", "No", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 100, btnY, 60, 30, hwnd, (HMENU)IDNO, NULL, NULL);
            SendMessage(hBtnNo, WM_SETFONT, (WPARAM)hFont, TRUE);
        }
        else
        {
            HWND hBtnOK = CreateWindow("BUTTON", "OK", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 70, btnY, 60, 30, hwnd, (HMENU)IDOK, NULL, NULL);
            SendMessage(hBtnOK, WM_SETFONT, (WPARAM)hFont, TRUE);
        }
        return 0;
    }
    case WM_CTLCOLORSTATIC:
    {
        HDC hdc = (HDC)wParam;
        SetTextColor(hdc, COL_TEXT);
        SetBkColor(hdc, COL_BG);
        return (LRESULT)hBrushBg;
    }
    case WM_CTLCOLORBTN:
    {
        HDC hdc = (HDC)wParam;
        SetTextColor(hdc, COL_TEXT);
        SetBkColor(hdc, COL_BTN_BG);
        return (LRESULT)hBrushBtn;
    }
    case WM_COMMAND:
    {
        struct MsgBoxParams *params = (struct MsgBoxParams *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDYES)
        {
            params->result = LOWORD(wParam);
        }
        else if (LOWORD(wParam) == IDNO)
        {
            params->result = IDNO;
        }
        DestroyWindow(hwnd);
        break;
    }
    case WM_DESTROY:
    {
        DeleteObject(hBrushBg);
        DeleteObject(hBrushBtn);
        DeleteObject(hFont);
        return 0;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        FillRect(hdc, &ps.rcPaint, hBrushBg);
        EndPaint(hwnd, &ps);
        return 0;
    }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}
void EnforceDarkMode(HWND hwnd)
{
    BOOL value = TRUE;
    DwmSetWindowAttribute(hwnd, 20, &value, sizeof(value));
}
int CustomDarkMessageBox(HWND parent, const char *text, const char *title, UINT type)
{
    struct MsgBoxParams params = {text, title, type, 0};
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = MsgBoxProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "CustomMsgBox";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    RegisterClassA(&wc);
    RECT parentRect;
    GetWindowRect(parent, &parentRect);
    int width = 200;
    int height = 180;
    int x = parentRect.left + (parentRect.right - parentRect.left - width) / 2;
    int y = parentRect.top + (parentRect.bottom - parentRect.top - height) / 2;
    HWND hwnd = CreateWindowA("CustomMsgBox", title, WS_POPUP | WS_CAPTION | WS_SYSMENU, x, y, width, height, parent, NULL, wc.hInstance, (LPVOID)&params);
    if (!hwnd)
        return 0;
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)&params);
    EnforceDarkMode(hwnd);
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    EnableWindow(parent, FALSE);
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!IsDialogMessage(hwnd, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (!IsWindow(hwnd))
            break;
    }
    EnableWindow(parent, TRUE);
    SetFocus(parent);
    return params.result;
}
HBRUSH hBrushBg, hBrushInput, hBrushBtn;
CryptoParams g_params = {
    ARGON2_TIME_COST,
    ARGON2_MEMORY_COST,
    ARGON2_PARALLELISM};
int ValidateFileExtension(const char *path, const char *ext)
{
    size_t len = strlen(path);
    size_t ext_len = strlen(ext);
    if (len < ext_len)
        return 0;
    const char *file_ext = path + len - ext_len;
    return _stricmp(file_ext, ext) == 0;
}
void SetProgressBar(int percent)
{
    SendMessage(hProgress, PBM_SETPOS, (WPARAM)percent, 0);
}
int GetCryptoParams(HWND hwnd, CryptoParams *params)
{
    char buf[64];
    if (SendMessage(hChkCustom, BM_GETCHECK, 0, 0) == BST_CHECKED)
    {
        GetWindowTextA(hEditTimeCost, buf, sizeof(buf));
        params->time_cost = atoi(buf);
        if (params->time_cost < 1 || params->time_cost > 100)
        {
            return 0;
        }
        GetWindowTextA(hEditMemCost, buf, sizeof(buf));
        params->memory_cost = atoi(buf);
        if (params->memory_cost < 8192 || params->memory_cost > 2097152)
        {
            CustomDarkMessageBox(hwnd, "Memory cost must be between 8192-2097152KBs.", "Error", MB_OK | MB_ICONERROR);
            return 0;
        }
        GetWindowTextA(hEditParallel, buf, sizeof(buf));
        params->parallelism = atoi(buf);
        if (params->parallelism < 1 || params->parallelism > 16)
        {
            CustomDarkMessageBox(hwnd, "Parallelism must be between 1-16.", "Error", MB_OK | MB_ICONERROR);
            return 0;
        }
    }
    else
    {
        params->time_cost = ARGON2_TIME_COST;
        params->memory_cost = ARGON2_MEMORY_COST;
        params->parallelism = ARGON2_PARALLELISM;
    }
    return 1;
}
void ProcessAction(HWND hwnd)
{
    char input[MAX_PATH], output[MAX_PATH], pass[256];
    GetWindowTextA(hEditInput, input, MAX_PATH);
    GetWindowTextA(hEditOutput, output, MAX_PATH);
    GetWindowTextA(hEditPass, pass, 256);
    if (strlen(input) == 0)
    {
        CustomDarkMessageBox(hwnd, "No input file was provided, c'mon man.", "Error", MB_OK | MB_ICONERROR);
        return;
    }
    DWORD attrs = GetFileAttributesA(input);
    if (attrs == INVALID_FILE_ATTRIBUTES)
    {
        CustomDarkMessageBox(hwnd, "The selected input file does not exist...?", "Error", MB_OK | MB_ICONERROR);
        return;
    }
    if (strstr(input, ".bxe"))
    {
        if (!ValidateFileExtension(input, ".bxe"))
        {
            CustomDarkMessageBox(hwnd, "Input file must have a [.bxe] extension.", "Error", MB_OK | MB_ICONERROR);
            return;
        }
        SetProgressBar(10);
        const char *p = (strlen(pass) > 0) ? pass : DEFAULT_PASSWORD;
        SetProgressBar(50);
        if (decrypt_and_run(input, p) == 0)
        {
            SetProgressBar(100);
            CustomDarkMessageBox(hwnd, "Execution completed successfully.", "Success", MB_OK | MB_ICONINFORMATION);
        }
        else
        {
            SetProgressBar(0);
            CustomDarkMessageBox(hwnd, "Failed to decrypt and or execute file; password may be wrong or corrupt / invalid file.", "Error", MB_OK | MB_ICONERROR);
        }
    }
    else
    {
        if (!ValidateFileExtension(input, ".exe"))
        {
            CustomDarkMessageBox(hwnd, "Input file must have a [.exe] extension.", "Error", MB_OK | MB_ICONERROR);
            return;
        }
        if (strlen(output) == 0)
        {
            snprintf(output, MAX_PATH, "%s.bxe", input);
            char *ext = strstr(output, ".exe");
            if (ext)
                strcpy(ext, ".bxe");
            SetWindowTextA(hEditOutput, output);
        }

        if (!ValidateFileExtension(output, ".bxe"))
        {
            CustomDarkMessageBox(hwnd, "Output file must have a [.bxe] extension.", "Error", MB_OK | MB_ICONERROR);
            return;
        }
        FILE *test_in = fopen(input, "rb");
        if (!test_in)
        {
            CustomDarkMessageBox(hwnd, "Cannot open input file; check permissions?", "Error", MB_OK | MB_ICONERROR);
            return;
        }
        fseek(test_in, 0, SEEK_END);
        long size = ftell(test_in);
        fclose(test_in);
        if (size > 200 * 1024 * 1024)
        {
            char msg[256];
            snprintf(msg, sizeof(msg), "Warning: file is %.2f MBs, output will be %.2f MBs.\nContinue?", size / (1024.0 * 1024.0), (size * 8) / (1024.0 * 1024.0));
            if (CustomDarkMessageBox(hwnd, msg, "Large file warning", MB_YESNO | MB_ICONWARNING) != IDYES)
            {
                return;
            }
        }
        if (!GetCryptoParams(hwnd, &g_params))
        {
            return;
        }
        SetProgressBar(10);
        const char *p = (strlen(pass) > 0) ? pass : DEFAULT_PASSWORD;
        SetProgressBar(30);
        if (encrypt_and_binaryify(input, output, p) == 0)
        {
            SetProgressBar(100);
            CustomDarkMessageBox(hwnd, "File encryption passed with; 0 errors.", "Success", MB_OK | MB_ICONINFORMATION);
        }
        else
        {
            SetProgressBar(0);
            CustomDarkMessageBox(hwnd, "Oops, failed to encrypt selected file...", "Error", MB_OK | MB_ICONERROR);
        }
    }
    SetProgressBar(0);
}
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
    {
        EnforceDarkMode(hwnd);
        hBrushBg = CreateSolidBrush(COL_BG);
        hBrushInput = CreateSolidBrush(COL_INPUT_BG);
        hBrushBtn = CreateSolidBrush(COL_BTN_BG);
        hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
        hFontBold = CreateFont(16, 0, 0, 0, FW_BOLD, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
        int y = 25;
        int margin = 25;
        int labelW = 140;
        int inputW = 420;
        int btnW = 40;
        HWND hLabel = CreateWindow("STATIC", "Input file (.exe / .bxe):", WS_VISIBLE | WS_CHILD, margin, y, labelW, 20, hwnd, NULL, NULL, NULL);
        SendMessage(hLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
        hEditInput = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL, margin + labelW + 10, y, inputW, 28, hwnd, (HMENU)ID_EDIT_INPUT, NULL, NULL);
        SendMessage(hEditInput, WM_SETFONT, (WPARAM)hFont, TRUE);
        CreateWindow("BUTTON", "...", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, margin + labelW + inputW + 20, y, btnW, 28, hwnd, (HMENU)ID_BTN_BROWSE_IN, NULL, NULL);
        y += 35;
        hLabel = CreateWindow("STATIC", "Output file (.bxe):", WS_VISIBLE | WS_CHILD, margin, y, labelW, 20, hwnd, NULL, NULL, NULL);
        SendMessage(hLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
        hEditOutput = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL, margin + labelW + 10, y, inputW, 28, hwnd, (HMENU)ID_EDIT_OUTPUT, NULL, NULL);
        SendMessage(hEditOutput, WM_SETFONT, (WPARAM)hFont, TRUE);
        CreateWindow("BUTTON", "...", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, margin + labelW + inputW + 20, y, btnW, 28, hwnd, (HMENU)ID_BTN_BROWSE_OUT, NULL, NULL);
        y += 35;
        hLabel = CreateWindow("STATIC", "Password (optional):", WS_VISIBLE | WS_CHILD, margin, y, labelW, 20, hwnd, NULL, NULL, NULL);
        SendMessage(hLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
        hEditPass = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_VISIBLE | WS_CHILD | ES_PASSWORD | ES_AUTOHSCROLL, margin + labelW + 10, y, inputW + btnW + 10, 28, hwnd, (HMENU)ID_EDIT_PASS, NULL, NULL);
        SendMessage(hEditPass, WM_SETFONT, (WPARAM)hFont, TRUE);
        y += 40;
        CreateWindow("STATIC", "----------------------------------------------------", WS_VISIBLE | WS_CHILD, margin, y, 630, 20, hwnd, NULL, NULL, NULL);
        y += 25;
        hChkCustom = CreateWindow("BUTTON", "Use custom Argon2ID parameters", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, margin, y, 300, 25, hwnd, (HMENU)ID_CHK_USE_CUSTOM, NULL, NULL);
        SendMessage(hChkCustom, WM_SETFONT, (WPARAM)hFontBold, TRUE);
        y += 30;
        hLabel = CreateWindow("STATIC", "Time cost (iterations):", WS_VISIBLE | WS_CHILD, margin + 20, y, 180, 20, hwnd, NULL, NULL, NULL);
        SendMessage(hLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
        hEditTimeCost = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "4", WS_VISIBLE | WS_CHILD | ES_NUMBER, margin + 210, y, 100, 26, hwnd, (HMENU)ID_EDIT_TIME_COST, NULL, NULL);
        SendMessage(hEditTimeCost, WM_SETFONT, (WPARAM)hFont, TRUE);
        EnableWindow(hEditTimeCost, FALSE);
        y += 35;
        hLabel = CreateWindow("STATIC", "Memory cost (KB):", WS_VISIBLE | WS_CHILD, margin + 20, y, 180, 20, hwnd, NULL, NULL, NULL);
        SendMessage(hLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
        hEditMemCost = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "98304", WS_VISIBLE | WS_CHILD | ES_NUMBER, margin + 210, y, 100, 26, hwnd, (HMENU)ID_EDIT_MEM_COST, NULL, NULL);
        SendMessage(hEditMemCost, WM_SETFONT, (WPARAM)hFont, TRUE);
        EnableWindow(hEditMemCost, FALSE);
        y += 35;
        hLabel = CreateWindow("STATIC", "Parallelism (threadz):", WS_VISIBLE | WS_CHILD, margin + 20, y, 180, 20, hwnd, NULL, NULL, NULL);
        SendMessage(hLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
        hEditParallel = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "4", WS_VISIBLE | WS_CHILD | ES_NUMBER, margin + 210, y, 100, 26, hwnd, (HMENU)ID_EDIT_PARALLEL, NULL, NULL);
        SendMessage(hEditParallel, WM_SETFONT, (WPARAM)hFont, TRUE);
        EnableWindow(hEditParallel, FALSE);
        y += 50;
        CreateWindow("STATIC", "----------------------------------------------------", WS_VISIBLE | WS_CHILD, margin, y, 630, 20, hwnd, NULL, NULL, NULL);
        y += 20;
        HWND hBtnProcess = CreateWindow("BUTTON", "Process", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, margin, y, 200, 45, hwnd, (HMENU)ID_BTN_PROCESS, NULL, NULL);
        SendMessage(hBtnProcess, WM_SETFONT, (WPARAM)hFontBold, TRUE);
        y += 50;
        hProgress = CreateWindowEx(0, PROGRESS_CLASS, NULL, WS_VISIBLE | WS_CHILD | PBS_SMOOTH, margin, y, 630, 22, hwnd, (HMENU)ID_PROGRESS, NULL, NULL);
        SendMessage(hProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
        SendMessage(hProgress, PBM_SETSTEP, 1, 0);
        y += 32;
        break;
    }
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
        SendMessage((HWND)lParam, WM_SETFONT, (WPARAM)hFont, TRUE);
        HDC hdc = (HDC)wParam;
        SetTextColor(hdc, COL_TEXT);
        SetBkColor(hdc, COL_BTN_BG);
        return (LRESULT)hBrushBtn;
    }
    case WM_COMMAND:
    {
        if (LOWORD(wParam) == ID_BTN_PROCESS)
        {
            ProcessAction(hwnd);
        }
        else if (LOWORD(wParam) == ID_CHK_USE_CUSTOM)
        {
            BOOL enabled = (SendMessage(hChkCustom, BM_GETCHECK, 0, 0) == BST_CHECKED);
            EnableWindow(hEditTimeCost, enabled);
            EnableWindow(hEditMemCost, enabled);
            EnableWindow(hEditParallel, enabled);
        }
        else if (LOWORD(wParam) == ID_BTN_BROWSE_IN)
        {
            char file[MAX_PATH] = {0};
            OPENFILENAMEA ofn = {0};
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hwnd;
            ofn.lpstrFile = file;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrFilter = "Executable files\0*.exe;*.bxe\0All files\0*.*\0";
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
            if (GetOpenFileNameA(&ofn))
            {
                SetWindowTextA(hEditInput, file);

                if (strstr(file, ".exe"))
                {
                    char outFile[MAX_PATH];
                    strcpy(outFile, file);
                    char *ext = strstr(outFile, ".exe");
                    if (ext)
                        strcpy(ext, ".bxe");
                    SetWindowTextA(hEditOutput, outFile);
                }
                RedrawWindow(hEditInput, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
                RedrawWindow(hEditOutput, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
            }
        }
        else if (LOWORD(wParam) == ID_BTN_BROWSE_OUT)
        {
            char file[MAX_PATH] = {0};
            OPENFILENAMEA ofn = {0};
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hwnd;
            ofn.lpstrFile = file;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrFilter = "BXE files\0*.bxe\0All files\0*.*\0";
            ofn.lpstrDefExt = "bxe";
            ofn.Flags = OFN_OVERWRITEPROMPT;
            if (GetSaveFileNameA(&ofn))
            {
                SetWindowTextA(hEditOutput, file);
                RedrawWindow(hEditOutput, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
            }
        }
        break;
    }
    case WM_DESTROY:
        DeleteObject(hBrushBg);
        DeleteObject(hBrushInput);
        DeleteObject(hBrushBtn);
        DeleteObject(hFont);
        DeleteObject(hFontBold);
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
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_PROGRESS_CLASS;
    InitCommonControlsEx(&icex);
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "binf";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    RegisterClassA(&wc);
    HWND hwnd = CreateWindowA("binf", "binf - hi lol :>", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, NULL, wc.hInstance, NULL);
    ShowWindow(hwnd, SW_SHOW);
    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

// end