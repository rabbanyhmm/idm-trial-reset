#ifndef APP_HPP
#define APP_HPP

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <commctrl.h>
#include <shellapi.h>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "shell32.lib")

class TabbedApp {
public:
    TabbedApp(HINSTANCE hInstance);
    ~TabbedApp();

    bool Initialize(int nCmdShow);
    int Run();

private:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    void CreateTabControl();
    void CreateTabPages();
    void ShowTabPage(int index);

    HINSTANCE m_hInstance;
    HWND m_hwndMain;
    HWND m_hwndTab;
    HANDLE m_hMutex;
    HCURSOR m_hHandCursor;

    // tab 0: trial reset view
    HWND m_hBtnReset;
    HWND m_hChkAutoReset;

    // tab 1: registration view
    HWND m_hBtnRegister;
    HWND m_hLblRegisterInfo;

    // tab 2: help view
    HWND m_hLblHelpText;
    HWND m_hBtnChat;
    HWND m_hBtnUpdate;
};

#endif // APP_HPP
