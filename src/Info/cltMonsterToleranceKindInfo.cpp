#include "Info/cltMonsterToleranceKindInfo.h"

static constexpr const char* kDelims = "\t\n";

int cltMonsterToleranceKindInfo::Initialize(char* filename)
{
    Free();
    if (!filename) return 0;

    FILE* f = g_clTextFileManager.fopen(filename);
    if (!f) return 0;

    char buf[1024] = {};

    // 略過三行表頭
    if (!std::fgets(buf, sizeof(buf), f) ||
        !std::fgets(buf, sizeof(buf), f) ||
        !std::fgets(buf, sizeof(buf), f)) {
        g_clTextFileManager.fclose(f);
        return 0;
    }

    // 計數
    fpos_t pos{};
    std::fgetpos(f, &pos);
    m_count = 0;
    while (std::fgets(buf, sizeof(buf), f)) ++m_count;

    if (m_count <= 0) {
        g_clTextFileManager.fclose(f);
        return 0;
    }

    // 配置並清零（每筆 28 bytes）
    m_list = static_cast<strMonsterToleranceKindInfo*>(operator new[](m_count * sizeof(strMonsterToleranceKindInfo)));
    std::memset(m_list, 0, m_count * sizeof(strMonsterToleranceKindInfo));

    // 回到資料起點
    std::fsetpos(f, &pos);

    int idx = 0;
    bool ok_all = true;

    if (std::fgets(buf, sizeof(buf), f)) {
        while (true) {
            // 使用 int* 的位移語意與反編譯一致
            int* p = reinterpret_cast<int*>(&m_list[idx]);

            char* t = std::strtok(buf, kDelims); // 1) threshold
            if (!t) { ok_all = false; break; }
            p[0] = std::atoi(t);

            t = std::strtok(nullptr, kDelims);   // 2) 防禦(100分率) → [2]
            if (!t) { ok_all = false; break; }
            p[2] = std::atoi(t);

            t = std::strtok(nullptr, kDelims);   // 3) 攻擊(100分率) → [1]
            if (!t) { ok_all = false; break; }
            p[1] = std::atoi(t);

            t = std::strtok(nullptr, kDelims);   // 4) 技能攻擊(100分率) → [3]
            if (!t) { ok_all = false; break; }
            p[3] = std::atoi(t);

            t = std::strtok(nullptr, kDelims);   // 5) 命中(1000分率) → [4]
            if (!t) { ok_all = false; break; }
            p[4] = std::atoi(t);

            t = std::strtok(nullptr, kDelims);   // 6) 迴避(1000分率) → [5]
            if (!t) { ok_all = false; break; }
            p[5] = std::atoi(t);

            t = std::strtok(nullptr, kDelims);   // 7) 爆擊(1000分率) → [6]
            if (!t) { ok_all = false; break; }
            p[6] = std::atoi(t);

            ++idx;
            if (!std::fgets(buf, sizeof(buf), f)) break; // 正常到 EOF
            if (idx >= m_count) break;
        }
    }
    else {
        // 沒有任何資料行也視為成功（與反編譯 LABEL_16 路徑一致）
        ok_all = true;
    }

    g_clTextFileManager.fclose(f);

    if (!ok_all || idx != m_count) {
        Free();
        return 0;
    }
    return 1;
}

void cltMonsterToleranceKindInfo::Free()
{
    if (m_list) {
        operator delete[](m_list);
        m_list = nullptr;
    }
    m_count = 0;
}

strMonsterToleranceKindInfo*
cltMonsterToleranceKindInfo::GetMonsterToleranceKindInfo(unsigned int a2)
{
    if (!m_list || m_count <= 0) return nullptr;

    // 與反編譯一致：尋找第一筆 threshold > a2，
    // 若找到且不是第一筆，回傳前一筆；若全都 <= a2，回傳最後一筆；若 a2 比第一筆還小，回傳 nullptr。
    int i = 0;
    const unsigned int* p = reinterpret_cast<const unsigned int*>(m_list);
    while (i < m_count && a2 >= p[0]) {
        ++i;
        p += 7; // 每筆 7 ints
    }
    if (i == 0) return nullptr;                          // 尚未達到第一門檻
    if (i == m_count) return &m_list[m_count - 1];       // 超過所有門檻 ⇒ 回傳最後一筆
    return &m_list[i - 1];                                // 介於第 (i-1) 與第 i 筆之間 ⇒ 回前一筆
}

strMonsterToleranceKindInfo*
cltMonsterToleranceKindInfo::GetMonsterToleranceKindInfo_exactly(unsigned int a2)
{
    if (!m_list || m_count <= 0) return nullptr;
    const unsigned int* p = reinterpret_cast<const unsigned int*>(m_list);
    for (int i = 0; i < m_count; ++i, p += 7) {
        if (a2 == p[0]) return &m_list[i];
    }
    return nullptr;
}
