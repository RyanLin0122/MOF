#include "Info/cltClimateKindInfo.h"
#include <cstring>
#include <cstdlib>
#include <cctype>

// ──────────────────────────────────────────────────────────────────────────
// 反編譯：mofclient.c:342909 IsDigit
//   - 空字串回傳 1（TRUE）
//   - 迴圈：遇 '+' 或 '-' 即跳過（無論位置），再檢查當前位元組是否為 0..9
//   - 第一個非數字（且非 +/-）-> 0；走到 NUL -> 1
// ──────────────────────────────────────────────────────────────────────────
static int IsDigit_GT(const char* s) {
    const unsigned char* p = reinterpret_cast<const unsigned char*>(s);
    if (!*p) return 1;
    for (;;) {
        if (*p == '+' || *p == '-') ++p;
        if (!std::isdigit(*p)) return 0;
        if (!*++p) return 1;
    }
}

// 反編譯：mofclient.c:342945 IsAlphaNumeric
//   - 空字串回傳 1（TRUE）
//   - 全字元 isalnum -> 1；遇任一非 alnum -> 0
static int IsAlphaNumeric_GT(const char* s) {
    const unsigned char* p = reinterpret_cast<const unsigned char*>(s);
    if (!*p) return 1;
    while (std::isalnum(*p)) {
        if (!*++p) return 1;
    }
    return 0;
}

cltClimateKindInfo::cltClimateKindInfo() : m_count(0), m_items(nullptr) {}

// 反編譯：mofclient.c:294703
//   v2 = (toupper(s[0]) + 31) << 11;
//   v3 = atoi(s+1) (當 WORD 截斷)；若 v3 < 0x800 → 回 v2 | v3 (再以 WORD 截斷)；否則 0
uint16_t cltClimateKindInfo::TranslateKindCode(char* s) {
    if (!s) return 0;
    if (std::strlen(s) != 5) return 0;
    int hi = (std::toupper(static_cast<unsigned char>(s[0])) + 31) << 11;
    uint16_t num = static_cast<uint16_t>(std::atoi(s + 1));
    if (num < 0x800u) return static_cast<uint16_t>(hi | num);
    return 0;
}

// 反編譯：mofclient.c:294435 cltClimateKindInfo::Initialize
//
// 原始流程（與下方 1:1 對應）：
//   1. fopen；若失敗 → return 0
//   2. 連續 fgets ×3 跳過表頭（任一失敗 → fclose 後 return 0）
//   3. 預掃資料行行數 → ++*(DWORD*)this（即 m_count）
//   4. operator new(m_count << 6)（每筆 64 bytes）+ memset(0)
//   5. fsetpos 回到資料起點，逐行 strtok 分割：
//        - 解析失敗（任一 token 為 NULL，或 IsDigit/IsAlphaNumeric 不通過）
//          → break 直接離開 while(1)，v49 (回傳值) 維持 0
//        - 解析成功 → v7 += 64，再 fgets 讀下一行；fgets 為 NULL 時
//          goto LABEL_50（v49 = 1），代表「資料尾正常結束」
//        - 第一個資料行 fgets 即失敗 → 走 else 分支 LABEL_50（v49 = 1）
//   6. fclose；return v49
int cltClimateKindInfo::Initialize(char* filename) {
    if (!filename) return 0;

    FILE* fp = g_clTextFileManager.fopen(filename);
    if (!fp) return 0;

    char Delimiter[4] = { '\t', '\n', '\0', '\0' };
    char Buffer[1024] = { 0 };
    int  v49 = 0;

    // 反編譯：if ( fgets && fgets && fgets ) ... else 直接掉到 fclose（return 0）
    if (std::fgets(Buffer, 1023, fp)
        && std::fgets(Buffer, 1023, fp)
        && std::fgets(Buffer, 1023, fp))
    {
        fpos_t Position{};
        std::fgetpos(fp, &Position);

        // 預掃行數（反編譯：for (; fgets(...); ++*(DWORD*)this);）
        m_count = 0;
        while (std::fgets(Buffer, 1023, fp)) ++m_count;

        // 反編譯：v5 = operator new(*(DWORD*)this << 6); memset(v5, 0, m_count<<6);
        m_items = static_cast<strClimateInfo*>(
            operator new(sizeof(strClimateInfo) * static_cast<size_t>(m_count)));
        std::memset(m_items, 0, sizeof(strClimateInfo) * static_cast<size_t>(m_count));

        std::fsetpos(fp, &Position);

        // 反編譯：v7 = m_items；以 BYTE 偏移直接寫入。
        // 解析失敗 → break；解析成功 → v7 += 64 後再 fgets。
        char* v7 = reinterpret_cast<char*>(m_items);

        if (std::fgets(Buffer, 1023, fp)) {
            for (;;) {
                // [0] 기후 아이디 / 氣候 ID（5 碼，TranslateKindCode）
                char* tok = std::strtok(Buffer, Delimiter);
                if (!tok) break;
                *reinterpret_cast<uint16_t*>(v7 + 0) = TranslateKindCode(tok);

                // [1] 이름(기획용) / 名稱（企劃用，僅作 NULL 檢查不存）
                if (!std::strtok(nullptr, Delimiter)) break;

                // [2] 기후명 ID / 氣候名 ID
                tok = std::strtok(nullptr, Delimiter);
                if (!tok || !IsDigit_GT(tok)) break;
                *reinterpret_cast<uint16_t*>(v7 + 2) = static_cast<uint16_t>(std::atoi(tok));

                // [3] 이동 속도 감소율 / 移動速度減少率（千分率）
                tok = std::strtok(nullptr, Delimiter);
                if (!tok || !IsDigit_GT(tok)) break;
                *reinterpret_cast<uint16_t*>(v7 + 4) = static_cast<uint16_t>(std::atoi(tok));

                // [4] HP 자동회복 증가 / HP 自動回復增加（千分率，可負）
                tok = std::strtok(nullptr, Delimiter);
                if (!tok || !IsDigit_GT(tok)) break;
                *reinterpret_cast<int32_t*>(v7 + 8) = std::atoi(tok);

                // [5] MP 자동증가 감소 / MP 自動增加減少（千分率，可負）
                tok = std::strtok(nullptr, Delimiter);
                if (!tok || !IsDigit_GT(tok)) break;
                *reinterpret_cast<int32_t*>(v7 + 12) = std::atoi(tok);

                // [6] 획득 경험치 증가 / 獲得經驗增加（千分率）
                tok = std::strtok(nullptr, Delimiter);
                if (!tok || !IsDigit_GT(tok)) break;
                *reinterpret_cast<uint16_t*>(v7 + 16) = static_cast<uint16_t>(std::atoi(tok));

                // [7] 낙뢰 생성 시간 간격 / 落雷生成間隔（秒）
                tok = std::strtok(nullptr, Delimiter);
                if (!tok || !IsDigit_GT(tok)) break;
                *reinterpret_cast<int32_t*>(v7 + 20) = std::atoi(tok);

                // [8] 낙뢰 생성 확률 / 落雷生成機率（千分率）
                tok = std::strtok(nullptr, Delimiter);
                if (!tok || !IsDigit_GT(tok)) break;
                *reinterpret_cast<uint16_t*>(v7 + 24) = static_cast<uint16_t>(std::atoi(tok));

                // [9] 낙뢰 명중 확률 / 落雷命中機率（千分率）
                tok = std::strtok(nullptr, Delimiter);
                if (!tok || !IsDigit_GT(tok)) break;
                *reinterpret_cast<uint16_t*>(v7 + 26) = static_cast<uint16_t>(std::atoi(tok));

                // [10] 아이템 드롭률 증가 / 物品掉落率增加（千分率）
                tok = std::strtok(nullptr, Delimiter);
                if (!tok || !IsDigit_GT(tok)) break;
                *reinterpret_cast<uint16_t*>(v7 + 28) = static_cast<uint16_t>(std::atoi(tok));

                // [11] 부여 속성 / 賦予屬性（僅作 NULL 檢查不存）
                if (!std::strtok(nullptr, Delimiter)) break;

                // [12] 지속시간 / 持續時間（秒）
                tok = std::strtok(nullptr, Delimiter);
                if (!tok || !IsDigit_GT(tok)) break;
                *reinterpret_cast<int32_t*>(v7 + 32) = std::atoi(tok);

                // [13] 단위 개수 / 單位數量
                tok = std::strtok(nullptr, Delimiter);
                if (!tok || !IsDigit_GT(tok)) break;
                *reinterpret_cast<uint16_t*>(v7 + 36) = static_cast<uint16_t>(std::atoi(tok));

                // [14] 리소스 아이디 / 資源 ID（hex；IsAlphaNumeric → sscanf "%x"）
                tok = std::strtok(nullptr, Delimiter);
                if (!tok || !IsAlphaNumeric_GT(tok)) break;
                std::sscanf(tok, "%x", reinterpret_cast<unsigned int*>(v7 + 40));

                // [15] 시작 블럭 / 起始區塊
                tok = std::strtok(nullptr, Delimiter);
                if (!tok || !IsDigit_GT(tok)) break;
                *reinterpret_cast<uint16_t*>(v7 + 44) = static_cast<uint16_t>(std::atoi(tok));

                // [16] 최대 블럭 수 / 最大區塊數
                tok = std::strtok(nullptr, Delimiter);
                if (!tok || !IsDigit_GT(tok)) break;
                *reinterpret_cast<uint16_t*>(v7 + 46) = static_cast<uint16_t>(std::atoi(tok));

                // [17] 애니메이션 / 動畫
                tok = std::strtok(nullptr, Delimiter);
                if (!tok || !IsDigit_GT(tok)) break;
                *reinterpret_cast<uint16_t*>(v7 + 48) = static_cast<uint16_t>(std::atoi(tok));

                // [18] 떨어지는 속도 / 下雪速度
                tok = std::strtok(nullptr, Delimiter);
                if (!tok || !IsDigit_GT(tok)) break;
                *reinterpret_cast<uint16_t*>(v7 + 50) = static_cast<uint16_t>(std::atoi(tok));

                // [19] 눈오는지의 여부 / 是否下雪（0/1）
                tok = std::strtok(nullptr, Delimiter);
                if (!tok || !IsDigit_GT(tok)) break;
                *reinterpret_cast<int32_t*>(v7 + 52) = std::atoi(tok);

                // [20] 기후 아이콘 리소스 ID / 氣候圖示資源 ID（hex）
                tok = std::strtok(nullptr, Delimiter);
                if (!tok || !IsAlphaNumeric_GT(tok)) break;
                std::sscanf(tok, "%x", reinterpret_cast<unsigned int*>(v7 + 56));

                // [21] 블럭 아이디 / 區塊 ID（最後一欄，反編譯為 (!v45 || !IsDigit) 合併判斷）
                tok = std::strtok(nullptr, Delimiter);
                if (!tok || !IsDigit_GT(tok)) break;
                *reinterpret_cast<uint16_t*>(v7 + 60) = static_cast<uint16_t>(std::atoi(tok));

                v7 += 64;

                // 反編譯：if (!fgets) goto LABEL_50;  // v49 = 1
                if (!std::fgets(Buffer, 1023, fp)) {
                    v49 = 1;
                    break;
                }
            }
        } else {
            // 反編譯：else 分支即 LABEL_50：v49 = 1
            v49 = 1;
        }
    }

    g_clTextFileManager.fclose(fp);
    return v49;
}

void cltClimateKindInfo::Free() {
    if (m_items) {
        operator delete(static_cast<void*>(m_items));
        m_items = nullptr;
    }
    m_count = 0;
}

// 反編譯：mofclient.c:294682
//   for (i = base; *i != a2; i += 32)  // 32 WORD = 64 bytes
//       if (++idx >= m_count) return 0;
//   return base + 32*idx;
strClimateInfo* cltClimateKindInfo::GetClimateKindInfo(uint16_t code) {
    int n = m_count;
    if (n <= 0) return nullptr;

    uint16_t* base = reinterpret_cast<uint16_t*>(m_items);
    int idx = 0;
    for (uint16_t* p = base; *p != code; p += 32) {
        if (++idx >= n) return nullptr;
    }
    return reinterpret_cast<strClimateInfo*>(&base[32 * idx]);
}
