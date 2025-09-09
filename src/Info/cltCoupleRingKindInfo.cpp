#include "Info/cltCoupleRingKindInfo.h"
#include <cstring>
#include <cstdlib>
#include <cctype>

// ((toupper(s[0]) + 31) << 11) | atoi(s+1)
// 長度需為 5 且尾四碼(十進位) < 0x800，否則回傳 0
uint16_t cltCoupleRingKindInfo::TranslateKindCode(char* s) {
    if (!s) return 0;
    if (std::strlen(s) != 5) return 0;
    const int hi = (std::toupper(static_cast<unsigned char>(s[0])) + 31) << 11;
    const unsigned int num = static_cast<unsigned int>(std::atoi(s + 1));
    if (num < 0x800u) return static_cast<uint16_t>(hi | num);
    return 0;
}

int cltCoupleRingKindInfo::Initialize(char* filename) {
    if (!filename) return 0;

    FILE* fp = g_clTextFileManager.fopen(filename);
    if (!fp) return 0;

    const char* DELIMS = "\t\n";
    char line[1024] = { 0 };

    // 跳過表頭三行（與反編譯一致）
    if (!std::fgets(line, sizeof(line), fp) ||
        !std::fgets(line, sizeof(line), fp) ||
        !std::fgets(line, sizeof(line), fp)) {
        g_clTextFileManager.fclose(fp);
        return 0;
    }

    // 記錄資料起點並先數行以決定配置
    fpos_t pos{};
    std::fgetpos(fp, &pos);
    m_count = 0;
    while (std::fgets(line, sizeof(line), fp)) ++m_count;

    // 配置（每筆 20 bytes）並清 0
    m_items = (m_count > 0)
        ? static_cast<strCoupleRingKindInfo*>(operator new(sizeof(strCoupleRingKindInfo) * m_count))
        : nullptr;
    if (m_items) std::memset(m_items, 0, sizeof(strCoupleRingKindInfo) * m_count);

    // 回到資料起點
    std::fsetpos(fp, &pos);

    bool ok = false; // 只有正常跑到 EOF（或無資料）才視為成功
    int idx = 0;

    if (std::fgets(line, sizeof(line), fp)) {
        do {
            // 1) 커플링ID -> translate
            char* tok = std::strtok(line, DELIMS);
            if (!tok) break;
            uint16_t kind = TranslateKindCode(tok);
            if (kind == 0) break;

            // 2) 企劃用名稱（丟棄）
            tok = std::strtok(nullptr, DELIMS);
            if (!tok) break;

            // 3) 텍스트 코드 -> WORD
            tok = std::strtok(nullptr, DELIMS);
            if (!tok) break;
            uint16_t textId = static_cast<uint16_t>(std::atoi(tok));

            // 4) 경험치 상승률(%) -> DWORD
            tok = std::strtok(nullptr, DELIMS);
            if (!tok) break;
            int32_t expRate = std::atoi(tok);

            // 5) 배우자 소환여부 -> DWORD
            tok = std::strtok(nullptr, DELIMS);
            if (!tok) break;
            int32_t summon = std::atoi(tok);

            // 6) 커플링리소스 ID（十六進位）
            tok = std::strtok(nullptr, DELIMS);
            if (!tok) break;
            uint32_t resHex = 0;
            std::sscanf(tok, "%x", &resHex);

            // 7) 블럭아이디 -> WORD
            tok = std::strtok(nullptr, DELIMS);
            if (!tok) break;
            uint16_t blockId = static_cast<uint16_t>(std::atoi(tok));

            // 8) 이펙트파일명 -> translate
            tok = std::strtok(nullptr, DELIMS);
            if (!tok) break;
            uint16_t effect = TranslateKindCode(tok);

            if (idx < m_count) {
                strCoupleRingKindInfo& r = m_items[idx];
                r.ringKind = kind;
                r.textId = textId;
                r.expRatePercent = expRate;
                r.canSummonSpouse = summon;
                r.resourceIdHex = resHex;
                r.blockId = blockId;
                r.effectKind = effect;
                ++idx;
            }

        } while (std::fgets(line, sizeof(line), fp));

        ok = true; // 正常讀到 EOF 視為成功
    }
    else {
        ok = true; // 無資料列也視為成功（與反編譯一致）
    }

    g_clTextFileManager.fclose(fp);

    if (ok) {
        if (idx < m_count) m_count = idx;
        static_assert(sizeof(strCoupleRingKindInfo) == 20, "size must be 20 bytes");
        return 1;
    }
    else {
        Free();
        return 0;
    }
}

void cltCoupleRingKindInfo::Free() {
    if (m_items) {
        operator delete(static_cast<void*>(m_items));
        m_items = nullptr;
    }
    m_count = 0;
}

strCoupleRingKindInfo* cltCoupleRingKindInfo::GetCoupleRingKindInfo(uint16_t code) {
    // 線性搜尋（每筆 stride = 10 WORDs = 20 bytes）
    for (int i = 0; i < m_count; ++i) {
        if (m_items[i].ringKind == code) return &m_items[i];
    }
    return nullptr;
}
