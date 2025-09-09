#include "Info/cltFrontServerInfo.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>

// ====== 全域物件預設值 ======
unsigned int  g_DebugLevel = 0;          // 預設不顯示錯誤

// Windows 相容（非 Windows 則退回 std::remove）
#ifdef _WIN32
#include <Windows.h>
#endif

cltFrontServerInfo::cltFrontServerInfo() {
    // *((_WORD*)this + 120) = 0;  // m_wCount
    // *((_WORD*)this + 121) = 0;  // m_wIndex
    // *((_DWORD*)this + 61) = 1;  // m_dwFirstCall
    m_wCount = 0;
    m_wIndex = 0;
    m_dwFirstCall = 1;

    // memset(this, 0, 0xF0u); // 只清前 240 bytes（即 m_aInfos）
    std::memset(m_aInfos, 0, sizeof(m_aInfos));
}

int cltFrontServerInfo::Initialize() {
    // 忠實還原：NationCode==4 直接讀 dat；否則先解密到 tmp.dat
    if (g_MoFFont.GetNationCode() == 4 ||
        (FileCrypt::GetInstance()->DecoderFileCrypt("MofData/FrontServerList.dat", "tmp.dat")))
    {
        const char* path = (g_MoFFont.GetNationCode() == 4) ? "MofData/FrontServerList.dat" : "tmp.dat";
        int ok = ParseFromFile(path);

        if (g_MoFFont.GetNationCode() != 4) {
            DeleteFileCompat("tmp.dat");
        }
        return ok;
    }
    else {
        // 解密失敗處理（與反編譯一致）
        if (g_DebugLevel == 2) {
#ifdef _WIN32
            char buf[128];
            std::snprintf(buf, sizeof(buf), "FrontServerInfo Error : %d", 1);
            MessageBoxA(nullptr, buf, "Master of Fantasy", 0);
#else
            std::fprintf(stderr, "FrontServerInfo Error : %d\n", 1);
#endif
        }
        // 仍嘗試刪掉 tmp.dat（與原始流程對齊）
        if (g_MoFFont.GetNationCode() != 4) {
            DeleteFileCompat("tmp.dat");
        }
        return 0;
    }
}

int cltFrontServerInfo::InitializeFromFiles(const char* datPath, const char* tmpPath) {
    // 完整重現：NationCode==4 直接讀 datPath；否則先解密 datPath -> tmpPath
    if (g_MoFFont.GetNationCode() == 4 ||
        (FileCrypt::GetInstance()->DecoderFileCrypt(datPath, tmpPath)))
    {
        const char* path = (g_MoFFont.GetNationCode() == 4) ? datPath : tmpPath;
        int ok = ParseFromFile(path);

        if (g_MoFFont.GetNationCode() != 4) {
            DeleteFileCompat(tmpPath);
        }
        return ok;
    }
    else {
        if (g_DebugLevel == 2) {
#ifdef _WIN32
            char buf[128];
            std::snprintf(buf, sizeof(buf), "FrontServerInfo Error : %d", 1);
            MessageBoxA(nullptr, buf, "Master of Fantasy", 0);
#else
            std::fprintf(stderr, "FrontServerInfo Error : %d\n", 1);
#endif
        }
        if (g_MoFFont.GetNationCode() != 4) {
            DeleteFileCompat(tmpPath);
        }
        return 0;
    }
}

const stFrontServerInfo* cltFrontServerInfo::GetRandomServerInfo() {
    if (m_dwFirstCall == 0) {
        return nullptr; // 與反編譯一致：旗標為 0 時回傳 0
    }
    // v2 = ++m_wIndex < m_wCount; m_dwFirstCall=0; if(!v2) m_wIndex=0;
    m_wIndex = static_cast<uint16_t>(m_wIndex + 1);
    bool less = (m_wIndex < m_wCount);
    m_dwFirstCall = 0;
    if (!less) m_wIndex = 0;
    return (m_wCount > 0) ? &m_aInfos[m_wIndex] : nullptr;
}

int cltFrontServerInfo::ParseFromFile(const char* filePath) {
    // 忠實使用 C I/O 與 strtok
    FILE* fp = std::fopen(filePath, "rb");
    if (!fp) return 0;

    char line[1024] = { 0 };
    // 讀掉前三行表頭
    if (!std::fgets(line, sizeof(line), fp) ||
        !std::fgets(line, sizeof(line), fp) ||
        !std::fgets(line, sizeof(line), fp))
    {
        std::fclose(fp);
        return 0;
    }

    // 記下位置並先計數
    std::fpos_t pos;
    std::fgetpos(fp, &pos);
    m_wCount = 0;
    while (std::fgets(line, sizeof(line), fp)) {
        ++m_wCount;
    }

    // 回到起點重新解析
    std::fsetpos(fp, &pos);

    // 逐行解析（TAB/換行為分隔）
    const char* delim = "\t\n";
    uint16_t written = 0;

    while (std::fgets(line, sizeof(line), fp)) {
        // 第一個 token（No.）
        char* tok = std::strtok(line, delim);
        if (!tok) continue;

        // 第二個 token（Type）
        char* typeTok = std::strtok(nullptr, delim);
        if (!typeTok) break;

        // 忠實還原：strncmp("TEST", token, strlen(token)) == 0
        if (std::strncmp("TEST", typeTok, std::strlen(typeTok)) == 0) {
            // 跳過 IP 與 Port 兩個 token
            if (!std::strtok(nullptr, delim) || !std::strtok(nullptr, delim)) break;
            if (m_wCount > 0) --m_wCount; // 減計數
        }
        else {
            // 讀 IP/Host
            char* hostTok = std::strtok(nullptr, delim);
            if (!hostTok) break;

            // 讀 Port
            char* portTok = std::strtok(nullptr, delim);
            if (!portTok) break;

            if (written < 10) { // 原程式不檢查溢位；此處加上保護以避免越界
                std::strcpy(m_aInfos[written].Host, hostTok); // 忠實用 strcpy（無截斷）
                m_aInfos[written].Port = std::atoi(portTok);
                ++written;
            }
        }
    }

    std::fclose(fp);

    // 若有效筆數 > 1，做隨機交換（與反編譯一致的寫法）
    if (m_wCount > 1) {
        ShuffleLikeDecompiled();
    }

    return 1;
}

void cltFrontServerInfo::ShuffleLikeDecompiled() {
    // 反編譯碼迴圈大意：
    // for (i=0; i<m_wCount; ++i) swap(a[i], a[rand()%m_wCount])
    for (uint16_t i = 0; i < m_wCount && i < 10; ++i) {
        uint16_t r = static_cast<uint16_t>(std::rand() % (m_wCount ? m_wCount : 1));
        if (r < 10) {
            stFrontServerInfo tmp = m_aInfos[r];
            m_aInfos[r] = m_aInfos[i];
            m_aInfos[i] = tmp;
        }
    }
}

bool cltFrontServerInfo::DeleteFileCompat(const char* path) {
#ifdef _WIN32
    return ::DeleteFileA(path) != 0;
#else
    return std::remove(path) == 0;
#endif
}
