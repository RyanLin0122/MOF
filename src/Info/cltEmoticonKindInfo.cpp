#include "Info/cltEmoticonKindInfo.h"
#include <cstring>
#include <cstdlib>
#include <cctype>

cltEmoticonKindInfo::cltEmoticonKindInfo() {
    // vftable 由編譯器處理；其餘初始化按反編譯：m_rowCnt=0
    m_rowCnt = 0;
}

cltEmoticonKindInfo::~cltEmoticonKindInfo() {
    // 反編譯：先設 vftable，再呼叫 Free；C++ 自動處理 vptr，這裡直接 Free()
    Free();
}

uint16_t cltEmoticonKindInfo::TranslateKindCode(char* s) {
    if (!s) return 0;
    if (std::strlen(s) != 5) return 0;
    const int hi = (std::toupper(static_cast<unsigned char>(s[0])) + 31) << 11;
    const unsigned int num = static_cast<unsigned int>(std::atoi(s + 1));
    if (num < 0x800u) return static_cast<uint16_t>(hi | num);
    return 0;
}

int cltEmoticonKindInfo::Initialize(char* filename) {
    if (!filename) return 0;

    m_rowCnt = 0;
    const char* DELIMS = "\t\n";
    char line[1024] = { 0 };

    FILE* fp = g_clTextFileManager.fopen(filename);
    if (!fp) return 0;

    // 跳過表頭三行
    if (!std::fgets(line, sizeof(line), fp) ||
        !std::fgets(line, sizeof(line), fp) ||
        !std::fgets(line, sizeof(line), fp)) {
        g_clTextFileManager.fclose(fp);
        return 0;
    }

    // 記錄資料起點並預掃行數
    fpos_t pos{};
    std::fgetpos(fp, &pos);
    while (std::fgets(line, sizeof(line), fp)) ++m_rowCnt;

    // 配置原始列緩衝（188 bytes/列）
    if (m_rowCnt > 0) {
        m_rows = static_cast<stEmoticonWordInfo*>(operator new(sizeof(stEmoticonWordInfo) * m_rowCnt));
        std::memset(m_rows, 0, sizeof(stEmoticonWordInfo) * m_rowCnt);
    }

    // 回到資料起點，開始逐列剖析
    std::fsetpos(fp, &pos);

    int idx = 0;
    bool ok = false;

    if (std::fgets(line, sizeof(line), fp)) {
        do {
            // 1) 說明（丟棄）
            if (!std::strtok(line, DELIMS)) break;

            // 2) 이모티콘 이름 -> +0 WORD（數字）
            char* tok = std::strtok(nullptr, DELIMS);
            if (!tok) break;
            uint16_t nameId = static_cast<uint16_t>(std::atoi(tok));

            // 3) 이모티콘 종류 -> +8 DWORD
            tok = std::strtok(nullptr, DELIMS);
            if (!tok) break;
            uint32_t kind = static_cast<uint32_t>(std::atoi(tok));

            // 4) 아이템 아이디（Ixxxx 或 "0"）-> +4 WORD (Translate or 0)
            tok = std::strtok(nullptr, DELIMS);
            if (!tok) break;
            uint16_t itemKind = (std::strcmp(tok, "0") == 0) ? 0 : TranslateKindCode(tok);

            // 5) 대표 아이템 아이디（Ixxxx 或 "0"）-> +2 WORD
            tok = std::strtok(nullptr, DELIMS);
            if (!tok) break;
            uint16_t repItemKind = (std::strcmp(tok, "0") == 0) ? 0 : TranslateKindCode(tok);

            // 6) 리소스 ID（十六進字串；不得含 '+'）
            tok = std::strtok(nullptr, DELIMS);
            if (!tok) break;
            if (std::strstr(tok, "+")) { ok = false; break; }
            uint32_t resId = 0;
            std::sscanf(tok, "%x", &resId);

            // 7) 블록 ID -> +16 WORD
            tok = std::strtok(nullptr, DELIMS);
            if (!tok) break;
            uint16_t blockId = static_cast<uint16_t>(std::atoi(tok));

            // 8) 툴팁 ID -> +18 WORD
            tok = std::strtok(nullptr, DELIMS);
            if (!tok) break;
            uint16_t tooltipId = static_cast<uint16_t>(std::atoi(tok));

            // 9) 표현 낱말 리스트（文字ID；以 DCTTextManager 取字串後用 '/' 分割）
            tok = std::strtok(nullptr, DELIMS);
            if (!tok) break;
            uint16_t wordsTextId = static_cast<uint16_t>(std::atoi(tok));
            if (wordsTextId == 0) break;

            const char* phrase = g_DCTTextManager.GetText(wordsTextId);
            if (!phrase) phrase = "";

            // 以 '/' 切詞（最多 10 個，每格 16 bytes）
            char buf[512];
            std::strncpy(buf, phrase, sizeof(buf) - 1);
            buf[sizeof(buf) - 1] = '\0';

            int wordCount = 0;
            char* w = std::strtok(buf, "/");
            stEmoticonWordInfo rec{};
            rec.nameId = nameId;
            rec.repItemKind = repItemKind;
            rec.itemKind = itemKind;
            rec.kind = kind;
            rec.resIdHex = resId;
            rec.blockId = blockId;
            rec.tooltipId = tooltipId;
            rec.wordsTextId = wordsTextId;

            auto copyWord = [](char dst[16], const char* src) {
                if (!src) { dst[0] = '\0'; return; }
                std::strncpy(dst, src, 15);
                dst[15] = '\0';
                };

            if (w) { copyWord(rec.word0, w); ++wordCount; }
            char* nxt = nullptr;
            char* cursor = nullptr; // 為了可讀性
            (void)cursor;

            char* p = nullptr;
            // 使用 strtok(nullptr,"/") 取後續詞
            for (int slot = 1; slot < 10 && (nxt = std::strtok(nullptr, "/")); ++slot) {
                switch (slot) {
                case 1: copyWord(rec.word1, nxt); break;
                case 2: copyWord(rec.word2, nxt); break;
                case 3: copyWord(rec.word3, nxt); break;
                case 4: copyWord(rec.word4, nxt); break;
                case 5: copyWord(rec.word5, nxt); break;
                case 6: copyWord(rec.word6, nxt); break;
                case 7: copyWord(rec.word7, nxt); break;
                case 8: copyWord(rec.word8, nxt); break;
                case 9: copyWord(rec.word9, nxt); break;
                }
                ++wordCount;
            }
            rec.wordCount = static_cast<uint32_t>(wordCount);

            if (idx < m_rowCnt) {
                // 直接以位元拷貝語意模擬 qmemcpy(v5+44, rec, 0xBC)
                std::memcpy(&m_rows[idx], &rec, sizeof(stEmoticonWordInfo));
                ++idx;
            }

        } while (std::fgets(line, sizeof(line), fp));
        ok = true; // 正常到 EOF 視為成功
    }
    else {
        ok = true; // 無資料列亦視為成功
    }

    g_clTextFileManager.fclose(fp);

    if (!ok) {
        Free();
        return 0;
    }

    // 建立 984B 組資料
    return InitEmoticonItem();
}

void cltEmoticonKindInfo::Free() {
    if (m_rows) { operator delete(static_cast<void*>(m_rows));  m_rows = nullptr; }
    if (m_items) { operator delete(static_cast<void*>(m_items)); m_items = nullptr; }
    m_rowCnt = 0;
    m_itemCnt = 0;
}

// === 組裝：嚴格對照反編譯版位與流程 ===
int cltEmoticonKindInfo::InitEmoticonItem() {
    const int rc = m_rowCnt;
    if (rc % 5) return 0; // 必須為 5 的倍數

    // 依「代表道具ID」的變更次數估計組數（+1）
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

    m_itemCnt = groupsMinusOne + 1; // 反編譯：*((this)+4) = v6 + 1;
    m_items = static_cast<stEmoticonItemInfo*>(operator new(sizeof(stEmoticonItemInfo) * m_itemCnt));
    std::memset(m_items, 0, sizeof(stEmoticonItemInfo) * m_itemCnt);

    // 第一段：把 repItemKind==0 的列，依序拷到「每組」的第一筆 slot（+44）
    int copiedZero = 0;
    for (int i = 0; i < rc; ++i) {
        const stEmoticonWordInfo* src = reinterpret_cast<const stEmoticonWordInfo*>(
            reinterpret_cast<const uint8_t*>(m_rows) + i * 188
            );
        if (src->repItemKind == 0) {
            // 對應：qmemcpy(dest + 44, src, 0xBC)
            std::memcpy(reinterpret_cast<uint8_t*>(m_items) + copiedZero * 188 + 44,
                src, sizeof(stEmoticonWordInfo));
            ++copiedZero;
        }
    }
    int baseIndex = copiedZero; // v14/v35

    // 第二段：將每組的「代表道具ID」寫到組頭 +40
    if (m_itemCnt > 1) {
        for (int g = 1; g < m_itemCnt; ++g) {
            const int groupOfs = g * sizeof(stEmoticonItemInfo);
            // 從 baseIndex 起掃至尾，每列的 repItemKind 複寫到該組 +40
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

    // 第三段：對每一組：
    //  - 找到「代表道具ID」相符的 5 連列
    //  - 將其「아이템 아이디」(record +4) 依序寫入組頭 idList[20] 的當頁區段
    //  - 並把 5 列的 188B 區塊拷到該組 +44 起，連續 5 次
    if (m_itemCnt > 1) {
        int wordOffsetWords = 492; // 以 WORD 計算的偏移：等於 984 bytes
        for (int g = 1; g < m_itemCnt; ++g) {
            const int groupOfs = g * sizeof(stEmoticonItemInfo);
            int writeCountThisPage = 0;

            for (int r = baseIndex; r < rc; /*手動遞增*/) {
                const uint16_t repInGroup =
                    *reinterpret_cast<uint16_t*>(
                        reinterpret_cast<uint8_t*>(m_items) + groupOfs + 40
                        );
                const uint8_t* rowPtr = reinterpret_cast<const uint8_t*>(m_rows) + r * 188;
                const uint16_t repOfRow = *reinterpret_cast<const uint16_t*>(rowPtr + 2);

                if (repInGroup == repOfRow) {
                    // 寫入當組的 idList（WORD*）：位置 = 2 * (已寫數 + wordOffsetWords)
                    uint16_t* idListBase = reinterpret_cast<uint16_t*>(m_items);
                    uint16_t itemIdCode = *reinterpret_cast<const uint16_t*>(rowPtr + 4);
                    idListBase[writeCountThisPage + wordOffsetWords] = itemIdCode;
                    ++writeCountThisPage;

                    // 連續拷 5 次 188B 到組區（+44 起）
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
                    // 配合反編譯的位移復原
                    r += 5; // i = i + 4; 然後 ++ 變成 +5
                }
                else {
                    ++r;
                }
            }

            // 下一組切到下一頁（等效 v33 += 492）
            wordOffsetWords += 492;
        }
    }

    return 1;
}

// ===== 查詢（對照反編譯） =====

stEmoticonItemInfo* cltEmoticonKindInfo::GetEmoticonItemInfoByID(uint16_t a2) {
    if (a2 == 0) return m_items; // 索引 0 的組
    const int cnt = m_itemCnt;
    int idx = 1; // 從 1 開始掃
    if (cnt > 1) {
        uint16_t* p = reinterpret_cast<uint16_t*>(
            reinterpret_cast<uint8_t*>(m_items) + sizeof(stEmoticonItemInfo) /*+984*/
            );
        while (idx < cnt) {
            // 檢查當組開頭 20 個 WORD
            for (int j = 0; j < 20; ++j) {
                if (p[j] == a2) {
                    return reinterpret_cast<stEmoticonItemInfo*>(
                        reinterpret_cast<uint8_t*>(m_items) + sizeof(stEmoticonItemInfo) * idx
                        );
                }
            }
            ++idx;
            p = reinterpret_cast<uint16_t*>(
                reinterpret_cast<uint8_t*>(p) + sizeof(stEmoticonItemInfo) // +984 bytes -> 下一組頁面的 id 區
                );
        }
    }
    return nullptr;
}

stEmoticonItemInfo* cltEmoticonKindInfo::GetEmoticonItemInfoByIndex(uint8_t a2) {
    if (a2 < static_cast<uint8_t>(m_itemCnt))
        return reinterpret_cast<stEmoticonItemInfo*>(
            reinterpret_cast<uint8_t*>(m_items) + sizeof(stEmoticonItemInfo) * a2
            );
    return nullptr;
}

stEmoticonItemInfo* cltEmoticonKindInfo::GetEmoticonItemInfoByKind(int a2) {
    if (!a2) return nullptr;
    const int cnt = m_itemCnt;
    for (int i = 0; i < cnt; ++i) {
        uint8_t* base = reinterpret_cast<uint8_t*>(m_items) + sizeof(stEmoticonItemInfo) * i;
        // 檢查五個 slot 的 kind（偏移 base + 44 + 8，再每 188B 一筆）
        const uint32_t* p = reinterpret_cast<const uint32_t*>(base + 52);
        for (int s = 0; s < 5; ++s) {
            if (*p == static_cast<uint32_t>(a2))
                return reinterpret_cast<stEmoticonItemInfo*>(base);
            p += (188 / 4); // +=47 DWORD
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
        // 檢查當組 idList 的 20 個 WORD
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
