#include "Info/cltWeddingHallKindInfo.h"
#include <cstring>
#include <cstdlib>
#include <cctype>

// mofclient.c:343301 — ((toupper(s[0]) + 31) << 11) | atoi(s+1)
// 長度必須為 5；atoi 結果必須 < 0x800（GT 用 unsigned __int16 承接後遮罩）。
// 回傳值在 GT 為 unsigned __int16，因此最終結果會被截短至 16 bits。
uint16_t cltWeddingHallKindInfo::TranslateKindCode(char* s) {
    if (std::strlen(s) != 5) return 0;
    const int hi = (std::toupper(static_cast<unsigned char>(s[0])) + 31) << 11;
    const unsigned int num = static_cast<unsigned int>(std::atoi(s + 1));
    if (num < 0x800u) {
        return static_cast<uint16_t>(hi | num);  // GT: result(uint16) = v2 | v3
    }
    return 0;
}

// mofclient.c:343045 cltWeddingHallKindInfo::Initialize
// GT 控制流摘要：
//   1) fopen；若失敗 return 0
//   2) 連續三次 fgets 跳過表頭；任一失敗 return 0
//   3) fgetpos → 預掃 fgets 直到 EOF 以決定 m_count
//   4) operator new(56 * m_count) + memset 0
//   5) fsetpos 回到資料起點
//   6) 若首次 fgets 為 NULL（無資料列）→ goto LABEL_30：v32=1 後 fclose 並 return 1
//   7) 進入 while(2) 解析迴圈：
//        - token1 為空 → break → fclose / return v32(=0)
//        - 第 1 欄 TranslateKindCode 寫入 +0；若 v9==0 → break → return 0
//        - tokens 2..16 任一失敗 → 透過 if 巢狀 fall-through 到 break → return 0
//        - tokens 17/18 (內層 do{}while(v31<1)) 任一失敗 → goto LABEL_32 → return 0
//        - 一列完成 → v7 += 56；fgets 下一列
//          * 若 fgets 成功 → continue
//          * 若 fgets 為 NULL → goto LABEL_30 → return 1
int cltWeddingHallKindInfo::Initialize(char* filename) {
    FILE* fp = g_clTextFileManager.fopen(filename);
    if (!fp) return 0;

    const char* DELIMS = "\t\n";
    char line[1024] = { 0 };

    // 跳過三行表頭：標題列 / 注釋列 / 欄位名列
    if (!std::fgets(line, sizeof(line), fp) ||
        !std::fgets(line, sizeof(line), fp) ||
        !std::fgets(line, sizeof(line), fp)) {
        g_clTextFileManager.fclose(fp);
        return 0;
    }

    // 預掃資料列數
    fpos_t pos{};
    std::fgetpos(fp, &pos);
    m_count = 0;
    while (std::fgets(line, sizeof(line), fp)) ++m_count;

    // 配置記錄陣列並全部歸零（與 GT memset 行為一致；m_count==0 時 GT 仍呼叫 operator new(0)）
    m_items = static_cast<strWeddingHallKindInfo*>(operator new(sizeof(strWeddingHallKindInfo) * static_cast<size_t>(m_count)));
    if (m_count > 0 && m_items) {
        std::memset(m_items, 0, sizeof(strWeddingHallKindInfo) * m_count);
    }

    std::fsetpos(fp, &pos);

    // 對應 GT v32：return value（0 = 失敗，1 = 成功）
    int v32 = 0;
    int idx = 0;

    // GT: if (!_fgets(...)) goto LABEL_30; → v32=1; return 1
    if (!std::fgets(line, sizeof(line), fp)) {
        g_clTextFileManager.fclose(fp);
        return 1;
    }

    // GT: while(2) { strtok... }
    while (true) {
        // 1) 웨딩id / 婚禮 ID — TranslateKindCode → +0
        char* tok = std::strtok(line, DELIMS);
        if (!tok) break;  // GT: if(v8==0) break → fall through LABEL_32 with v32=0

        if (idx >= m_count) break;  // 防呆（在 GT 中 m_count 由預掃決定，理論上等量）
        strWeddingHallKindInfo& r = m_items[idx];

        const uint16_t wKind = TranslateKindCode(tok);
        r.weddingKind = wKind;             // GT: *(_WORD *)v7 = v9; （無條件寫入，含 0 的情況）
        if (wKind == 0) break;             // GT: if(!v9) fall through → break → return 0

        // 2) 이용권이름(기획자용) / 利用券名稱（企劃用） — 僅檢查非空，內容丟棄
        if (!std::strtok(nullptr, DELIMS)) break;

        // 3) 결혼식이용권id / 結婚式利用券 ID — cltItemKindInfo::TranslateKindCode → +2
        tok = std::strtok(nullptr, DELIMS);
        if (!tok) break;
        r.ticketItemKind = cltItemKindInfo::TranslateKindCode(tok);

        // 4) 맵id(웨딩홀) / 婚禮堂 地圖 ID — cltMapInfo::TranslateKindCode → +4
        tok = std::strtok(nullptr, DELIMS);
        if (!tok) break;
        r.mapWeddingHall = cltMapInfo::TranslateKindCode(tok);

        // 5) 웨딩홀 x좌표 / 婚禮堂 X — atoi → +8
        tok = std::strtok(nullptr, DELIMS);
        if (!tok) break;
        r.hallX = std::atoi(tok);

        // 6) 웨딩홀 y좌표 / 婚禮堂 Y — atoi → +12
        tok = std::strtok(nullptr, DELIMS);
        if (!tok) break;
        r.hallY = std::atoi(tok);

        // 7) 맵id(스튜디오) / 工作室 地圖 ID — cltMapInfo::TranslateKindCode → +16
        tok = std::strtok(nullptr, DELIMS);
        if (!tok) break;
        r.mapStudio = cltMapInfo::TranslateKindCode(tok);

        // 8) 스튜디오 x좌표 / 工作室 X — atoi → +20
        tok = std::strtok(nullptr, DELIMS);
        if (!tok) break;
        r.studioX = std::atoi(tok);

        // 9) 스튜디오 y좌표 / 工作室 Y — atoi → +24
        tok = std::strtok(nullptr, DELIMS);
        if (!tok) break;
        r.studioY = std::atoi(tok);

        // 10) 맵id(허니문) / 蜜月 地圖 ID — cltMapInfo::TranslateKindCode → +28
        tok = std::strtok(nullptr, DELIMS);
        if (!tok) break;
        r.mapHoneymoon = cltMapInfo::TranslateKindCode(tok);

        // 11) 허니문 x좌표 / 蜜月 X — atoi → +32
        tok = std::strtok(nullptr, DELIMS);
        if (!tok) break;
        r.honeymoonX = std::atoi(tok);

        // 12) 허니문 y좌표 / 蜜月 Y — atoi → +36
        tok = std::strtok(nullptr, DELIMS);
        if (!tok) break;
        r.honeymoonY = std::atoi(tok);

        // 13) extraregenmonsterkind / 額外重生怪物 ID — cltExtraRegenMonsterKindInfo::TranslateKindCode → +40
        tok = std::strtok(nullptr, DELIMS);
        if (!tok) break;
        r.extraRegenMonster = cltExtraRegenMonsterKindInfo::TranslateKindCode(tok);

        // 14) 진행시간 / 進行時間（秒）— atoi → +44
        tok = std::strtok(nullptr, DELIMS);
        if (!tok) break;
        r.durationSec = std::atoi(tok);

        // 15) 부케 id / 花束 ID — cltItemKindInfo::TranslateKindCode → +52
        //     注意：txt 欄位順序為「부케 id, 부케 수량, 청첩장 ID, 청첩장 수량」，
        //     但 GT 將其分別寫到 +52,+54,+48,+50（v24 由 +50 出發指向 *(v24-1)/v24）。
        tok = std::strtok(nullptr, DELIMS);
        if (!tok) break;
        r.bouquetItemKind = cltItemKindInfo::TranslateKindCode(tok);

        // 16) 부케 수량 / 花束 數量 — atoi → +54
        tok = std::strtok(nullptr, DELIMS);
        if (!tok) break;
        r.bouquetCount = static_cast<uint16_t>(std::atoi(tok));

        // 17) 청첩장 ID / 喜帖 物品 ID — cltItemKindInfo::TranslateKindCode → +48
        //     GT: 內層 do{}while(v31<1) 第一次迭代；v25 為 NULL 則 goto LABEL_32（return 0）
        tok = std::strtok(nullptr, DELIMS);
        if (!tok) break;
        r.invitationItemKind = cltItemKindInfo::TranslateKindCode(tok);

        // 18) 청첩장 수량 / 喜帖 數量（新郎新婦各自分發）— atoi → +50
        tok = std::strtok(nullptr, DELIMS);
        if (!tok) break;
        r.invitationCount = static_cast<uint16_t>(std::atoi(tok));

        ++idx;

        // GT: v7 += 56; if (!_fgets) goto LABEL_30 (success); else continue;
        if (!std::fgets(line, sizeof(line), fp)) {
            v32 = 1;
            break;
        }
    }

    g_clTextFileManager.fclose(fp);

    if (v32 == 1) {
        // 防呆：期望 56 bytes（對應反編譯 stride 56 與 28 WORDs）
        static_assert(sizeof(strWeddingHallKindInfo) == 56, "strWeddingHallKindInfo must be 56 bytes");
        return 1;
    }

    // GT 在失敗時並未 Free，但釋放配置可避免測試/重入泄漏；
    // 因此這裡採取保守做法（不影響成功路徑的對外行為）。
    Free();
    return 0;
}

// mofclient.c:343227
void cltWeddingHallKindInfo::Free() {
    if (m_items) {
        operator delete(static_cast<void*>(m_items));
        m_items = nullptr;
    }
    m_count = 0;
}

// mofclient.c:343238 — 線性掃描 weddingKind (+0)，stride 28 WORDs（=56 bytes）
strWeddingHallKindInfo* cltWeddingHallKindInfo::GetWeddingHallKindInfo(uint16_t code) {
    for (int i = 0; i < m_count; ++i) {
        if (m_items[i].weddingKind == code) return &m_items[i];
    }
    return nullptr;
}

// mofclient.c:343259 — 線性掃描 ticketItemKind (+2)
strWeddingHallKindInfo* cltWeddingHallKindInfo::GetWeddingHallKindInfoByItemKind(uint16_t itemKind) {
    for (int i = 0; i < m_count; ++i) {
        if (m_items[i].ticketItemKind == itemKind) return &m_items[i];
    }
    return nullptr;
}

// mofclient.c:343280 — 線性掃描 mapWeddingHall (+4)
strWeddingHallKindInfo* cltWeddingHallKindInfo::GetWeddingHallKindInfoByMapKind(uint16_t mapKind) {
    for (int i = 0; i < m_count; ++i) {
        if (m_items[i].mapWeddingHall == mapKind) return &m_items[i];
    }
    return nullptr;
}
