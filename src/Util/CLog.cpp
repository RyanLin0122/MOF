// CLog — 1:1 還原 mofclient.c 0x4D82C0~0x4D8690
#include "Util/CLog.h"

#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <ctime>

// Singleton<CLog>::m_singleton — 0x4D8330 寫入；ctor 設定為 this。
template<>
CLog* Singleton<CLog>::m_singleton = nullptr;

// 對齊 mofclient.c：UIManager::Free 透過 `logger` 別名取用 Singleton<CLog>::m_singleton。
CLog*& logger = Singleton<CLog>::m_singleton;

//----- (004D82C0) --------------------------------------------------------
CLog::CLog(unsigned int flags, int nWidth, int nHeight, LPCSTR lpFileName)
{
    Singleton<CLog>::m_singleton = this;
    m_hFile      = nullptr;
    m_dwFlag     = flags;

    if ((flags & kQuiet) != 0) {
        return;
    }

    if ((flags & kFile) != 0) {
        std::strcpy(m_szFileName, lpFileName);
    } else {
        m_szFileName[0] = 0;
    }

    if ((flags & kWindow) != 0) {
        CreateLogWindow(nWidth, nHeight);
    } else {
        m_hLogWindow = nullptr;
    }

    m_hFile = CreateFileA(lpFileName,
                          GENERIC_WRITE,
                          0,
                          nullptr,
                          CREATE_ALWAYS,
                          FILE_ATTRIBUTE_NORMAL,
                          nullptr);
    Log("Logging Started...");
}

//----- (004D83B0) --------------------------------------------------------
CLog::~CLog()
{
    if (m_hFile) {
        CloseHandle(m_hFile);
    }
    if (m_hLogWindow) {
        DestroyWindow(m_hLogWindow);
    }
    Singleton<CLog>::m_singleton = nullptr;
}

//----- (004D83F0) --------------------------------------------------------
void CLog::CreateLogWindow(int nWidth, int nHeight)
{
    WNDCLASSA wc{};
    wc.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS; // 35 = 0x23
    wc.lpfnWndProc   = CLog::WndProcLog;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = GetModuleHandleA(nullptr);
    wc.hIcon         = LoadIconA(nullptr, MAKEINTRESOURCEA(0x7F05)); // mofclient.c 對齊
    wc.hCursor       = LoadCursorA(nullptr, MAKEINTRESOURCEA(0x7F00)); // IDC_ARROW
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName  = nullptr;
    wc.lpszClassName = "LogWindow";
    RegisterClassA(&wc);

    int x = GetSystemMetrics(SM_CXSCREEN) - nWidth;
    HMODULE hInst = GetModuleHandleA(nullptr);
    m_hLogWindow = CreateWindowExA(
        0,
        "LogWindow",
        "Log",
        WS_POPUP | WS_BORDER | WS_SYSMENU | WS_VISIBLE,
        x, 0,
        nWidth, nHeight,
        nullptr, nullptr,
        hInst,
        nullptr);

    RECT rc;
    GetClientRect(m_hLogWindow, &rc);
    HMODULE hInst2 = GetModuleHandleA(nullptr);
    m_hListBox = CreateWindowExA(
        0,
        "LISTBOX",
        "",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY,
        0, 0,
        rc.right, rc.bottom,
        m_hLogWindow,
        nullptr,
        hInst2,
        nullptr);

    ShowWindow(m_hLogWindow, SW_SHOW);
    ShowWindow(m_hListBox, SW_SHOW);
}

//----- (004D8510) --------------------------------------------------------
int CLog::Log(const char* Format, ...)
{
    if (!Format || !*Format) return 0;
    if ((m_dwFlag & kQuiet) != 0) return 0;

    char date[128];
    char tm_[128];
    char fileBuf[512];
    char wbuf[512];
    char lparam[1024];

    va_list args;
    va_start(args, Format);

    _strdate(date);
    _strtime(tm_);
    std::vsprintf(lparam, Format, args);

    if ((m_dwFlag & kStdout) != 0) {
        std::printf("(date[%s] time[%s]) : %s\n", date, tm_, lparam);
    }

    if ((m_dwFlag & kFile) != 0) {
        // 第二次 vsprintf —— 對齊 mofclient.c：原始碼確實呼叫了兩次。
        va_list args2;
        va_start(args2, Format);
        std::vsprintf(fileBuf, Format, args2);
        va_end(args2);

        int len = wsprintfA(wbuf, "%s\r\n", fileBuf);
        DWORD bytesWritten = 0;
        WriteFile(m_hFile, wbuf, (DWORD)len, &bytesWritten, nullptr);
    }

    if ((m_dwFlag & kWindow) != 0 && m_hListBox) {
        SendMessageA(m_hListBox, LB_ADDSTRING, 0, (LPARAM)lparam);
        LRESULT count = SendMessageA(m_hListBox, LB_GETCOUNT, 0, 0);
        SendMessageA(m_hListBox, LB_SETTOPINDEX, (WPARAM)(count - 1), 0);
    }

    va_end(args);
    return 1;
}

//----- (004D8670) --------------------------------------------------------
LRESULT CALLBACK CLog::WndProcLog(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    return DefWindowProcA(hWnd, Msg, wParam, lParam);
}
