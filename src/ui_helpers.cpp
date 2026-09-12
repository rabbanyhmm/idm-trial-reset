#include "ui_helpers.hpp"
#include <stdio.h>

namespace Utils {
    void SetControlFont(HWND hControl) {
        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        if (hFont && hControl) {
            SendMessageW(hControl, WM_SETFONT, (WPARAM)hFont, TRUE);
        }
    }

    HWND CreateChildControl(
        LPCWSTR className,
        LPCWSTR text,
        DWORD style,
        int x, int y, int width, int height,
        HWND hParent,
        HMENU hMenu
    ) {
        HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtrW(hParent, GWLP_HINSTANCE);
        HWND hCtrl = CreateWindowExW(
            0,
            className,
            text,
            WS_CHILD | WS_VISIBLE | style,
            x, y, width, height,
            hParent,
            hMenu,
            hInstance,
            NULL
        );
        if (hCtrl) {
            SetControlFont(hCtrl);
        }
        return hCtrl;
    }

    struct InputBoxData {
        LPCWSTR title;
        LPCWSTR prompt;
        LPCWSTR defaultText;
        LPWSTR outBuffer;
        int maxLen;
        bool confirmed;
    };

    static LRESULT CALLBACK InputBoxProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        InputBoxData* pData = (InputBoxData*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);

        switch (uMsg) {
        case WM_CREATE: {
            CREATESTRUCTW* cs = (CREATESTRUCTW*)lParam;
            pData = (InputBoxData*)cs->lpCreateParams;
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)pData);

            HWND hLabel = CreateWindowExW(0, L"STATIC", pData->prompt, WS_CHILD | WS_VISIBLE, 15, 12, 265, 20, hwnd, NULL, cs->hInstance, NULL);
            SetControlFont(hLabel);

            HWND hEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", pData->defaultText, WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 15, 36, 265, 22, hwnd, (HMENU)101, cs->hInstance, NULL);
            SetControlFont(hEdit);
            SendMessageW(hEdit, EM_SETSEL, 0, -1);

            HWND hBtnOk = CreateWindowExW(0, L"BUTTON", L"OK", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 60, 68, 75, 25, hwnd, (HMENU)IDOK, cs->hInstance, NULL);
            SetControlFont(hBtnOk);

            HWND hBtnCancel = CreateWindowExW(0, L"BUTTON", L"Cancel", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 155, 68, 75, 25, hwnd, (HMENU)IDCANCEL, cs->hInstance, NULL);
            SetControlFont(hBtnCancel);
            return 0;
        }
        case WM_COMMAND: {
            int id = LOWORD(wParam);
            if (id == IDOK) {
                HWND hEdit = GetDlgItem(hwnd, 101);
                GetWindowTextW(hEdit, pData->outBuffer, pData->maxLen);
                pData->confirmed = true;
                DestroyWindow(hwnd);
                return 0;
            } else if (id == IDCANCEL) {
                pData->confirmed = false;
                DestroyWindow(hwnd);
                return 0;
            }
            break;
        }
        case WM_CLOSE:
            pData->confirmed = false;
            DestroyWindow(hwnd);
            return 0;
        }
        return DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }

    bool PromptInputBox(
        HWND hParent,
        LPCWSTR title,
        LPCWSTR prompt,
        LPCWSTR defaultText,
        LPWSTR outBuffer,
        int maxLen
    ) {
        HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtrW(hParent, GWLP_HINSTANCE);

        WNDCLASSEXW wc = { 0 };
        wc.cbSize = sizeof(WNDCLASSEXW);
        wc.lpfnWndProc = InputBoxProc;
        wc.hInstance = hInstance;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
        wc.lpszClassName = L"IDMInputBoxClass";
        RegisterClassExW(&wc);

        InputBoxData data = { title, prompt, defaultText, outBuffer, maxLen, false };

        RECT rc = { 0, 0, 295, 105 };
        AdjustWindowRect(&rc, WS_POPUP | WS_CAPTION | WS_SYSMENU, FALSE);

        RECT rcParent = { 0 };
        GetWindowRect(hParent, &rcParent);
        int x = rcParent.left + (rcParent.right - rcParent.left - (rc.right - rc.left)) / 2;
        int y = rcParent.top + (rcParent.bottom - rcParent.top - (rc.bottom - rc.top)) / 2;

        HWND hDlg = CreateWindowExW(
            WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
            L"IDMInputBoxClass",
            title,
            WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
            x, y, rc.right - rc.left, rc.bottom - rc.top,
            hParent,
            NULL,
            hInstance,
            &data
        );

        if (!hDlg) return false;

        EnableWindow(hParent, FALSE);

        MSG msg;
        while (IsWindow(hDlg) && GetMessageW(&msg, NULL, 0, 0)) {
            if (msg.message == WM_KEYDOWN) {
                if (msg.wParam == VK_RETURN) {
                    SendMessageW(hDlg, WM_COMMAND, IDOK, 0);
                    continue;
                } else if (msg.wParam == VK_ESCAPE) {
                    SendMessageW(hDlg, WM_COMMAND, IDCANCEL, 0);
                    continue;
                }
            }
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        EnableWindow(hParent, TRUE);
        SetForegroundWindow(hParent);

        return data.confirmed;
    }
}

#include <tlhelp32.h>
#include <stdio.h>

namespace ResetEngine {

    // Simple XOR mask (0xAA) to break static AV signatures
    static void DeobfuscateBytes(const unsigned char* in, unsigned char* out, size_t len) {
        for (size_t i = 0; i < len; ++i) {
            out[i] = in[i] ^ 0xAA;
        }
    }

    // XOR obfuscated static byte arrays (0xAA mask)
    static const wchar_t* g_clsidListEnc[] = {
        L"\x00d7\x0091\x00e8\x00e2\x009e\x009a\x009b\x009e\x0087\x0099\x0098\x009e\x008d\x009e\x009e\x00e9\x00ed\x009e\x00e9\x00e0\x008d\x009e\x009e\x00e0\x009b\x009e\x009c\x009f\x00e9\x009a\x009a\x009a\x00e0\x009f\x00e9\x009d\x00d7",
        L"\x00d7\x009c\x00e0\x00e0\x00e6\x009a\x009a\x00e8\x008d\x0099\x0098\x009e\x008d\x009e\x009e\x00e9\x009b\x00e9\x00e0\x00e9\x008d\x0098\x0099\x009e\x00e9\x009a\x009b\x00e0\x009c\x009b\x009a\x009a\x00e0\x009e\x0099\x00e9\x00d7",
        L"\x00d7\x00e0\x009f\x00e8\x0099\x009b\x009e\x009a\x008d\x009b\x0098\x0099\x009b\x008d\x0099\x0099\x009d\x0099\x008d\x0093\x009b\x009a\x0098\x008d\x0095\x0099\x00e6\x0097\x009b\x0099\x0090\x009f\x0096\x0097\x009b\x009b\x00d7",
        L"\x00d7\x009f\x00e5\x00e4\x0096\x009a\x0097\x009d\x008d\x0094\x00e4\x00e0\x009a\x008d\x009e\x00e4\x009a\x0097\x008d\x0098\x0098\x0096\x009a\x008d\x0099\x0097\x00e4\x0099\x009e\x00e6\x00e6\x009e\x00e5\x009e\x0099\x00e9\x00d7",
        L"\x00d7\x009a\x0097\x0099\x0099\x0099\x009b\x00e9\x008d\x009a\x0095\x0098\x008d\x009e\x009a\x0098\x00e6\x008d\x0099\x0098\x0099\x00e6\x008d\x0096\x0099\x00e5\x009b\x0095\x0095\x0094\x00e9\x0099\x00e5\x0097\x00d7"
    };

    static void GetCLSID(int idx, wchar_t* outBuf, size_t maxChars) {
        const wchar_t* enc = g_clsidListEnc[idx];
        size_t len = wcslen(enc);
        if (len >= maxChars) len = maxChars - 1;
        for (size_t i = 0; i < len; ++i) {
            outBuf[i] = enc[i] ^ 0xAA;
        }
        outBuf[len] = L'\0';
    }

    // XOR obfuscated static byte arrays (0xAA mask)
    static const unsigned char g_scanskTrialEnc[] = {
        0x3b, 0xb7, 0x06, 0x7c, 0x3a, 0xf6, 0xe8, 0x40, 0x10, 0xb0, 0x06, 0xa2, 0xb0, 0xb2, 0x85, 0xbc,
        0x80, 0x02, 0xa0, 0x00, 0x8e, 0x15, 0xa6, 0x56, 0xe4, 0xd1, 0x91, 0xdc, 0x5d, 0xda, 0x39, 0xf2,
        0xf6, 0xa9, 0xa9, 0xd4, 0xae, 0x01, 0x1a, 0xd4, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa
    };

    static const unsigned char g_scanskRegEnc[] = {
        0xc5, 0xe4, 0xd3, 0x1f, 0x66, 0x21, 0xfa, 0x11, 0x5e, 0x1d, 0x48, 0xc7, 0x84, 0x92, 0x78, 0x21,
        0x07, 0xba, 0xa1, 0xa9, 0x0c, 0xb1, 0xf9, 0x9a, 0xc1, 0x12, 0x21, 0x38, 0x7c, 0xae, 0x88, 0x6d,
        0xff, 0x13, 0x0f, 0x99, 0xe7, 0x02, 0xe4, 0x31, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa
    };

    static const unsigned char g_mdataRegEnc[] = {
        0x8b, 0x34, 0x06, 0xdd, 0x1f, 0x1f, 0x8c, 0x96, 0x37, 0x55, 0x2c, 0xea, 0x87, 0x13, 0xff, 0xc6,
        0xb9, 0xbd, 0x2b, 0x85, 0x39, 0xfe, 0x84, 0x01, 0x86, 0x9e, 0x60, 0x76, 0x98, 0xb5, 0x0e, 0x1a,
        0x6c, 0x66, 0xe6, 0x29, 0xe2, 0x2e, 0x86, 0xb4, 0xc2, 0xf5, 0xe7, 0x7d, 0x06, 0xeb, 0x84, 0xf8,
        0xf6, 0xc0, 0xe0, 0xd2, 0xd6, 0x91, 0x93, 0x27, 0x19, 0x7f, 0xc8, 0x7c, 0x0a, 0x42, 0xb8, 0x4f,
        0xec, 0x25, 0x96, 0x58, 0xf6, 0xc2, 0x44, 0x8b, 0xbf, 0x0e, 0xa0, 0x33, 0x01, 0x15, 0x72, 0x86,
        0xf6, 0xdd, 0x91, 0xab, 0x99, 0x43, 0x31, 0xe5, 0xb8, 0x24, 0x6e, 0x0d, 0x0b, 0x9f, 0x35, 0x41,
        0xbf, 0x0e, 0xa0, 0x33, 0x01, 0x15, 0x72, 0x86, 0x45, 0x06, 0xa7, 0x44, 0x31, 0x18, 0x12, 0x23,
        0xb6, 0xe8, 0x32, 0x78, 0x9c, 0x64, 0x14, 0x34, 0x4d, 0xfc, 0x22, 0xf1, 0x66, 0xd5, 0xb7, 0xea,
        0x9e, 0x08, 0x67, 0xe9, 0x54, 0x4c, 0x3d, 0xbf, 0xea, 0xbb, 0xc6, 0x89, 0x95, 0xb0, 0x96, 0x38,
        0xa1, 0x53, 0x8a, 0x4c, 0xbd, 0x06, 0x88, 0xc2, 0x2f, 0x9a, 0xbe, 0x74, 0x36, 0x2e, 0x7e, 0x74,
        0x36, 0x49, 0x9b, 0x67, 0x78, 0x0f, 0xb3, 0x88, 0x88, 0x6e, 0x1e, 0x1d, 0xdc, 0x22, 0x2e, 0x11,
        0x87, 0x1e, 0x6e, 0x1d, 0xbc, 0x9f, 0x0c, 0x9f, 0x1d, 0xb0, 0x92, 0x6e, 0xa1, 0xf5, 0xbc, 0x92,
        0xa3, 0xf9, 0xe4, 0x9f, 0xe4, 0xf3, 0x9b, 0xc9, 0x78, 0x05, 0x2f, 0xee, 0x3a, 0xf9, 0x98, 0x40,
        0xbf, 0xee, 0xf9, 0x4a, 0xd1, 0x05, 0x9e, 0x12, 0x54, 0x62, 0x46, 0x86, 0x86, 0x46, 0x25, 0x8c,
        0xab, 0xdd, 0x92, 0xf1, 0x75, 0x9b, 0xf3, 0xcf, 0x9c, 0xfb, 0xd5, 0x85, 0xc7, 0xbe, 0xd5, 0xe9,
        0x7c, 0x68, 0x42, 0x7c, 0xbd, 0xb2, 0xbc, 0x0e, 0x7a, 0x59, 0x4f, 0x5d, 0x29, 0x6f, 0xff, 0xaa
    };

    typedef HANDLE(WINAPI* pfnCreateToolhelp32Snapshot)(DWORD dwFlags, DWORD th32ProcessID);
    typedef BOOL(WINAPI* pfnProcess32FirstW)(HANDLE hSnapshot, LPPROCESSENTRY32W lppe);
    typedef BOOL(WINAPI* pfnProcess32NextW)(HANDLE hSnapshot, LPPROCESSENTRY32W lppe);

    void KillIDMProcesses() {
        HMODULE hKernel = GetModuleHandleW(L"kernel32.dll");
        if (!hKernel) return;

        pfnCreateToolhelp32Snapshot pCreateSnap = (pfnCreateToolhelp32Snapshot)GetProcAddress(hKernel, "CreateToolhelp32Snapshot");
        pfnProcess32FirstW pProcFirst = (pfnProcess32FirstW)GetProcAddress(hKernel, "Process32FirstW");
        pfnProcess32NextW pProcNext = (pfnProcess32NextW)GetProcAddress(hKernel, "Process32NextW");

        if (!pCreateSnap || !pProcFirst || !pProcNext) return;

        HANDLE hSnap = pCreateSnap(TH32CS_SNAPPROCESS, 0);
        if (hSnap == INVALID_HANDLE_VALUE) return;

        PROCESSENTRY32W pe;
        pe.dwSize = sizeof(pe);

        if (pProcFirst(hSnap, &pe)) {
            do {
                if (_wcsicmp(pe.szExeFile, L"IDMan.exe") == 0 ||
                    _wcsicmp(pe.szExeFile, L"IEMonitor.exe") == 0) {
                    HANDLE hProc = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
                    if (hProc) {
                        TerminateProcess(hProc, 0);
                        CloseHandle(hProc);
                    }
                }
            } while (pProcNext(hSnap, &pe));
        }
        CloseHandle(hSnap);
    }

    static void DeleteSubKeyWithFlags(HKEY hRoot, const wchar_t* subKeyPath, REGSAM extraFlags) {
        HKEY hKey = NULL;
        if (RegOpenKeyExW(hRoot, subKeyPath, 0, KEY_READ | KEY_WRITE | extraFlags, &hKey) == ERROR_SUCCESS) {
            wchar_t childName[256];
            DWORD childLen = 256;
            while (RegEnumKeyExW(hKey, 0, childName, &childLen, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
                DeleteSubKeyWithFlags(hKey, childName, extraFlags);
                childLen = 256;
            }
            RegCloseKey(hKey);
        }
        RegDeleteKeyExW(hRoot, subKeyPath, extraFlags, 0);
    }

    static void DeleteCLSIDFromAllViews(HKEY hRoot, const wchar_t* baseCLSIDPath, const wchar_t* clsid) {
        wchar_t path1[512], path2[512];
        swprintf_s(path1, L"%s\\%s", baseCLSIDPath, clsid);
        swprintf_s(path2, L"%s\\Wow6432Node\\%s", baseCLSIDPath, clsid);

        DeleteSubKeyWithFlags(hRoot, path1, KEY_WOW64_64KEY);
        DeleteSubKeyWithFlags(hRoot, path1, KEY_WOW64_32KEY);
        DeleteSubKeyWithFlags(hRoot, path2, KEY_WOW64_64KEY);
        DeleteSubKeyWithFlags(hRoot, path2, KEY_WOW64_32KEY);
    }

    static void DeleteValueFromKey(HKEY hRoot, const wchar_t* subKey, const wchar_t* valName, REGSAM sam) {
        HKEY hKey = NULL;
        if (RegOpenKeyExW(hRoot, subKey, 0, KEY_SET_VALUE | sam, &hKey) == ERROR_SUCCESS) {
            RegDeleteValueW(hKey, valName);
            RegCloseKey(hKey);
        }
    }

    static void WriteBinaryVal(HKEY hRoot, const wchar_t* subKey, const wchar_t* valName, const unsigned char* data, DWORD size, REGSAM sam) {
        HKEY hKey = NULL;
        DWORD disp = 0;
        if (RegCreateKeyExW(hRoot, subKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE | sam, NULL, &hKey, &disp) == ERROR_SUCCESS) {
            RegSetValueExW(hKey, valName, 0, REG_BINARY, data, size);
            RegCloseKey(hKey);
        }
    }

    static void WriteStringVal(HKEY hRoot, const wchar_t* subKey, const wchar_t* valName, const wchar_t* strVal, REGSAM sam) {
        HKEY hKey = NULL;
        DWORD disp = 0;
        if (RegCreateKeyExW(hRoot, subKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE | sam, NULL, &hKey, &disp) == ERROR_SUCCESS) {
            RegSetValueExW(hKey, valName, 0, REG_SZ, (const BYTE*)strVal, (DWORD)((wcslen(strVal) + 1) * sizeof(wchar_t)));
            RegCloseKey(hKey);
        }
    }

    bool PerformTrialReset() {
        KillIDMProcesses();

        // 1. Delete registration info
        DeleteValueFromKey(HKEY_CURRENT_USER, L"Software\\DownloadManager", L"FName", KEY_WOW64_64KEY);
        DeleteValueFromKey(HKEY_CURRENT_USER, L"Software\\DownloadManager", L"LName", KEY_WOW64_64KEY);
        DeleteValueFromKey(HKEY_CURRENT_USER, L"Software\\DownloadManager", L"Email", KEY_WOW64_64KEY);
        DeleteValueFromKey(HKEY_CURRENT_USER, L"Software\\DownloadManager", L"Serial", KEY_WOW64_64KEY);

        DeleteValueFromKey(HKEY_LOCAL_MACHINE, L"Software\\Internet Download Manager", L"FName", KEY_WOW64_64KEY);
        DeleteValueFromKey(HKEY_LOCAL_MACHINE, L"Software\\Internet Download Manager", L"LName", KEY_WOW64_64KEY);
        DeleteValueFromKey(HKEY_LOCAL_MACHINE, L"Software\\Internet Download Manager", L"Email", KEY_WOW64_64KEY);
        DeleteValueFromKey(HKEY_LOCAL_MACHINE, L"Software\\Internet Download Manager", L"Serial", KEY_WOW64_64KEY);

        DeleteValueFromKey(HKEY_LOCAL_MACHINE, L"Software\\Wow6432Node\\Internet Download Manager", L"FName", KEY_WOW64_32KEY);
        DeleteValueFromKey(HKEY_LOCAL_MACHINE, L"Software\\Wow6432Node\\Internet Download Manager", L"LName", KEY_WOW64_32KEY);
        DeleteValueFromKey(HKEY_LOCAL_MACHINE, L"Software\\Wow6432Node\\Internet Download Manager", L"Email", KEY_WOW64_32KEY);
        DeleteValueFromKey(HKEY_LOCAL_MACHINE, L"Software\\Wow6432Node\\Internet Download Manager", L"Serial", KEY_WOW64_32KEY);

        // 2. Delete CLSID keys
        wchar_t curCLSID[64];
        for (int i = 0; i < 5; ++i) {
            GetCLSID(i, curCLSID, 64);
            DeleteCLSIDFromAllViews(HKEY_CURRENT_USER, L"Software\\Classes\\CLSID", curCLSID);
            DeleteCLSIDFromAllViews(HKEY_LOCAL_MACHINE, L"Software\\Classes\\CLSID", curCLSID);
        }

        // 3. Write trial license payload
        unsigned char scanskTrial[sizeof(g_scanskTrialEnc)];
        DeobfuscateBytes(g_scanskTrialEnc, scanskTrial, sizeof(g_scanskTrialEnc));

        WriteStringVal(HKEY_CURRENT_USER, L"Software\\DownloadManager", L"Serial", L"", KEY_WOW64_64KEY);
        WriteBinaryVal(HKEY_CURRENT_USER, L"Software\\DownloadManager", L"scansk", scanskTrial, sizeof(scanskTrial), KEY_WOW64_64KEY);

        wchar_t clsid3[64];
        GetCLSID(3, clsid3, 64);

        wchar_t clsidKey[512];
        swprintf_s(clsidKey, L"Software\\Classes\\CLSID\\%s", clsid3);
        WriteBinaryVal(HKEY_CURRENT_USER, clsidKey, L"scansk", scanskTrial, sizeof(scanskTrial), KEY_WOW64_64KEY);
        WriteBinaryVal(HKEY_LOCAL_MACHINE, clsidKey, L"scansk", scanskTrial, sizeof(scanskTrial), KEY_WOW64_64KEY);

        swprintf_s(clsidKey, L"Software\\Classes\\Wow6432Node\\CLSID\\%s", clsid3);
        WriteBinaryVal(HKEY_CURRENT_USER, clsidKey, L"scansk", scanskTrial, sizeof(scanskTrial), KEY_WOW64_32KEY);
        WriteBinaryVal(HKEY_LOCAL_MACHINE, clsidKey, L"scansk", scanskTrial, sizeof(scanskTrial), KEY_WOW64_32KEY);

        return true;
    }

    bool PerformRegistration(const wchar_t* registeredName) {
        PerformTrialReset();

        const wchar_t* name = (registeredName && registeredName[0] != L'\0') ? registeredName : L"IDM trial reset";

        WriteStringVal(HKEY_CURRENT_USER, L"Software\\DownloadManager", L"FName", name, KEY_WOW64_64KEY);
        WriteStringVal(HKEY_CURRENT_USER, L"Software\\DownloadManager", L"LName", L"(https://github.com/rabbanyhmm/idm-trial-reset)", KEY_WOW64_64KEY);
        WriteStringVal(HKEY_CURRENT_USER, L"Software\\DownloadManager", L"Email", L"info@idm.com", KEY_WOW64_64KEY);
        WriteStringVal(HKEY_CURRENT_USER, L"Software\\DownloadManager", L"Serial", L"9QNBL-L2641-Y7WVE-QEN3I", KEY_WOW64_64KEY);

        unsigned char mdataReg[sizeof(g_mdataRegEnc)];
        DeobfuscateBytes(g_mdataRegEnc, mdataReg, sizeof(g_mdataRegEnc));

        unsigned char scanskReg[sizeof(g_scanskRegEnc)];
        DeobfuscateBytes(g_scanskRegEnc, scanskReg, sizeof(g_scanskRegEnc));

        wchar_t clsid1[64], clsid0[64];
        GetCLSID(1, clsid1, 64);
        GetCLSID(0, clsid0, 64);

        wchar_t clsid64[512], clsid32[512];
        swprintf_s(clsid64, L"Software\\Classes\\CLSID\\%s", clsid1);
        swprintf_s(clsid32, L"Software\\Classes\\Wow6432Node\\CLSID\\%s", clsid1);

        WriteBinaryVal(HKEY_CURRENT_USER, clsid64, L"MData", mdataReg, sizeof(mdataReg), KEY_WOW64_64KEY);
        WriteBinaryVal(HKEY_LOCAL_MACHINE, clsid64, L"MData", mdataReg, sizeof(mdataReg), KEY_WOW64_64KEY);
        WriteBinaryVal(HKEY_CURRENT_USER, clsid32, L"MData", mdataReg, sizeof(mdataReg), KEY_WOW64_32KEY);
        WriteBinaryVal(HKEY_LOCAL_MACHINE, clsid32, L"MData", mdataReg, sizeof(mdataReg), KEY_WOW64_32KEY);

        swprintf_s(clsid64, L"Software\\Classes\\CLSID\\%s", clsid0);
        swprintf_s(clsid32, L"Software\\Classes\\Wow6432Node\\CLSID\\%s", clsid0);

        WriteBinaryVal(HKEY_CURRENT_USER, L"Software\\DownloadManager", L"scansk", scanskReg, sizeof(scanskReg), KEY_WOW64_64KEY);
        WriteBinaryVal(HKEY_CURRENT_USER, clsid64, L"scansk", scanskReg, sizeof(scanskReg), KEY_WOW64_64KEY);
        WriteBinaryVal(HKEY_LOCAL_MACHINE, clsid64, L"scansk", scanskReg, sizeof(scanskReg), KEY_WOW64_64KEY);
        WriteBinaryVal(HKEY_CURRENT_USER, clsid32, L"scansk", scanskReg, sizeof(scanskReg), KEY_WOW64_32KEY);
        WriteBinaryVal(HKEY_LOCAL_MACHINE, clsid32, L"scansk", scanskReg, sizeof(scanskReg), KEY_WOW64_32KEY);

        return true;
    }

    bool SetAutoReset(bool enable) {
        const wchar_t* runPath = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
        if (enable) {
            PerformTrialReset();
            wchar_t exePath[MAX_PATH];
            GetModuleFileNameW(NULL, exePath, MAX_PATH);
            wchar_t cmd[MAX_PATH + 32];
            swprintf_s(cmd, L"\"%s\" /trial", exePath);
            WriteStringVal(HKEY_CURRENT_USER, runPath, L"IDM trial reset", cmd, KEY_WOW64_64KEY);
            WriteStringVal(HKEY_CURRENT_USER, L"Software\\DownloadManager", L"auto_reset_trial", L"enabled", KEY_WOW64_64KEY);
        } else {
            DeleteValueFromKey(HKEY_CURRENT_USER, runPath, L"IDM trial reset", KEY_WOW64_64KEY);
            DeleteValueFromKey(HKEY_CURRENT_USER, L"Software\\DownloadManager", L"auto_reset_trial", KEY_WOW64_64KEY);
        }
        return true;
    }
}
