#include "Info/cltMapAreaKindInfo.h"
#include <cstring>
#include <cstdlib>
#include <cctype>

// 與其他 KindCode 相同：長度=5，首字母轉大寫編碼，尾 4 碼十進位且 < 0x800
uint16_t cltMapAreaKindInfo::TranslateKindCode(char* s) {
    if (!s) return 0;
    if (std::strlen(s) != 5) return 0;
    const int hi = (std::toupper(static_cast<unsigned char>(s[0])) + 31) << 11;
    const unsigned int num = static_cast<unsigned int>(std::atoi(s + 1));
    if (num < 0x800u) return static_cast<uint16_t>(hi | num);
    return 0;
}

int cltMapAreaKindInfo::Initialize(char* filename) {
    if (!filename) return 0;

    FILE* fp = g_clTextFileManager.fopen(filename);
    if (!fp) return 0;

    const char* DELIMS = "\t\n";
    char line[1024] = { 0 };
    int ok = 0;

    // 跳過表頭 3 行（與反編譯一致）
    if (!std::fgets(line, sizeof(line), fp) ||
        !std::fgets(line, sizeof(line), fp) ||
        !std::fgets(line, sizeof(line), fp)) {
        g_clTextFileManager.fclose(fp);
        return 0;
    }

    // 預掃行數，決定配置大小
    fpos_t pos{};
    std::fgetpos(fp, &pos);
    m_count = 0;
    while (std::fgets(line, sizeof(line), fp)) ++m_count;

    if (m_count == 0) {
        // 反編譯：若行數 0，直接當成功
        g_clTextFileManager.fclose(fp);
        return 1;
    }

    m_items = static_cast<strMapAreaKindInfo*>(operator new(sizeof(strMapAreaKindInfo) * m_count));
    std::memset(m_items, 0, sizeof(strMapAreaKindInfo) * m_count);

    // 回到資料起點
    std::fsetpos(fp, &pos);

    int idx = 0;

    if (std::fgets(line, sizeof(line), fp)) {
        do {
            // 1) ID -> areaKind
            char* tok = std::strtok(line, DELIMS);
            if (!tok) break;
            uint16_t area = TranslateKindCode(tok);

            // 2) mapkind -> mapKind（必須非 0）
            tok = std::strtok(nullptr, DELIMS);
            if (!tok) break;
            uint16_t mapk = cltMapInfo::TranslateKindCode(tok);
            if (mapk == 0) break;

            // 3) x
            tok = std::strtok(nullptr, DELIMS);
            if (!tok) break;
            int32_t x = std::atoi(tok);

            // 4) y
            tok = std::strtok(nullptr, DELIMS);
            if (!tok) break;
            int32_t y = std::atoi(tok);

            // 5) width
            tok = std::strtok(nullptr, DELIMS);
            if (!tok) break;
            int32_t w = std::atoi(tok);

            // 6) height
            tok = std::strtok(nullptr, DELIMS);
            if (!tok) break;
            int32_t h = std::atoi(tok);

            if (idx < m_count) {
                strMapAreaKindInfo& r = m_items[idx];
                r.areaKind = area;
                r.mapKind = mapk;
                r.x = x;
                r.y = y;
                r.width = w;
                r.height = h;
                ++idx;
            }
        } while (std::fgets(line, sizeof(line), fp));

        // 僅當正常到 EOF（未中途 break）視為成功
        ok = 1;
    }
    else {
        // 無資料列也算成功
        ok = 1;
    }

    g_clTextFileManager.fclose(fp);

    if (ok) {
        if (idx < m_count) m_count = idx;
        static_assert(sizeof(strMapAreaKindInfo) == 20, "strMapAreaKindInfo must be 20 bytes");
        return 1;
    }
    else {
        Free();
        return 0;
    }
}

void cltMapAreaKindInfo::Free() {
    if (m_items) {
        operator delete(static_cast<void*>(m_items));
        m_items = nullptr;
    }
    m_count = 0;
}

strMapAreaKindInfo* cltMapAreaKindInfo::GetMapAreaKindInfo(uint16_t code) {
    // 線性搜尋；步距 10 WORD (=20 bytes) 與反編譯一致
    for (int i = 0; i < m_count; ++i) {
        if (m_items[i].areaKind == code) return &m_items[i];
    }
    return nullptr;
}
