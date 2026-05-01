#include "Info/cltMoF_NameTagInfo.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <new>

// =============================================================================
// cltMoF_NameTagInfo::Initialize  ── 對應 mofclient.c:316913 (sub_581280)
//
// 反編譯關鍵流程 (逐段對照)：
//   1) 失敗旗標 v15 = 0；fopen 失敗直接 return 0
//   2) 連讀 3 行表頭 (fgets size = 10239)；任一失敗 → 跳到 fclose、return 0
//   3) fgetpos 記錄資料起點
//   4) 逐行 fgets (size = 4095) 計次到 EOF：++ this->m_count
//   5) operator new (20 * count) 配置 m_table；
//      memset(m_table, 0, 4 * ((20 * count) >> 2))  (= 20*count, 因為 20*count 必為 4 倍數)
//   6) fsetpos 回到資料起點
//   7) 逐行解析 (fgets size = 2048)：
//        v7[0]   = TranslateKindCode(tok1)        // NameTag ID  N****
//        skip      tok2 (기획자용이름)
//        sscanf("%x", v7+2, tok3)                  // 이미지 ID (HEX → DWORD)
//        v7[4]   = atoi(tok4)                      // 이미지 블록ID
//        sscanf("%x", v7+6, tok5)                  // PC방 사용자 이미지 ID (HEX)
//        v7[8]   = atoi(tok6)                      // PC방 사용자 블록ID
//        v7 += 10                                  // 推進 20 bytes
//        若任一 strtok / fgets 失敗 → 中斷迴圈 (v15 不變保持 0)
//        若下一行 fgets 回 NULL (EOF) → goto LABEL_15 設 v15 = 1
//   8) fclose、return v15
//
// 重要：反編譯版「不會」在開始時釋放舊資料，「不會」在解析失敗時釋放已配置記憶體
// (留待解構子處理)。為達邏輯完全等價，本實作保持完全一致行為。
// =============================================================================
int cltMoF_NameTagInfo::Initialize(char* filename)
{
    // === 反編譯區段：v15 = 0; Delimiter = "\t\n"; Buffer 區塊 zero-init ===
    char Delimiter[3];
    Delimiter[0] = '\t';
    Delimiter[1] = '\n';
    Delimiter[2] = '\0';

    int v15 = 0;  // 成功旗標（與反編譯命名一致）

    // 對應 mofclient.c:316928..316929 之 Buffer/v18：總長 1 + 2044 + 2 + 1 = 2048 bytes
    // 此處用單一 buffer[2048] 表達同一塊棧上空間。
    char Buffer[2048];
    std::memset(Buffer, 0, sizeof(Buffer));

    // === fopen ===
    FILE* fp = g_clTextFileManager.fopen(filename);
    if (!fp)
    {
        return 0;
    }

    // === 連讀 3 行表頭 (反編譯 fgets size = 10239) ===
    if (std::fgets(Buffer, 10239, fp) &&
        std::fgets(Buffer, 10239, fp) &&
        std::fgets(Buffer, 10239, fp))
    {
        // === fgetpos 記錄資料起點 ===
        fpos_t Position;
        std::fgetpos(fp, &Position);

        // === 逐行 fgets (size = 4095) 計次：++ this->m_count ===
        while (std::fgets(Buffer, 4095, fp))
        {
            ++m_count;
        }

        // === operator new 配置 + memset 歸零 ===
        // 反編譯：v5 = operator new(20 * count); memset(v5, 0, 4*((20*count)>>2));
        // 由於 20*count 永為 4 的倍數 → 等價於 memset(v5, 0, 20*count)。
        m_table = static_cast<strNameTagInfo*>(::operator new(static_cast<std::size_t>(20) * m_count));
        std::memset(m_table, 0, 4 * ((static_cast<std::size_t>(20) * m_count) >> 2));

        // === 回到資料起點 ===
        std::fsetpos(fp, &Position);

        // 對應反編譯 v7 = (_WORD*)this->m_table
        unsigned __int16* v7 = reinterpret_cast<unsigned __int16*>(m_table);

        // === 解析迴圈：先讀第一行；空檔則直接成功（count == 0 路徑） ===
        if (std::fgets(Buffer, 2048, fp))
        {
            // 反編譯：while(1) { ... break / goto LABEL_15 }
            for (;;)
            {
                // (1) NameTag ID
                char* v8 = std::strtok(Buffer, Delimiter);
                if (!v8) break;
                v7[0] = TranslateItemKindCode(v8);

                // (2) 기획자용이름  (skip)
                if (!std::strtok(nullptr, Delimiter)) break;

                // (3) 이미지 ID  (HEX)
                char* v9 = std::strtok(nullptr, Delimiter);
                if (!v9) break;
                std::sscanf(v9, "%x", reinterpret_cast<unsigned int*>(v7 + 2));

                // (4) 이미지 블록ID
                char* v10 = std::strtok(nullptr, Delimiter);
                if (!v10) break;
                v7[4] = static_cast<unsigned __int16>(std::atoi(v10));

                // (5) PC방 사용자 이미지 ID  (HEX)
                char* v11 = std::strtok(nullptr, Delimiter);
                if (!v11) break;
                std::sscanf(v11, "%x", reinterpret_cast<unsigned int*>(v7 + 6));

                // (6) PC방 사용자 블록ID
                char* v12 = std::strtok(nullptr, Delimiter);
                if (!v12) break;
                v7[8] = static_cast<unsigned __int16>(std::atoi(v12));

                // 推進 20 bytes (10 個 WORD)
                v7 += 10;

                // 下一行；EOF → 視為成功 (LABEL_15)
                if (!std::fgets(Buffer, 2048, fp))
                {
                    v15 = 1;
                    break;
                }
            }
        }
        else
        {
            // 反編譯 else 分支：第一次 fgets(2048) 即 NULL → 設 v15 = 1 (count 通常為 0)
            v15 = 1;
        }
    }

    // === fclose、return v15 ===
    g_clTextFileManager.fclose(fp);
    return v15;
}

// =============================================================================
// cltMoF_NameTagInfo::GetNameTagInfoByKind ── 對應 mofclient.c:316996
// 線性搜尋：對 m_count 筆資料逐筆比對 NameTagKind == kind；找不到回 nullptr。
// 反編譯使用 i += 10 (WORD 指標)；此處用 strNameTagInfo* 一筆等價。
// =============================================================================
strNameTagInfo* cltMoF_NameTagInfo::GetNameTagInfoByKind(unsigned __int16 kind)
{
    if (m_count == 0) return nullptr;
    for (unsigned __int16 i = 0; i < m_count; ++i)
    {
        if (m_table[i].NameTagKind == kind)
        {
            return &m_table[i];
        }
    }
    return nullptr;
}
