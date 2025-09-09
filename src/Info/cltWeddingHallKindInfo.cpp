#include "Info/cltWeddingHallKindInfo.h"
#include <cstring>
#include <cstdlib>
#include <cctype>

// ((toupper(s[0]) + 31) << 11) | atoi(s+1)；長度必須 5 且數值 < 0x800
uint16_t cltWeddingHallKindInfo::TranslateKindCode(char* s) {
    if (!s) return 0;
    if (std::strlen(s) != 5) return 0;
    const int hi = (std::toupper(static_cast<unsigned char>(s[0])) + 31) << 11;
    const unsigned int num = static_cast<unsigned int>(std::atoi(s + 1));
    if (num < 0x800u) return static_cast<uint16_t>(hi | num);
    return 0;
}

int cltWeddingHallKindInfo::Initialize(char* filename) {
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

    // 預掃行數，決定配置大小
    fpos_t pos{};
    std::fgetpos(fp, &pos);
    m_count = 0;
    while (std::fgets(line, sizeof(line), fp)) ++m_count;

    m_items = (m_count > 0)
        ? static_cast<strWeddingHallKindInfo*>(operator new(sizeof(strWeddingHallKindInfo) * m_count))
        : nullptr;
    if (m_items) std::memset(m_items, 0, sizeof(strWeddingHallKindInfo) * m_count);

    std::fsetpos(fp, &pos);

    bool ok = false; // 僅當正常讀到 EOF（或無資料）時為成功
    int idx = 0;

    // 逐列剖析
    if (std::fgets(line, sizeof(line), fp)) {
        do {
            // 1) 웨딩id
            char* tok = std::strtok(line, DELIMS);
            if (!tok) { ok = false; break; }
            uint16_t wKind = TranslateKindCode(tok);
            if (wKind == 0) { ok = false; break; }

            // 2) 利用券名稱(企劃用) —— 丟棄
            tok = std::strtok(nullptr, DELIMS);
            if (!tok) { ok = false; break; }

            // 3) 結婚式利用券 id（Ixxxx）
            tok = std::strtok(nullptr, DELIMS);
            if (!tok) { ok = false; break; }
            uint16_t ticketKind = cltItemKindInfo::TranslateKindCode(tok);

            // 4) 맵id(웨딩홀)（Axxxx）
            tok = std::strtok(nullptr, DELIMS);
            if (!tok) { ok = false; break; }
            uint16_t mapHall = cltMapInfo::TranslateKindCode(tok);

            // 5) (웨딩홀) X
            tok = std::strtok(nullptr, DELIMS);
            if (!tok) { ok = false; break; }
            int hallX = std::atoi(tok);

            // 6) (웨딩홀) Y
            tok = std::strtok(nullptr, DELIMS);
            if (!tok) { ok = false; break; }
            int hallY = std::atoi(tok);

            // 7) 맵id(스튜디오)
            tok = std::strtok(nullptr, DELIMS);
            if (!tok) { ok = false; break; }
            uint16_t mapStudio = cltMapInfo::TranslateKindCode(tok);

            // 8) (스튜디오) 工作室 X
            tok = std::strtok(nullptr, DELIMS);
            if (!tok) { ok = false; break; }
            int studioX = std::atoi(tok);

            // 9) (스튜디오) 工作室 Y
            tok = std::strtok(nullptr, DELIMS);
            if (!tok) { ok = false; break; }
            int studioY = std::atoi(tok);

            // 10) 맵地圖id(허니문)
            tok = std::strtok(nullptr, DELIMS);
            if (!tok) { ok = false; break; }
            uint16_t mapHoneymoon = cltMapInfo::TranslateKindCode(tok);

            // 11) (허니문) 蜜月 X
            tok = std::strtok(nullptr, DELIMS);
            if (!tok) { ok = false; break; }
            int honeymoonX = std::atoi(tok);

            // 12) (허니문) 蜜月 Y
            tok = std::strtok(nullptr, DELIMS);
            if (!tok) { ok = false; break; }
            int honeymoonY = std::atoi(tok);

            // 13) extraregenmonsterkind
            tok = std::strtok(nullptr, DELIMS);
            if (!tok) { ok = false; break; }
            uint16_t extraRegen = cltExtraRegenMonsterKindInfo::TranslateKindCode(tok);

            // 14) 진행시간 處理時間 (秒)
            tok = std::strtok(nullptr, DELIMS);
            if (!tok) { ok = false; break; }
            int durationSec = std::atoi(tok);

            // 15) 부케 (花束) id（Ixxxx）
            tok = std::strtok(nullptr, DELIMS);
            if (!tok) { ok = false; break; }
            uint16_t bouquetId = cltItemKindInfo::TranslateKindCode(tok);

            // 16) 부케 數量
            tok = std::strtok(nullptr, DELIMS);
            if (!tok) { ok = false; break; }
            uint16_t bouquetCnt = static_cast<uint16_t>(std::atoi(tok));

            // 17) 청첩장 邀請函 ID（Ixxxx）
            tok = std::strtok(nullptr, DELIMS);
            if (!tok) { ok = false; break; }
            uint16_t invitationId = cltItemKindInfo::TranslateKindCode(tok);

            // 18) 청첩장 邀請函數量
            tok = std::strtok(nullptr, DELIMS);
            if (!tok) { ok = false; break; }
            uint16_t invitationCnt = static_cast<uint16_t>(std::atoi(tok));

            if (idx < m_count) {
                strWeddingHallKindInfo& r = m_items[idx];
                r.weddingKind = wKind;
                r.ticketItemKind = ticketKind;
                r.mapWeddingHall = mapHall;
                r.hallX = hallX;
                r.hallY = hallY;
                r.mapStudio = mapStudio;
                r.studioX = studioX;
                r.studioY = studioY;
                r.mapHoneymoon = mapHoneymoon;
                r.honeymoonX = honeymoonX;
                r.honeymoonY = honeymoonY;
                r.extraRegenMonster = extraRegen;
                r.durationSec = durationSec;
                r.bouquetItemKind = bouquetId;
                r.bouquetCount = bouquetCnt;
                r.invitationItemKind = invitationId;
                r.invitationCount = invitationCnt;
                ++idx;
            }
        } while (std::fgets(line, sizeof(line), fp));

        // 只有正常讀到 EOF 才算成功
        ok = true;
    }
    else {
        // 無資料列亦視為成功
        ok = true;
    }

    g_clTextFileManager.fclose(fp);

    if (ok) {
        if (idx < m_count) m_count = idx;
        // 防呆：期望 56 bytes
        static_assert(sizeof(strWeddingHallKindInfo) == 56, "strWeddingHallKindInfo must be 56 bytes");
        return 1;
    }
    else {
        Free();
        return 0;
    }
}

void cltWeddingHallKindInfo::Free() {
    if (m_items) {
        operator delete(static_cast<void*>(m_items));
        m_items = nullptr;
    }
    m_count = 0;
}

strWeddingHallKindInfo* cltWeddingHallKindInfo::GetWeddingHallKindInfo(uint16_t code) {
    for (int i = 0; i < m_count; ++i) {
        if (m_items[i].weddingKind == code) return &m_items[i];
    }
    return nullptr;
}

strWeddingHallKindInfo* cltWeddingHallKindInfo::GetWeddingHallKindInfoByItemKind(uint16_t itemKind) {
    // 依反編譯：從結構 +2（ticketItemKind）開始比對，每筆 stride = 28 WORDs = 56 bytes
    for (int i = 0; i < m_count; ++i) {
        if (m_items[i].ticketItemKind == itemKind) return &m_items[i];
    }
    return nullptr;
}

strWeddingHallKindInfo* cltWeddingHallKindInfo::GetWeddingHallKindInfoByMapKind(uint16_t mapKind) {
    // 依反編譯：從結構 +4（mapWeddingHall）開始比對
    for (int i = 0; i < m_count; ++i) {
        if (m_items[i].mapWeddingHall == mapKind) return &m_items[i];
    }
    return nullptr;
}
