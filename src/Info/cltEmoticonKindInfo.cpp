#include "Info/cltEmoticonKindInfo.h"
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cctype>

// ----------------------------------------------------------------------------
// 反編譯來源：mofclient.c:299162（ctor）/ 299183（dtor）/ 299191（Initialize）
//                 299339（InitEmoticonItem）/ 299545..299636（Get*）/ 299645（Free）
// 嚴格保留 GT 的下列特殊行為：
//   * Initialize 在 parse 失敗（任一 strtok 回 NULL、'+' 出現於 resID、
//     wordsTextId==0 等）後 **不**呼叫 Free()，僅 fclose + return 0。
//   * Free() 只把 m_rows / m_items 置 0，**不**動 m_rowCnt / m_itemCnt。
//   * Initialize 配置 m_rows 後 **不** memset(0)；每列依解析流程逐欄寫入；
//     若中途 break，未寫到的欄位會保留 heap 殘值。
//   * 詞條解析直接把 DCTTextManager.GetText() 結果以 sprintf 寫回 strtok 槽
//     （同 GT _wsprintfA），再以 "/" 切分後 strcpy 至 word0..word9（無長度
//     檢查），詞數 v19 無上限累加 — 詞數>10 會踩進 +182(_padB)/+184(wordCount)。
//   * GetEmoticonItemInfoByIndex 用 int 比對 m_itemCnt（u8 → 經 integer
//     promotion → int），不做 u8 截斷。
//   * TranslateKindCode 不做 nullptr 防護。
// ----------------------------------------------------------------------------

cltEmoticonKindInfo::cltEmoticonKindInfo() {
    // GT (mofclient.c:299167-299168)：vftable + *((DWORD*)this+2)=0
    // 對應這裡的 m_rowCnt = 0；其餘成員交由 default-member-initializer
    // 在類別宣告處設為 nullptr / 0（與 BSS-allocated g_clEmoticonKindInfo 等價）。
    m_rowCnt = 0;
}

cltEmoticonKindInfo::~cltEmoticonKindInfo() {
    // GT (mofclient.c:299185-299186)：vftable + Free()
    Free();
}

// ===== mofclient.c:299660 — TranslateKindCode =====
// 直接 strlen，無 null check（呼叫端皆為 strtok 結果，不會傳 null）。
uint16_t cltEmoticonKindInfo::TranslateKindCode(char* s) {
    if (std::strlen(s) != 5) return 0;
    const int v3 = (std::toupper(static_cast<unsigned char>(s[0])) + 31) << 11;
    const uint16_t v4 = static_cast<uint16_t>(std::atoi(s + 1));
    if (v4 < 0x800u) return static_cast<uint16_t>(v3 | v4);
    return 0;
}

// ===== mofclient.c:299191 — Initialize =====
int cltEmoticonKindInfo::Initialize(char* filename) {
    char Delimiter[6] = { '\t', '\n', '\0', '\0', '\0', '\0' };
    char WordDelim[2] = { '/',  '\0' };
    int  v28 = 0;                 // 預設回傳值（失敗路徑保持為 0）
    char Buffer[1024];
    std::memset(Buffer, 0, sizeof(Buffer));

    m_rowCnt = 0;                 // GT line 299236: *((DWORD*)this+2) = 0;

    FILE* fp = g_clTextFileManager.fopen(filename);
    if (!fp) return 0;

    if (std::fgets(Buffer, 1023, fp)
        && std::fgets(Buffer, 1023, fp)
        && std::fgets(Buffer, 1023, fp))
    {
        fpos_t Position{};
        std::fgetpos(fp, &Position);

        // 預掃資料列數
        for (; std::fgets(Buffer, 1023, fp); ) ++m_rowCnt;

        // 配置 m_rows（不 memset；GT 沒做）
        m_rows = static_cast<stEmoticonWordInfo*>(
            operator new(sizeof(stEmoticonWordInfo) * static_cast<size_t>(m_rowCnt)));

        std::fsetpos(fp, &Position);

        char* v5 = reinterpret_cast<char*>(m_rows);

        if (!std::fgets(Buffer, 1023, fp)) {
            // 無資料列：直接走 LABEL_32（成功路徑）
            v28 = InitEmoticonItem();
            g_clTextFileManager.fclose(fp);
            return v28;
        }

        // 外層 while 條件：strtok(Buffer, Delimiter) — 第一個 token (描述) 直接丟棄
        while (std::strtok(Buffer, Delimiter)) {
            // [1] 이모티콘 이름 -> +0 WORD
            char* v6 = std::strtok(nullptr, Delimiter);
            if (!v6) break;
            *reinterpret_cast<uint16_t*>(v5 + 0) = static_cast<uint16_t>(std::atoi(v6));

            // [2] 이모티콘 종류 -> +8 DWORD
            char* v7 = std::strtok(nullptr, Delimiter);
            if (!v7) break;
            *reinterpret_cast<uint32_t*>(v5 + 8) = static_cast<uint32_t>(std::atoi(v7));

            // [3] 아이템 아이디 -> +4 WORD ("0" → 0; 否則 TranslateKindCode)
            char* v8 = std::strtok(nullptr, Delimiter);
            if (!v8) break;
            *reinterpret_cast<uint16_t*>(v5 + 4) =
                (std::strcmp(v8, "0") == 0) ? 0 : TranslateKindCode(v8);

            // [4] 대표 아이템 아이디 -> +2 WORD ("0" → 0; 否則 TranslateKindCode)
            char* v9 = std::strtok(nullptr, Delimiter);
            if (!v9) break;
            *reinterpret_cast<uint16_t*>(v5 + 2) =
                (std::strcmp(v9, "0") == 0) ? 0 : TranslateKindCode(v9);

            // [5] 리소스 ID（hex；含 '+' 視為 parse error）
            char* v10 = std::strtok(nullptr, Delimiter);
            if (!v10) break;
            if (std::strstr(v10, "+")) break;
            unsigned int v27 = 0;
            std::sscanf(v10, "%x", &v27);
            *reinterpret_cast<uint32_t*>(v5 + 12) = v27;

            // [6] 블록 ID -> +16 WORD
            char* v12 = std::strtok(nullptr, Delimiter);
            if (!v12) break;
            *reinterpret_cast<uint16_t*>(v5 + 16) = static_cast<uint16_t>(std::atoi(v12));

            // [7] 툴팁 ID -> +18 WORD
            char* v13 = std::strtok(nullptr, Delimiter);
            if (!v13) break;
            *reinterpret_cast<uint16_t*>(v5 + 18) = static_cast<uint16_t>(std::atoi(v13));

            // [8] 표현 낱말 리스트 ID -> +20 WORD（==0 視為 parse error）
            char* v15 = std::strtok(nullptr, Delimiter);
            if (!v15) break;
            const uint16_t v16 = static_cast<uint16_t>(std::atoi(v15));
            *reinterpret_cast<uint16_t*>(v5 + 20) = v16;
            if (!v16) break;

            // ===== GT 詞條解析（mofclient.c:299300-299322）=====
            //   _wsprintfA(v15, "%s", DCTTextManager::GetText(v16))
            //   v15 是上一個 strtok 取得的指標（落在 Buffer[1024] 內）；
            //   等同把 phrase 直接覆寫回該 token 槽，再用 "/" 二次 strtok。
            //   word0..word9 各 16 bytes 槽；GT 用 strcpy 不檢查長度。
            //   v19 (wordCount) 不檢查上限，>10 詞會位移寫入 +182/+184。
            const char* v17 = g_DCTTextManager.GetText(v16);
            if (!v17) v17 = "";
            std::sprintf(v15, "%s", v17);   // 等同 _wsprintfA(v15, "%s", v17)

            char* v18 = std::strtok(v15, WordDelim);
            if (!v18) break;

            int v19 = 1;
            std::strcpy(v5 + 22, v18);      // word0

            char* v20 = std::strtok(nullptr, WordDelim);
            if (v20) {
                char* v21 = v5 + 38;        // word1 起點，每次 +16
                do {
                    std::strcpy(v21, v20);  // 不檢查長度（GT 行為）
                    ++v19;                   // 不上限累加
                    v21 += 16;
                    v20 = std::strtok(nullptr, WordDelim);
                } while (v20);
            }

            *reinterpret_cast<uint32_t*>(v5 + 184) = static_cast<uint32_t>(v19);

            v5 += 188;

            if (!std::fgets(Buffer, 1023, fp)) {
                // LABEL_32 成功路徑：呼叫 InitEmoticonItem 並回傳其結果
                v28 = InitEmoticonItem();
                g_clTextFileManager.fclose(fp);
                return v28;
            }
        }
        // 外層 while 條件失敗 或 內部 break：fall through 到 fclose / return 0
    }

    g_clTextFileManager.fclose(fp);
    return v28;  // 失敗路徑：v28 仍為 0；不呼叫 Free()
}

// ===== mofclient.c:299645 — Free =====
//   只把 m_rows / m_items 置 0；不動 m_rowCnt / m_itemCnt（GT 行為）。
void cltEmoticonKindInfo::Free() {
    if (m_rows) {
        operator delete(static_cast<void*>(m_rows));
        m_rows = nullptr;
    }
    if (m_items) {
        operator delete(static_cast<void*>(m_items));
        m_items = nullptr;
    }
}

// ===== mofclient.c:299339 — InitEmoticonItem =====
int cltEmoticonKindInfo::InitEmoticonItem() {
    const int rc = m_rowCnt;
    if (rc % 5) return 0; // 必須為 5 的倍數

    // Phase 0：依「대표 아이템 아이디」變更次數估計群組數（+1）
    int groupsMinusOne = 0;
    uint16_t prevRep = 0;
    if (rc > 0) {
        const uint16_t* p = reinterpret_cast<const uint16_t*>(
            reinterpret_cast<const uint8_t*>(m_rows) + 2 /* +2: repItemKind */
            );
        for (int i = 0; i < rc; ++i) {
            if (prevRep != *p) { prevRep = *p; ++groupsMinusOne; }
            p = reinterpret_cast<const uint16_t*>(
                reinterpret_cast<const uint8_t*>(p) + 188 /* 每列 188B 前進 */
                );
        }
    }

    m_itemCnt = groupsMinusOne + 1;
    m_items = static_cast<stEmoticonItemInfo*>(operator new(sizeof(stEmoticonItemInfo) * m_itemCnt));
    std::memset(m_items, 0, sizeof(stEmoticonItemInfo) * m_itemCnt);

    // Phase 1：把 repItemKind==0 的列依序拷到「每組」的第一筆 slot（+44）
    int copiedZero = 0;
    for (int i = 0; i < rc; ++i) {
        const stEmoticonWordInfo* src = reinterpret_cast<const stEmoticonWordInfo*>(
            reinterpret_cast<const uint8_t*>(m_rows) + i * 188
            );
        if (src->repItemKind == 0) {
            std::memcpy(reinterpret_cast<uint8_t*>(m_items) + copiedZero * 188 + 44,
                src, sizeof(stEmoticonWordInfo));
            ++copiedZero;
        }
    }
    int baseIndex = copiedZero;

    // Phase 2：將每組的「대표 아이템 아이디」寫到組頭 +40
    if (m_itemCnt > 1) {
        for (int g = 1; g < m_itemCnt; ++g) {
            const int groupOfs = g * sizeof(stEmoticonItemInfo);
            for (int r = baseIndex; r < rc; ++r) {
                const uint16_t rep = *reinterpret_cast<const uint16_t*>(
                    reinterpret_cast<const uint8_t*>(m_rows) + r * 188 + 2
                    );
                uint16_t& headerRep =
                    *reinterpret_cast<uint16_t*>(reinterpret_cast<uint8_t*>(m_items) + groupOfs + 40);
                if (headerRep != rep) headerRep = rep;
            }
        }
    }

    // Phase 3：v34（writeCountTotal）跨群組累計，永不重置
    if (m_itemCnt > 1) {
        int wordOffsetWords = 492;
        int writeCountTotal = 0;
        for (int g = 1; g < m_itemCnt; ++g) {
            const int groupOfs = g * sizeof(stEmoticonItemInfo);

            for (int r = baseIndex; r < rc; /*手動遞增*/) {
                const uint16_t repInGroup =
                    *reinterpret_cast<uint16_t*>(
                        reinterpret_cast<uint8_t*>(m_items) + groupOfs + 40
                        );
                const uint8_t* rowPtr = reinterpret_cast<const uint8_t*>(m_rows) + r * 188;
                const uint16_t repOfRow = *reinterpret_cast<const uint16_t*>(rowPtr + 2);

                if (repInGroup == repOfRow) {
                    uint16_t* idListBase = reinterpret_cast<uint16_t*>(m_items);
                    uint16_t itemIdCode = *reinterpret_cast<const uint16_t*>(rowPtr + 4);
                    idListBase[writeCountTotal + wordOffsetWords] = itemIdCode;
                    ++writeCountTotal;

                    int times = 5;
                    int ofs = groupOfs + 44;
                    int srcOfs = r * 188;
                    while (times--) {
                        std::memcpy(reinterpret_cast<uint8_t*>(m_items) + ofs,
                            reinterpret_cast<const uint8_t*>(m_rows) + srcOfs,
                            sizeof(stEmoticonWordInfo));
                        ofs += 188;
                        srcOfs += 188;
                    }
                    r += 5;
                }
                else {
                    ++r;
                }
            }
            wordOffsetWords += 492;
        }
    }

    return 1;
}

// ===== 查詢介面 =====

stEmoticonItemInfo* cltEmoticonKindInfo::GetEmoticonItemInfoByID(uint16_t a2) {
    if (a2 == 0) return m_items;
    const int cnt = m_itemCnt;
    int idx = 1;
    if (cnt > 1) {
        uint16_t* p = reinterpret_cast<uint16_t*>(
            reinterpret_cast<uint8_t*>(m_items) + sizeof(stEmoticonItemInfo)
            );
        while (idx < cnt) {
            for (int j = 0; j < 20; ++j) {
                if (p[j] == a2) {
                    return reinterpret_cast<stEmoticonItemInfo*>(
                        reinterpret_cast<uint8_t*>(m_items) + sizeof(stEmoticonItemInfo) * idx
                        );
                }
            }
            ++idx;
            p = reinterpret_cast<uint16_t*>(
                reinterpret_cast<uint8_t*>(p) + sizeof(stEmoticonItemInfo)
                );
        }
    }
    return nullptr;
}

// ===== mofclient.c:299545 — GetEmoticonItemInfoByIndex =====
//   GT: if (a2 < *((int*)this+4)) — a2(u8) 經 integer promotion 變 int，
//   再與完整 m_itemCnt(int) 比較，**不做 u8 截斷**。
stEmoticonItemInfo* cltEmoticonKindInfo::GetEmoticonItemInfoByIndex(uint8_t a2) {
    if (static_cast<int>(a2) < m_itemCnt) {
        return reinterpret_cast<stEmoticonItemInfo*>(
            reinterpret_cast<uint8_t*>(m_items) + sizeof(stEmoticonItemInfo) * a2
            );
    }
    return nullptr;
}

stEmoticonItemInfo* cltEmoticonKindInfo::GetEmoticonItemInfoByKind(int a2) {
    if (!a2) return nullptr;
    const int cnt = m_itemCnt;
    for (int i = 0; i < cnt; ++i) {
        uint8_t* base = reinterpret_cast<uint8_t*>(m_items) + sizeof(stEmoticonItemInfo) * i;
        const uint32_t* p = reinterpret_cast<const uint32_t*>(base + 52);
        for (int s = 0; s < 5; ++s) {
            if (*p == static_cast<uint32_t>(a2))
                return reinterpret_cast<stEmoticonItemInfo*>(base);
            p += (188 / 4);
        }
    }
    return nullptr;
}

stEmoticonWordInfo* cltEmoticonKindInfo::GetEmoticonWordInfoByKind(int a2) {
    if (!a2) return nullptr;
    const int cnt = m_itemCnt;
    for (int i = 0; i < cnt; ++i) {
        uint8_t* base = reinterpret_cast<uint8_t*>(m_items) + sizeof(stEmoticonItemInfo) * i;
        const uint32_t* p = reinterpret_cast<const uint32_t*>(base + 52);
        for (int s = 0; s < 5; ++s) {
            if (*p == static_cast<uint32_t>(a2))
                return reinterpret_cast<stEmoticonWordInfo*>(base + 44 + 188 * s);
            p += (188 / 4);
        }
    }
    return nullptr;
}

int cltEmoticonKindInfo::GetEmoticonItemCnt() {
    return m_itemCnt;
}

int cltEmoticonKindInfo::IsEmoticonItem(uint16_t a2) {
    int idx = 0;
    if (m_itemCnt <= 0) return 0;
    int ofs = 0;
    while (idx < m_itemCnt) {
        if (!GetEmoticonItemInfoByIndex(static_cast<uint8_t>(idx))) {
            ++idx; ofs += sizeof(stEmoticonItemInfo); continue;
        }
        const uint16_t* list = reinterpret_cast<const uint16_t*>(
            reinterpret_cast<const uint8_t*>(m_items) + ofs
            );
        for (int j = 0; j < 20; ++j) {
            if (list[j] == a2) return 1;
        }
        ++idx;
        ofs += sizeof(stEmoticonItemInfo);
    }
    return 0;
}
