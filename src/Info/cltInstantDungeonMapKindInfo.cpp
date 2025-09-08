#include "Info/cltInstantDungeonMapKindInfo.h"

static constexpr const char* kDelims = "\t\n";

int cltInstantDungeonMapKindInfo::Initialize(char* filename)
{
    Free();
    if (!filename) return 0;

    FILE* f = g_clTextFileManager.fopen(filename);
    FILE* Stream = f;
    int   ok = 0;
    if (!f) return 0;

    // 使用大行緩衝（原始使用 0x1FFF）
    char buf[8192] = {};

    // 略過 3 行表頭
    if (std::fgets(buf, sizeof(buf), f) &&
        std::fgets(buf, sizeof(buf), f) &&
        std::fgets(buf, sizeof(buf), f))
    {
        // 計數
        fpos_t pos{};
        std::fgetpos(f, &pos);
        while (std::fgets(buf, sizeof(buf), f)) ++m_count;

        if (m_count == 0) { // 沒有資料行也視為成功
            g_clTextFileManager.fclose(Stream);
            return 1;
        }

        // 配置
        m_list = static_cast<strInstantDungeonMapKindInfo*>(
            operator new[](m_count * sizeof(strInstantDungeonMapKindInfo)));
        std::memset(m_list, 0, m_count * sizeof(strInstantDungeonMapKindInfo));

        // 回到資料起點
        std::fsetpos(f, &pos);

        int idx = 0;
        if (std::fgets(buf, sizeof(buf), f))
        {
            while (true)
            {
                strInstantDungeonMapKindInfo& r = m_list[idx];
                std::memset(&r, 0, sizeof(r));

                // 1) i_map_id (Dxxxx)
                char* t = std::strtok(buf, kDelims);
                if (!t) break;
                r.i_map_id = TranslateKindCode(t);

                // 2) map_id (Ixxxx)
                t = std::strtok(nullptr, kDelims); if (!t) break;
                r.map_id = cltMapInfo::TranslateKindCode(t);

                // 3) x
                t = std::strtok(nullptr, kDelims); if (!t) break;
                r.x = std::atoi(t);

                // 4) y
                t = std::strtok(nullptr, kDelims); if (!t) break;
                r.y = std::atoi(t);

                // p1: portaltype, dest, x, y
                t = std::strtok(nullptr, kDelims); if (!t) break;
                r.p1_portaltype = GetIndunPortalType(t);

                t = std::strtok(nullptr, kDelims); if (!t) break;
                r.p1_dest = TranslateKindCode(t);

                t = std::strtok(nullptr, kDelims); if (!t) break;
                r.p1_x = std::atoi(t);

                t = std::strtok(nullptr, kDelims); if (!t) break;
                r.p1_y = std::atoi(t);

                // p2: portaltype, dest, x, y
                t = std::strtok(nullptr, kDelims); if (!t) break;
                r.p2_portaltype = GetIndunPortalType(t);

                t = std::strtok(nullptr, kDelims); if (!t) break;
                r.p2_dest = TranslateKindCode(t);

                t = std::strtok(nullptr, kDelims); if (!t) break;
                r.p2_x = std::atoi(t);

                t = std::strtok(nullptr, kDelims); if (!t) break;
                r.p2_y = std::atoi(t);

                // m1..m4（每組：mob_id, init, max, x, y, x+dx, y+dy, regen）
                // m1
                t = std::strtok(nullptr, kDelims); if (!t) goto PARSE_FAIL;
                r.m1_mob_id = cltCharKindInfo::TranslateKindCode(t);
                t = std::strtok(nullptr, kDelims); if (!t) goto PARSE_FAIL; r.m1_initnum = std::atoi(t);
                t = std::strtok(nullptr, kDelims); if (!t) goto PARSE_FAIL; r.m1_maxnum = std::atoi(t);
                t = std::strtok(nullptr, kDelims); if (!t) goto PARSE_FAIL; r.m1_x = std::atoi(t);
                t = std::strtok(nullptr, kDelims); if (!t) goto PARSE_FAIL; r.m1_y = std::atoi(t);
                t = std::strtok(nullptr, kDelims); if (!t) goto PARSE_FAIL; r.m1_width = r.m1_x + std::atoi(t);
                t = std::strtok(nullptr, kDelims); if (!t) goto PARSE_FAIL; r.m1_height = r.m1_y + std::atoi(t);
                t = std::strtok(nullptr, kDelims); if (!t) goto PARSE_FAIL; r.m1_regentime = std::atoi(t);

                // m2
                t = std::strtok(nullptr, kDelims); if (!t) goto PARSE_FAIL;
                r.m2_mob_id = cltCharKindInfo::TranslateKindCode(t);
                t = std::strtok(nullptr, kDelims); if (!t) goto PARSE_FAIL; r.m2_initnum = std::atoi(t);
                t = std::strtok(nullptr, kDelims); if (!t) goto PARSE_FAIL; r.m2_maxnum = std::atoi(t);
                t = std::strtok(nullptr, kDelims); if (!t) goto PARSE_FAIL; r.m2_x = std::atoi(t);
                t = std::strtok(nullptr, kDelims); if (!t) goto PARSE_FAIL; r.m2_y = std::atoi(t);
                t = std::strtok(nullptr, kDelims); if (!t) goto PARSE_FAIL; r.m2_width = r.m2_x + std::atoi(t);
                t = std::strtok(nullptr, kDelims); if (!t) goto PARSE_FAIL; r.m2_height = r.m2_y + std::atoi(t);
                t = std::strtok(nullptr, kDelims); if (!t) goto PARSE_FAIL; r.m2_regentime = std::atoi(t);

                // m3
                t = std::strtok(nullptr, kDelims); if (!t) goto PARSE_FAIL;
                r.m3_mob_id = cltCharKindInfo::TranslateKindCode(t);
                t = std::strtok(nullptr, kDelims); if (!t) goto PARSE_FAIL; r.m3_initnum = std::atoi(t);
                t = std::strtok(nullptr, kDelims); if (!t) goto PARSE_FAIL; r.m3_maxnum = std::atoi(t);
                t = std::strtok(nullptr, kDelims); if (!t) goto PARSE_FAIL; r.m3_x = std::atoi(t);
                t = std::strtok(nullptr, kDelims); if (!t) goto PARSE_FAIL; r.m3_y = std::atoi(t);
                t = std::strtok(nullptr, kDelims); if (!t) goto PARSE_FAIL; r.m3_width = r.m3_x + std::atoi(t);
                t = std::strtok(nullptr, kDelims); if (!t) goto PARSE_FAIL; r.m3_height = r.m3_y + std::atoi(t);
                t = std::strtok(nullptr, kDelims); if (!t) goto PARSE_FAIL; r.m3_regentime = std::atoi(t);

                // m4
                t = std::strtok(nullptr, kDelims); if (!t) goto PARSE_FAIL;
                r.m4_mob_id = cltCharKindInfo::TranslateKindCode(t);
                t = std::strtok(nullptr, kDelims); if (!t) goto PARSE_FAIL; r.m4_initnum = std::atoi(t);
                t = std::strtok(nullptr, kDelims); if (!t) goto PARSE_FAIL; r.m4_maxnum = std::atoi(t);
                t = std::strtok(nullptr, kDelims); if (!t) goto PARSE_FAIL; r.m4_x = std::atoi(t);
                t = std::strtok(nullptr, kDelims); if (!t) goto PARSE_FAIL; r.m4_y = std::atoi(t);
                t = std::strtok(nullptr, kDelims); if (!t) goto PARSE_FAIL; r.m4_width = r.m4_x + std::atoi(t);
                t = std::strtok(nullptr, kDelims); if (!t) goto PARSE_FAIL; r.m4_height = r.m4_y + std::atoi(t);
                t = std::strtok(nullptr, kDelims); if (!t) goto PARSE_FAIL; r.m4_regentime = std::atoi(t);

                // r1（寬高相對 m1_x/m1_y）
                t = std::strtok(nullptr, kDelims); if (!t) goto PARSE_FAIL;
                r.r1_mob_id = cltCharKindInfo::TranslateKindCode(t);
                t = std::strtok(nullptr, kDelims); if (!t) goto PARSE_FAIL; r.r1_initnum = std::atoi(t);
                t = std::strtok(nullptr, kDelims); if (!t) goto PARSE_FAIL; r.r1_maxnum = std::atoi(t);
                t = std::strtok(nullptr, kDelims); if (!t) goto PARSE_FAIL; r.r1_x = std::atoi(t);
                t = std::strtok(nullptr, kDelims); if (!t) goto PARSE_FAIL; r.r1_y = std::atoi(t);
                t = std::strtok(nullptr, kDelims); if (!t) goto PARSE_FAIL; r.r1_width = r.m1_x + std::atoi(t);
                t = std::strtok(nullptr, kDelims); if (!t) goto PARSE_FAIL; r.r1_height = r.m1_y + std::atoi(t);
                t = std::strtok(nullptr, kDelims); if (!t) goto PARSE_FAIL; r.r1_regentime = std::atoi(t);

                // r2（寬高相對 m2_x/m2_y）
                t = std::strtok(nullptr, kDelims); if (!t) goto PARSE_FAIL;
                r.r2_mob_id = cltCharKindInfo::TranslateKindCode(t);
                t = std::strtok(nullptr, kDelims); if (!t) goto PARSE_FAIL; r.r2_initnum = std::atoi(t);
                t = std::strtok(nullptr, kDelims); if (!t) goto PARSE_FAIL; r.r2_maxnum = std::atoi(t);
                t = std::strtok(nullptr, kDelims); if (!t) goto PARSE_FAIL; r.r2_x = std::atoi(t);
                t = std::strtok(nullptr, kDelims); if (!t) goto PARSE_FAIL; r.r2_y = std::atoi(t);
                t = std::strtok(nullptr, kDelims); if (!t) goto PARSE_FAIL; r.r2_width = r.m2_x + std::atoi(t);
                t = std::strtok(nullptr, kDelims); if (!t) goto PARSE_FAIL; r.r2_height = r.m2_y + std::atoi(t);
                t = std::strtok(nullptr, kDelims); if (!t) goto PARSE_FAIL; r.r2_regentime = std::atoi(t);

                // 下一行
                ++idx;
                if (!std::fgets(buf, sizeof(buf), f)) { ok = 1; break; }
                if (idx >= m_count) { ok = 1; break; }
                continue;

            PARSE_FAIL:
                ok = 0;
                break;
            }
        }
        else {
            ok = 1; // 第一筆就讀不到資料也視為成功
        }
    }

    g_clTextFileManager.fclose(Stream);
    if (!ok) { Free(); }
    return ok;
}

void cltInstantDungeonMapKindInfo::Free()
{
    if (m_list) {
        operator delete[](m_list);
        m_list = nullptr;
    }
    m_count = 0;
}

strInstantDungeonMapKindInfo*
cltInstantDungeonMapKindInfo::GetInstantDungeonMapKindInfo(uint16_t a2)
{
    if (!m_list || m_count <= 0) return nullptr;

    // 以 WORD* 逐筆掃描（步距 118 WORD = 236 bytes）
    uint16_t* p = reinterpret_cast<uint16_t*>(m_list);
    for (int i = 0; i < m_count; ++i, p += 118) {
        if (p[0] == a2) // kind 位於偏移 0
            return reinterpret_cast<strInstantDungeonMapKindInfo*>(p);
    }
    return nullptr;
}

uint16_t cltInstantDungeonMapKindInfo::TranslateKindCode(char* s)
{
    if (!s || std::strlen(s) != 5) return 0;
    int hi = (std::toupper(static_cast<unsigned char>(s[0])) + 31) << 11;
    int lo = std::atoi(s + 1);
    return (lo < 0x800) ? static_cast<uint16_t>(hi | lo) : 0;
}

int cltInstantDungeonMapKindInfo::GetIndunPortalType(char* s)
{
    if (!s) return 0;
    if (_stricmp(s, "FIX") == 0)    return 1;
    if (_stricmp(s, "RANDOM") == 0) return 2;
    if (_stricmp(s, "EXIT") == 0)   return 3;
    return 0;
}
