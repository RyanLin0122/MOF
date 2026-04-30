#include "Info/cltMakingItemKindInfo.h"

// mofclient.c:342909  IsDigit(char*)
//
//   - 空字串（首字節 == 0）→ 1（true）
//   - 每次迭代先檢查一次 '+'/'-' 並跳過（不限起始位置）
//   - 然後要求 isdigit；通過則前進，碰到 NUL 即 true
//
// 與「只允許 0..9 的純數字字串」不同；此處刻意保留 GT 行為。
bool cltMakingItemKindInfo::IsDigit(const char* s)
{
    if (!s) return false;        // GT 並無 null 檢查；本還原為防呆
    if (!*s) return true;        // GT：空字串 → true
    while (true) {
        if (*s == '+' || *s == '-') ++s;
        if (!std::isdigit(static_cast<unsigned char>(*s))) return false;
        ++s;
        if (!*s) return true;
    }
}

// mofclient.c:309494
int cltMakingItemKindInfo::Initialize(char* filename)
{
    // GT 並未在 Initialize 開頭釋放舊資源；保留 GT 語義（global instance 在
    // 程式生命週期僅呼叫一次 Initialize；舊資源由 Free()/dtor 處理）。

    // GT 緩衝區：char Buffer[1024]，全部 memset 0。
    char Buffer[1024];
    std::memset(Buffer, 0, sizeof(Buffer));

    // strcpy(Delimiter, "\t\n") → 4-byte 區塊（佔位 \t \n \0 \0）
    char Delimiter[4] = { '\t', '\n', '\0', '\0' };

    int v40 = 0;  // 對應 mofclient.c:309533 v40（return value）

    FILE* fp = g_clTextFileManager.fopen(filename);
    if (!fp) return 0;

    if (std::fgets(Buffer, 1023, fp)
        && std::fgets(Buffer, 1023, fp)
        && std::fgets(Buffer, 1023, fp))
    {
        fpos_t Position{};
        std::fgetpos(fp, &Position);

        // 第一遍：數行
        while (std::fgets(Buffer, 1023, fp)) ++count_;

        // 對應 GT：v5 = operator new(60 * count_); memset 0；
        // 此處用 malloc/free（POD raw memory，效果相同），與 Free() 配對。
        if (count_ > 0) {
            table_ = static_cast<strMakingItemKindInfo*>(
                std::malloc(sizeof(strMakingItemKindInfo) * static_cast<size_t>(count_)));
            std::memset(table_, 0,
                        sizeof(strMakingItemKindInfo) * static_cast<size_t>(count_));
        }

        std::fsetpos(fp, &Position);

        // v7 = table_ — 以位元組指標步進 60 bytes/筆
        char* v7 = reinterpret_cast<char*>(table_);

        if (std::fgets(Buffer, 1023, fp)) {
            for (;;) {
                // (1) 제조 ID — TranslateKindCode（5 碼 → 16-bit）。GT 不檢查 IsDigit。
                char* tok = std::strtok(Buffer, Delimiter);
                if (!tok) break;
                *reinterpret_cast<uint16_t*>(v7 + 0) = TranslateKindCode(tok);

                // (2) 생산 결과 아이템 ID — 必須 != 0
                tok = std::strtok(nullptr, Delimiter);
                if (!tok) break;
                uint16_t result = cltItemKindInfo::TranslateKindCode(tok);
                *reinterpret_cast<uint16_t*>(v7 + 2) = result;
                if (!result) break;

                // (3) 분류 — atoi（GT 不做 IsDigit 檢查）
                tok = std::strtok(nullptr, Delimiter);
                if (!tok) break;
                int cat = std::atoi(tok);
                *reinterpret_cast<int32_t*>(v7 + 4) = cat;
                if (cat >= 1000 && (cat / 1000) % 2 == 0) ++indexCount_;

                // (4) 생산 개수 — IsDigit + atoi，必須 != 0
                tok = std::strtok(nullptr, Delimiter);
                if (!tok || !IsDigit(tok)) break;
                int produce = std::atoi(tok);
                *reinterpret_cast<int32_t*>(v7 + 8) = produce;
                if (!produce) break;

                // (5) 성공확률 — IsDigit + atoi，0 < x <= 10000
                tok = std::strtok(nullptr, Delimiter);
                if (!tok || !IsDigit(tok)) break;
                int success = std::atoi(tok);
                *reinterpret_cast<int32_t*>(v7 + 12) = success;
                if (!success || success > 10000) break;

                // (6) EXP보너스 — IsDigit + atoi，< 100000
                tok = std::strtok(nullptr, Delimiter);
                if (!tok || !IsDigit(tok)) break;
                int expBonus = std::atoi(tok);
                *reinterpret_cast<int32_t*>(v7 + 16) = expBonus;
                if (expBonus >= 100000) break;

                // (7) 10 組 (Kind, Count)
                int v23 = 0;
                uint16_t* v24 = reinterpret_cast<uint16_t*>(v7 + 22); // → Ingredient[0].Count
                bool ing_break = false;
                bool advance_to_next_row = false;
                while (true) {
                    // 7a) Kind — TranslateKindCode（不檢查 IsDigit；可為 "0" 或 G****）
                    char* k = std::strtok(nullptr, Delimiter);
                    if (!k) { ing_break = true; break; }
                    *(v24 - 1) = cltItemKindInfo::TranslateKindCode(k);

                    // 7b) Count — IsDigit + atoi（不檢查值範圍）
                    char* c = std::strtok(nullptr, Delimiter);
                    if (!c || !IsDigit(c)) { ing_break = true; break; }
                    *v24 = static_cast<uint16_t>(std::atoi(c));

                    ++v23;
                    v24 += 2;
                    if (v23 >= 10) {
                        v7 += 60;
                        if (!std::fgets(Buffer, 1023, fp)) {
                            v40 = 1;                 // → LABEL_32：成功
                        } else {
                            advance_to_next_row = true;
                        }
                        break;
                    }
                }
                if (ing_break) break;
                if (v40 == 1) break;
                if (!advance_to_next_row) break;     // 防呆；GT 邏輯不會走到
                // 否則繼續解析下一行（goto LABEL_8）
            }
        } else {
            // 第一筆 fgets 即 NULL（資料區為空但 header OK）→ LABEL_32：v40 = 1
            v40 = 1;
        }
    }

    g_clTextFileManager.fclose(fp);

    // mofclient.c:309661 LABEL_32：成功才建立 index_
    if (v40 == 1 && indexCount_ > 0) {
        index_ = static_cast<strMakingItemKindInfo**>(
            std::malloc(sizeof(strMakingItemKindInfo*) * static_cast<size_t>(indexCount_)));
        std::memset(index_, 0,
                    sizeof(strMakingItemKindInfo*) * static_cast<size_t>(indexCount_));
        int w = 0;
        for (int i = 0; i < count_; ++i) {
            int32_t c = table_[i].Category;
            if (c >= 1000 && (c / 1000) % 2 == 0) {
                index_[w++] = &table_[i];
            }
        }
    }
    return v40;
}

// mofclient.c:309698
void cltMakingItemKindInfo::Free()
{
    if (table_) { std::free(table_); table_ = nullptr; }
    count_ = 0;
    if (index_) { std::free(index_); index_ = nullptr; }
    indexCount_ = 0;
}

// mofclient.c:309718
strMakingItemKindInfo* cltMakingItemKindInfo::GetMakingItemKindInfo(uint16_t makingId)
{
    if (count_ <= 0) return nullptr;
    // GT：以 WORD 指標步進 30（= 60 bytes / sizeof(WORD)）做線性掃描，
    // 比對 record 起始（offset 0 = MakingID）。
    for (int i = 0; i < count_; ++i) {
        if (table_[i].MakingID == makingId) return &table_[i];
    }
    return nullptr;
}

// mofclient.c:309739
strMakingItemKindInfo* cltMakingItemKindInfo::GetMakingItemKindInfoByItemID(uint16_t firstIngredientKind)
{
    if (indexCount_ <= 0) return nullptr;
    // GT：掃 index_，比對 *(WORD*)(*entry + 20) = Ingredient[0].Kind。
    for (int i = 0; i < indexCount_; ++i) {
        if (index_[i] && index_[i]->Ingredient[0].Kind == firstIngredientKind) {
            return index_[i];
        }
    }
    return nullptr;
}

// mofclient.c:309760
uint16_t cltMakingItemKindInfo::TranslateKindCode(char* s)
{
    if (!s) return 0;                  // GT 無 null 檢查；防呆
    if (std::strlen(s) != 5) return 0;
    int hi = (std::toupper(static_cast<unsigned char>(s[0])) + 31) << 11;
    int lo = std::atoi(s + 1);
    if (lo < 0x800) return static_cast<uint16_t>(hi | lo);
    return 0;
}
