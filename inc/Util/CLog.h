#pragma once
//
// CLog — mofclient.c 還原（位址 0x4D82C0~0x4D8690）
//
// 一個檔案 / Listbox / stdout 三向日誌輸出器，透過 Singleton<CLog>::m_singleton
// 提供全域單例。原 binary 在進入點建立，以 flag bitmask 控制行為：
//
//   bit 0 (1)  寫到 stdout（_printf）
//   bit 1 (2)  寫到檔案（CreateFileA / WriteFile，使用 lpFileName）
//   bit 2 (4)  寫到 LogWindow（自繪 LISTBOX）
//   bit 3 (8)  關閉所有輸出
//
// 物件 layout（對齊原 binary，DWORD 索引）：
//   +0   uint32_t  m_dwFlag
//   +4   char      m_szFileName[260]
//   +264 HWND      m_hLogWindow      (DWORD index 66)
//   +268 HWND      m_hListBox        (DWORD index 67)
//   +272 HANDLE    m_hFile           (DWORD index 68)
//
#include <windows.h>
#include <cstdarg>

class CLog;

// Singleton<CLog>::m_singleton — mofclient.c 把 CLog 的單例位址存在這個全域，
// 讓 CLog::Log 不必明確取單例（直接呼叫時會把 m_singleton 當 this）。
template <typename T>
struct Singleton {
    static T* m_singleton;
};

class CLog {
public:
    enum Flag : unsigned int {
        kStdout = 0x1,
        kFile   = 0x2,
        kWindow = 0x4,
        kQuiet  = 0x8,
    };

    CLog(unsigned int flags, int nWidth, int nHeight, LPCSTR lpFileName);
    ~CLog();

    void CreateLogWindow(int nWidth, int nHeight);
    int  Log(const char* Format, ...);

    static LRESULT CALLBACK WndProcLog(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

public:
    unsigned int m_dwFlag;            // +0
    char         m_szFileName[260];   // +4..+263
    HWND         m_hLogWindow;        // +264
    HWND         m_hListBox;          // +268
    HANDLE       m_hFile;             // +272
};

// 全域 logger 別名 — mofclient.c 用 `logger` 取得 CLog* (== Singleton<CLog>::m_singleton)
extern CLog*& logger;
