#include "Info/cltLessonKindInfo.h"

static constexpr const char* kDelims = "\t\n";

int cltLessonKindInfo::Initialize(char* filename)
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
    m_count = 0;
    while (std::fgets(buf, sizeof(buf), f)) ++m_count;

    // 配置
    if (m_count <= 0) { g_clTextFileManager.fclose(f); return 0; }
    m_list = static_cast<strLessonKindInfo*>(operator new[](m_count * sizeof(strLessonKindInfo)));
    std::memset(m_list, 0, m_count * sizeof(strLessonKindInfo));

    // 重置到資料起點
    std::fsetpos(f, &pos);

    int idx = 0;
    bool ok_all = true;

    if (std::fgets(buf, sizeof(buf), f))
    {
        while (true)
        {
            strLessonKindInfo& r = m_list[idx];
            std::memset(&r, 0, sizeof(r));

            // 1) 수업 종류
            char* tok = std::strtok(buf, kDelims);
            if (!tok) { ok_all = false; break; }
            uint8_t kind = (uint8_t)GetLessonKind(tok);
            r.kind = kind;
            if (kind == 0) { ok_all = false; break; }

            // 2) 설명（略過）
            if (!std::strtok(nullptr, kDelims)) { ok_all = false; break; }

            // 3) 랭킹용 수업 종류
            tok = std::strtok(nullptr, kDelims);
            if (!tok) { ok_all = false; break; }
            uint8_t rank_kind = (uint8_t)GetLessonKindOfRanking(tok);
            r.ranking_kind = rank_kind;
            if (rank_kind == 0) { ok_all = false; break; }

            // 4) 수업 타입
            tok = std::strtok(nullptr, kDelims);
            if (!tok) { ok_all = false; break; }
            uint8_t t = GetLessonType(tok);
            r.type = t;
            if (t == 4) { ok_all = false; break; }

            // 5) 이름 코드 (WORD)
            tok = std::strtok(nullptr, kDelims);
            if (!tok || !IsDigitStr(tok)) { ok_all = false; break; }
            r.name_code = (uint16_t)std::atoi(tok);

            // 6) 설명 코드 (WORD)
            tok = std::strtok(nullptr, kDelims);
            if (!tok || !IsDigitStr(tok)) { ok_all = false; break; }
            r.desc_code = (uint16_t)std::atoi(tok);

            // 7) 아이콘 리소스 아이디 (DWORD, hex)
            tok = std::strtok(nullptr, kDelims);
            if (!tok || !IsAlphaNumStr(tok)) { ok_all = false; break; }
            unsigned int hexv = 0;
            std::sscanf(tok, "%x", &hexv);
            r.icon_res_id = hexv;

            // 8) 블록 아이디 (WORD)
            tok = std::strtok(nullptr, kDelims);
            if (!tok || !IsDigitStr(tok)) { ok_all = false; break; }
            r.block_id = (uint16_t)std::atoi(tok);

            // 9) PlaceCode (WORD) —— 反編譯最後寫入到 +8
            tok = std::strtok(nullptr, kDelims);
            if (!tok || !IsDigitStr(tok)) { ok_all = false; break; }
            r.place_code = (uint16_t)std::atoi(tok);

            // 準備下一行
            ++idx;
            if (!std::fgets(buf, sizeof(buf), f)) {
                // 正常讀到 EOF
                break;
            }
            if (idx >= m_count) break;
        }
    }
    else {
        // 第一筆資料行就讀不到：當作成功（與反編譯 LABEL_26 邏輯對齊）
        ok_all = true;
    }

    g_clTextFileManager.fclose(f);

    // 完整成功：idx 應等於 m_count
    if (!ok_all || idx != m_count) {
        Free();
        return 0;
    }
    return 1;
}

void cltLessonKindInfo::Free()
{
    if (m_list) {
        operator delete[](m_list);
        m_list = nullptr;
    }
    m_count = 0;
}

strLessonKindInfo* cltLessonKindInfo::GetLessonKindInfo(uint8_t a2)
{
    if (!m_list || m_count <= 0) return nullptr;
    for (int i = 0; i < m_count; ++i) {
        if (m_list[i].kind == a2)
            return &m_list[i];
    }
    return nullptr;
}

strLessonKindInfo* cltLessonKindInfo::GetLessonKindInfoByIndex(int a2)
{
    if (!m_list || a2 < 0 || a2 >= m_count) return nullptr;
    return (strLessonKindInfo*)((uint8_t*)m_list + 20u * a2);
}

int cltLessonKindInfo::GetLessonKind(char* a2)
{
    if (std::strcmp(a2, "SWORD_1") == 0)    return 10;
    if (std::strcmp(a2, "SWORD_2") == 0)    return 11;
    if (std::strcmp(a2, "BOW_1") == 0)      return 20;
    if (std::strcmp(a2, "BOW_2") == 0)      return 21;
    if (std::strcmp(a2, "MAGIC_1") == 0)    return 30;
    if (std::strcmp(a2, "MAGIC_2") == 0)    return 31;
    if (std::strcmp(a2, "THEOLOGY_1") == 0) return 40;
    if (std::strcmp(a2, "THEOLOGY_2") == 0) return 41;
    return 0;
}

int cltLessonKindInfo::GetLessonKindOfRanking(char* a2)
{
    // 與 GetLessonKind 相同的對映
    return GetLessonKind(a2);
}

uint8_t cltLessonKindInfo::GetLessonType(char* a2)
{
    if (std::strcmp(a2, "SWORD") == 0)    return 0;
    if (std::strcmp(a2, "BOW") == 0)      return 1; // 反編譯的 `string` 即 BOW
    if (std::strcmp(a2, "MAGIC") == 0)    return 2;
    if (std::strcmp(a2, "THEOLOGY") == 0) return 3;
    return 4; // 非法
}

int cltLessonKindInfo::IsValidLessonKind(uint8_t a2)
{
    if (!m_list || m_count <= 0) return 0;
    for (int i = 0; i < m_count; i += 1) {
        if (m_list[i].kind == a2) return 1;
    }
    return 0;
}
