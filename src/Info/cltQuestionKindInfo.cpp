#include "Info/cltQuestionKindInfo.h"
#include <cstring>
#include <cstdlib>
#include <cctype>

// ---------------------------------------------------------------------
// uint16_t cltQuestionKindInfo::TranslateKindCode(char* a1)
//   GT: mofclient.c:325279
//     if (strlen(a1) != 5) return 0;
//     v2 = (toupper(*a1) + 31) << 11;
//     v3 = atoi(a1 + 1);                  // 16-bit truncated
//     return (v3 < 0x800) ? (v2 | v3) : 0;
// ---------------------------------------------------------------------
uint16_t cltQuestionKindInfo::TranslateKindCode(char* s) {
    if (std::strlen(s) != 5) return 0;
    const int v2 = (std::toupper(static_cast<unsigned char>(*s)) + 31) << 11;
    const uint16_t v3 = static_cast<uint16_t>(std::atoi(s + 1));
    if (v3 < 0x800u) {
        return static_cast<uint16_t>(v2 | v3);
    }
    return 0;
}

// ---------------------------------------------------------------------
// int cltQuestionKindInfo::Initialize(char* filename)
//   GT: mofclient.c:325108-325193
//
//   - fopen via g_clTextFileManager；失敗回 0。
//   - 跳過三行表頭（標題列、空白列、欄位名稱列）；任一 fgets 失敗 → 直接
//     跳到 fclose，回傳 v16==0。
//   - fgetpos 紀錄資料起點，再以 fgets 計算資料筆數寫入 m_count。
//   - operator new(12 * m_count) 配置；memset 全 0。
//   - fsetpos 回到資料起點。
//   - 第一行 fgets：
//       * 失敗 → goto LABEL_16（v16 = 1，回 1，視同無資料但格式正確）。
//       * 成功 → 進入 while(1) 解析；
//   - 每一輪 strtok("\t\n") 七次：
//       (1) 文제 ID         → TranslateKindCode → *(WORD*)(rec+0)
//       (2) 답(기획용)      → 讀完後丟棄
//       (3) 학년             → atoi             → *(BYTE*)(rec+2)
//       (4) 시험문제 코드   → atoi             → *(WORD*)(rec+4)
//       (5) 축약 설명        → atoi             → *(WORD*)(rec+6)
//       (6) 답_몬스터       → cltCharKindInfo::TranslateKindCode → *(WORD*)(rec+8)
//       (7) 답_NPC          → cltNPCInfo::TranslateKindCode      → *(WORD*)(rec+10)
//     任一 strtok 回 NULL → break（v16 維持 0，回 0；GT 在此情況下並未呼叫 Free）。
//   - 寫完一筆後 rec += 12，再 fgets：失敗則 goto LABEL_16（v16=1）。
//   - 最後 fclose、回傳 v16。
// ---------------------------------------------------------------------
int cltQuestionKindInfo::Initialize(char* filename) {
    // GT(IDA): char Delimiter[2]; strcpy(Delimiter, "\t\n");
    //   ↳ 反編譯把 stack 位置對齊到 4 bytes、宣告為 [2]，但 strcpy 會寫入 3 個
    //     byte（含結尾 '\0'）；strtok 需要 NUL-terminated 字串，因此實際上的
    //     有效內容是 "\t\n\0"。我們直接以 4-byte 陣列保留同樣語意。
    char Delimiter[4] = { '\t', '\n', '\0', '\0' };
    int  v16 = 0;
    char Buffer[1024];
    std::memset(Buffer, 0, sizeof(Buffer));

    FILE* fp = g_clTextFileManager.fopen(filename);
    if (!fp) {
        return 0;
    }

    if (std::fgets(Buffer, 1023, fp)
        && std::fgets(Buffer, 1023, fp)
        && std::fgets(Buffer, 1023, fp))
    {
        fpos_t Position{};
        std::fgetpos(fp, &Position);

        // 第一輪掃描：純粹數行得到資料筆數。
        m_count = 0;
        while (std::fgets(Buffer, 1023, fp)) {
            ++m_count;
        }

        // GT: operator new(12 * count)；count==0 時呼叫 new(0) 仍會回傳合法指標。
        m_items = static_cast<strQuestionKindInfo*>(
            operator new(sizeof(strQuestionKindInfo) * static_cast<size_t>(m_count)));
        std::memset(m_items, 0, sizeof(strQuestionKindInfo) * static_cast<size_t>(m_count));

        std::fsetpos(fp, &Position);

        char* v7 = reinterpret_cast<char*>(m_items);

        if (std::fgets(Buffer, 1023, fp)) {
            for (;;) {
                // [0] 문제 ID
                char* v8 = std::strtok(Buffer, Delimiter);
                if (!v8) break;
                *reinterpret_cast<uint16_t*>(v7 + 0) = TranslateKindCode(v8);

                // [1] 답(기획용) — 讀後丟棄
                if (!std::strtok(nullptr, Delimiter)) break;

                // [2] 학년 (BYTE)
                char* v9 = std::strtok(nullptr, Delimiter);
                if (!v9) break;
                *reinterpret_cast<uint8_t*>(v7 + 2) = static_cast<uint8_t>(std::atoi(v9));

                // [3] 시험문제 코드 (WORD)
                char* v10 = std::strtok(nullptr, Delimiter);
                if (!v10) break;
                *reinterpret_cast<uint16_t*>(v7 + 4) = static_cast<uint16_t>(std::atoi(v10));

                // [4] 축약 설명 (WORD)
                char* v11 = std::strtok(nullptr, Delimiter);
                if (!v11) break;
                *reinterpret_cast<uint16_t*>(v7 + 6) = static_cast<uint16_t>(std::atoi(v11));

                // [5] 답_몬스터 (WORD) — Jxxxx → cltCharKindInfo::TranslateKindCode
                char* v12 = std::strtok(nullptr, Delimiter);
                if (!v12) break;
                *reinterpret_cast<uint16_t*>(v7 + 8) = cltCharKindInfo::TranslateKindCode(v12);

                // [6] 답_NPC (WORD) — Nxxxx → cltNPCInfo::TranslateKindCode
                char* v13 = std::strtok(nullptr, Delimiter);
                if (!v13) break;
                *reinterpret_cast<uint16_t*>(v7 + 10) = cltNPCInfo::TranslateKindCode(v13);

                v7 += 12;

                // GT: 下一行 fgets 失敗 → 視為「資料完整讀完」，goto LABEL_16
                if (!std::fgets(Buffer, 1023, fp)) {
                    v16 = 1;
                    break;
                }
            }
        } else {
            // GT 的 else { LABEL_16: v16 = 1; }
            v16 = 1;
        }
    }

    g_clTextFileManager.fclose(fp);
    return v16;
}

// ---------------------------------------------------------------------
// void cltQuestionKindInfo::Free()
//   GT: mofclient.c:325196
// ---------------------------------------------------------------------
void cltQuestionKindInfo::Free() {
    if (m_items) {
        operator delete(static_cast<void*>(m_items));
        m_items = nullptr;
    }
    m_count = 0;
}

// ---------------------------------------------------------------------
// strQuestionKindInfo* cltQuestionKindInfo::GetQuestionKindInfo(uint16_t a2)
//   GT: mofclient.c:325207
//     線性掃描；步距 = 6 個 _WORD = 12 bytes；首欄 (qKind) 與 a2 比對。
// ---------------------------------------------------------------------
strQuestionKindInfo* cltQuestionKindInfo::GetQuestionKindInfo(uint16_t a2) {
    const int v2 = m_count;
    if (v2 <= 0) return nullptr;

    int v3 = 0;
    uint16_t* v4 = reinterpret_cast<uint16_t*>(m_items);
    for (uint16_t* i = v4; *i != a2; i += 6) {
        if (++v3 >= v2) return nullptr;
    }
    return reinterpret_cast<strQuestionKindInfo*>(&v4[6 * v3]);
}

// ---------------------------------------------------------------------
// int cltQuestionKindInfo::GetQuestions(char a2, uint16_t* a3)
//   GT: mofclient.c:325228
//     依 grade(=a2) 線性挑出 qKind 寫到 a3，回傳數量。
// ---------------------------------------------------------------------
int cltQuestionKindInfo::GetQuestions(char a2, uint16_t* a3) {
    int result = 0;
    if (m_count <= 0) return 0;

    int v4 = 0;
    int v6 = 0;
    do {
        char* v7 = reinterpret_cast<char*>(m_items) + v6;
        if (*reinterpret_cast<uint8_t*>(v7 + 2) == static_cast<uint8_t>(a2)) {
            ++result;
            *a3++ = *reinterpret_cast<uint16_t*>(v7);
        }
        ++v4;
        v6 += 12;
    } while (v4 < m_count);

    return result;
}

// ---------------------------------------------------------------------
// IsAnswer_MonsterKind / IsAnswer_NpcKind
//   GT: mofclient.c:325257 / 325268
//     IDA 推斷的回傳型別是 strQuestionKindInfo*，但內容只是 (ansX != 0)
//     的 0/1 值，使用點全部當布林。我們直接以 bool 表示。
// ---------------------------------------------------------------------
bool cltQuestionKindInfo::IsAnswer_MonsterKind(uint16_t a2) {
    strQuestionKindInfo* p = GetQuestionKindInfo(a2);
    if (!p) return false;
    // GT: result = *((_WORD*)result + 4) != 0  → ansMonsterKind (offset +8)
    return p->ansMonsterKind != 0;
}

bool cltQuestionKindInfo::IsAnswer_NpcKind(uint16_t a2) {
    strQuestionKindInfo* p = GetQuestionKindInfo(a2);
    if (!p) return false;
    // GT: result = *((_WORD*)result + 5) != 0  → ansNpcKind (offset +10)
    return p->ansNpcKind != 0;
}
