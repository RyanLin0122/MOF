#include "Info/cltClassKindInfo.h"

static constexpr const char* kDelims = "\t\n";

cltClassKindInfo::cltClassKindInfo()
{
    m_list = nullptr;
    m_count = 0;
}

// 僅允許全部為 0..9；空字串或含非數字則視為非數字
bool cltClassKindInfo::IsDigitStr(const char* t)
{
    if (!t || !*t) return false;
    for (const unsigned char* p = (const unsigned char*)t; *p; ++p)
        if (!std::isdigit(*p)) return false;
    return true;
}

uint16_t cltClassKindInfo::TranslateKindCode(char* a1)
{
    if (!a1) return 0;
    if (std::strlen(a1) != 3) return 0;

    uint16_t c0 = (uint16_t)(std::toupper((unsigned char)a1[0]) - 'A') & 0x1F;
    uint16_t c1 = (uint16_t)(std::toupper((unsigned char)a1[1]) - 'A') & 0x1F;
    uint16_t c2 = (uint16_t)(std::toupper((unsigned char)a1[2]) - 'A') & 0x1F;

    // 與反編譯運算 (32 * (c1 | (32 * c0))) | c2 等價：((c0*32 + c1) * 32 + c2)
    return (uint16_t)(((c0 * 32) + c1) * 32 + c2);
}

int cltClassKindInfo::Initialize(char* filename)
{
    Free();
    if (!filename) return 0;

    FILE* f = g_clTextFileManager.fopen(filename);
    if (!f) return 0;

    char buf[1024] = {};
    // 略過 3 行表頭
    if (!std::fgets(buf, sizeof(buf), f) ||
        !std::fgets(buf, sizeof(buf), f) ||
        !std::fgets(buf, sizeof(buf), f))
    {
        g_clTextFileManager.fclose(f);
        return 0;
    }

    // 計數
    fpos_t pos{};
    std::fgetpos(f, &pos);
    uint16_t cnt = 0;
    while (std::fgets(buf, sizeof(buf), f))
        ++cnt;

    if (cnt == 0) {
        g_clTextFileManager.fclose(f);
        return 0;
    }

    // 配置
    m_list = static_cast<strClassKindInfo*>(operator new[](cnt * sizeof(strClassKindInfo)));
    std::memset(m_list, 0, cnt * sizeof(strClassKindInfo));
    m_count = cnt;

    // 回復到資料起點
    std::fsetpos(f, &pos);

    // 依反編譯：初始 atb = 1，每筆 *2
    uint64_t atb_next = 1ULL;

    uint16_t idx = 0;
    bool ok_all = true;

    while (idx < m_count && std::fgets(buf, sizeof(buf), f))
    {
        strClassKindInfo& r = m_list[idx];
        std::memset(&r, 0, sizeof(r));

        char* tok = std::strtok(buf, kDelims);            // 1: 클래스 아이디
        if (!tok) { ok_all = false; break; }
        r.kind = (_stricmp(tok, "NONE") == 0) ? 0 : TranslateKindCode(tok);

        // atb（低/高位）— 與反編譯一致，給定唯一 2^n
        r.atb = atb_next;

        // 2: 클래스 이름（略過）
        if (!std::strtok(nullptr, kDelims)) { ok_all = false; break; }

        // 3: 클래스 이름 코드（WORD）
        tok = std::strtok(nullptr, kDelims);
        if (!tok || !IsDigitStr(tok)) { ok_all = false; break; }
        r.name_code = (uint16_t)std::atoi(tok);

        // 4: 전직 가능 클래스（WORD, 3 碼）
        tok = std::strtok(nullptr, kDelims);
        if (!tok) { ok_all = false; break; }
        r.from_class = TranslateKindCode(tok);

        // 5: 전직 단계（BYTE）
        tok = std::strtok(nullptr, kDelims);
        if (!tok || !IsDigitStr(tok)) { ok_all = false; break; }
        r.job_step = (uint8_t)std::atoi(tok);

        // 6: 최소 레벨（BYTE）
        tok = std::strtok(nullptr, kDelims);
        if (!tok || !IsDigitStr(tok)) { ok_all = false; break; }
        r.min_level = (uint8_t)std::atoi(tok);

        // 7: 타겟탐색범위（DWORD）
        tok = std::strtok(nullptr, kDelims);
        if (!tok || !IsDigitStr(tok)) { ok_all = false; break; }
        r.target_search_range = (uint32_t)std::atoi(tok);

        // 8~16: 9 個 WORD（最低屬性/熟練度）
        uint16_t* pWord[9] = {
            &r.min_attack, &r.min_dex, &r.min_con, &r.min_int,
            &r.mastery_sword, &r.mastery_magic, &r.mastery_archery,
            &r.mastery_priest, &r.mastery_thief
        };
        for (int k = 0; k < 9; ++k) {
            tok = std::strtok(nullptr, kDelims);
            if (!tok || !IsDigitStr(tok)) { ok_all = false; break; }
            *pWord[k] = (uint16_t)std::atoi(tok);
        }
        if (!ok_all) break;

        // 17: 코멘트 코드（WORD）
        tok = std::strtok(nullptr, kDelims);
        if (!tok || !IsDigitStr(tok)) { ok_all = false; break; }
        r.comment_code = (uint16_t)std::atoi(tok);

        // 18: 마크이미지 블록 아이디（WORD）
        tok = std::strtok(nullptr, kDelims);
        if (!tok || !IsDigitStr(tok)) { ok_all = false; break; }
        r.mark_img_block_id = (uint16_t)std::atoi(tok);

        // 19~21: 三個 DWORD（依原表：HP/Mana 與一個額外整數；檔案中皆為整數）
        tok = std::strtok(nullptr, kDelims);
        if (!tok || !IsDigitStr(tok)) { ok_all = false; break; }
        r.base_buff_uses = (uint32_t)std::atoi(tok);

        tok = std::strtok(nullptr, kDelims);
        if (!tok || !IsDigitStr(tok)) { ok_all = false; break; }
        r.hp_regen = (uint32_t)std::atoi(tok);

        tok = std::strtok(nullptr, kDelims);
        if (!tok || !IsDigitStr(tok)) { ok_all = false; break; }
        r.mana_regen = (uint32_t)std::atoi(tok);

        // 22: 物品1代碼（5 碼；或 "0"）
        tok = std::strtok(nullptr, kDelims);
        if (!tok) { ok_all = false; break; }
        r.item1_code = cltItemKindInfo::TranslateKindCode(tok); // 非 5 碼→0

        // 23: 物品1數量（WORD）
        tok = std::strtok(nullptr, kDelims);
        if (!tok || !IsDigitStr(tok)) { ok_all = false; break; }
        r.item1_count = (uint16_t)std::atoi(tok);

        // 24: 物品2代碼
        tok = std::strtok(nullptr, kDelims);
        if (!tok) { ok_all = false; break; }
        r.item2_code = cltItemKindInfo::TranslateKindCode(tok);

        // 25: 物品2數量
        tok = std::strtok(nullptr, kDelims);
        if (!tok || !IsDigitStr(tok)) { ok_all = false; break; }
        r.item2_count = (uint16_t)std::atoi(tok);

        // 下一筆
        ++idx;
        atb_next <<= 1;
    }

    g_clTextFileManager.fclose(f);

    if (!ok_all || idx != m_count) {
        Free();
        return 0;
    }
    return 1;
}

void cltClassKindInfo::Free()
{
    if (m_list) {
        operator delete[](m_list);
        m_list = nullptr;
    }
    m_count = 0;
}

strClassKindInfo* cltClassKindInfo::GetClassKindInfo(uint16_t a2)
{
    if (!m_list || m_count == 0) return nullptr;
    for (uint16_t i = 0; i < m_count; ++i)
        if (m_list[i].kind == a2)
            return &m_list[i];
    return nullptr;
}

strClassKindInfo* cltClassKindInfo::GetClassKindInfoByIndex(unsigned int a2)
{
    if (!m_list || a2 >= m_count) return nullptr;
    return (strClassKindInfo*)((uint8_t*)m_list + 72u * a2);
}

strClassKindInfo* cltClassKindInfo::GetClassKindInfoByAtb(uint64_t a2)
{
    if (!m_list || m_count == 0) return nullptr;
    for (uint16_t i = 0; i < m_count; ++i)
        if (m_list[i].atb == a2)
            return &m_list[i];
    return nullptr;
}

int cltClassKindInfo::GetClassKindsByAtb(uint64_t mask, uint16_t* a3)
{
    // 與反編譯邏輯等價：對每個 set bit（2^n）找出對應類別
    int written = 0;
    if (!m_list || m_count == 0 || !a3) return 0;

    while (mask) {
        uint64_t bit = mask & (~mask + 1);    // 取最低位的 set bit
        mask ^= bit;                          // 清除該 bit
        if (auto* rec = GetClassKindInfoByAtb(bit)) {
            *a3++ = rec->kind;
            ++written;
        }
    }
    return written;
}

int cltClassKindInfo::GetClassAtb(char* String)
{
    if (!String || *String == '0') return 0;

    int lo = 0;           // 低 32-bit（回傳值）
    int hi_ignored = 0;   // 僅為符合反編譯：仍計算但不回傳

    char delims[2]; std::strcpy(delims, "|");
    for (char* tok = std::strtok(String, delims); tok; tok = std::strtok(nullptr, delims)) {
        uint16_t k = TranslateKindCode(tok);
        if (auto* rec = GetClassKindInfo(k)) {
            lo |= (int)(rec->atb & 0xFFFFFFFFULL);
            hi_ignored |= (int)((rec->atb >> 32) & 0xFFFFFFFFULL);
        }
    }
    return lo; // 與 IDA 版本一致：僅回傳低 32 位
}

uint64_t cltClassKindInfo::GetClassAtb(const char* s)
{
    if (!s || *s == '0') return 0;

    uint64_t mask = 0;

    // 逐段以 '|' 切開（不修改呼叫端字串）
    const char* p = s;
    while (true) {
        const char* q = std::strchr(p, '|');
        size_t len = q ? static_cast<size_t>(q - p) : std::strlen(p);

        if (len > 0) {
            // TranslateKindCode 需要 C-style 字元陣列
            char token[16] = {};
            if (len >= sizeof(token)) len = sizeof(token) - 1; // 保守保護
            std::memcpy(token, p, len);
            token[len] = '\0';

            uint16_t k = TranslateKindCode(token);
            if (auto* rec = GetClassKindInfo(k)) {
                mask |= rec->atb;  // rec->atb 已是 uint64_t
            }
        }

        if (!q) break;   // 最後一段
        p = q + 1;
    }
    return mask;
}