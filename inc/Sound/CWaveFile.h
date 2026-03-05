#pragma once

#include <cstddef>
#include <cstdint>
#include <Windows.h>
#include <mmsystem.h>
#include "FileSystem/CMOFPacking.h"
#include "global.h"

using DWORD = unsigned long;
using LONG = long;
using UINT = unsigned int;
using WORD = unsigned short;
using BYTE = unsigned char;
using HPSTR = char*;
using LPCSTR = const char*;
using LPSTR = char*;
using HGLOBAL = void*;


class CWaveFile {
public:
    CWaveFile();
    ~CWaveFile();

    int Open(LPSTR pszFileName, tWAVEFORMATEX* pwfx, UINT mode);
    UINT OpenFromMemory(BYTE* data, UINT size, tWAVEFORMATEX* pwfx, UINT mode);
    int ReadMMIO();
    UINT GetSize();
    int ResetFile();
    int Read(BYTE* buffer, UINT bytesToRead, UINT* bytesRead);
    int Close();
    UINT WriteMMIO(tWAVEFORMATEX* pwfx);
    int Write(UINT bytesToWrite, BYTE* src, UINT* bytesWritten);
    const tWAVEFORMATEX* GetFormat() const { return m_pwfx; }

private:
    tWAVEFORMATEX* m_pwfx = nullptr;       // +0
    HMMIO m_hmmio = nullptr;               // +4
    MMCKINFO m_ck;                         // +8
    MMIOINFO m_mmioinfoOut{};              // +52
    MMCKINFO m_ckRiff;                     // +28
    DWORD m_dwSize = 0;                    // +48
    UINT m_mode = 0;                       // +124
    UINT m_bIsReadingFromMemory = 0;       // +128
    BYTE* m_pbData = nullptr;              // +132
    BYTE* m_pbDataCur = nullptr;           // +136
    UINT m_ulDataSize = 0;                 // +140
    void* m_pResourceBuffer = nullptr;     // +144
};

class WAVLoader {
public:
    WAVLoader();
    ~WAVLoader();

    void loadWAVFileIntoBuffer(char* path);

    DWORD riff = 0;
    DWORD riffChunkSize = 0;
    BYTE fmtChunk[20]{};
    DWORD dataTag = 0;
    DWORD dataSize = 0;
    BYTE* data = nullptr;
};
