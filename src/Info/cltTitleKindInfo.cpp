#include "Info/cltTitleKindInfo.h"

// ===== 小工具：共用的代碼轉換規則（供 *KindInfo stubs 使用） =====
static uint16_t ConvertAlphaNumeric16(const char* s) {
    if (!s || !*s) return 0;
    // 允許 5 或 6 位（技能 A01001 可能為 6，其餘多為 5），但數字值必須 < 0x800 與反編譯一致
    size_t len = std::strlen(s);
    if (len != 5 && len != 6) return 0;

    unsigned char ch = static_cast<unsigned char>(std::toupper(static_cast<unsigned char>(s[0])));
    int num = std::atoi(s + 1);
    if (num >= 0x800) return 0;
    return static_cast<uint16_t>(((ch + 31) << 11) | (num & 0x7FF));
}

int cltTitleKindInfo::Initialize(char* filename)
{
    // 與反編譯一致：以 cltTextFileManager 開檔（會從封包解壓到 txt.tmp 再以文字模式重開）
    FILE* fp = g_clTextFileManager.fopen(filename);
    if (!fp) return 0;

    const char* DELIM = "\t\n";
    char line[1024] = { 0 };

    // 跳過前三行（反編譯中連續三次 _fgets）
    if (!std::fgets(line, sizeof(line), fp) ||
        !std::fgets(line, sizeof(line), fp) ||
        !std::fgets(line, sizeof(line), fp))
    {
        g_clTextFileManager.fclose(fp);
        return 0;
    }

    // 記錄當前檔案位置
    fpos_t pos{};
    std::fgetpos(fp, &pos);

    // 第一次掃描：計數（每讀到一行就 +1）
    m_count = 0;
    while (std::fgets(line, sizeof(line), fp)) {
        ++m_count;
    }

    // 配置並清 0（與 operator new + memset 對齊行為）
    Free();
    if (m_count <= 0) {
        g_clTextFileManager.fclose(fp);
        return 0;
    }
    m_list = new strTitleKindInfo[m_count](); // value-init 為 0

    // 回到資料開始位置，第二次掃描：逐列解析
    std::fsetpos(fp, &pos);

    bool ok = false;           // 只有完整跑到 EOF 才設為成功（貼近反編譯 v27=1 的語意）
    int   idx = 0;

    if (std::fgets(line, sizeof(line), fp)) {
        for (;;) {
            // 逐欄以 \t 與 \n 為分隔（與反編譯 strcpy(Delimiter,"\t\n") 對齊）
            char* tok = std::strtok(line, DELIM);
            if (!tok) break;

            // 1) 호칭_ID -> title_code
            uint16_t tcode = TranslateKindCode(tok);
            m_list[idx].title_code = tcode;
            if (tcode == 0) break;

            // 2) 호칭(韓文名稱) 存在性檢查（反編譯僅檢查存在，不保存）
            if (!std::strtok(nullptr, DELIM)) break;

            // 3) 이름_code -> name_code (WORD)
            tok = std::strtok(nullptr, DELIM);
            if (!tok) break;
            m_list[idx].name_code = static_cast<uint16_t>(std::atoi(tok));

            // 4) lv_min (DWORD)
            tok = std::strtok(nullptr, DELIM);
            if (!tok) break;
            m_list[idx].lv_min = std::atoi(tok);

            // 5) lv_max (DWORD)
            tok = std::strtok(nullptr, DELIM);
            if (!tok) break;
            m_list[idx].lv_max = std::atoi(tok);

            // 6) condition (字串轉枚舉)
            tok = std::strtok(nullptr, DELIM);
            if (!tok) break;
            int cond = TranslateGetCondition(tok);
            m_list[idx].condition = cond;
            if (cond != 0) {
                switch (cond) {
                case TGC_COMPLETEQUEST: {
                    tok = std::strtok(nullptr, DELIM);
                    if (!tok) break;
                    m_list[idx].param1_code = cltQuestKindInfo::TranslateKindCode(tok);
                } break;

                case TGC_GETSKILL: {
                    tok = std::strtok(nullptr, DELIM);
                    if (!tok) break;
                    m_list[idx].param1_code = cltSkillKindInfo::TranslateKindCode(tok);
                } break;

                case TGC_SETEMBLEM: {
                    tok = std::strtok(nullptr, DELIM);
                    if (!tok) break;
                    m_list[idx].param1_code = cltEmblemKindInfo::TranslateKindCode(tok);
                } break;

                case TGC_GETITEM: {
                    tok = std::strtok(nullptr, DELIM);
                    if (!tok) break;
                    m_list[idx].param1_code = cltItemKindInfo::TranslateKindCode(tok);
                    tok = std::strtok(nullptr, DELIM);
                    if (!tok) break;
                    m_list[idx].param2 = std::atoi(tok);
                } break;

                case TGC_NONE: {
                    // 反編譯：只檢查再吃兩個欄位存在即可（不存值）
                    if (!std::strtok(nullptr, DELIM) || !std::strtok(nullptr, DELIM))
                        break;
                } break;

                case TGC_KILLMONSTER: {
                    tok = std::strtok(nullptr, DELIM);
                    if (!tok) break;
                    m_list[idx].param1_code = cltCharKindInfo::TranslateKindCode(tok);

                    tok = std::strtok(nullptr, DELIM);
                    if (!tok) break;
                    m_list[idx].param2 = std::atoi(tok);

                    tok = std::strtok(nullptr, DELIM);
                    if (!tok) break;
                    m_list[idx].param3 = std::atoi(tok);

                    // 反編譯邏輯：若怪物代碼為 0，則 param3 需非 0 才算有效
                    if (m_list[idx].param1_code == 0 && m_list[idx].param3 == 0)
                        break;
                } break;

                default:
                    // 未知條件（反編譯 TranslateGetCondition 會回傳 256）—直接接受並前進
                    break;
                }
            }

            // 成功解析一列 → 前進
            ++idx;
            if (!std::fgets(line, sizeof(line), fp)) {
                ok = true;    // 只有完整吃到 EOF 才視為成功（貼近 v27=1）
                break;
            }
            continue; // 繼續處理下一行

            // 任何 break(外層) 都會跳出 for(;;) 視為失敗
            break;
        }
    }
    else {
        // 沒有任何資料行，直接視為成功（反編譯 LABEL_35 分支）
        ok = true;
    }

    g_clTextFileManager.fclose(fp);

    // 若中途失敗，保持與反編譯一致地回傳 0；成功則 1
    if (!ok) return 0;

    // 若實際成功行數 < 計數（例如尾端空行），允許保留空白（反編譯使用固定配置）
    // 可選：m_count = idx; 但為了嚴格貼近反編譯，不改 m_count。
    return 1;
}

void cltTitleKindInfo::Free()
{
    if (m_list) {
        delete[] m_list;
        m_list = nullptr;
    }
    m_count = 0;
}

uint16_t cltTitleKindInfo::TranslateKindCode(const char* s)
{
    if (!s || std::strlen(s) != 5) return 0;
    unsigned char ch = static_cast<unsigned char>(std::toupper(static_cast<unsigned char>(s[0])));
    int num = std::atoi(s + 1);
    if (num >= 0x800) return 0;
    return static_cast<uint16_t>(((ch + 31) << 11) | (num & 0x7FF));
}

strTitleKindInfo* cltTitleKindInfo::GetTitleKindInfo(uint16_t code)
{
    if (!m_list || m_count <= 0) return nullptr;
    for (int i = 0; i < m_count; ++i) {
        if (m_list[i].title_code == code)
            return &m_list[i];
    }
    return nullptr;
}

int cltTitleKindInfo::TranslateGetCondition(const char* s)
{
    if (!s) return 0;
    // 對齊反編譯：大小寫不敏感
#ifdef _WIN32
    auto ieq = [](const char* a, const char* b) { return _stricmp(a, b) == 0; };
#else
    auto ieq = [](const char* a, const char* b) { return strcasecmp(a, b) == 0; };
#endif
    if (ieq(s, "COMPLETEQUEST")) return TGC_COMPLETEQUEST;
    if (ieq(s, "GETSKILL"))      return TGC_GETSKILL;
    if (ieq(s, "SETEMBLEM"))     return TGC_SETEMBLEM;
    if (ieq(s, "GETITEM"))       return TGC_GETITEM;
    if (ieq(s, "KILLMONSTER"))   return TGC_KILLMONSTER;

    // 反編譯的特殊計算：若是 "NONE" → 5；否則 → 256（非 0，走 default 接受）
    // 直接還原語意結果即可：
    if (ieq(s, "NONE")) return TGC_NONE;
    return 256;
}