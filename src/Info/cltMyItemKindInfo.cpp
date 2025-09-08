#include "Info/cltMyItemKindInfo.h"

static constexpr const char* kDelims = "\t\n";

int cltMyItemKindInfo::Initialize(char* filename)
{
    Free();
    if (!filename) return 0;

    FILE* f = g_clTextFileManager.fopen(filename);
    FILE* Stream = f;
    int ok = 0;
    if (!f) return 0;

    char buf[1024] = {};

    // 略過三行表頭
    if (std::fgets(buf, sizeof(buf), f) &&
        std::fgets(buf, sizeof(buf), f) &&
        std::fgets(buf, sizeof(buf), f))
    {
        // 計數
        fpos_t pos{};
        std::fgetpos(f, &pos);
        while (std::fgets(buf, sizeof(buf), f)) ++m_count;

        // 配置（即使 m_count==0 也沿用反編譯流程）
        m_list = static_cast<strMyItemKindInfo*>(operator new[](m_count * sizeof(strMyItemKindInfo)));
        if (m_count) std::memset(m_list, 0, m_count * sizeof(strMyItemKindInfo));

        // 回到資料起點
        std::fsetpos(f, &pos);

        int idx = 0;
        if (!std::fgets(buf, sizeof(buf), f)) {
            ok = 1; // 無任何資料行：視為成功
        }
        else {
            while (true)
            {
                strMyItemKindInfo& r = m_list[idx];
                std::memset(&r, 0, sizeof(r));

                // 1) ID
                char* t = std::strtok(buf, kDelims);
                if (!t) break;
                r.kind = TranslateKindCode(t);

                // 2) NAME(NOTUSE) 略過
                if (!std::strtok(nullptr, kDelims)) break;

                // 3) NAMETEXTCODE
                t = std::strtok(nullptr, kDelims); if (!t) break;
                r.name_text_code = (uint16_t)std::atoi(t);

                // 4) 리소스아이디 (hex)
                t = std::strtok(nullptr, kDelims); if (!t) break;
                unsigned int hexv = 0;
                std::sscanf(t, "%x", &hexv);
                r.resource_id_hex = hexv;

                // 5) 블럭넘버
                t = std::strtok(nullptr, kDelims); if (!t) break;
                r.block_no = (uint16_t)std::atoi(t);

                // 6) 타입
                t = std::strtok(nullptr, kDelims); if (!t) break;
                r.type = ParseType(t);

                // 7) DUPLICATE (T/F)
                t = std::strtok(nullptr, kDelims); if (!t) break;
                r.duplicate_flag = (std::toupper((unsigned char)t[0]) == 'T') ? 1 : 0;

                // 8) EXPADV(100)
                t = std::strtok(nullptr, kDelims); if (!t) break;
                r.exp_adv_100 = std::atoi(t);

                // 9) DROPMONEYAMOUNTADV(100)
                t = std::strtok(nullptr, kDelims); if (!t) break;
                r.dropmoney_adv_100 = std::atoi(t);

                // 10) 프리미엄퀵슬롯여부
                t = std::strtok(nullptr, kDelims); if (!t) break;
                r.premium_quickslot = std::atoi(t);

                // 11) 배우자 일일 소환 충전
                t = std::strtok(nullptr, kDelims); if (!t) break;
                r.spouse_daily_recall = std::atoi(t);

                // 12) 효과설명 텍스트코드
                t = std::strtok(nullptr, kDelims); if (!t) break;
                r.effect_text_code = (uint16_t)std::atoi(t);

                // 下一列
                ++idx;
                if (!std::fgets(buf, sizeof(buf), f)) { ok = 1; break; }
                if (idx >= m_count) { ok = 1; break; }
            }
        }
    }

    g_clTextFileManager.fclose(Stream);
    if (!ok) { Free(); }
    return ok;
}

void cltMyItemKindInfo::Free()
{
    if (m_list) {
        operator delete[](m_list);
        m_list = nullptr;
    }
    m_count = 0;
}

strMyItemKindInfo* cltMyItemKindInfo::GetMyItemKindInfo(uint16_t a2)
{
    if (!m_list || m_count <= 0) return nullptr;

    // 以 WORD* 方式與反編譯步距一致（每筆 18 WORD = 36 bytes）
    uint16_t* p = reinterpret_cast<uint16_t*>(m_list);
    for (int i = 0; i < m_count; ++i, p += 18) {
        if (p[0] == a2)
            return reinterpret_cast<strMyItemKindInfo*>(p);
    }
    return nullptr;
}

uint8_t cltMyItemKindInfo::GetMyItemType(uint16_t a2)
{
    if (!m_list || m_count <= 0) return 0;

    uint16_t* p = reinterpret_cast<uint16_t*>(m_list);
    for (int i = 0; i < m_count; ++i, p += 18) {
        if (p[0] == a2) {
            // 反編譯：v4[18*i + 16]（WORD 讀取），回傳值為 8-bit
            const uint16_t w = p[16]; // 偏移 +32
            return static_cast<uint8_t>(w & 0xFF);
        }
    }
    return 0;
}

uint16_t cltMyItemKindInfo::TranslateKindCode(char* a1)
{
    if (!a1 || std::strlen(a1) != 5) return 0;

    int hi = (std::toupper(static_cast<unsigned char>(a1[0])) + 31) << 11;
    int lo = std::atoi(a1 + 1);
    return (lo < 0x800) ? static_cast<uint16_t>(hi | lo) : 0;
}
