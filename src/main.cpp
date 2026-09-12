#include "app.hpp"
#include "ui_helpers.hpp"
#include <windows.h>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    if (pCmdLine && wcsstr(pCmdLine, L"/trial")) {
        ResetEngine::PerformTrialReset();
        ResetEngine::SetAutoReset(true);
        return 0;
    }

    // Force initialize common controls v6
    INITCOMMONCONTROLSEX icex = { 0 };
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_TAB_CLASSES | ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icex);

    TabbedApp app(hInstance);
    if (!app.Initialize(nCmdShow)) {
        return 1;
    }
    return app.Run();
}
