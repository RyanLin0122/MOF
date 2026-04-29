// CCmdLine — 1:1 還原 mofclient.c 0x4FA080~0x4FA0F0
#include "Util/CCmdLine.h"

#include <cstring>

CCmdLine::CCmdLine()
{
    // mofclient.c ctor: 直接 return this — 不清空 buffer。
    // 但首字元保留 '\0' 以避免讀到未定義 byte。
    m_szAuth[0] = '\0';
}

int CCmdLine::SetCmdLine(const char* lpCmdLine)
{
    SetAuthParameter(lpCmdLine);
    return 1;
}

void CCmdLine::SetAuthParameter(const char* str)
{
    if (!str) { m_szAuth[0] = '\0'; return; }
    std::strncpy(m_szAuth, str, sizeof(m_szAuth) - 1);
    m_szAuth[sizeof(m_szAuth) - 1] = '\0';
}

const char* CCmdLine::GetAuthParameter() const
{
    return m_szAuth;
}
