// CLogWriter — 1:1 還原 mofclient.c 0x55CDA0~0x55CFA0
#include "Util/CLogWriter.h"

#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <ctime>

//----- (0055CDA0) --------------------------------------------------------
CLogWriter::CLogWriter()
    : m_pFile(nullptr)
{
    std::memset(m_pad, 0, sizeof(m_pad));
    InitializeCriticalSection(&m_cs);
}

//----- (0055CDC0) --------------------------------------------------------
CLogWriter::~CLogWriter()
{
    EnterCriticalSection(&m_cs);
    if (m_pFile) {
        std::fclose(m_pFile);
        m_pFile = nullptr;
    }
    LeaveCriticalSection(&m_cs);
    DeleteCriticalSection(&m_cs);
}

//----- (0055CE00) --------------------------------------------------------
int CLogWriter::Start(const char* fileName)
{
    EnterCriticalSection(&m_cs);
    m_pFile = std::fopen(fileName, "at");
    LeaveCriticalSection(&m_cs);
    // mofclient.c: `(*(_DWORD *)this != 0) - 1`  →  m_pFile != null ? 0 : -1
    return (m_pFile != nullptr) ? 0 : -1;
}

//----- (0055CE40) --------------------------------------------------------
void CLogWriter::End()
{
    EnterCriticalSection(&m_cs);
    if (m_pFile) {
        std::fclose(m_pFile);
        m_pFile = nullptr;
    }
    LeaveCriticalSection(&m_cs);
}

//----- (0055CE80) --------------------------------------------------------
void CLogWriter::Write(const char* str)
{
    if (m_pFile) {
        std::fprintf(m_pFile, "%s\r\n", str);
    }
}

//----- (0055CEA0) --------------------------------------------------------
void CLogWriter::PrintF(const char* a2, int a3, const char* Format, ...)
{
    EnterCriticalSection(&m_cs);
    if (m_pFile) {
        char buffer[1024] = {0};
        char header[768];

        va_list args;
        va_start(args, Format);
        _vsnprintf(buffer, 1024, Format, args);
        va_end(args);

        std::printf("%s\n", buffer);

        int n = std::sprintf(header, "File: %s\t", a2);
        n += std::sprintf(header + n, "Line: %d\t", a3);
        std::sprintf(header + n, "Contents:%s", buffer);
        Write(header);
    }
    LeaveCriticalSection(&m_cs);
}

//----- (0055CFA0) --------------------------------------------------------
void CLogWriter::PrintF(const char* a2, const char* Format, ...)
{
    EnterCriticalSection(&m_cs);

    char dateBuf[129] = {0};
    char fullPath[1025] = {0};
    char message[2048] = {0};
    char body[768];
    char curDate[128];
    char curTime[128];

    time_t t = std::time(nullptr);
    struct tm* lt = std::localtime(&t);
    std::strftime(dateBuf, 128, "%Y%m%d", lt);
    std::sprintf(fullPath, "[%s] %s", dateBuf, a2);

    Start(fullPath);
    if (m_pFile) {
        va_list args;
        va_start(args, Format);
        _vsnprintf(message, 2048, Format, args);
        va_end(args);

        std::printf("%s\n", message);
        _strtime(curTime);
        _strdate(curDate);

        int n = std::sprintf(body, "[%s %s]\r\n", curDate, curTime);
        std::sprintf(body + n, "Contents:%s", message);
        Write(body);
        End();
    }

    LeaveCriticalSection(&m_cs);
}
