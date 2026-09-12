#ifndef UI_HELPERS_HPP
#define UI_HELPERS_HPP

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <commctrl.h>

namespace Utils {
    // applies system font so controls look clean
    void SetControlFont(HWND hControl);

    // wrapper for win32 child control creation
    HWND CreateChildControl(
        LPCWSTR className,
        LPCWSTR text,
        DWORD style,
        int x, int y, int width, int height,
        HWND hParent,
        HMENU hMenu
    );

    // modal input dialog matching AutoIt InputBox
    bool PromptInputBox(
        HWND hParent,
        LPCWSTR title,
        LPCWSTR prompt,
        LPCWSTR defaultText,
        LPWSTR outBuffer,
        int maxLen
    );
}

namespace ResetEngine {
    void KillIDMProcesses();
    bool PerformTrialReset();
    bool PerformRegistration(const wchar_t* registeredName);
    bool SetAutoReset(bool enable);
}

#endif // UI_HELPERS_HPP
