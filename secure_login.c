#include <windows.h>
#include <stdio.h>

#define IDT_TIMER 1

const char* correctUsername = "root";
const char* correctPassword = "root";

char userInput[256] = "";
int inputPos = 0;
int stage = 0;
int authenticated = 0;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK LowLevelKeyboardProc(int, WPARAM, LPARAM);
HHOOK hKeyboardHook;

void HideSystemUI() {
    HWND hTaskbar = FindWindow("Shell_TrayWnd", NULL);
    if (hTaskbar) ShowWindow(hTaskbar, SW_HIDE);
    HWND hStart = FindWindow("Button", NULL);
    if (hStart) ShowWindow(hStart, SW_HIDE);
    ShowCursor(FALSE);
    ClipCursor(&(RECT){0, 0, 0, 0});
}

void DrawPrompt(HDC hdc, RECT rect) {
    SetTextColor(hdc, RGB(0, 255, 0));
    SetBkMode(hdc, TRANSPARENT);

    char buffer[512];
    const char* prompt = (stage == 0) ? "User: " : "Passkey: ";

    if (stage == 1) {
        char hidden[256] = {0};
        memset(hidden, '*', inputPos);
        hidden[inputPos] = '\0';
        sprintf(buffer, "%s%s", prompt, hidden);
    } else {
        sprintf(buffer, "%s%s", prompt, userInput);
    }

    DrawText(hdc, buffer, -1, &rect, DT_LEFT | DT_TOP | DT_SINGLELINE);

    if (authenticated) {
        RECT newRect = rect;
        newRect.top += 40;
        DrawText(hdc, "Secure System Lock", -1, &newRect, DT_LEFT | DT_TOP | DT_SINGLELINE);
        newRect.top += 40;
        DrawText(hdc, "Process can be now terminated gracefully...", -1, &newRect, DT_LEFT | DT_TOP | DT_SINGLELINE);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow) {

    HideSystemUI();

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "SecureLoginClass";
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        WS_EX_TOPMOST,
        "SecureLoginClass",
        "Secure System Lock",
        WS_POPUP | WS_VISIBLE,
        0, 0,
        GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
        NULL, NULL, hInstance, NULL);

    HFONT hFont = CreateFont(
        24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN,
        "Courier New");

    SendMessage(hwnd, WM_SETFONT, (WPARAM)hFont, TRUE);

    SetTimer(hwnd, IDT_TIMER, 10, NULL);           // repaint
    SetTimer(hwnd, 2, 10, NULL);                   // reassert topmost & focus

    hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(hKeyboardHook);
    return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_TIMER:
        if (wParam == 2) {
            // Reassert the app is always on top, covering all desktops
            SetWindowPos(hwnd, HWND_TOPMOST, 0, 0,
                GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
                SWP_NOACTIVATE | SWP_SHOWWINDOW);
            SetForegroundWindow(hwnd);
        } else {
            InvalidateRect(hwnd, NULL, TRUE);
        }
        break;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rect;
        GetClientRect(hwnd, &rect);
        DrawPrompt(hdc, rect);
        EndPaint(hwnd, &ps);
        break;
    }
    case WM_CHAR:
        if (authenticated && wParam == VK_RETURN) {
            PostQuitMessage(0);
        } else if (wParam == VK_RETURN) {
            userInput[inputPos] = '\0';
            if (stage == 0 && strcasecmp(userInput, correctUsername) == 0) {
                stage = 1;
            } else if (stage == 1 && strcasecmp(userInput, correctPassword) == 0) {
                authenticated = 1;
            }
            inputPos = 0;
            memset(userInput, 0, sizeof(userInput));
        } else if (wParam == VK_BACK && inputPos > 0) {
            userInput[--inputPos] = '\0';
        } else if (inputPos < 255 && wParam >= 32 && wParam <= 126) {
            userInput[inputPos++] = (char)wParam;
            userInput[inputPos] = '\0';
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* kbd = (KBDLLHOOKSTRUCT*)lParam;

        // Block: Alt+Tab, Ctrl+Esc, Windows keys, Ctrl+Shift+Esc
        BOOL block = ((kbd->vkCode == VK_TAB && (GetAsyncKeyState(VK_MENU) & 0x8000)) ||
                      (kbd->vkCode == VK_ESCAPE && (GetAsyncKeyState(VK_CONTROL) & 0x8000)) ||
                      (kbd->vkCode == VK_ESCAPE && (GetAsyncKeyState(VK_SHIFT) & 0x8000)) ||
                      kbd->vkCode == VK_LWIN || kbd->vkCode == VK_RWIN);

        if (block)
            return 1;
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}
