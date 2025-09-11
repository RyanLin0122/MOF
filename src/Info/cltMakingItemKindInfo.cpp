#include "Info/cltMakingItemKindInfo.h"


bool cltMakingItemKindInfo::IsDigitString(const char* s)
{
    if (!s || !*s) return false;
    for (const unsigned char* p = reinterpret_cast<const unsigned char*>(s); *p; ++p)
        if (!std::isdigit(*p)) return false;
    return true;
}

int cltMakingItemKindInfo::Initialize(char* filename)
{
    Free();

    const char* Delim = "\t\n";

    FILE* fp = g_clTextFileManager.fopen(filename);
    if (!fp) return 0;

    char line[1024] = { 0 };

    // 前 3 行為標頭/備註，與反編譯一致先吃掉
    if (!std::fgets(line, sizeof(line), fp) ||
        !std::fgets(line, sizeof(line), fp) ||
        !std::fgets(line, sizeof(line), fp)) {
        g_clTextFileManager.fclose(fp);
        return 0;
    }

    // 先數行
    fpos_t pos{};
    std::fgetpos(fp, &pos);
    while (std::fgets(line, sizeof(line), fp)) ++count_;

    if (count_ <= 0) { g_clTextFileManager.fclose(fp); return 0; }

    table_ = static_cast<strMakingItemKindInfo*>(std::malloc(sizeof(strMakingItemKindInfo) * count_));
    if (!table_) { g_clTextFileManager.fclose(fp); return 0; }
    std::memset(table_, 0, sizeof(strMakingItemKindInfo) * count_);

    // 回到資料起點
    std::fsetpos(fp, &pos);

    int idx = 0;
    // 讀每一行
    while (idx < count_ && std::fgets(line, sizeof(line), fp)) {
        strMakingItemKindInfo& rec = table_[idx];

        // 1) 제조 ID (A****)
        char* tok = std::strtok(line, Delim);
        if (!tok) { Free(); g_clTextFileManager.fclose(fp); return 0; }
        rec.MakingID = TranslateKindCode(tok);

        // 2) 생산 결과 아이템 ID (H****)
        tok = std::strtok(nullptr, Delim);
        if (!tok) { Free(); g_clTextFileManager.fclose(fp); return 0; }
        rec.ResultItemID = cltItemKindInfo::TranslateKindCode(tok);
        if (rec.ResultItemID == 0) { Free(); g_clTextFileManager.fclose(fp); return 0; }

        // 3) 분류
        tok = std::strtok(nullptr, Delim);
        if (!tok || !IsDigitString(tok)) { Free(); g_clTextFileManager.fclose(fp); return 0; }
        rec.Category = std::atoi(tok);
        // 建立索引的計數條件：>=1000 且 千位數為偶數
        if (rec.Category >= 1000 && ((rec.Category / 1000) % 2 == 0)) ++indexCount_;

        // 4) 생산 개수
        tok = std::strtok(nullptr, Delim);
        if (!tok || !IsDigitString(tok)) { Free(); g_clTextFileManager.fclose(fp); return 0; }
        rec.ProduceCount = std::atoi(tok);
        if (rec.ProduceCount == 0) { Free(); g_clTextFileManager.fclose(fp); return 0; }

        // 5) 성공확률 (<= 10000)
        tok = std::strtok(nullptr, Delim);
        if (!tok || !IsDigitString(tok)) { Free(); g_clTextFileManager.fclose(fp); return 0; }
        rec.SuccessPermyriad = std::atoi(tok);
        if (rec.SuccessPermyriad == 0 || rec.SuccessPermyriad > 10000) { Free(); g_clTextFileManager.fclose(fp); return 0; }

        // 6) EXP보너스 (< 100000)
        tok = std::strtok(nullptr, Delim);
        if (!tok || !IsDigitString(tok)) { Free(); g_clTextFileManager.fclose(fp); return 0; }
        rec.ExpBonus = std::atoi(tok);
        if (rec.ExpBonus >= 100000) { Free(); g_clTextFileManager.fclose(fp); return 0; }

        // 7) 10組材料 (아이템_i, 아이템_i_수량)
        for (int i = 0; i < 10; ++i) {
            // 物品代碼（G**** or 0）
            tok = std::strtok(nullptr, Delim);
            if (!tok) { Free(); g_clTextFileManager.fclose(fp); return 0; }
            rec.Ingredient[i].Kind = cltItemKindInfo::TranslateKindCode(tok); // 0 代表無此材料欄

            // 數量（可為 0）
            tok = std::strtok(nullptr, Delim);
            if (!tok || !IsDigitString(tok)) { Free(); g_clTextFileManager.fclose(fp); return 0; }
            rec.Ingredient[i].Count = static_cast<uint16_t>(std::atoi(tok));
        }

        ++idx;
    }

    // 建立索引（與反編譯一致：僅收錄「分類千位為偶數且>=1000」的配方）
    if (indexCount_ > 0) {
        index_ = static_cast<strMakingItemKindInfo**>(std::malloc(sizeof(strMakingItemKindInfo*) * indexCount_));
        if (!index_) { Free(); g_clTextFileManager.fclose(fp); return 0; }
        std::memset(index_, 0, sizeof(strMakingItemKindInfo*) * indexCount_);

        int w = 0;
        for (int i = 0; i < count_; ++i) {
            const int c = table_[i].Category;
            if (c >= 1000 && ((c / 1000) % 2 == 0)) {
                index_[w++] = &table_[i];
            }
        }
    }

    g_clTextFileManager.fclose(fp);
    return 1;
}

void cltMakingItemKindInfo::Free()
{
    if (table_) { std::free(table_); table_ = nullptr; }
    count_ = 0;

    if (index_) { std::free(index_); index_ = nullptr; }
    indexCount_ = 0;
}

strMakingItemKindInfo* cltMakingItemKindInfo::GetMakingItemKindInfo(uint16_t makingId)
{
    if (!table_ || count_ <= 0) return nullptr;
    for (int i = 0; i < count_; ++i) {
        if (table_[i].MakingID == makingId) return &table_[i];
    }
    return nullptr;
}

strMakingItemKindInfo* cltMakingItemKindInfo::GetMakingItemKindInfoByItemID(uint16_t firstIngredientKind)
{
    // 反編譯：只掃描 index_，匹配位於位移 +20 的 WORD（即 Ingredient[0].Kind）
    if (!index_ || indexCount_ <= 0) return nullptr;
    for (int i = 0; i < indexCount_; ++i) {
        if (index_[i] && index_[i]->Ingredient[0].Kind == firstIngredientKind) {
            return index_[i];
        }
    }
    return nullptr;
}

uint16_t cltMakingItemKindInfo::TranslateKindCode(char* s)
{
    if (!s) return 0;
    if (std::strlen(s) != 5) return 0;
    int hi = (std::toupper(static_cast<unsigned char>(s[0])) + 31) << 11;
    int lo = std::atoi(s + 1);
    if (lo < 0x800) return static_cast<uint16_t>(hi | lo);
    return 0;
}
