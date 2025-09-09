#include "Info/cltExtraRegenMonsterKindInfo.h"
#include <cstring>
#include <cstdlib>
#include <cctype>

uint16_t cltExtraRegenMonsterKindInfo::TranslateKindCode(char* s) {
    // 與其他 *KindInfo 相同規則：長度=5，首字母轉大寫編碼，尾四碼十進位且 < 0x800
    if (!s) return 0;
    if (std::strlen(s) != 5) return 0;
    const int hi = (std::toupper(static_cast<unsigned char>(s[0])) + 31) << 11;
    const unsigned int num = static_cast<unsigned int>(std::atoi(s + 1));
    if (num < 0x800u) return static_cast<uint16_t>(hi | num);
    return 0;
}

int cltExtraRegenMonsterKindInfo::Initialize(char* filename) {
    if (!filename) return 0;

    FILE* fp = g_clTextFileManager.fopen(filename);
    if (!fp) return 0;

    const char* DELIMS = "\t\n";
    char line[1024] = { 0 };

    // 跳過表頭 3 行
    if (!std::fgets(line, sizeof(line), fp) ||
        !std::fgets(line, sizeof(line), fp) ||
        !std::fgets(line, sizeof(line), fp)) {
        g_clTextFileManager.fclose(fp);
        return 0;
    }

    // 預掃行數
    fpos_t pos{};
    std::fgetpos(fp, &pos);
    m_count = 0;
    while (std::fgets(line, sizeof(line), fp)) ++m_count;

    if (m_count == 0) {
        g_clTextFileManager.fclose(fp);
        return 1; // 反編譯：若無資料直接回傳成功
    }

    // 配置（200 bytes/筆）
    m_items = static_cast<strExtraRegenMonsterKindInfo*>(operator new(sizeof(strExtraRegenMonsterKindInfo) * m_count));
    std::memset(m_items, 0, sizeof(strExtraRegenMonsterKindInfo) * m_count);

    // 回到資料起點
    std::fsetpos(fp, &pos);

    // 逐列剖析
    int idx = 0;
    char mapListBuf[1024] = { 0 }; // 暫存第3欄（map_secter_ID，可含多個以 '|' 分隔）

    if (std::fgets(line, sizeof(line), fp)) {
        do {
            // 1) ID（Fxxxx）
            char* tok = std::strtok(line, DELIMS);
            if (!tok) break;
            uint16_t kind = TranslateKindCode(tok);
            if (kind == 0) break;

            strExtraRegenMonsterKindInfo rec{};
            rec.kind = kind;

            // 2) map_secter_ID(企劃用) —— 丟棄
            tok = std::strtok(nullptr, DELIMS);
            if (!tok) break;

            // 3) map_secter_ID（可能 "B0002|B0003|..."）先存入暫存字串，稍後再切
            tok = std::strtok(nullptr, DELIMS);
            if (!tok) break;
            std::strncpy(mapListBuf, tok, sizeof(mapListBuf) - 1);
            mapListBuf[sizeof(mapListBuf) - 1] = '\0';

            // 4) 초기 리젠 개수
            tok = std::strtok(nullptr, DELIMS);
            if (!tok) break;
            rec.initCount = std::atoi(tok);

            // 5) min_regen
            tok = std::strtok(nullptr, DELIMS);
            if (!tok) break;
            rec.minRegen = std::atoi(tok);

            // 6) max_regen
            tok = std::strtok(nullptr, DELIMS);
            if (!tok) break;
            rec.maxRegen = std::atoi(tok);

            // 7) interval（至少 1；差值需可整除）
            tok = std::strtok(nullptr, DELIMS);
            if (!tok) break;
            int iv = std::atoi(tok);
            if (iv < 1) iv = 1;
            rec.interval = iv;
            const int diff = rec.maxRegen - rec.minRegen;
            if (diff % rec.interval != 0) {
                // 與反編譯一致：若不整除，不再解析 TYPE/怪物/地區；直接跳下一行
                // （為了與原邏輯對齊，不視為錯誤）
                goto commit_and_next_line;
            }

            // 8) TYPE（RANDOM / ONCE / 其他(FIX)）
            tok = std::strtok(nullptr, DELIMS);
            if (!tok) break;
            if (!::_stricmp(tok, "RANDOM"))      rec.type = 1;
            else if (!::_stricmp(tok, "ONCE"))   rec.type = 2;
            else                                    rec.type = 0;

            // 9) 之後最多 20 組：charkind_i_ID, hp
            for (int i = 0; i < 20; ++i) {
                char* kindTok = std::strtok(nullptr, DELIMS);
                if (!kindTok) break;
                uint16_t ck = cltCharKindInfo::TranslateKindCode(kindTok);
                rec.mob[i].charKind = ck;
                // 若 kind 無效，停止繼續（與反編譯相符）
                if (ck == 0) break;

                char* hpTok = std::strtok(nullptr, DELIMS);
                if (!hpTok) break;
                rec.mob[i].hp = std::atoi(hpTok);

                rec.mobCnt++;
            }

            // 10) 拆第 3 欄的地區清單（以 '|' 分割，最多 5 個）
            {
                char buf[1024];
                std::strncpy(buf, mapListBuf, sizeof(buf) - 1);
                buf[sizeof(buf) - 1] = '\0';
                char* p = std::strtok(buf, "|");
                while (p && rec.mapAreaCnt < 5) {
                    uint16_t mk = cltMapAreaKindInfo::TranslateKindCode(p);
                    if (mk == 0) break; // 與反編譯的保守邏輯一致：遇 0 即停止
                    rec.mapArea[rec.mapAreaCnt++] = mk;
                    p = std::strtok(nullptr, "|");
                }
            }

        commit_and_next_line:
            if (idx < m_count) {
                m_items[idx] = rec; // 位元拷貝等價於 qmemcpy
                ++idx;
            }

        } while (std::fgets(line, sizeof(line), fp));
    }

    g_clTextFileManager.fclose(fp);

    if (idx < m_count) m_count = idx;
    return 1;
}

void cltExtraRegenMonsterKindInfo::Free() {
    if (m_items) {
        operator delete(static_cast<void*>(m_items));
        m_items = nullptr;
    }
    m_count = 0;
}

strExtraRegenMonsterKindInfo*
cltExtraRegenMonsterKindInfo::GetExtraRegenMonsterKindInfo(uint16_t a2) {
    // 線性搜尋（反編譯步距為 100 WORD = 200 bytes）
    for (int i = 0; i < m_count; ++i) {
        if (m_items[i].kind == a2) return &m_items[i];
    }
    return nullptr;
}

strExtraRegenMonsterKindInfo*
cltExtraRegenMonsterKindInfo::GetExtraRegenMonsterKindInfoByIndex(int a2) {
    if (a2 < 0 || a2 >= m_count) return nullptr;
    return &m_items[a2];
}
