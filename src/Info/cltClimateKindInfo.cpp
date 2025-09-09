#include "Info/cltClimateKindInfo.h"
#include <cstring>
#include <cstdlib>
#include <cctype>

static inline bool IsDigitCompat(const char* s) {
    if (!s || !*s) return false;
    const char* p = s;
    if (*p == '+' || *p == '-') ++p;    // 檔案中有可能出現負值
    if (!*p) return false;
    for (; *p; ++p) if (*p < '0' || *p > '9') return false;
    return true;
}

static inline bool IsAlphaNumericCompat(const char* s) {
    if (!s || !*s) return false;
    for (const char* p = s; *p; ++p)
        if (!std::isalnum(static_cast<unsigned char>(*p))) return false;
    return true;
}

cltClimateKindInfo::cltClimateKindInfo() : m_count(0), m_items(nullptr) {}

uint16_t cltClimateKindInfo::TranslateKindCode(char* s) {
    if (!s) return 0;
    if (std::strlen(s) != 5) return 0;
    int hi = (std::toupper(static_cast<unsigned char>(s[0])) + 31) << 11;
    unsigned int num = static_cast<unsigned int>(std::atoi(s + 1));
    if (num < 0x800u) return static_cast<uint16_t>(hi | num);
    return 0;
}

int cltClimateKindInfo::Initialize(char* filename) {
    if (!filename) return 0;

    FILE* fp = g_clTextFileManager.fopen(filename);
    if (!fp) return 0;

    const char* DELIMS = "\t\n";
    char line[1024] = { 0 };
    int ok = 0;

    // 跳過表頭 3 行
    if (!std::fgets(line, sizeof(line), fp) ||
        !std::fgets(line, sizeof(line), fp) ||
        !std::fgets(line, sizeof(line), fp)) {
        g_clTextFileManager.fclose(fp);
        return 0;
    }

    // 預掃行數 -> m_count（反編譯：++*(_DWORD*)this）
    fpos_t pos{};
    std::fgetpos(fp, &pos);
    m_count = 0;
    while (std::fgets(line, sizeof(line), fp)) ++m_count;

    // 配置：每筆 64 bytes（反編譯：new (*this << 6)）
    if (m_count > 0) {
        m_items = static_cast<strClimateInfo*>(operator new(sizeof(strClimateInfo) * m_count));
        std::memset(m_items, 0, sizeof(strClimateInfo) * m_count);
    }

    // 回到資料起點
    std::fsetpos(fp, &pos);

    int idx = 0;
    if (std::fgets(line, sizeof(line), fp)) {
        do {
            strClimateInfo rec{};
            // 1) 氣候代碼
            char* tok = std::strtok(line, DELIMS);
            if (!tok) break;
            rec.kind = TranslateKindCode(tok);

            // 2) 名稱（略過）
            if (!std::strtok(nullptr, DELIMS)) break;

            // 3) 기후명 ID
            tok = std::strtok(nullptr, DELIMS);
            if (!tok || !IsDigitCompat(tok)) break;
            rec.nameTextId = static_cast<uint16_t>(std::atoi(tok));

            // 4) 移動速度減少(‰)
            tok = std::strtok(nullptr, DELIMS);
            if (!tok || !IsDigitCompat(tok)) break;
            rec.moveSpeedPermil = static_cast<uint16_t>(std::atoi(tok));

            // 5) HP自回增加(‰)
            tok = std::strtok(nullptr, DELIMS);
            if (!tok || !IsDigitCompat(tok)) break;
            rec.hpRegenIncPermil = std::atoi(tok);

            // 6) MP自增減少(‰)
            tok = std::strtok(nullptr, DELIMS);
            if (!tok || !IsDigitCompat(tok)) break;
            rec.mpRegenDecPermil = std::atoi(tok);

            // 7) 經驗增加(‰)
            tok = std::strtok(nullptr, DELIMS);
            if (!tok || !IsDigitCompat(tok)) break;
            rec.expGainIncPermil = static_cast<uint16_t>(std::atoi(tok));

            // 8) 落雷生成間隔(秒)
            tok = std::strtok(nullptr, DELIMS);
            if (!tok || !IsDigitCompat(tok)) break;
            rec.lightningInterval = std::atoi(tok);

            // 9) 落雷生成機率(‰)
            tok = std::strtok(nullptr, DELIMS);
            if (!tok || !IsDigitCompat(tok)) break;
            rec.lightningCreatePermil = static_cast<uint16_t>(std::atoi(tok));

            // 10) 被雷擊中機率(‰)
            tok = std::strtok(nullptr, DELIMS);
            if (!tok || !IsDigitCompat(tok)) break;
            rec.lightningHitPermil = static_cast<uint16_t>(std::atoi(tok));

            // 11) 掉落率增加(‰)
            tok = std::strtok(nullptr, DELIMS);
            if (!tok || !IsDigitCompat(tok)) break;
            rec.itemDropIncPermil = static_cast<uint16_t>(std::atoi(tok));

            // 12) 「賦予屬性」 -> 反編譯直接呼叫 strtok 並忽略此欄
            if (!std::strtok(nullptr, DELIMS)) break;

            // 13) 持續時間(秒)
            tok = std::strtok(nullptr, DELIMS);
            if (!tok || !IsDigitCompat(tok)) break;
            rec.durationSec = std::atoi(tok);

            // 14) 單位數量
            tok = std::strtok(nullptr, DELIMS);
            if (!tok || !IsDigitCompat(tok)) break;
            rec.unitCount = static_cast<uint16_t>(std::atoi(tok));

            // 15) 資源ID（以 16 進位解析）
            tok = std::strtok(nullptr, DELIMS);
            if (!tok || !IsAlphaNumericCompat(tok)) break;
            std::sscanf(tok, "%x", &rec.resourceId);

            // 16) 起始區塊
            tok = std::strtok(nullptr, DELIMS);
            if (!tok || !IsDigitCompat(tok)) break;
            rec.startBlock = static_cast<uint16_t>(std::atoi(tok));

            // 17) 最大區塊數
            tok = std::strtok(nullptr, DELIMS);
            if (!tok || !IsDigitCompat(tok)) break;
            rec.maxBlockCount = static_cast<uint16_t>(std::atoi(tok));

            // 18) 動畫
            tok = std::strtok(nullptr, DELIMS);
            if (!tok || !IsDigitCompat(tok)) break;
            rec.animation = static_cast<uint16_t>(std::atoi(tok));

            // 19) 下雪速度
            tok = std::strtok(nullptr, DELIMS);
            if (!tok || !IsDigitCompat(tok)) break;
            rec.snowSpeed = static_cast<uint16_t>(std::atoi(tok));

            // 20) 是否下雪(0/1)
            tok = std::strtok(nullptr, DELIMS);
            if (!tok || !IsDigitCompat(tok)) break;
            rec.isSnowing = std::atoi(tok);

            // 21) 氣候圖示資源ID（16進位）
            tok = std::strtok(nullptr, DELIMS);
            if (!tok || !IsAlphaNumericCompat(tok)) break;
            std::sscanf(tok, "%x", &rec.iconResId);

            // 22) 區塊ID
            tok = std::strtok(nullptr, DELIMS);
            if (!tok || !IsDigitCompat(tok)) break;
            rec.blockId = static_cast<uint16_t>(std::atoi(tok));

            // 寫入陣列
            if (idx < m_count) {
                m_items[idx] = rec;
                ++idx;
            }

        } while (std::fgets(line, sizeof(line), fp));

        // 僅在正常走到 EOF 時視為成功（與反編譯 v49=1 行為一致）
        ok = 1;
    }

    g_clTextFileManager.fclose(fp);

    // 若實際讀入筆數比預掃少，修正 m_count（不影響外部行為）
    if (ok && idx < m_count) m_count = idx;

    return ok;
}

void cltClimateKindInfo::Free() {
    if (m_items) {
        operator delete(static_cast<void*>(m_items));
        m_items = nullptr;
    }
    m_count = 0;
}

strClimateInfo* cltClimateKindInfo::GetClimateKindInfo(uint16_t code) {
    int n = m_count;
    if (n <= 0) return nullptr;

    // 反編譯：以 WORD* 逐筆，步距 32 WORD (=64 bytes)
    uint16_t* base = reinterpret_cast<uint16_t*>(m_items);
    for (int i = 0; i < n; ++i) {
        if (base[i * 32 + 0] == code) {
            return reinterpret_cast<strClimateInfo*>(&base[i * 32]);
        }
    }
    return nullptr;
}
