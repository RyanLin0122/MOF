// cltRegistry — 1:1 還原 mofclient.c 0x519930~0x519AC0
#include "Util/cltRegistry.h"

#include <cstdlib>
#include <cstring>

//----- (00519930) --------------------------------------------------------
cltRegistry::cltRegistry()
    : m_hKey(nullptr), m_nReserved(0)
{
    std::strcpy(m_szVersion, "");
}

//----- (00519970) --------------------------------------------------------
cltRegistry::~cltRegistry()
{
    if (m_hKey) {
        RegCloseKey(m_hKey);
    }
}

//----- (00519980) --------------------------------------------------------
BOOL cltRegistry::Open(LPCSTR lpSubKey)
{
    // mofclient.c 把 `this` 當作 PHKEY 直接傳入；對齊：寫到 m_hKey。
    if (RegOpenKeyExA(HKEY_CURRENT_USER, lpSubKey, 0, KEY_READ, &m_hKey) == ERROR_SUCCESS) {
        return TRUE;
    }
    return RegOpenKeyExA(HKEY_LOCAL_MACHINE, lpSubKey, 0, KEY_READ, &m_hKey) == ERROR_SUCCESS;
}

//----- (005199D0) --------------------------------------------------------
HKEY cltRegistry::GetPath(LPBYTE lpData, int cbData)
{
    HKEY result = m_hKey;
    if (m_hKey) {
        DWORD cb = (DWORD)cbData;
        result = (HKEY)(uintptr_t)(RegQueryValueExA(m_hKey, "Path", nullptr, nullptr, lpData, &cb) == ERROR_SUCCESS);
    }
    return result;
}

//----- (00519A00) --------------------------------------------------------
int cltRegistry::GetVersion()
{
    if (!m_hKey) return 0;
    DWORD cbData = 256;
    if (RegQueryValueExA(m_hKey, "GameVersion", nullptr, nullptr,
                         (LPBYTE)m_szVersion, &cbData) != ERROR_SUCCESS) {
        return 0;
    }
    return GetVersionNumber();
}

//----- (00519A40) --------------------------------------------------------
int cltRegistry::GetVersionNumber()
{
    char* tok = std::strtok(m_szVersion, ".");
    int v = 1000000 * (tok ? std::atoi(tok) : 0);
    tok = std::strtok(nullptr, ".");
    v += 10000 * (tok ? std::atoi(tok) : 0);
    tok = std::strtok(nullptr, ".");
    v += 100 * (tok ? std::atoi(tok) : 0);
    tok = std::strtok(nullptr, ".");
    v += (tok ? std::atoi(tok) : 0);
    return v;
}

//----- (00519AC0) --------------------------------------------------------
// 原 binary 此 overload 接 `std::string` (layout: ptr/len/cap)；本還原僅在
// 內部 fallback 路徑被呼叫，這裡保留行為等價的字串解析骨架。傳入 0 會把
// nullstr ("") 作為版本字串處理。
int cltRegistry::GetVersionNumber(int stringObj)
{
    const char* str = "";
    if (stringObj) {
        // mofclient.c 把 std::string layout (ptr/len/cap) 視為 4-byte indexed；
        // x64 上 stringObj 已經是壓回 int 的指標位 — 沿用偏移 4 取出第二欄位（buf ptr）。
        const char* p = *reinterpret_cast<const char* const*>(
            reinterpret_cast<const char*>(static_cast<intptr_t>(stringObj)) + 4);
        if (p) str = p;
    }
    char buf[256];
    std::strncpy(buf, str, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char* tok = std::strtok(buf, ".");
    int v = 1000000 * (tok ? std::atoi(tok) : 0);
    tok = std::strtok(nullptr, ".");
    v += 10000 * (tok ? std::atoi(tok) : 0);
    tok = std::strtok(nullptr, ".");
    v += 100 * (tok ? std::atoi(tok) : 0);
    tok = std::strtok(nullptr, ".");
    v += (tok ? std::atoi(tok) : 0);
    return v;
}
