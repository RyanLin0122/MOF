#include "Info/cltMonsterToleranceKindInfo.h"

int cltMonsterToleranceKindInfo::Initialize(char* filename)
{
    char buf[1024] = {};
    int v17 = 0;

    FILE* f = g_clTextFileManager.fopen(filename);
    if (!f) return 0;

    // 略過三行表頭
    if (std::fgets(buf, 1023, f) && std::fgets(buf, 1023, f) && std::fgets(buf, 1023, f))
    {
        fpos_t pos{};
        std::fgetpos(f, &pos);

        // 計數：直接累加 m_count（GT 不先歸零）
        for (; std::fgets(buf, 1023, f); ++m_count)
            ;

        // 配置並清零（每筆 28 bytes）
        m_list = static_cast<strMonsterToleranceKindInfo*>(operator new(m_count * sizeof(strMonsterToleranceKindInfo)));
        std::memset(m_list, 0, m_count * sizeof(strMonsterToleranceKindInfo));

        // 回到資料起點
        std::fsetpos(f, &pos);

        int* v7 = reinterpret_cast<int*>(m_list);
        bool parse_failed = false;

        if (std::fgets(buf, 1023, f))
        {
            while (true)
            {
                char* t = std::strtok(buf, "\t\n");
                if (!t) { parse_failed = true; break; }
                v7[0] = std::atoi(t);

                t = std::strtok(nullptr, "\t\n");
                if (!t) { parse_failed = true; break; }
                v7[2] = std::atoi(t);

                t = std::strtok(nullptr, "\t\n");
                if (!t) { parse_failed = true; break; }
                v7[1] = std::atoi(t);

                t = std::strtok(nullptr, "\t\n");
                if (!t) { parse_failed = true; break; }
                v7[3] = std::atoi(t);

                t = std::strtok(nullptr, "\t\n");
                if (!t) { parse_failed = true; break; }
                v7[4] = std::atoi(t);

                t = std::strtok(nullptr, "\t\n");
                if (!t) { parse_failed = true; break; }
                v7[5] = std::atoi(t);

                t = std::strtok(nullptr, "\t\n");
                if (!t) { parse_failed = true; break; }
                v7[6] = std::atoi(t);

                v7 += 7;
                if (!std::fgets(buf, 1023, f))
                    break; // 正常到 EOF → 成功
            }
        }

        // GT: 只有 strtok 失敗才不設 v17=1；
        // fgets 返回 null（EOF）或無資料行都走 LABEL_16 路徑（v17=1）
        if (!parse_failed)
            v17 = 1;
    }

    g_clTextFileManager.fclose(f);
    return v17;
}

void cltMonsterToleranceKindInfo::Free()
{
    if (m_list) {
        operator delete(m_list);
        m_list = nullptr;
    }
    m_count = 0;
}

strMonsterToleranceKindInfo*
cltMonsterToleranceKindInfo::GetMonsterToleranceKindInfo(unsigned int a2)
{
    int v2 = m_count;
    int v3 = 0;

    if (v2 > 0) {
        const unsigned int* v4 = reinterpret_cast<const unsigned int*>(m_list);
        while (a2 >= *v4) {
            ++v3;
            v4 += 7;
            if (v3 >= v2)
                goto LABEL_5;
        }
        if (v3)
            return &m_list[v3 - 1];
    }

LABEL_5:
    if (v3 == v2)
        return &m_list[v2 - 1];
    return nullptr;
}

strMonsterToleranceKindInfo*
cltMonsterToleranceKindInfo::GetMonsterToleranceKindInfo_exactly(unsigned int a2)
{
    int v2 = m_count;
    if (v2 <= 0)
        return nullptr;

    const unsigned int* p = reinterpret_cast<const unsigned int*>(m_list);
    for (int i = 0; i < v2; ++i, p += 7) {
        if (a2 == p[0])
            return &m_list[i];
    }
    return nullptr;
}
