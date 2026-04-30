#include "Info/cltEnchantKindInfo.h"

// =============================================================================
// cltEnchantKindInfo::Initialize
// 反編譯位置：mofclient.c:300026 (sub_0056D9F0)
//
// 與 IDA 反編譯碼結構一致的還原：
//   1) cltTextFileManager::fopen 開檔（封包解壓 → txt.tmp → 文字模式重開）
//   2) 連續三次 fgets 跳過前三行（標題 / 說明 / 欄位列）
//   3) fgetpos 紀錄資料起點，再用 fgets 逐行計數至 EOF
//   4) operator new(84 * count) 配置整塊資料表並 memset 清零
//      （注意：即便 count == 0，反編譯碼仍會配置 0 bytes，並嘗試一次 fgets）
//   5) fsetpos 回到資料起點，先 fgets 一行；若該行為 NULL 直接 LABEL_37 return 1
//   6) 解析每一筆共 27 個欄位；任一欄位解析失敗就 fclose 並 return 0
//      （注意：失敗時不釋放 table，由解構式 Free() 負責收尾，與反編譯碼一致）
//   7) 每筆解析成功後再 fgets 下一行；NULL → LABEL_37 return 1
// =============================================================================
int cltEnchantKindInfo::Initialize(char* filename)
{
    // Delimiter[2] in IDA, "\t\n" 含結尾共3 bytes — 反編譯碼 strcpy 會多寫一個位元組
    // 到下一個堆疊變數，但隨即被覆寫，無實際副作用。此處用合法的 [3] 處理。
    char Delimiter[3] = "\t\n";

    FILE* fp = g_clTextFileManager.fopen(filename);
    if (!fp) return 0;

    // 反編譯碼採用 1024 bytes 的堆疊緩衝區，size 參數傳入 1023。
    char Buffer[1024] = { 0 };

    // 1) 跳過 3 行標頭/欄位列；任一行讀取失敗就 fclose 並 return 0。
    if (!std::fgets(Buffer, 1023, fp) ||
        !std::fgets(Buffer, 1023, fp) ||
        !std::fgets(Buffer, 1023, fp)) {
        g_clTextFileManager.fclose(fp);
        return 0;
    }

    // 2) 紀錄資料起點，逐行計數。
    fpos_t Position{};
    std::fgetpos(fp, &Position);
    count_ = 0;
    while (std::fgets(Buffer, 1023, fp)) {
        ++count_;
    }

    // 3) 配置 84 * count_ bytes（與反編譯碼一致：operator new + memset(0)）。
    //    即便 count_ == 0 仍會走到分配與後續 fgets，與 IDA 流程保持一致。
    table_ = static_cast<strEnchantKindInfo*>(::operator new(sizeof(strEnchantKindInfo) * static_cast<size_t>(count_)));
    if (count_ > 0) {
        std::memset(table_, 0, sizeof(strEnchantKindInfo) * static_cast<size_t>(count_));
    }

    // 4) 回到資料起點。
    std::fsetpos(fp, &Position);

    // 5) 第一次讀取資料行；若立刻 NULL 表示沒有資料行，反編譯碼回傳 1。
    if (!std::fgets(Buffer, 1023, fp)) {
        // LABEL_37：v40 = 1; fclose; return 1
        g_clTextFileManager.fclose(fp);
        return 1;
    }

    int recordIdx = 0;
    while (true) {
        strEnchantKindInfo& rec = table_[recordIdx];

        // [0] EnchantID（A0001…）；不檢查為 0（與反編譯碼一致）。
        char* tok = std::strtok(Buffer, Delimiter);
        if (!tok) {
            g_clTextFileManager.fclose(fp);
            return 0;
        }
        rec.EnchantID = TranslateKindCode(tok);

        // [1] 인챈트 결과 아이템 이름（強化結果物品名稱）→ 略；只檢查 NULL。
        if (!std::strtok(nullptr, Delimiter)) {
            g_clTextFileManager.fclose(fp);
            return 0;
        }

        // [2] 아이템 레벨（物品等級）→ 略；只檢查 NULL。
        if (!std::strtok(nullptr, Delimiter)) {
            g_clTextFileManager.fclose(fp);
            return 0;
        }

        // [3] ResultItem（強化結果物品 N****），必須非 0。
        tok = std::strtok(nullptr, Delimiter);
        if (!tok) {
            g_clTextFileManager.fclose(fp);
            return 0;
        }
        rec.ResultItem = cltItemKindInfo::TranslateKindCode(tok);
        if (rec.ResultItem == 0) {
            g_clTextFileManager.fclose(fp);
            return 0;
        }

        // [4] RequiredWeaponItem（必要武器物品 H****），必須非 0。
        tok = std::strtok(nullptr, Delimiter);
        if (!tok) {
            g_clTextFileManager.fclose(fp);
            return 0;
        }
        rec.RequiredWeaponItem = cltItemKindInfo::TranslateKindCode(tok);
        if (rec.RequiredWeaponItem == 0) {
            g_clTextFileManager.fclose(fp);
            return 0;
        }

        // [5..24] 五組（StoneKind, Success, Evaporate, Damage）。
        // 反編譯碼於每個 atoi 之後立即把值寫入欄位再做檢查；
        // 因此即便驗證失敗，當前欄位仍會留下「失敗時的值」。
        bool inner_ok = true;
        for (int i = 0; i < 5; ++i) {
            // Stone（빛나는 원석 G****），必須非 0。
            tok = std::strtok(nullptr, Delimiter);
            if (!tok) { inner_ok = false; break; }
            rec.StoneKind[i] = cltItemKindInfo::TranslateKindCode(tok);
            if (rec.StoneKind[i] == 0) { inner_ok = false; break; }

            // Success（확률, 千分比），IsDigit + atoi，必須非 0。
            tok = std::strtok(nullptr, Delimiter);
            if (!tok || !IsDigit(tok)) { inner_ok = false; break; }
            rec.SuccessPermille[i] = std::atoi(tok);
            if (rec.SuccessPermille[i] == 0) { inner_ok = false; break; }

            // Evaporate（증발 확률, 千分比），IsDigit + atoi，必須 < 1000。
            tok = std::strtok(nullptr, Delimiter);
            if (!tok || !IsDigit(tok)) { inner_ok = false; break; }
            rec.EvaporatePermille[i] = std::atoi(tok);
            if (rec.EvaporatePermille[i] >= 1000) { inner_ok = false; break; }

            // Damage（손상 확률, 千分比），IsDigit + atoi，必須 < 1000。
            tok = std::strtok(nullptr, Delimiter);
            if (!tok || !IsDigit(tok)) { inner_ok = false; break; }
            rec.DamagePermille[i] = std::atoi(tok);
            if (rec.DamagePermille[i] >= 1000) { inner_ok = false; break; }
        }
        if (!inner_ok) {
            g_clTextFileManager.fclose(fp);
            return 0;
        }

        // [25] Cost（비용），IsDigit + atoi，必須非 0。
        tok = std::strtok(nullptr, Delimiter);
        if (!tok || !IsDigit(tok)) {
            g_clTextFileManager.fclose(fp);
            return 0;
        }
        rec.Cost = std::atoi(tok);
        if (rec.Cost == 0) {
            g_clTextFileManager.fclose(fp);
            return 0;
        }

        // [26] OptionTextCode（옵션 텍스트코드），IsDigit + atoi，存為 WORD。
        tok = std::strtok(nullptr, Delimiter);
        if (!tok || !IsDigit(tok)) {
            g_clTextFileManager.fclose(fp);
            return 0;
        }
        rec.OptionTextCode = static_cast<uint16_t>(std::atoi(tok));

        ++recordIdx;

        // 讀下一行；若 NULL → LABEL_37 → return 1（成功）。
        if (!std::fgets(Buffer, 1023, fp)) {
            g_clTextFileManager.fclose(fp);
            return 1;
        }
    }
}

void cltEnchantKindInfo::Free()
{
    // 反編譯位置：mofclient.c:300213
    if (table_) {
        ::operator delete(table_);
        table_ = nullptr;
    }
    count_ = 0;
}

strEnchantKindInfo* cltEnchantKindInfo::GetEnchantKindInfo(uint16_t id)
{
    // 反編譯位置：mofclient.c:300224
    // 線性搜尋整張表，比對 EnchantID（位於記錄偏移 +0 的 WORD）。
    if (!table_ || count_ <= 0) return nullptr;
    for (int i = 0; i < count_; ++i) {
        if (table_[i].EnchantID == id) return &table_[i];
    }
    return nullptr;
}

strEnchantKindInfo* cltEnchantKindInfo::GetEnchantKindInfoByReqHuntItemKind(uint16_t reqWeaponKind)
{
    // 反編譯位置：mofclient.c:300245
    // 比對 RequiredWeaponItem（位於記錄偏移 +4 的 WORD）。
    if (!table_ || count_ <= 0) return nullptr;
    for (int i = 0; i < count_; ++i) {
        if (table_[i].RequiredWeaponItem == reqWeaponKind) return &table_[i];
    }
    return nullptr;
}

int cltEnchantKindInfo::IsEnchantableItem(uint16_t reqWeaponKind)
{
    // 反編譯位置：mofclient.c:300266
    return GetEnchantKindInfoByReqHuntItemKind(reqWeaponKind) ? 1 : 0;
}

int cltEnchantKindInfo::IsEnchantedItem(uint16_t resultItemKind)
{
    // 反編譯位置：mofclient.c:300285
    // 比對 ResultItem（位於記錄偏移 +2 的 WORD）。
    if (!table_ || count_ <= 0) return 0;
    for (int i = 0; i < count_; ++i) {
        if (table_[i].ResultItem == resultItemKind) return 1;
    }
    return 0;
}

uint16_t cltEnchantKindInfo::GetDamagedItem(uint16_t resultItemKind)
{
    // 反編譯位置：mofclient.c:300304
    // 由「強化結果」反查「需求武器」（即 +4 偏移）。
    if (!table_ || count_ <= 0) return 0;
    for (int i = 0; i < count_; ++i) {
        if (table_[i].ResultItem == resultItemKind) {
            return table_[i].RequiredWeaponItem;
        }
    }
    return 0;
}

uint16_t cltEnchantKindInfo::TranslateKindCode(char* a1)
{
    // 反編譯位置：mofclient.c:300325
    //   strlen != 5 -> 0
    //   hi = (toupper(a1[0]) + 31) << 11
    //   lo = atoi(a1 + 1)
    //   if (lo < 0x800) return hi | lo  else 0
    if (!a1) return 0;
    if (std::strlen(a1) != 5) return 0;
    int hi = (std::toupper(static_cast<unsigned char>(a1[0])) + 31) << 11;
    int lo = std::atoi(a1 + 1);
    if (lo < 0x800) return static_cast<uint16_t>(hi | lo);
    return 0;
}

// =============================================================================
// 反編譯位置：mofclient.c:342909  (全域 ::IsDigit)
// 規則（為了與反編譯碼語意完全一致而原樣搬過來）：
//   - 空字串回傳 1（true）
//   - 允許 '+' 或 '-' 出現在任何一個數字字元之前
//   - 任何其他非數字字元 -> 回傳 0
//   - 全部都通過 -> 回傳 1
// =============================================================================
bool cltEnchantKindInfo::IsDigit(const char* t)
{
    if (!t) return false;
    if (!*t) return true;  // 反編譯碼 if(!*a1) return 1
    const unsigned char* p = reinterpret_cast<const unsigned char*>(t);
    while (true) {
        if (*p == '+' || *p == '-') ++p;
        if (!std::isdigit(*p)) return false;
        ++p;
        if (!*p) return true;
    }
}
