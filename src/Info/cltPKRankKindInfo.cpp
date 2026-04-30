#include "Info/cltPKRankKindInfo.h"
#include <cstring>
#include <cstdlib>
#include <cctype>

// =============================================================================
// 反編譯來源：mofclient.c:322889-323061
// 解析格式（pkrank.txt，EUC-KR）：
//   3 行表頭 → N 行資料；資料分隔字元 = "\t\n"
//   欄位次序：[Kind] [Rank#略過] [TextID] [NeedPoint] [ResIDHex] [BlockID]
//             [LosePenalty] [Merit] [Public]
//   每筆 28 bytes，逐欄即時寫入該記錄對應 offset；任一欄缺失 → 中止迴圈，
//   v19 仍為 0，函式回傳 0；正常讀到 EOF 才設 v19=1。
// =============================================================================

// TranslateKindCode（mofclient.c:323046）
//   strlen 必須等於 5；結果 = ((toupper(s[0])+31) << 11) | atoi(s+1)（atoi < 0x800 才有效）。
uint16_t cltPKRankKindInfo::TranslateKindCode(char* s) {
    if (std::strlen(s) != 5) return 0;
    const int v2 = (std::toupper(static_cast<unsigned char>(s[0])) + 31) << 11;
    const unsigned int v3 = static_cast<unsigned int>(std::atoi(s + 1));
    if (v3 < 0x800u) return static_cast<uint16_t>(v2 | v3);
    return 0;
}

// Initialize（mofclient.c:322889）
//   依反編譯流程逐欄即時寫入 record（不採批次驗證後再寫入），保持與 GT 完全等價。
int cltPKRankKindInfo::Initialize(char* filename) {
    char Delimiter[3];
    std::strcpy(Delimiter, "\t\n");

    int v19 = 0;                       // 成功旗標（GT 區域變數）

    FILE* v3 = g_clTextFileManager.fopen(filename);
    FILE* v4 = v3;
    if (!v3) return 0;

    char Buffer[1024];                  // GT 為 [Buffer + v22[1020] + v23 + v24] 共 1024 bytes
    std::memset(Buffer, 0, sizeof(Buffer));

    // 表頭三行（標題 / 空行 / 欄位名）
    if (std::fgets(Buffer, 1023, v3)
        && std::fgets(Buffer, 1023, v4)
        && std::fgets(Buffer, 1023, v4))
    {
        // 記錄資料起點，先掃過一遍計算 record 數
        fpos_t Position{};
        std::fgetpos(v4, &Position);
        m_count = 0;
        while (std::fgets(Buffer, 1023, v4)) {
            ++m_count;
        }

        // 配置 28 * count bytes（GT 即使 count==0 也會 operator new(0)；此處比照辦理）
        void* v5 = operator new(sizeof(strPKRankKindInfo) * static_cast<size_t>(m_count));
        m_items = static_cast<strPKRankKindInfo*>(v5);
        std::memset(v5, 0, sizeof(strPKRankKindInfo) * static_cast<size_t>(m_count));

        std::fsetpos(v4, &Position);
        char* v7 = static_cast<char*>(v5);

        if (std::fgets(Buffer, 1023, v4)) {
            for (;;) {
                // [1] Kind 字串 → TranslateKindCode → +0 (WORD)
                char* v8 = std::strtok(Buffer, Delimiter);
                if (!v8) break;
                uint16_t v9 = TranslateKindCode(v8);
                *reinterpret_cast<uint16_t*>(v7 + 0) = v9;
                if (!v9) break;

                // [2] 階級數字 → 反編譯 strtok 後丟棄
                if (!std::strtok(nullptr, Delimiter)) break;

                // [3] textId → atoi → +2 (WORD)
                char* v10 = std::strtok(nullptr, Delimiter);
                if (!v10) break;
                *reinterpret_cast<uint16_t*>(v7 + 2) = static_cast<uint16_t>(std::atoi(v10));

                // [4] needPoint → atoi → +4 (DWORD)
                char* v11 = std::strtok(nullptr, Delimiter);
                if (!v11) break;
                *reinterpret_cast<int32_t*>(v7 + 4) = std::atoi(v11);

                // [5] resIdHex → sscanf "%x" → +8 (DWORD)
                char* v12 = std::strtok(nullptr, Delimiter);
                if (!v12) break;
                std::sscanf(v12, "%x", reinterpret_cast<uint32_t*>(v7 + 8));

                // [6] blockId → atoi → +12 (WORD)；+14/+15 由 memset 留白
                char* v13 = std::strtok(nullptr, Delimiter);
                if (!v13) break;
                *reinterpret_cast<uint16_t*>(v7 + 12) = static_cast<uint16_t>(std::atoi(v13));

                // [7] losePenalty → atoi → +16 (DWORD，可為負)
                char* v14 = std::strtok(nullptr, Delimiter);
                if (!v14) break;
                *reinterpret_cast<int32_t*>(v7 + 16) = std::atoi(v14);

                // [8] meritPoint → atoi → +20 (DWORD)
                char* v15 = std::strtok(nullptr, Delimiter);
                if (!v15) break;
                *reinterpret_cast<int32_t*>(v7 + 20) = std::atoi(v15);

                // [9] publicPoint → atoi → +24 (DWORD)
                char* v16 = std::strtok(nullptr, Delimiter);
                if (!v16) break;
                *reinterpret_cast<int32_t*>(v7 + 24) = std::atoi(v16);

                v7 += 28;

                // 讀下一行；EOF 即視為成功（goto LABEL_19）
                if (!std::fgets(Buffer, 1023, v4)) {
                    v19 = 1;
                    break;
                }
            }
        }
        else {
            // 第一行 fgets 即失敗（資料區為空）：GT 同樣設 v19 = 1
            v19 = 1;
        }
    }

    g_clTextFileManager.fclose(v4);
    return v19;
}

// Free（mofclient.c:322991）
void cltPKRankKindInfo::Free() {
    if (m_items) {
        operator delete(static_cast<void*>(m_items));
        m_items = nullptr;
    }
    m_count = 0;
}

// GetPKRankKindInfo（mofclient.c:323002）
//   線性搜尋；以 14 個 WORD（28 bytes）為步進。
strPKRankKindInfo* cltPKRankKindInfo::GetPKRankKindInfo(uint16_t code) {
    int v2 = m_count;
    int v3 = 0;
    if (v2 <= 0) return nullptr;
    uint16_t* v4 = reinterpret_cast<uint16_t*>(m_items);
    uint16_t* i = v4;
    while (*i != code) {
        if (++v3 >= v2) return nullptr;
        i += 14;
    }
    return reinterpret_cast<strPKRankKindInfo*>(&v4[14 * v3]);
}

// GetPKRankKindByPoint（mofclient.c:323023）
//   point < m_items[0].needPoint   → 0
//   只有一筆                        → m_items[0].kind
//   否則向後掃描 needPoint，回傳尚未越過該門檻的前一筆 kind；越過尾端則為最後一筆。
uint16_t cltPKRankKindInfo::GetPKRankKindByPoint(int point) {
    char* v2 = reinterpret_cast<char*>(m_items);
    if (point < *reinterpret_cast<int32_t*>(v2 + 4))
        return 0;
    int v4 = m_count;
    int v5 = 1;
    if (v4 <= 1)
        return *reinterpret_cast<uint16_t*>(v2 + 28 * v4 - 28);
    int32_t* i = reinterpret_cast<int32_t*>(v2 + 32);  // &m_items[1].needPoint
    while (point >= *i) {
        if (++v5 >= v4)
            return *reinterpret_cast<uint16_t*>(v2 + 28 * v4 - 28);
        i += 7;                                         // 28 bytes / 4
    }
    return *reinterpret_cast<uint16_t*>(v2 + 28 * v5 - 28);
}
