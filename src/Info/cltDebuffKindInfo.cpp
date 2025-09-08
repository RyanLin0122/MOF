#include "Info/cltDebuffKindInfo.h"
#include "Text/cltTextFileManager.h"  // 由你提供的檔案

// Delimiters 與反編譯碼一致：「\t\n」
static constexpr const char* kDelims = "\t\n";

// 小工具：安全 fgets，回傳是否成功
static inline bool ReadLine(char* buf, int size, FILE* f) {
    return std::fgets(buf, size, f) != nullptr;
}

// 與 IDA 邏輯一致的 TranslateKindCode
uint16_t cltDebuffKindInfo::TranslateKindCode(char* a1)
{
    if (!a1) return 0;

    // 反編譯碼：長度必須等於 5
    if (std::strlen(a1) != 5)
        return 0;

    // 第一碼字母 -> toupper，再加 31，左移 11 bits
    int high = (std::toupper(static_cast<unsigned char>(*a1)) + 31) << 11;

    // 後四碼轉數字，需 < 0x800
    int v = std::atoi(a1 + 1);
    if (v < 0x800)
        return static_cast<uint16_t>(high | v);

    return 0;
}

int cltDebuffKindInfo::Initialize(char* filename)
{
    Free();

    if (!filename) return 0;

    // 以 cltTextFileManager 開檔
    FILE* f = g_clTextFileManager.fopen(filename);
    if (!f) return 0;

    // 反編譯碼：先讀 3 行表頭
    char buffer[1024] = {};
    if (!ReadLine(buffer, sizeof(buffer), f) ||
        !ReadLine(buffer, sizeof(buffer), f) ||
        !ReadLine(buffer, sizeof(buffer), f))
    {
        g_clTextFileManager.fclose(f);
        return 0;
    }

    // 記錄位置並計數
    fpos_t pos{};
    std::fgetpos(f, &pos);

    int count = 0;
    while (ReadLine(buffer, sizeof(buffer), f)) {
        ++count;
    }

    // 配置並清零
    if (count <= 0) {
        g_clTextFileManager.fclose(f);
        return 0;
    }

    m_list = static_cast<strDebuffKindInfo*>(operator new[](count * sizeof(strDebuffKindInfo)));
    std::memset(m_list, 0, count * sizeof(strDebuffKindInfo));
    m_count = count;

    // 回到資料起點
    std::fsetpos(f, &pos);

    // 逐行解析
    int idx = 0;
    bool ok_all = true;

    while (idx < m_count && ReadLine(buffer, sizeof(buffer), f))
    {
        strDebuffKindInfo& rec = m_list[idx];
        std::memset(&rec, 0, sizeof(rec));

        // 逐欄位以 strtok 解析（與反編譯碼一致的呼叫順序）
        char* tok = std::strtok(buffer, kDelims); // ID
        if (!tok) { ok_all = false; break; }

        rec.kind = TranslateKindCode(tok);
        if (rec.kind == 0) { ok_all = false; break; }

        tok = std::strtok(nullptr, kDelims); // 企劃用名稱（忽略）
        if (!tok) { ok_all = false; break; }

        tok = std::strtok(nullptr, kDelims); // 지속시간
        if (!tok) { ok_all = false; break; }
        rec.duration = std::atoi(tok);

        tok = std::strtok(nullptr, kDelims); // 지속시간(보스)
        if (!tok) { ok_all = false; break; }
        rec.duration_boss = std::atoi(tok);

        tok = std::strtok(nullptr, kDelims); // 반복주기
        if (!tok) { ok_all = false; break; }
        rec.period = std::atoi(tok);

        tok = std::strtok(nullptr, kDelims); // 반복주기(보스)
        if (!tok) { ok_all = false; break; }
        rec.period_boss = std::atoi(tok);

        tok = std::strtok(nullptr, kDelims); // 데미지 타입
        if (!tok) { ok_all = false; break; }

        // 與反編譯碼一致的字串比對（不分大小寫）
        if (_stricmp(tok, "DMGTYPE_NONE") == 0) {
            rec.damage_type = 0;
        }
        else if (_stricmp(tok, "DMGTYPE_CONSTANT") == 0) {
            rec.damage_type = 1;
        }
        else if (_stricmp(tok, "DMGTYPE_RATE_CHP") == 0) {
            rec.damage_type = 2;
        }
        else if (_stricmp(tok, "DMGTYPE_CALC") == 0) {
            rec.damage_type = 3;
        }
        else {
            ok_all = false; break;
        }

        tok = std::strtok(nullptr, kDelims); // Param1
        if (!tok) { ok_all = false; break; }
        rec.dmg_param1 = std::atoi(tok);

        tok = std::strtok(nullptr, kDelims); // Param2
        if (!tok) { ok_all = false; break; }
        rec.dmg_param2 = std::atoi(tok);

        tok = std::strtok(nullptr, kDelims); // Param3
        if (!tok) { ok_all = false; break; }
        rec.dmg_param3 = std::atoi(tok);

        tok = std::strtok(nullptr, kDelims); // Param4
        if (!tok) { ok_all = false; break; }
        rec.dmg_param4 = std::atoi(tok);

        // 後四欄：怪物欄位（可為 "0" 或 5 碼代碼）
        tok = std::strtok(nullptr, kDelims); // Top-1
        if (!tok) { ok_all = false; break; }
        rec.mon_top1 = TranslateKindCode(tok);

        tok = std::strtok(nullptr, kDelims); // Top-2
        if (!tok) { ok_all = false; break; }
        rec.mon_top2 = TranslateKindCode(tok);

        tok = std::strtok(nullptr, kDelims); // Bot-1
        if (!tok) { ok_all = false; break; }
        rec.mon_bot1 = TranslateKindCode(tok);

        tok = std::strtok(nullptr, kDelims); // Bot-2
        if (!tok) { ok_all = false; break; }
        rec.mon_bot2 = TranslateKindCode(tok);

        ++idx;
    }

    // 關檔
    g_clTextFileManager.fclose(f);

    // 若非完整讀畢或中途失敗，依反編譯行為回傳 0
    if (!ok_all || idx != m_count) {
        Free();
        return 0;
    }

    // 成功
    return 1;
}

void cltDebuffKindInfo::Free()
{
    if (m_list) {
        operator delete[](m_list);
        m_list = nullptr;
    }
    m_count = 0;
}

strDebuffKindInfo* cltDebuffKindInfo::GetDebuffKindInfo(uint16_t a2)
{
    if (!m_list || m_count <= 0) return nullptr;

    // 與反編譯碼一致：逐 36 WORD (=72 bytes) 掃描
    for (int i = 0; i < m_count; ++i) {
        if (m_list[i].kind == a2)
            return &m_list[i];
    }
    return nullptr;
}
