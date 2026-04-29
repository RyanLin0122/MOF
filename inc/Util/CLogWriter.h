#pragma once
//
// CLogWriter — mofclient.c 還原（位址 0x55CDA0~0x55CFA0）
//
// 與 CLog 不同，CLogWriter 是 thread-safe 的單檔案 append-only logger，搭配
// 兩個 PrintF overload：一個帶檔/行號（多用於 assert），一個帶日期前綴。
//
// 物件 layout：
//   +0    FILE*               m_pFile
//   +4..  (132 bytes 保留 / 過去用作 stack-style filename buffer，IDA 未顯示用途)
//   +132  CRITICAL_SECTION    m_cs    (x86 size 24 / x64 40)
//
#include <windows.h>
#include <cstdio>
#include <cstdarg>

class CLogWriter {
public:
    CLogWriter();
    ~CLogWriter();

    int  Start(const char* fileName);
    void End();
    void Write(const char* str);

    // 對齊 mofclient.c overload：含 file/line metadata
    void PrintF(const char* a2, int a3, const char* Format, ...);
    // 對齊 mofclient.c overload：把日期 prefix 套到檔名後再開檔
    void PrintF(const char* a2, const char* Format, ...);

private:
    FILE*            m_pFile;        // +0
    char             m_pad[128];     // +4..+131 (mofclient.c 內未使用，但保留 binary 大小)
    CRITICAL_SECTION m_cs;           // +132
};
