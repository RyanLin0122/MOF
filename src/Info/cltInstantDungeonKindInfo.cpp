#include "Info/cltInstantDungeonKindInfo.h"

#if defined(_WIN32)
#include <windows.h>
#endif

static constexpr const char* kDelims = "\t\n";

int cltInstantDungeonKindInfo::Initialize(char* filename)
{
    Free();
    if (!filename) return 0;

    FILE* f = g_clTextFileManager.fopen(filename);
    FILE* Stream = f;
    int   ok = 0;
    if (!f) return 0;

    char buf[1024] = {};

    // 先略過 3 行表頭
    if (std::fgets(buf, sizeof(buf), f) &&
        std::fgets(buf, sizeof(buf), f) &&
        std::fgets(buf, sizeof(buf), f))
    {
        // 計數
        fpos_t pos{};
        std::fgetpos(f, &pos);
        while (std::fgets(buf, sizeof(buf), f)) ++m_count;

        if (m_count == 0) { // 與反編譯相同：無資料也回傳成功
            g_clTextFileManager.fclose(Stream);
            return 1;
        }

        // 配置
        m_list = static_cast<strInstantDungeonKindInfo*>(
            operator new[](m_count * sizeof(strInstantDungeonKindInfo)));
        std::memset(m_list, 0, m_count * sizeof(strInstantDungeonKindInfo));

        // 回到資料起點
        std::fsetpos(f, &pos);

        int idx = 0;
        if (std::fgets(buf, sizeof(buf), f))
        {
            while (true)
            {
                strInstantDungeonKindInfo& r = m_list[idx];
                std::memset(&r, 0, sizeof(r));

                char* t = std::strtok(buf, kDelims); // indun kind
                if (!t) break;
                r.kind = TranslateKindCode(t);

                // 企劃名（略過）
                if (!std::strtok(nullptr, kDelims)) break;

                // 인던 이름 텍스트코드（WORD）
                t = std::strtok(nullptr, kDelims); if (!t) break;
                r.name_text_code = (uint16_t)std::atoi(t);

                // BaseIndun_ID（WORD, Sxxxx 或 0）
                t = std::strtok(nullptr, kDelims); if (!t) break;
                r.base_indun = TranslateKindCode(t);

                // 레벨 제한（BYTE）
                t = std::strtok(nullptr, kDelims); if (!t) break;
                r.level_limit = (uint8_t)std::atoi(t);

                // 生成 아이템 ID(열쇠아이템)
                t = std::strtok(nullptr, kDelims); if (!t) break;
                r.key_item = TranslateKindCode(t);

                // NPC ID
                t = std::strtok(nullptr, kDelims); if (!t) break;
                r.npc_id = TranslateKindCode(t);

                // 보스몬스터 ID
                t = std::strtok(nullptr, kDelims); if (!t) break;
                r.boss_id = TranslateKindCode(t);

                // 랜덤몬스터등장
                t = std::strtok(nullptr, kDelims); if (!t) break;
                r.random_spawn = (uint16_t)std::atoi(t);

                // 느림보몬스터ID
                t = std::strtok(nullptr, kDelims); if (!t) break;
                r.slow_id = TranslateKindCode(t);

                // 느림보몬스터확률
                t = std::strtok(nullptr, kDelims); if (!t) break;
                r.slow_prob = (uint16_t)std::atoi(t);

                // 파워맨몬스터ID
                t = std::strtok(nullptr, kDelims); if (!t) break;
                r.power_id = TranslateKindCode(t);

                // 파워맨몬스터확률
                t = std::strtok(nullptr, kDelims); if (!t) break;
                r.power_prob = (uint16_t)std::atoi(t);

                // 인던 안내 텍스트 코드
                t = std::strtok(nullptr, kDelims); if (!t) break;
                r.guide_text_code = (uint16_t)std::atoi(t);

                // timelimit（DWORD）
                t = std::strtok(nullptr, kDelims); if (!t) break;
                r.time_limit = (uint32_t)std::atoi(t);

                // 로딩리소스ID（十六進位）
                t = std::strtok(nullptr, kDelims);
                if (!t || !IsAlphaNum(t)) break;
                unsigned int hexv = 0;
                std::sscanf(t, "%x", &hexv);
                r.loading_res_id = hexv;

                // i_map1_id..i_map4_id
                for (int k = 0; k < 4; ++k) {
                    t = std::strtok(nullptr, kDelims);
                    if (!t) { Stream = f; goto PARSE_END; }
                    r.map_id[k] = cltInstantDungeonMapKindInfo::TranslateKindCode(t);
                }

                ++idx;
                if (!std::fgets(buf, sizeof(buf), f)) { ok = 1; break; } // 完成
                if (idx >= m_count) { ok = 1; break; }
                continue;

            PARSE_END:
                // 中斷（失敗）
                ok = 0;
                break;
            }
        }
        else {
            ok = 1; // 與反編譯一致：第一筆就讀不到資料也視為成功
        }
    }

    g_clTextFileManager.fclose(Stream);
    if (!ok) { Free(); }
    return ok;
}

void cltInstantDungeonKindInfo::Free()
{
    if (m_list) {
        operator delete[](m_list);
        m_list = nullptr;
    }
    m_count = 0;
}

strInstantDungeonKindInfo*
cltInstantDungeonKindInfo::GetInstantDungeonKindInfo(uint16_t a2)
{
    if (!m_list || m_count <= 0) return nullptr;

    // 逐 22 WORD (= 44 bytes) 掃描 kind
    uint16_t* base = reinterpret_cast<uint16_t*>(m_list);
    for (int i = 0; i < m_count; ++i, base += 22) {
        if (base[0] == a2) return reinterpret_cast<strInstantDungeonKindInfo*>(base);
    }
    return nullptr;
}

strInstantDungeonKindInfo*
cltInstantDungeonKindInfo::GetInstantDungeonKindInfoByItem(uint16_t a2)
{
    if (!m_list || m_count <= 0) return nullptr;

    // 從 +8（key_item）開始，步距 22 WORD
    uint16_t* base = reinterpret_cast<uint16_t*>(m_list);
    uint16_t* p = reinterpret_cast<uint16_t*>((uint8_t*)m_list + 8);
    for (int i = 0; i < m_count; ++i, p += 22) {
        if (*p == a2) return reinterpret_cast<strInstantDungeonKindInfo*>((uint8_t*)m_list + 44 * i);
    }
    return nullptr;
}

uint16_t
cltInstantDungeonKindInfo::GetInstantDungeonKindByItem(uint16_t a2)
{
    if (!m_list || m_count <= 0) return 0;

    uint16_t* p = reinterpret_cast<uint16_t*>((uint8_t*)m_list + 8);
    for (int i = 0; i < m_count; ++i, p += 22) {
        if (*p == a2) return *reinterpret_cast<uint16_t*>((uint8_t*)m_list + 44 * i);
    }
    return 0;
}

strInstantDungeonKindInfo*
cltInstantDungeonKindInfo::GetInstantDungeonKindInfoByIndex(int a2)
{
    // 反編譯條件：a2 < 0 || a2 > count 時回傳 0（注意「>」非「>=」）
    if (a2 < 0 || a2 > m_count) return nullptr;
    return reinterpret_cast<strInstantDungeonKindInfo*>((uint8_t*)m_list + 44u * a2);
}

uint16_t cltInstantDungeonKindInfo::TranslateKindCode(char* a1)
{
    if (!a1 || std::strlen(a1) != 5) return 0;

    int hi = (std::toupper(static_cast<unsigned char>(a1[0])) + 31) << 11;
    int lo = std::atoi(a1 + 1);
    if (lo < 0x800) return static_cast<uint16_t>(hi | lo);

    // 超過 2047：顯示錯誤視窗（與反編譯一致）
#if defined(_WIN32)
    MessageBoxA(nullptr, "InstantDungeonKindInfo Error", "Error", 0);
    char buf[256]; std::snprintf(buf, sizeof(buf), "Numberic 2048 Over (%s)", a1);
    MessageBoxA(nullptr, buf, "Error", 0);
#endif
    return 0;
}
