#include "Info/cltMineKindInfo.h"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>


// 簡單的輸入驗證，對齊反編譯裡的 IsDigit / IsAlphaNumeric 用法
static inline bool IsDigitStr(const char* s) {
    if (!s || !*s) return false;
    for (const unsigned char* p = reinterpret_cast<const unsigned char*>(s); *p; ++p) {
        if (!std::isdigit(*p)) return false;
    }
    return true;
}
static inline bool IsAlphaNumericStr(const char* s) {
    if (!s || !*s) return false;
    // 檔案此欄後續用 "%x" 解析，這裡允許 0-9A-Fa-f
    for (const unsigned char* p = reinterpret_cast<const unsigned char*>(s); *p; ++p) {
        if (!std::isxdigit(*p)) return false;
    }
    return true;
}

cltMineKindInfo::cltMineKindInfo()
    : m_count(0), m_list(nullptr)
{}

// 反編譯：初始化流程（跳過前三行標頭，記錄位置，先數行數再配置，回到位置後逐行解析）
int cltMineKindInfo::Initialize(const char* path)
{
    char delim[] = "\t\n";
    int  ok = 0;

    FILE* fp = g_clTextFileManager.fopen((char*)path);
    if (!fp) return 0;

    // 跳過前三行
    char buf[1024];
    if (!std::fgets(buf, sizeof(buf), fp) ||
        !std::fgets(buf, sizeof(buf), fp) ||
        !std::fgets(buf, sizeof(buf), fp)) {
        g_clTextFileManager.fclose(fp);
        return 0;
    }

    // 記位置並數行
    fpos_t pos{};
    std::fgetpos(fp, &pos);
    m_count = 0;
    while (std::fgets(buf, sizeof(buf), fp)) {
        ++m_count;
    }

    // 配置（反編譯使用 operator new(56*count) 並且 memset）
    if (m_count > 0) {
        void* mem = ::operator new(static_cast<size_t>(56 * m_count));
        m_list = reinterpret_cast<strMineKindInfo*>(mem);
        std::memset(m_list, 0, static_cast<size_t>(56 * m_count));
    }
    else {
        // 空檔案也算成功（比照其他 Initialize 寫法）
        g_clTextFileManager.fclose(fp);
        return 1;
    }

    // 回到資料起點
    std::fsetpos(fp, &pos);

    // 逐行解析
    int idxBytes = 0; // 以 byte 偏移前進（反編譯使用 v7 += 56）
    while (std::fgets(buf, sizeof(buf), fp)) {
        strMineKindInfo* rec = reinterpret_cast<strMineKindInfo*>(
            reinterpret_cast<unsigned char*>(m_list) + idxBytes);

        // 開始切 token
        char* t = std::strtok(buf, delim);
        if (!t) break;
        rec->kind = TranslateKindCode(t);

        t = std::strtok(nullptr, delim);
        if (!t) break;
        rec->skill = TranslateKindCodeToSkill(t);

        t = std::strtok(nullptr, delim);
        if (!t || !IsDigitStr(t)) break;
        rec->attack = static_cast<std::uint32_t>(std::atoi(t));

        t = std::strtok(nullptr, delim);
        if (!t || !IsDigitStr(t)) break;
        rec->attackRange = static_cast<std::uint32_t>(std::atoi(t));

        t = std::strtok(nullptr, delim);
        if (!t || !IsDigitStr(t)) break;
        rec->detectRange = static_cast<std::uint32_t>(std::atoi(t));

        t = std::strtok(nullptr, delim);
        if (!t || !IsDigitStr(t)) break;
        rec->blockRatePermille = static_cast<std::uint32_t>(std::atoi(t));
        if (rec->blockRatePermille >= 1000) break;

        t = std::strtok(nullptr, delim);
        if (!t || !IsDigitStr(t)) break;
        rec->blockDurationMs = static_cast<std::uint32_t>(std::atoi(t));

        t = std::strtok(nullptr, delim);
        if (!t || !IsDigitStr(t)) break;
        rec->disappearMs = static_cast<std::uint32_t>(std::atoi(t)); // 先暫存到 disappear，後面

        // 注意反編譯的寫入順序：先把這欄寫到 +40（disappearMs），
        // 接著才是 stunProb / stunDur / freezeProb / freezeDur。
        // 我們只要對齊最終結構即可。

        t = std::strtok(nullptr, delim);
        if (!t || !IsDigitStr(t)) break;
        rec->stunRatePermille = static_cast<std::uint32_t>(std::atoi(t));
        if (rec->stunRatePermille >= 1000) break;

        t = std::strtok(nullptr, delim);
        if (!t || !IsDigitStr(t)) break;
        rec->stunDurationMs = static_cast<std::uint32_t>(std::atoi(t));

        t = std::strtok(nullptr, delim);
        if (!t || !IsDigitStr(t)) break;
        rec->freezeRatePermille = static_cast<std::uint32_t>(std::atoi(t));
        if (rec->freezeRatePermille >= 1000) break;

        t = std::strtok(nullptr, delim);
        if (!t || !IsDigitStr(t)) break;
        rec->freezeDurationMs = static_cast<std::uint32_t>(std::atoi(t));

        t = std::strtok(nullptr, delim);
        if (!t || !IsDigitStr(t)) break;
        rec->airGround = static_cast<std::uint8_t>(std::atoi(t));

        t = std::strtok(nullptr, delim);
        if (!t) break;
        if (std::strcmp(t, "FIX") == 0)       rec->moveType = 1;
        else if (std::strcmp(t, "MOVING") == 0) rec->moveType = 2;
        else break;

        t = std::strtok(nullptr, delim);
        if (!t) break;
        if (std::strcmp(t, "ONE") == 0)       rec->attackCountType = 1;
        else if (std::strcmp(t, "MULTI") == 0) rec->attackCountType = 2;
        else break;

        t = std::strtok(nullptr, delim);
        if (!t || !IsAlphaNumericStr(t)) break;
        {
            unsigned x = 0;
            std::sscanf(t, "%x", &x);
            rec->resourceIdHex = x;
        }

        t = std::strtok(nullptr, delim);
        if (!t) break;
        rec->totalFlame = static_cast<std::uint16_t>(std::atoi(t));

        // 下一筆
        idxBytes += 56;
    }

    // 成功條件：循序讀到檔尾（和反編譯一致，正常 EOF 才設為 1）
    if (!std::fgets(buf, sizeof(buf), fp)) {
        ok = 1;
    }

    g_clTextFileManager.fclose(fp);
    return ok;
}

void cltMineKindInfo::Free()
{
    if (m_list) {
        ::operator delete(m_list);
        m_list = nullptr;
    }
    m_count = 0;
}

strMineKindInfo* cltMineKindInfo::GetMineKindInfo(std::uint16_t code)
{
    if (m_count <= 0 || !m_list) return nullptr;
    // 反編譯以 WORD* 逐 28 WORDs（=56 bytes）掃描
    for (int i = 0; i < m_count; ++i) {
        strMineKindInfo* rec = reinterpret_cast<strMineKindInfo*>(
            reinterpret_cast<unsigned char*>(m_list) + 56 * i);
        if (rec->kind == code) return rec;
    }
    return nullptr;
}

// "M0001" 形式：長度必須 5，((toupper(M)+31)<<11) | atoi("0001")，>2047 回 0
std::uint16_t cltMineKindInfo::TranslateKindCode(const char* a1)
{
    if (!a1 || std::strlen(a1) != 5) return 0;
    int hi = (std::toupper(static_cast<unsigned char>(a1[0])) + 31) << 11;
    unsigned v = static_cast<unsigned>(std::atoi(a1 + 1));
    if (v < 0x800u) return static_cast<std::uint16_t>(hi | v);
    return 0;
}

// "A01641" / "Pxxxx"：長度 6；'A' → 0x8000 | num，'P' → 0x0000 | num；num < 0x8000
std::uint16_t cltMineKindInfo::TranslateKindCodeToSkill(const char* a1)
{
    if (!a1 || std::strlen(a1) != 6) return 0;
    int c = std::toupper(static_cast<unsigned char>(a1[0]));
    std::uint16_t hi = 0;
    if (c == 'A')      hi = 0x8000;
    else if (c == 'P') hi = 0x0000;
    else return 0;

    unsigned v = static_cast<unsigned>(std::atoi(a1 + 1));
    if (v < 0x8000u) return static_cast<std::uint16_t>(hi | v);
    return 0;
}