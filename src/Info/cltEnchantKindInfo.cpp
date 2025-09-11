#include "Info/cltEnchantKindInfo.h"

int cltEnchantKindInfo::Initialize(char* filename)
{
    // 與反編譯一致的分隔字元：Tab 與換行
    char Delimiter[3] = "\t\n";

    // 開檔（封包內解壓 → txt.tmp → 文字模式重開）
    FILE* fp = g_clTextFileManager.fopen(filename);
    if (!fp) return 0;

    // 依反編譯碼：前 3 行為標頭/說明行，直接讀掉
    char line[1024] = { 0 };
    if (!std::fgets(line, sizeof(line), fp) ||
        !std::fgets(line, sizeof(line), fp) ||
        !std::fgets(line, sizeof(line), fp)) {
        g_clTextFileManager.fclose(fp);
        return 0;
    }

    // 記錄位置，先計數行數（逐行 fgets 到 EOF）
    fpos_t pos{};
    std::fgetpos(fp, &pos);
    count_ = 0;
    while (std::fgets(line, sizeof(line), fp)) {
        ++count_;
    }

    // 配置表格（每筆 84 bytes，清零）
    if (count_ <= 0) {
        g_clTextFileManager.fclose(fp);
        return 0;
    }
    table_ = static_cast<strEnchantKindInfo*>(std::malloc(sizeof(strEnchantKindInfo) * count_));
    if (!table_) {
        g_clTextFileManager.fclose(fp);
        return 0;
    }
    std::memset(table_, 0, sizeof(strEnchantKindInfo) * count_);

    // 回到資料起點，逐行解析
    std::fsetpos(fp, &pos);
    int index = 0;

    while (index < count_ && std::fgets(line, sizeof(line), fp)) {
        strEnchantKindInfo& rec = table_[index];

        // 1) EnchantID (A0001…)
        char* tok = std::strtok(line, Delimiter);
        if (!tok) { Free(); g_clTextFileManager.fclose(fp); return 0; }
        rec.EnchantID = TranslateKindCode(tok);

        // 2) 強化結果名稱(跳過)
        if (!std::strtok(nullptr, Delimiter)) { Free(); g_clTextFileManager.fclose(fp); return 0; }

        // 3) 物品等級(跳過)
        if (!std::strtok(nullptr, Delimiter)) { Free(); g_clTextFileManager.fclose(fp); return 0; }

        // 4) ResultItem (N****)
        tok = std::strtok(nullptr, Delimiter);
        if (!tok) { Free(); g_clTextFileManager.fclose(fp); return 0; }
        rec.ResultItem = cltItemKindInfo::TranslateKindCode(tok);
        if (rec.ResultItem == 0) { Free(); g_clTextFileManager.fclose(fp); return 0; }

        // 5) RequiredWeaponItem (H****)
        tok = std::strtok(nullptr, Delimiter);
        if (!tok) { Free(); g_clTextFileManager.fclose(fp); return 0; }
        rec.RequiredWeaponItem = cltItemKindInfo::TranslateKindCode(tok);
        if (rec.RequiredWeaponItem == 0) { Free(); g_clTextFileManager.fclose(fp); return 0; }

        // 6) 五組：石頭代碼 + (成功/蒸發/損壞) 千分比
        for (int i = 0; i < 5; ++i) {
            // 石頭 G****
            tok = std::strtok(nullptr, Delimiter);
            if (!tok) { Free(); g_clTextFileManager.fclose(fp); return 0; }
            rec.StoneKind[i] = cltItemKindInfo::TranslateKindCode(tok);
            if (rec.StoneKind[i] == 0) { Free(); g_clTextFileManager.fclose(fp); return 0; }

            // 成功率（需為數字且不可為 0）
            tok = std::strtok(nullptr, Delimiter);
            if (!tok || !IsDigitStr(tok)) { Free(); g_clTextFileManager.fclose(fp); return 0; }
            rec.SuccessPermille[i] = std::atoi(tok);
            if (rec.SuccessPermille[i] == 0) { Free(); g_clTextFileManager.fclose(fp); return 0; }

            // 蒸發率（數字，且 < 1000）
            tok = std::strtok(nullptr, Delimiter);
            if (!tok || !IsDigitStr(tok)) { Free(); g_clTextFileManager.fclose(fp); return 0; }
            rec.EvaporatePermille[i] = std::atoi(tok);
            if (rec.EvaporatePermille[i] >= 1000) { Free(); g_clTextFileManager.fclose(fp); return 0; }

            // 損壞率（數字，且 < 1000）
            tok = std::strtok(nullptr, Delimiter);
            if (!tok || !IsDigitStr(tok)) { Free(); g_clTextFileManager.fclose(fp); return 0; }
            rec.DamagePermille[i] = std::atoi(tok);
            if (rec.DamagePermille[i] >= 1000) { Free(); g_clTextFileManager.fclose(fp); return 0; }
        }

        // 7) 費用（必須 > 0）
        tok = std::strtok(nullptr, Delimiter);
        if (!tok || !IsDigitStr(tok)) { Free(); g_clTextFileManager.fclose(fp); return 0; }
        rec.Cost = std::atoi(tok);
        if (rec.Cost == 0) { Free(); g_clTextFileManager.fclose(fp); return 0; }

        // 8) 文字選項代碼（WORD）
        tok = std::strtok(nullptr, Delimiter);
        if (!tok || !IsDigitStr(tok)) { Free(); g_clTextFileManager.fclose(fp); return 0; }
        rec.OptionTextCode = static_cast<uint16_t>(std::atoi(tok));

        ++index;
    }

    g_clTextFileManager.fclose(fp);
    // 全部成功解析才回傳 1（與反編譯碼一致）
    return 1;
}

void cltEnchantKindInfo::Free()
{
    if (table_) {
        std::free(table_);
        table_ = nullptr;
    }
    count_ = 0;
}

strEnchantKindInfo* cltEnchantKindInfo::GetEnchantKindInfo(uint16_t id)
{
    if (!table_ || count_ <= 0) return nullptr;
    for (int i = 0; i < count_; ++i) {
        if (table_[i].EnchantID == id) return &table_[i];
    }
    return nullptr;
}

strEnchantKindInfo* cltEnchantKindInfo::GetEnchantKindInfoByReqHuntItemKind(uint16_t reqWeaponKind)
{
    if (!table_ || count_ <= 0) return nullptr;
    for (int i = 0; i < count_; ++i) {
        if (table_[i].RequiredWeaponItem == reqWeaponKind) return &table_[i];
    }
    return nullptr;
}

int cltEnchantKindInfo::IsEnchantableItem(uint16_t reqWeaponKind)
{
    return GetEnchantKindInfoByReqHuntItemKind(reqWeaponKind) ? 1 : 0;
}

int cltEnchantKindInfo::IsEnchantedItem(uint16_t resultItemKind)
{
    if (!table_ || count_ <= 0) return 0;
    for (int i = 0; i < count_; ++i) {
        if (table_[i].ResultItem == resultItemKind) return 1;
    }
    return 0;
}

uint16_t cltEnchantKindInfo::GetDamagedItem(uint16_t resultItemKind)
{
    if (!table_ || count_ <= 0) return 0;
    for (int i = 0; i < count_; ++i) {
        if (table_[i].ResultItem == resultItemKind) {
            return table_[i].RequiredWeaponItem; // 對齊反編譯碼：回傳偏移 +4 的 WORD
        }
    }
    return 0;
}

uint16_t cltEnchantKindInfo::TranslateKindCode(char* a1)
{
    if (!a1) return 0;
    if (std::strlen(a1) != 5) return 0;
    int hi = (std::toupper(static_cast<unsigned char>(a1[0])) + 31) << 11;
    int lo = std::atoi(a1 + 1);
    if (lo < 0x800) return static_cast<uint16_t>(hi | lo);
    return 0;
}

// 僅允許全部為 0..9；空字串或含非數字則視為非數字
bool cltEnchantKindInfo::IsDigitStr(const char* t)
{
    if (!t || !*t) return false;
    for (const unsigned char* p = (const unsigned char*)t; *p; ++p)
        if (!std::isdigit(*p)) return false;
    return true;
}