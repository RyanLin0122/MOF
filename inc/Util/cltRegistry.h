#pragma once
//
// cltRegistry — mofclient.c 還原（位址 0x519930~0x519AC0）
//
// 用於讀取 HKEY_CURRENT_USER\Software\... 或對應 HKEY_LOCAL_MACHINE 路徑下
// 安裝的 game 資訊（"Path"、"GameVersion"）。
//
// 版本字串格式為 "AA.BB.CC.DD"，GetVersionNumber 將其壓成單一 int：
//     1000000*A + 10000*B + 100*C + D
//
// Layout（對齊原 binary）：
//   +0   HKEY  m_hKey
//   +4   int   m_nReserved   (mofclient.c ctor 寫 0)
//   +8   char  m_szVersion[256]
//
#include <windows.h>

class cltRegistry {
public:
    cltRegistry();
    ~cltRegistry();

    // mofclient.c 用 `this` 同時當 PHKEY；仍保留 phkResult 形式對齊原型。
    BOOL Open(LPCSTR lpSubKey);
    HKEY GetPath(LPBYTE lpData, int cbData);
    int  GetVersion();

    // 兩個 overload：non-static 在物件內取版本字串、static 對任意 std::string 解析。
    int  GetVersionNumber();
    static int GetVersionNumber(int stringObj);  // 對應 0x519AC0 (採 std::string layout)

public:
    HKEY m_hKey;            // +0
    int  m_nReserved;       // +4
    char m_szVersion[256];  // +8..+263
};
