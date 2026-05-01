#include "Info/cltPandoraKindInfo.h"
#include "Info/cltItemKindInfo.h"
#include "Text/cltTextFileManager.h"

extern cltTextFileManager g_clTextFileManager;

// mofclient.c:25386
cltItemKindInfo* cltPandoraKindInfo::m_pclItemKindInfo = nullptr;

// mofclient.c:342909  IsDigit(char*)（global 函式，這裡內聯為 static 私有）
//   - 空字串（首字節 == 0）→ true
//   - 每次迭代先檢查一次 '+'/'-' 並跳過（不限起始位置）
//   - 然後要求 isdigit；通過則前進，碰到 NUL 即 true
bool cltPandoraKindInfo::IsDigit(const char* s)
{
    if (!s) return false;
    if (!*s) return true;
    while (true) {
        if (*s == '+' || *s == '-') ++s;
        if (!std::isdigit(static_cast<unsigned char>(*s))) return false;
        ++s;
        if (!*s) return true;
    }
}

// mofclient.c:318263 (00582BE0)
void cltPandoraKindInfo::InitializeStaticVariable(cltItemKindInfo* a1)
{
    m_pclItemKindInfo = a1;
}

// mofclient.c:318269 (00582C10)
//
// GT 反編譯流程：
//   - Buffer[1024] 全部 memset 0；strcpy(Delimiter, "\t\n")
//   - v24 = 0
//   - fopen 失敗 → return 0
//   - 連讀 3 行 header（任一失敗 → fall through 到 fclose, 回 0）
//   - fgetpos 記錄；while(fgets) ++count_ → 計算資料行數
//   - operator new(124 * count_)；memset 全 0
//   - fsetpos 回到 header 後
//   - 逐行解析；首行 fgets 失敗時 v24=1 直接成功返回
//   - 每行 51 token（1 GroupID + 10 * 5 slot）；任一中途 break 即 fall through 回 0
//   - 全部 10 slot OK 後若 fgets 失敗 → v24=1 success
int cltPandoraKindInfo::Initialize(char* String2)
{
    // GT 緩衝區：char Buffer[1024]，全部 memset 0
    char Buffer[1024];
    std::memset(Buffer, 0, sizeof(Buffer));

    // GT：char Delimiter[2]; strcpy(Delimiter, "\t\n");
    // GT 將 "\t\n\0"（3 bytes）寫入 2-byte 棧緩衝區，依賴 stack alignment 容納終止符。
    // 此處比照原樣以維持與反編譯一致；終止字元落在後續對齊空間。
    char Delimiter[2];
    std::strcpy(Delimiter, "\t\n");

    int v24 = 0;  // 對應 mofclient.c:318291 v24（return value）

    FILE* fp = g_clTextFileManager.fopen(String2);
    if (!fp) return 0;

    FILE* Stream = fp;

    // 連讀 3 行 header；任一行失敗即 fall through 到 fclose（回 0）
    if (std::fgets(Buffer, 1023, fp)
        && std::fgets(Buffer, 1023, fp)
        && std::fgets(Buffer, 1023, fp))
    {
        // 紀錄資料起點，並計算剩餘行數
        std::fpos_t Position;
        std::fgetpos(fp, &Position);
        for (; std::fgets(Buffer, 1023, fp); ++count_)
            ;

        // 配置 124 * count_ bytes，全部清零
        // 注意：使用 operator new(size) 取 raw 記憶體，與 GT 完全一致
        void* raw = ::operator new(static_cast<size_t>(sizeof(strPandoraKindInfo)) * static_cast<size_t>(count_));
        table_ = static_cast<strPandoraKindInfo*>(raw);
        std::memset(table_, 0, static_cast<size_t>(sizeof(strPandoraKindInfo)) * static_cast<size_t>(count_));

        // 回到 header 後位置
        std::fsetpos(fp, &Position);

        // GT 用 _WORD* 走指標；此處保留位元組視角以便對應每個 slot 的偏移
        uint8_t* entryPtr = reinterpret_cast<uint8_t*>(table_);

        // 第一筆讀取——若立即 EOF（例如表格只有 header），即視為成功
        if (!std::fgets(Buffer, 1023, fp)) {
            v24 = 1;
            g_clTextFileManager.fclose(fp);
            return v24;
        }

        for (;;) {
            // 首 token：GroupID
            char* tok = std::strtok(Buffer, Delimiter);
            if (!tok) {
                // GT 在此處 fall through 到 fclose；v24 維持當前值（0 或 1）
                break;
            }

            // 對應 GT：*v7 = cltPandoraKindInfo::TranslateKindCode(v8)
            // v7 指向 entry 起點（GroupID 位於 +0x00）
            *reinterpret_cast<uint16_t*>(entryPtr + 0x00) =
                cltPandoraKindInfo::TranslateKindCode(tok);

            // GT：v23 = 0；v9 = v7 + 4 (_WORD) = entry + 8 bytes（slot 0 ItemID 起點）
            int slotIdx = 0;
            bool inner_break = false;

            while (true) {
                // 計算當前 slot 的偏移基址
                // GT v9 = entry + 8 + 12 * slotIdx
                // 由此推導：
                //   prob = *((DWORD*)v9 - 1) → entry + 4 + 12*slotIdx
                //   item = *v9               → entry + 8 + 12*slotIdx
                //   min  = v9[1]             → entry + 10 + 12*slotIdx
                //   max  = v9[2]             → entry + 12 + 12*slotIdx
                strPandoraSlot* slot = reinterpret_cast<strPandoraSlot*>(
                    entryPtr + 0x04 + sizeof(strPandoraSlot) * static_cast<size_t>(slotIdx));

                // (1) prob — IsDigit + atoi
                char* tProb = std::strtok(nullptr, Delimiter);
                if (!tProb) { inner_break = true; break; }
                if (!IsDigit(tProb)) { inner_break = true; break; }
                slot->Probability = std::atoi(tProb);

                // (2) item — cltItemKindInfo::TranslateKindCode（不做 IsDigit）
                char* tItem = std::strtok(nullptr, Delimiter);
                if (!tItem) { inner_break = true; break; }
                slot->ItemID = cltItemKindInfo::TranslateKindCode(tItem);

                // (3) name — 略過（必須消費 1 個 token）
                if (!std::strtok(nullptr, Delimiter)) { inner_break = true; break; }

                // (4) min — IsDigit + atoi
                char* tMin = std::strtok(nullptr, Delimiter);
                if (!tMin) { inner_break = true; break; }
                if (!IsDigit(tMin)) { inner_break = true; break; }
                slot->MinCount = static_cast<uint16_t>(std::atoi(tMin));

                // (5) max — IsDigit + atoi
                char* tMax = std::strtok(nullptr, Delimiter);
                if (!tMax) { inner_break = true; break; }
                if (!IsDigit(tMax)) { inner_break = true; break; }
                slot->MaxCount = static_cast<uint16_t>(std::atoi(tMax));

                // 若 prob != 0，必須通過完整有效性驗證
                // GT：直接呼叫 IsValidItem，不檢查 m_pclItemKindInfo 是否為 null。
                if (slot->Probability) {
                    if (!slot->ItemID) { inner_break = true; break; }
                    if (!cltPandoraKindInfo::m_pclItemKindInfo->IsValidItem(slot->ItemID))
                    { inner_break = true; break; }
                    if (!slot->MinCount) { inner_break = true; break; }
                    if (!slot->MaxCount || slot->MaxCount < slot->MinCount)
                    { inner_break = true; break; }
                }

                // 該 slot OK；前進至下一 slot
                if (++slotIdx >= 10) {
                    // 一行 10 slot 全部解析成功；前進到下一 entry
                    entryPtr += sizeof(strPandoraKindInfo);
                    if (std::fgets(Buffer, 1023, Stream)) {
                        // 進入下一 entry 解析（goto LABEL_10）
                        break;  // 跳出 inner while，外層 for 繼續
                    }
                    // EOF — 整個檔案解析成功
                    v24 = 1;
                    g_clTextFileManager.fclose(fp);
                    return v24;
                }
            }

            if (inner_break) {
                // 解析失敗：fall through 到 fclose，v24=0
                break;
            }
            // 正常情況下 inner while 透過 break 跳出（已成功讀完 10 slot 並前進到下一行）；
            // 此處進入 for 下一輪，從 strtok(Buffer, …) 開始解析新行
        }
    }

    g_clTextFileManager.fclose(fp);
    return v24;
}

// mofclient.c:318398 (00582F30)
void cltPandoraKindInfo::Free()
{
    if (table_) {
        ::operator delete(static_cast<void*>(table_));
        table_ = nullptr;
    }
    count_ = 0;
}

// mofclient.c:318409 (00582F60)
//   依 GroupID 線性搜尋；找不到回 nullptr
strPandoraKindInfo* cltPandoraKindInfo::GetPandoraKindInfo(uint16_t a2)
{
    if (count_ <= 0) return nullptr;
    for (int i = 0; i < count_; ++i) {
        if (table_[i].GroupID == a2)
            return &table_[i];
    }
    return nullptr;
}

// mofclient.c:318430 (00582FA0)
//
// GT 邏輯：
//   v6 = 0
//   *outItem = 0；*outCount = 0
//   for ( i = (int*)((char*)entry + 4); !*i || rand()%10000 >= *i; i += 3 )
//       if (++v6 >= 10) return 0;
//   *outItem  = entry.Slots[v6].ItemID
//   v9       = entry.Slots[v6].MinCount
//   *outCount = v9 + rand() % (entry.Slots[v6].MaxCount - v9 + 1)
//   return 1
//
// 注意：GT 不在 GenerateItem 內部 reseed rand，呼叫端負責。
int cltPandoraKindInfo::GenerateItem(uint16_t a2, uint16_t* a3, uint16_t* a4)
{
    strPandoraKindInfo* entry = GetPandoraKindInfo(a2);
    if (!entry) return 0;

    int v6 = 0;
    *a3 = 0;
    *a4 = 0;

    // 依序檢查 slot；命中條件為 prob != 0 且 rand()%10000 < prob
    while (entry->Slots[v6].Probability == 0
           || (std::rand() % 10000) >= entry->Slots[v6].Probability)
    {
        if (++v6 >= 10) return 0;
    }

    *a3 = entry->Slots[v6].ItemID;
    uint16_t minCount = entry->Slots[v6].MinCount;
    *a4 = static_cast<uint16_t>(
        minCount + std::rand() % (entry->Slots[v6].MaxCount - minCount + 1));
    return 1;
}

// mofclient.c:318457 (00583090 / 00583040)
//   strlen 必須 == 5；hi = (toupper(s[0]) + 31) << 11；lo = atoi(s+1)
//   lo 必 < 0x800，否則回 0
uint16_t cltPandoraKindInfo::TranslateKindCode(char* a1)
{
    if (std::strlen(a1) != 5) return 0;

    int v2 = (std::toupper(static_cast<unsigned char>(a1[0])) + 31) << 11;
    uint16_t v3 = static_cast<uint16_t>(std::atoi(a1 + 1));
    if (v3 < 0x800u)
        return static_cast<uint16_t>(v2 | v3);
    return 0;
}
