#include "Info/cltMapAreaKindInfo.h"
#include <cstring>
#include <cstdlib>
#include <cctype>

// 反編譯位置: mofclient.c:310278  (5-byte 區域 KindCode 翻譯)
//
// 演算法（與 cltMapInfo::TranslateKindCode 完全相同）:
//   if (strlen(s) != 5) return 0;
//   v2 = (toupper(s[0]) + 31) << 11;     // int 暫存
//   v3 = (uint16_t)atoi(s + 1);          // GT 將 atoi 截為 uint16
//   if (v3 < 0x800) return (uint16_t)(v2 | v3);
//   else            return 0;
uint16_t cltMapAreaKindInfo::TranslateKindCode(char* s) {
    if (std::strlen(s) != 5) return 0;
    const int v2 = (std::toupper(static_cast<unsigned char>(s[0])) + 31) << 11;
    // GT 在比較前先把 atoi 截為 uint16；保留此截斷以與反編譯結果完全等價。
    const uint16_t v3 = static_cast<uint16_t>(std::atoi(s + 1));
    if (v3 < 0x800u) return static_cast<uint16_t>(v2 | v3);
    return 0;
}

// 反編譯位置: mofclient.c:310172
// 流程（v18 = 回傳值）：
//   1. fopen；失敗 → return 0
//   2. 連讀 3 行表頭；任一失敗 → fclose, return 0
//   3. fgetpos 紀錄資料起點，預掃 fgets 累計 m_count
//   4. 若 m_count == 0 → 「return 1」（GT 不呼叫 fclose；故意鏡射此 leak）
//   5. operator new(20 * m_count) + memset 0
//   6. fsetpos 回到資料起點
//   7. 第一個資料列 fgets：失敗 → 跳到 LABEL_18 (v18 = 1)；成功 → 進入 while
//   8. while 內六欄解析；任一 strtok 失敗或 mapKind == 0 → break (v18 留 0)
//      字段寫入順序 (v9 = m_items)：
//        +0 areaKind = TranslateKindCode(tok0)         （tok 為 NULL 時 break，不寫）
//        +2 mapKind  = cltMapInfo::TranslateKindCode(tok1)
//                     （先寫，再檢查 mapk==0；若為 0 則 break）
//        +4 x        = atoi(tok2)
//        +8 y        = atoi(tok3)
//        +12 width   = atoi(tok4)
//        +16 height  = atoi(tok5)                      （NULL 檢查在寫入之前）
//      v9 += 20，再 fgets；NULL → goto LABEL_18 (v18 = 1)
//   9. fclose, return v18
int cltMapAreaKindInfo::Initialize(char* filename) {
    char Delimiter[3];
    std::strcpy(Delimiter, "\t\n");

    char Buffer[1024];
    std::memset(Buffer, 0, sizeof(Buffer));

    int v18 = 0;

    FILE* fp = g_clTextFileManager.fopen(filename);
    if (!fp) return 0;

    if (std::fgets(Buffer, 1023, fp) &&
        std::fgets(Buffer, 1023, fp) &&
        std::fgets(Buffer, 1023, fp))
    {
        fpos_t Position{};
        std::fgetpos(fp, &Position);

        // 預掃資料行數；GT 直接 ++m_count，不重置（仰賴建構式置零）。
        for ( ; std::fgets(Buffer, 1023, fp); ++m_count)
            ;

        if (m_count == 0) {
            // GT 在此直接 return 1 而不呼叫 fclose；忠實複製此 leak 以維持邏輯等價。
            return 1;
        }

        m_items = static_cast<strMapAreaKindInfo*>(
            operator new(sizeof(strMapAreaKindInfo) * static_cast<size_t>(m_count)));
        std::memset(m_items, 0, sizeof(strMapAreaKindInfo) * static_cast<size_t>(m_count));

        std::fsetpos(fp, &Position);

        char* v9 = reinterpret_cast<char*>(m_items);

        if (std::fgets(Buffer, 1023, fp)) {
            while (true) {
                // [0] ID -> areaKind @ +0  (TranslateKindCode)
                char* v10 = std::strtok(Buffer, Delimiter);
                if (!v10) break;
                *reinterpret_cast<uint16_t*>(v9 + 0) = TranslateKindCode(v10);

                // [1] mapkind -> mapKind @ +2  (cltMapInfo::TranslateKindCode)
                // GT 先寫值再檢查；mapk == 0 視為 parse 失敗。
                char* v11 = std::strtok(nullptr, Delimiter);
                if (!v11) break;
                const uint16_t v12 = cltMapInfo::TranslateKindCode(v11);
                *reinterpret_cast<uint16_t*>(v9 + 2) = v12;
                if (v12 == 0) break;

                // [2] x @ +4
                char* v13 = std::strtok(nullptr, Delimiter);
                if (!v13) break;
                *reinterpret_cast<int32_t*>(v9 + 4) = std::atoi(v13);

                // [3] y @ +8
                char* v14 = std::strtok(nullptr, Delimiter);
                if (!v14) break;
                *reinterpret_cast<int32_t*>(v9 + 8) = std::atoi(v14);

                // [4] width @ +12
                char* v15 = std::strtok(nullptr, Delimiter);
                if (!v15) break;
                *reinterpret_cast<int32_t*>(v9 + 12) = std::atoi(v15);

                // [5] heigth (sic, 檔案欄名拼錯) -> height @ +16
                char* v16 = std::strtok(nullptr, Delimiter);
                if (!v16) break;
                *reinterpret_cast<int32_t*>(v9 + 16) = std::atoi(v16);

                v9 += 20;
                if (!std::fgets(Buffer, 1023, fp)) {
                    // LABEL_18: 正常 EOF。
                    v18 = 1;
                    break;
                }
            }
        } else {
            // LABEL_18 之 else 分支（理論上 prescan>0 後 fsetpos 不會立即 EOF）。
            v18 = 1;
        }
    }

    g_clTextFileManager.fclose(fp);
    return v18;
}

// 反編譯位置: mofclient.c:310267
void cltMapAreaKindInfo::Free() {
    if (m_items) {
        operator delete(static_cast<void*>(m_items));
        m_items = nullptr;
    }
    m_count = 0;
}

// 反編譯位置: mofclient.c:310296
//   線性搜尋 m_items；步距 10 WORD (= 20 bytes / strMapAreaKindInfo)。
//   找不到回傳 nullptr；找到回傳指向該筆的指標。
strMapAreaKindInfo* cltMapAreaKindInfo::GetMapAreaKindInfo(uint16_t code) {
    if (m_count <= 0) return nullptr;
    for (int i = 0; i < m_count; ++i) {
        if (m_items[i].areaKind == code) return &m_items[i];
    }
    return nullptr;
}
