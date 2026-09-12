#include "app.hpp"
#include "ui_helpers.hpp"
#include <shellapi.h>

#define ID_TAB_CONTROL   1000
#define ID_BTN_RESET     1001
#define ID_CHK_AUTO      1002
#define ID_BTN_REGISTER  1003
#define ID_BTN_CHAT      1004
#define ID_BTN_UPDATE    1005

#define IDI_APP_ICON     101

TabbedApp::TabbedApp(HINSTANCE hInstance)
    : m_hInstance(hInstance),
      m_hwndMain(NULL),
      m_hwndTab(NULL),
      m_hMutex(NULL),
      m_hHandCursor(NULL),
      m_hBtnReset(NULL),
      m_hChkAutoReset(NULL),
      m_hBtnRegister(NULL),
      m_hLblRegisterInfo(NULL),
      m_hLblHelpText(NULL),
      m_hBtnChat(NULL),
      m_hBtnUpdate(NULL)
{
}

TabbedApp::~TabbedApp() {
    if (m_hMutex) {
        CloseHandle(m_hMutex);
        m_hMutex = NULL;
    }
}

bool TabbedApp::Initialize(int nCmdShow) {
    // Single instance mutex matching _B7(@ScriptName) in AutoIt
    m_hMutex = CreateMutexW(NULL, TRUE, L"Global\\IDMTrialReset_SingleInstance");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        if (m_hMutex) {
            CloseHandle(m_hMutex);
            m_hMutex = NULL;
        }
        return false;
    }

    m_hHandCursor = LoadCursor(NULL, IDC_HAND);
    HICON hAppIcon = LoadIconW(m_hInstance, MAKEINTRESOURCEW(IDI_APP_ICON));

    WNDCLASSEXW wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = TabbedApp::WindowProc;
    wc.hInstance = m_hInstance;
    wc.hIcon = hAppIcon ? hAppIcon : LoadIcon(NULL, IDI_APPLICATION);
    wc.hIconSm = hAppIcon ? hAppIcon : LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = L"IDMTrialResetUI";

    if (!RegisterClassExW(&wc)) {
        DWORD err = GetLastError();
        if (err != ERROR_CLASS_ALREADY_EXISTS) {
            return false;
        }
    }

    // Exact AutoIt inner client dimensions: 325 x 112
    RECT rc = { 0, 0, 325, 112 };
    AdjustWindowRect(&rc, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);

    m_hwndMain = CreateWindowExW(
        0,
        L"IDMTrialResetUI",
        L"IDM trial reset",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rc.right - rc.left,
        rc.bottom - rc.top,
        NULL,
        NULL,
        m_hInstance,
        this
    );

    if (!m_hwndMain) {
        return false;
    }

    if (hAppIcon) {
        SendMessageW(m_hwndMain, WM_SETICON, ICON_BIG, (LPARAM)hAppIcon);
        SendMessageW(m_hwndMain, WM_SETICON, ICON_SMALL, (LPARAM)hAppIcon);
    }

    CreateTabControl();
    CreateTabPages();
    ShowTabPage(0);

    ShowWindow(m_hwndMain, nCmdShow);
    UpdateWindow(m_hwndMain);

    return true;
}

void TabbedApp::CreateTabControl() {
    // Exact AutoIt tab coordinates: x=1, y=0, w=325, h=112
    m_hwndTab = CreateWindowExW(
        0,
        WC_TABCONTROLW,
        L"",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
        1, 0, 325, 112,
        m_hwndMain,
        (HMENU)ID_TAB_CONTROL,
        m_hInstance,
        NULL
    );

    if (m_hwndTab) {
        Utils::SetControlFont(m_hwndTab);

        TCITEMW tie = { 0 };
        tie.mask = TCIF_TEXT;

        wchar_t t1[] = L"Trial reset";
        tie.pszText = t1;
        TabCtrl_InsertItem(m_hwndTab, 0, &tie);

        wchar_t t2[] = L"Register";
        tie.pszText = t2;
        TabCtrl_InsertItem(m_hwndTab, 1, &tie);

        wchar_t t3[] = L"Help";
        tie.pszText = t3;
        TabCtrl_InsertItem(m_hwndTab, 2, &tie);
    }
}

void TabbedApp::CreateTabPages() {
    HWND hParent = m_hwndMain;

    // --- Tab 0: Trial reset ---
    m_hBtnReset = Utils::CreateChildControl(
        L"BUTTON", L"Reset the IDM trial now",
        BS_PUSHBUTTON,
        78, 40, 180, 35,
        hParent, (HMENU)ID_BTN_RESET
    );

    m_hChkAutoReset = Utils::CreateChildControl(
        L"BUTTON", L"Automatically",
        BS_AUTOCHECKBOX,
        128, 80, 80, 20,
        hParent, (HMENU)ID_CHK_AUTO
    );

    // --- Tab 1: Register ---
    m_hBtnRegister = Utils::CreateChildControl(
        L"BUTTON", L"Register IDM now",
        BS_PUSHBUTTON,
        78, 40, 180, 35,
        hParent, (HMENU)ID_BTN_REGISTER
    );

    m_hLblRegisterInfo = Utils::CreateChildControl(
        L"STATIC", L"If IDM will be blocked then Register again or use Trial reset",
        SS_LEFT,
        27, 80, 282, 17,
        hParent, NULL
    );

    // --- Tab 2: Help ---
    m_hLblHelpText = Utils::CreateChildControl(
        L"STATIC", L"Trial reset ---> Reset the IDM trial, fix blocked, fake serial...\r\nRegister -----> Register IDM",
        SS_LEFT,
        15, 35, 308, 30,
        hParent, NULL
    );

    m_hBtnChat = Utils::CreateChildControl(
        L"BUTTON", L"GitHub Repo",
        BS_PUSHBUTTON,
        45, 73, 115, 25,
        hParent, (HMENU)ID_BTN_CHAT
    );

    m_hBtnUpdate = Utils::CreateChildControl(
        L"BUTTON", L"About",
        BS_PUSHBUTTON,
        166, 73, 105, 25,
        hParent, (HMENU)ID_BTN_UPDATE
    );
}

void TabbedApp::ShowTabPage(int index) {
    ShowWindow(m_hBtnReset, SW_HIDE);
    ShowWindow(m_hChkAutoReset, SW_HIDE);
    ShowWindow(m_hBtnRegister, SW_HIDE);
    ShowWindow(m_hLblRegisterInfo, SW_HIDE);
    ShowWindow(m_hLblHelpText, SW_HIDE);
    ShowWindow(m_hBtnChat, SW_HIDE);
    ShowWindow(m_hBtnUpdate, SW_HIDE);

    switch (index) {
    case 0:
        ShowWindow(m_hBtnReset, SW_SHOW);
        ShowWindow(m_hChkAutoReset, SW_SHOW);
        break;
    case 1:
        ShowWindow(m_hBtnRegister, SW_SHOW);
        ShowWindow(m_hLblRegisterInfo, SW_SHOW);
        break;
    case 2:
        ShowWindow(m_hLblHelpText, SW_SHOW);
        ShowWindow(m_hBtnChat, SW_SHOW);
        ShowWindow(m_hBtnUpdate, SW_SHOW);
        break;
    }
}

LRESULT CALLBACK TabbedApp::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    TabbedApp* pApp = NULL;

    if (uMsg == WM_NCCREATE) {
        CREATESTRUCTW* pCreate = (CREATESTRUCTW*)lParam;
        pApp = (TabbedApp*)pCreate->lpCreateParams;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)pApp);
        if (pApp) {
            pApp->m_hwndMain = hwnd;
        }
        return DefWindowProcW(hwnd, uMsg, wParam, lParam);
    } else {
        pApp = (TabbedApp*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
    }

    if (pApp) {
        return pApp->HandleMessage(hwnd, uMsg, wParam, lParam);
    }

    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

LRESULT TabbedApp::HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_SETCURSOR: {
        HWND hTarget = (HWND)wParam;
        // Replicate GUICtrlSetCursor(..., 0) hand cursor on buttons
        if (hTarget == m_hBtnReset || hTarget == m_hBtnRegister ||
            hTarget == m_hBtnChat || hTarget == m_hBtnUpdate) {
            if (m_hHandCursor) {
                SetCursor(m_hHandCursor);
                return TRUE;
            }
        }
        break;
    }
    case WM_NOTIFY: {
        LPNMHDR pnmh = (LPNMHDR)lParam;
        if (pnmh->idFrom == ID_TAB_CONTROL && pnmh->code == TCN_SELCHANGE) {
            int curSel = TabCtrl_GetCurSel(m_hwndTab);
            ShowTabPage(curSel);
        }
        break;
    }
    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        switch (wmId) {
        case ID_BTN_RESET: {
            SetWindowTextW(m_hBtnReset, L"Please wait...");
            UpdateWindow(m_hBtnReset);
            ResetEngine::PerformTrialReset();
            SetWindowTextW(m_hBtnReset, L"Reset the IDM trial now");
            MessageBoxW(hwnd, L"You have 30 day trial now!", L"Reset IDM trial", MB_OK | MB_TOPMOST | MB_ICONINFORMATION);
            break;
        }

        case ID_CHK_AUTO: {
            LRESULT state = SendMessageW(m_hChkAutoReset, BM_GETCHECK, 0, 0);
            if (state == BST_CHECKED) {
                SetWindowTextW(m_hBtnReset, L"Please wait...");
                UpdateWindow(m_hBtnReset);
                ResetEngine::SetAutoReset(true);
                SetWindowTextW(m_hBtnReset, L"Reset the IDM trial now");
                MessageBoxW(hwnd, L"The IDM trial will be reset automatically.", L"Auto reset", MB_OK | MB_TOPMOST | MB_ICONINFORMATION);
            } else {
                ResetEngine::SetAutoReset(false);
                MessageBoxW(hwnd, L"The IDM trial will NOT be reset automatically.", L"Auto reset", MB_OK | MB_TOPMOST | MB_ICONINFORMATION);
            }
            break;
        }

        case ID_BTN_REGISTER: {
            wchar_t nameBuf[128] = { 0 };
            if (Utils::PromptInputBox(hwnd, L"Register IDM", L"Type your name here: ", L"IDM trial reset", nameBuf, 128)) {
                SetWindowTextW(m_hBtnRegister, L"Please wait...");
                UpdateWindow(m_hBtnRegister);
                SendMessageW(m_hChkAutoReset, BM_SETCHECK, BST_UNCHECKED, 0);
                ResetEngine::PerformRegistration(nameBuf);
                SetWindowTextW(m_hBtnRegister, L"Register IDM now");
                MessageBoxW(hwnd, L"IDM is registered now!", L"Register IDM", MB_OK | MB_TOPMOST | MB_ICONINFORMATION);
            }
            break;
        }

        case ID_BTN_CHAT: {
            ShellExecuteW(NULL, L"open", L"https://github.com/rabbanyhmm/idm-trial-reset", NULL, NULL, SW_SHOWNORMAL);
            break;
        }

        case ID_BTN_UPDATE: {
            int res = MessageBoxW(
                hwnd,
                L"IDM Trial Reset Utility v1.0\r\n\r\n"
                L"Rebuilt with native Win32 C++\r\n"
                L"Developer: Rabbany\r\n"
                L"GitHub: https://github.com/rabbanyhmm\r\n\r\n"
                L"Would you like to open the developer profile on GitHub?",
                L"About & Credits",
                MB_YESNO | MB_ICONINFORMATION | MB_TOPMOST
            );
            if (res == IDYES) {
                ShellExecuteW(NULL, L"open", L"https://github.com/rabbanyhmm", NULL, NULL, SW_SHOWNORMAL);
            }
            break;
        }
        }
        break;
    }
    case WM_DESTROY:
        if (m_hMutex) {
            CloseHandle(m_hMutex);
            m_hMutex = NULL;
        }
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

int TabbedApp::Run() {
    MSG msg = { 0 };
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
}
