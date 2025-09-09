#include "Info/cltPKRankKindInfo.h"
#include <cstring>
#include <cstdlib>
#include <cctype>

// ===== 內部小工具 =====
static inline char* next_token(char* s, const char* delims) {
    return std::strtok(s, delims);
}

// 轉碼：'R' + 4位數 -> 16-bit：((toupper(s[0]) + 31) << 11) | atoi(s+1)
// 長度必須為 5，且數值 < 0x800，否則回傳 0
uint16_t cltPKRankKindInfo::TranslateKindCode(char* s) {
    if (!s) return 0;
    if (std::strlen(s) != 5) return 0;

    const int high = (std::toupper(static_cast<unsigned char>(s[0])) + 31) << 11;
    const unsigned int num = static_cast<unsigned int>(std::atoi(s + 1));
    if (num < 0x800u) {
        return static_cast<uint16_t>(high | num);
    }
    return 0;
}

int cltPKRankKindInfo::Initialize(char* filename) {
    if (!filename) return 0;

    // 開檔（經由文字檔管理器，會解壓至暫存檔再以文字模式開啟）
    FILE* fp = g_clTextFileManager.fopen(filename);
    if (!fp) return 0;

    const char* DELIMS = "\t\n";
    char line[1024] = { 0 };

    // 反編譯顯示：先讀 3 行表頭
    if (!std::fgets(line, sizeof(line), fp) ||
        !std::fgets(line, sizeof(line), fp) ||
        !std::fgets(line, sizeof(line), fp)) {
        g_clTextFileManager.fclose(fp);
        return 0;
    }

    // 記錄資料起點
    fpos_t pos{};
    std::fgetpos(fp, &pos);

    // 先計算資料列數（逐行到 EOF）
    m_count = 0;
    while (std::fgets(line, sizeof(line), fp)) {
        ++m_count;
    }

    // 配置連續記憶體（每筆 28 bytes）
    if (m_count < 0) m_count = 0;
    if (m_count > 0) {
        m_items = static_cast<strPKRankKindInfo*>(operator new(sizeof(strPKRankKindInfo) * m_count));
        std::memset(m_items, 0, sizeof(strPKRankKindInfo) * m_count);
    }
    else {
        m_items = nullptr;
    }

    // 回到資料起點
    std::fsetpos(fp, &pos);

    bool ok = false; // 依反編譯，正常解析到 EOF 後才置為成功

    // 開始逐列解析
    int idx = 0;
    if (std::fgets(line, sizeof(line), fp)) {
        do {
            // 逐列以 \t / \n 斷詞；反編譯在取得第一欄後，會先多呼叫一次 strtok 丟棄第二欄
            char* tok = nullptr;

            // 第1欄：Kind 字串（如 R0001）
            tok = next_token(line, DELIMS);
            if (!tok) break;

            uint16_t kind = TranslateKindCode(tok);
            if (kind == 0) break; // 非法代碼直接結束

            // 第2欄：階級(數字) － 反編譯刻意略過
            tok = next_token(nullptr, DELIMS);
            if (!tok) break;

            // 第3欄：textId（WORD）
            tok = next_token(nullptr, DELIMS);
            if (!tok) break;
            uint16_t textId = static_cast<uint16_t>(std::atoi(tok));

            // 第4欄：needPoint（DWORD/int）
            tok = next_token(nullptr, DELIMS);
            if (!tok) break;
            int32_t needPoint = std::atoi(tok);

            // 第5欄：resIdHex（以 %x 解析）
            tok = next_token(nullptr, DELIMS);
            if (!tok) break;
            uint32_t resIdHex = 0;
            std::sscanf(tok, "%x", &resIdHex);

            // 第6欄：blockId（WORD）
            tok = next_token(nullptr, DELIMS);
            if (!tok) break;
            uint16_t blockId = static_cast<uint16_t>(std::atoi(tok));

            // 第7欄：losePenalty（DWORD/int，可為負）
            tok = next_token(nullptr, DELIMS);
            if (!tok) break;
            int32_t losePenalty = std::atoi(tok);

            // 第8欄：meritPoint（DWORD/int）
            tok = next_token(nullptr, DELIMS);
            if (!tok) break;
            int32_t meritPoint = std::atoi(tok);

            // 第9欄：publicPoint（DWORD/int）
            tok = next_token(nullptr, DELIMS);
            if (!tok) break;
            int32_t publicPoint = std::atoi(tok);

            if (idx < m_count) {
                strPKRankKindInfo& r = m_items[idx];
                r.kind = kind;
                r.textId = textId;
                r.needPoint = needPoint;
                r.resIdHex = resIdHex;
                r.blockId = blockId;
                r._pad = 0; // 對齊用
                r.losePenalty = losePenalty;
                r.meritPoint = meritPoint;
                r.publicPoint = publicPoint;
                ++idx;
            }

            // 讀取下一列；若 EOF（對應反編譯的 goto LABEL_19）則視為成功
        } while (std::fgets(line, sizeof(line), fp));

        ok = true; // 正常走到 EOF
    }
    else {
        // 沒有任何資料列也設定為成功（模擬反編譯在 LABEL_19 賦值 v19 = 1 的語意）
        ok = true;
    }

    g_clTextFileManager.fclose(fp);

    // 若解析過程中途中斷，truncate 到已填入的筆數
    if (ok) {
        // 若實填 < 預計 m_count，縮小 m_count 以反映實際筆數
        if (idx < m_count) m_count = idx;
        return 1;
    }
    else {
        Free();
        return 0;
    }
}

void cltPKRankKindInfo::Free() {
    if (m_items) {
        operator delete(static_cast<void*>(m_items));
        m_items = nullptr;
    }
    m_count = 0;
}

strPKRankKindInfo* cltPKRankKindInfo::GetPKRankKindInfo(uint16_t code) {
    // 線性搜尋（每筆 stride = 14 WORDs = 28 bytes）
    for (int i = 0; i < m_count; ++i) {
        if (m_items[i].kind == code) {
            return &m_items[i];
        }
    }
    return nullptr;
}

uint16_t cltPKRankKindInfo::GetPKRankKindByPoint(int point) {
    if (!m_items || m_count <= 0) return 0;

    // 若 point 小於第一筆的 needPoint，回傳 0（無階級）
    if (point < m_items[0].needPoint)
        return 0;

    if (m_count == 1)
        return m_items[0].kind;

    // 反編譯邏輯：從第二筆開始，若 point >= 每筆 needPoint，繼續往後，直到越界則回傳最後一筆 kind
    int idx = 1; // 目前對應第 idx 筆（人類從 1 起算、0 為第一筆）
    while (idx < m_count) {
        if (point >= m_items[idx].needPoint) {
            ++idx;
        }
        else {
            break;
        }
    }

    // 若停在第 idx 筆代表 point 並未達到該筆，回傳前一筆 kind
    // 若跑到尾端（idx == m_count），回傳最後一筆 kind
    return m_items[idx - 1].kind;
}
