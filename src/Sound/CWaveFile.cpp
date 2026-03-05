#include "Sound/CWaveFile.h"

#include <cstdlib>
#include <cstring>


CWaveFile::CWaveFile() {
    m_pwfx = nullptr;
    m_hmmio = nullptr;
    m_pResourceBuffer = nullptr;
    m_dwSize = 0;
    m_bIsReadingFromMemory = 0;
}

CWaveFile::~CWaveFile() {
    Close();
    if (!m_bIsReadingFromMemory) {
        if (m_pwfx) {
            operator delete(m_pwfx);
            m_pwfx = nullptr;
        }
    }
}

int CWaveFile::Open(LPSTR pszFileName, tWAVEFORMATEX* pwfx, UINT mode) {
    m_mode = mode;
    m_bIsReadingFromMemory = 0;

    if (mode == 1) {
        if (!pszFileName) return -2147024809;

        if (m_pwfx) {
            operator delete(m_pwfx);
            m_pwfx = nullptr;
        }

        m_hmmio = mmioOpenA(pszFileName, nullptr, 0x10000u);
        if (!m_hmmio) {
            HRSRC res = FindResourceA(nullptr, pszFileName, "WAVE");
            if (!res) {
                res = FindResourceA(nullptr, pszFileName, "WAV");
                if (!res) return -2147467259;
            }

            HGLOBAL hg = LoadResource(nullptr, res);
            if (!hg) return -2147467259;

            DWORD size = SizeofResource(nullptr, res);
            if (!size) return -2147467259;

            const void* locked = LockResource(hg);
            if (!locked) return -2147467259;

            m_pResourceBuffer = operator new(size);
            std::memcpy(m_pResourceBuffer, locked, size);

            MMIOINFO info{};
            info.fccIOProc = 541934925;
            info.cchBuffer = static_cast<LONG>(size);
            info.pchBuffer = static_cast<char*>(m_pResourceBuffer);
            m_hmmio = mmioOpenA(nullptr, &info, 0x10000u);
        }

        int hr = ReadMMIO();
        if (hr >= 0) {
            hr = ResetFile();
            if (hr >= 0) m_dwSize = m_ck.cksize;
        }
        else {
            mmioClose(m_hmmio, 0);
        }
        return hr;
    }

    m_hmmio = mmioOpenA(pszFileName, nullptr, 0x11002u);
    if (!m_hmmio) return -2147467259;

    int hr = static_cast<int>(WriteMMIO(pwfx));
    if (hr >= 0) {
        return ResetFile();
    }
    mmioClose(m_hmmio, 0);
    return hr;
}

UINT CWaveFile::OpenFromMemory(BYTE* data, UINT size, tWAVEFORMATEX* pwfx, UINT mode) {
    m_pwfx = pwfx;
    m_pbData = data;
    m_pbDataCur = data;
    m_ulDataSize = size;
    m_bIsReadingFromMemory = 1;
    return mode != 1 ? 0x80004001u : 0;
}

int CWaveFile::ReadMMIO() {
    m_pwfx = nullptr;
    if (mmioDescend(m_hmmio, &m_ckRiff, nullptr, 0)) return -2147467259;
    if (m_ckRiff.ckid != 1179011410) return -2147467259;
    if (m_ckRiff.fccType != 1163280727) return -2147467259;

    MMCKINFO ckIn{};
    ckIn.ckid = 544501094;
    if (mmioDescend(m_hmmio, &ckIn, &m_ckRiff, 0x10u) || ckIn.cksize < 0x10) return -2147467259;

    char head[16]{};
    if (mmioRead(m_hmmio, head, 16) != 16) return -2147467259;

    if (*reinterpret_cast<WORD*>(head) == 1) {
        auto* p = static_cast<std::uint32_t*>(operator new(0x12u));
        m_pwfx = reinterpret_cast<tWAVEFORMATEX*>(p);
        if (!p) return -2147467259;
        p[0] = *reinterpret_cast<std::uint32_t*>(head);
        p[1] = *reinterpret_cast<std::uint32_t*>(head + 4);
        p[2] = *reinterpret_cast<std::uint32_t*>(head + 8);
        p[3] = *reinterpret_cast<std::uint32_t*>(head + 12);
        *reinterpret_cast<WORD*>(reinterpret_cast<char*>(p) + 16) = 0;
    }
    else {
        WORD cb = 0;
        if (mmioRead(m_hmmio, reinterpret_cast<HPSTR>(&cb), 2) != 2) return -2147467259;
        auto* p = static_cast<std::uint32_t*>(operator new(static_cast<size_t>(cb) + 18));
        m_pwfx = reinterpret_cast<tWAVEFORMATEX*>(p);
        if (!p) return -2147467259;
        p[0] = *reinterpret_cast<std::uint32_t*>(head);
        p[1] = *reinterpret_cast<std::uint32_t*>(head + 4);
        p[2] = *reinterpret_cast<std::uint32_t*>(head + 8);
        p[3] = *reinterpret_cast<std::uint32_t*>(head + 12);
        *reinterpret_cast<WORD*>(reinterpret_cast<char*>(p) + 16) = cb;
        if (mmioRead(m_hmmio, reinterpret_cast<HPSTR>(reinterpret_cast<char*>(p) + 18), cb) != cb) {
            operator delete(m_pwfx);
            m_pwfx = nullptr;
            return -2147467259;
        }
    }

    if (mmioAscend(m_hmmio, &ckIn, 0)) {
        if (m_pwfx) {
            operator delete(m_pwfx);
            m_pwfx = nullptr;
        }
        return -2147467259;
    }
    return 0;
}

UINT CWaveFile::GetSize() const { return m_dwSize; }

int CWaveFile::ResetFile() {
    if (m_bIsReadingFromMemory) {
        m_pbDataCur = m_pbData;
        return 0;
    }

    if (!m_hmmio) return -2147221008;

    if (m_mode == 1) {
        if (mmioSeek(m_hmmio, m_ck.dwDataOffset + 4, 0) == -1) return -2147467259;
        m_ck.ckid = 1635017060;
        if (!mmioDescend(m_hmmio, &m_ck, &m_ckRiff, 0x10u)) return 0;
        return -2147467259;
    }

    m_ck.ckid = 1635017060;
    m_ck.cksize = 0;
    if (mmioCreateChunk(m_hmmio, &m_ck, 0)) return -2147467259;
    if (!mmioGetInfo(m_hmmio, &m_mmioinfoOut, 0)) return 0;
    return -2147467259;
}

int CWaveFile::Read(BYTE* buffer, UINT bytesToRead, UINT* bytesRead) {
    if (!m_bIsReadingFromMemory) {
        if (!m_hmmio) return -2147221008;
        if (!buffer || !bytesRead) return -2147024809;

        *bytesRead = 0;
        MMIOINFO info{};
        if (mmioGetInfo(m_hmmio, &info, 0)) return -2147467259;

        UINT toRead = bytesToRead;
        if (toRead > m_ck.cksize) toRead = m_ck.cksize;
        m_ck.cksize -= toRead;

        if (!toRead) {
            if (mmioSetInfo(m_hmmio, &info, 0)) return -2147467259;
            *bytesRead = toRead;
            return 0;
        }

        UINT copied = 0;
        HPSTR end = info.pchEndRead;
        char* next = info.pchNext;
        while (1) {
            if (next == end) {
                if (mmioAdvance(m_hmmio, &info, 0)) break;
                next = info.pchNext;
                end = info.pchEndRead;
                if (info.pchNext == info.pchEndRead) break;
            }
            BYTE ch = static_cast<BYTE>(*next++);
            buffer[copied++] = ch;
            info.pchNext = next;
            if (copied >= toRead) {
                if (mmioSetInfo(m_hmmio, &info, 0)) return -2147467259;
                *bytesRead = toRead;
                return 0;
            }
        }
        return -2147467259;
    }

    if (!m_pbDataCur) return -2147221008;
    if (bytesRead) *bytesRead = 0;

    UINT size = bytesToRead;
    BYTE* begin = m_pbData;
    BYTE* cur = m_pbDataCur;
    if (reinterpret_cast<std::uintptr_t>(cur) + bytesToRead > reinterpret_cast<std::uintptr_t>(begin) + m_ulDataSize) {
        size = static_cast<UINT>(m_ulDataSize - static_cast<UINT>(cur - begin));
    }
    std::memcpy(buffer, cur, size);
    if (bytesRead) *bytesRead = size;
    return 0;
}

int CWaveFile::Close() {
    if (m_mode == 1) {
        mmioClose(m_hmmio, 0);
        m_hmmio = nullptr;
        if (m_pResourceBuffer) {
            operator delete(m_pResourceBuffer);
            m_pResourceBuffer = nullptr;
            return 0;
        }
    }
    else {
        m_mmioinfoOut.dwFlags |= 0x10000000u;
        if (!m_hmmio) return -2147221008;
        if (mmioSetInfo(m_hmmio, &m_mmioinfoOut, 0)) return -2147467259;
        if (mmioAscend(m_hmmio, &m_ck, 0)) return -2147467259;
        if (mmioAscend(m_hmmio, &m_ckRiff, 0)) return -2147467259;
        mmioSeek(m_hmmio, 0, 0);
        if (mmioDescend(m_hmmio, &m_ckRiff, nullptr, 0)) return -2147467259;

        m_ck.ckid = 1952670054;
        if (!mmioDescend(m_hmmio, &m_ck, &m_ckRiff, 0x10u)) {
            char zero[4]{};
            mmioWrite(m_hmmio, zero, 4);
            mmioAscend(m_hmmio, &m_ck, 0);
        }

        if (mmioAscend(m_hmmio, &m_ckRiff, 0)) return -2147467259;
        mmioClose(m_hmmio, 0);
        m_hmmio = nullptr;
    }
    return 0;
}

UINT CWaveFile::WriteMMIO(tWAVEFORMATEX* pwfx) {
    char minusOne[4]{};
    *reinterpret_cast<int*>(minusOne) = -1;

    m_ckRiff.fccType = 1163280727;
    m_ckRiff.cksize = 0;
    if (mmioCreateChunk(m_hmmio, &m_ckRiff, 0x20u)) return -2147467259;

    m_ck.ckid = 544501094;
    m_ck.cksize = 16;
    if (mmioCreateChunk(m_hmmio, &m_ck, 0)) return -2147467259;

    LONG want = (pwfx->wFormatTag == 1) ? 16 : static_cast<LONG>(pwfx->cbSize + 18);
    if (mmioWrite(m_hmmio, reinterpret_cast<const char*>(pwfx), want) != want) return -2147467259;

    if (mmioAscend(m_hmmio, &m_ck, 0)) return -2147467259;

    MMCKINFO data{};
    data.ckid = 1952670054;
    data.cksize = 0;
    if (mmioCreateChunk(m_hmmio, &data, 0)) return -2147467259;

    if (mmioWrite(m_hmmio, minusOne, 4) == 4) {
        return mmioAscend(m_hmmio, &data, 0) != 0 ? 0x80004005u : 0;
    }
    return -2147467259;
}

int CWaveFile::Write(UINT bytesToWrite, BYTE* src, UINT* bytesWritten) {
    if (m_bIsReadingFromMemory) return -2147467263;
    if (!m_hmmio) return -2147221008;
    if (!bytesWritten || !src) return -2147024809;

    UINT idx = 0;
    *bytesWritten = 0;
    if (!bytesToWrite) return 0;

    while (1) {
        if (m_mmioinfoOut.pchNext == m_mmioinfoOut.pchEndRead) {
            m_mmioinfoOut.dwFlags |= 0x10000000u;
            if (mmioAdvance(m_hmmio, &m_mmioinfoOut, 1u)) break;
        }
        *reinterpret_cast<BYTE*>(m_mmioinfoOut.pchNext++) = src[idx++];
        ++*bytesWritten;
        if (idx >= bytesToWrite) return 0;
    }
    return -2147467259;
}

WAVLoader::WAVLoader() = default;
WAVLoader::~WAVLoader() = default;

void WAVLoader::loadWAVFileIntoBuffer(char* path) {
    char localPath[256]{};
    if (!path) return;

    std::strcpy(localPath, path);
    char* changed = CMofPacking::GetInstance()->ChangeString(localPath);
    CMofPacking::GetInstance()->FileReadBackGroundLoading(changed);

    int readBytes = CMofPacking::GetInstance()->GetBufferSize();
    if (readBytes > 0) {
        char* buf = CMofPacking::GetInstance()->m_backgroundLoadBufferField;

        riff = *reinterpret_cast<DWORD*>(buf + 12);
        riffChunkSize = *reinterpret_cast<DWORD*>(buf + 16);
        std::memcpy(fmtChunk, buf + 20, riffChunkSize);

        char* chunk = buf + 20 + riffChunkSize;
        while (1) {
            dataTag = *reinterpret_cast<DWORD*>(chunk);
            dataSize = *reinterpret_cast<DWORD*>(chunk + 4);
            char* dataBegin = chunk + 8;
            if (!_strnicmp(reinterpret_cast<const char*>(&dataTag), "data", 4u)) {
                data = static_cast<BYTE*>(operator new(dataSize));
                std::memcpy(data, dataBegin, dataSize);
                break;
            }
            chunk = dataBegin + dataSize;
        }
    }
}

