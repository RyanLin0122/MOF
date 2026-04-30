#include "Info/cltCoupleRingKindInfo.h"
#include <cstring>
#include <cstdlib>
#include <cctype>

// ===== mofclient.c:294951 — cltCoupleRingKindInfo::TranslateKindCode =====
//   ((toupper(s[0]) + 31) << 11) | (uint16_t)atoi(s+1)
//   - 長度需為 5；尾四碼以 atoi 取出後截斷為 16-bit，需 < 0x800，
//     否則整體回傳 0。高位元因 << 11 會超過 16-bit，但寫入端為 WORD，
//     由 (v2 | v3) 隱含 16-bit 截斷（GT 用 ax 暫存器存回）。
uint16_t cltCoupleRingKindInfo::TranslateKindCode(char* s) {
    if (!s) return 0;
    if (std::strlen(s) != 5) return 0;
    const int v2 = (std::toupper(static_cast<unsigned char>(s[0])) + 31) << 11;
    const uint16_t v3 = static_cast<uint16_t>(std::atoi(s + 1));
    if (v3 < 0x800u) {
        return static_cast<uint16_t>(v2 | v3);
    }
    return 0;
}

// ===== mofclient.c:294822 — cltCoupleRingKindInfo::Initialize =====
//
//   - 跳過 3 行表頭（fgets buf, 1023, fp，三次皆需成功）
//   - 以 fgetpos 記錄資料起點，再用一個迴圈計算總列數 -> m_count
//   - operator new(20 * m_count) + memset(0)
//   - fsetpos 回到資料起點
//   - 以 v7 起點為 m_items，每讀完一列推進 20 bytes
//   - 任一 strtok 回 NULL 或第一欄 TranslateKindCode==0 -> break，回傳 0
//     （此時 m_items 仍保留，m_count 仍為總列數，未填到的 row 為全 0）
//   - 一列完整解析後若下一個 fgets 回 NULL -> v18 = 1（成功）
//   - 表頭之後第一個 fgets 即回 NULL（無資料列）-> v18 = 1（成功）
//
// 寫入順序刻意保留 GT 行為：第一欄 ringKind 在判斷 v9==0 前就先寫入 +0，
// 雖然實務上因 memset(0) 的關係結果相同，但保留 byte-level 一致。
int cltCoupleRingKindInfo::Initialize(char* filename) {
    FILE* fp = g_clTextFileManager.fopen(filename);
    if (!fp) return 0;

    char Delimiter[4] = { '\t', '\n', '\0', '\0' };
    int  v18 = 0;
    char Buffer[1024];
    std::memset(Buffer, 0, sizeof(Buffer));

    // 三行表頭
    if (std::fgets(Buffer, 1023, fp)
        && std::fgets(Buffer, 1023, fp)
        && std::fgets(Buffer, 1023, fp))
    {
        fpos_t Position{};
        std::fgetpos(fp, &Position);

        // 計算資料列數
        m_count = 0;
        while (std::fgets(Buffer, 1023, fp)) {
            ++m_count;
        }

        // 一次配置 20 * m_count，並全部清零
        m_items = static_cast<strCoupleRingKindInfo*>(
            operator new(sizeof(strCoupleRingKindInfo) * static_cast<size_t>(m_count)));
        std::memset(m_items, 0, sizeof(strCoupleRingKindInfo) * static_cast<size_t>(m_count));

        std::fsetpos(fp, &Position);

        char* v7 = reinterpret_cast<char*>(m_items);

        if (std::fgets(Buffer, 1023, fp)) {
            for (;;) {
                // [0] 커플링ID — 先寫後判（GT 行為）
                char* v8 = std::strtok(Buffer, Delimiter);
                if (!v8) break;
                const uint16_t v9 = TranslateKindCode(v8);
                *reinterpret_cast<uint16_t*>(v7 + 0) = v9;
                if (!v9) break;

                // [1] 커플링명(기획자용) — 丟棄
                if (!std::strtok(nullptr, Delimiter)) break;

                // [2] 텍스트 코드 -> textId @ +2 (WORD)
                char* v10 = std::strtok(nullptr, Delimiter);
                if (!v10) break;
                *reinterpret_cast<uint16_t*>(v7 + 2) = static_cast<uint16_t>(std::atoi(v10));

                // [3] 경험치 상승률(%) -> expRatePercent @ +4 (DWORD)
                char* v11 = std::strtok(nullptr, Delimiter);
                if (!v11) break;
                *reinterpret_cast<int32_t*>(v7 + 4) = std::atoi(v11);

                // [4] 배우자 소환여부 -> canSummonSpouse @ +8 (DWORD)
                char* v12 = std::strtok(nullptr, Delimiter);
                if (!v12) break;
                *reinterpret_cast<int32_t*>(v7 + 8) = std::atoi(v12);

                // [5] 커플링리소스 ID (hex) -> resourceIdHex @ +12 (DWORD)
                char* v13 = std::strtok(nullptr, Delimiter);
                if (!v13) break;
                std::sscanf(v13, "%x", reinterpret_cast<unsigned int*>(v7 + 12));

                // [6] 블럭아이디 -> blockId @ +16 (WORD)
                char* v14 = std::strtok(nullptr, Delimiter);
                if (!v14) break;
                *reinterpret_cast<uint16_t*>(v7 + 16) = static_cast<uint16_t>(std::atoi(v14));

                // [7] 이펙트파일명 -> effectKind @ +18 (WORD)
                char* v15 = std::strtok(nullptr, Delimiter);
                if (!v15) break;
                *reinterpret_cast<uint16_t*>(v7 + 18) = TranslateKindCode(v15);

                v7 += 20;

                if (!std::fgets(Buffer, 1023, fp)) {
                    v18 = 1;
                    break;
                }
            }
        }
        else {
            v18 = 1; // 無資料列也視為成功
        }
    }

    g_clTextFileManager.fclose(fp);
    return v18;
}

// ===== mofclient.c:294919 — cltCoupleRingKindInfo::Free =====
void cltCoupleRingKindInfo::Free() {
    if (m_items) {
        operator delete(static_cast<void*>(m_items));
        m_items = nullptr;
    }
    m_count = 0;
}

// ===== mofclient.c:294930 — cltCoupleRingKindInfo::GetCoupleRingKindInfo =====
//   GT 用 _WORD* 以 stride 10 巡訪（10 WORDs = 20 bytes）：
//     for (i = base; *i != a2; i += 10)
//         if (++idx >= m_count) return 0;
//     return &base[10 * idx];
strCoupleRingKindInfo* cltCoupleRingKindInfo::GetCoupleRingKindInfo(uint16_t code) {
    if (m_count <= 0) return nullptr;
    for (int i = 0; i < m_count; ++i) {
        if (m_items[i].ringKind == code) return &m_items[i];
    }
    return nullptr;
}
