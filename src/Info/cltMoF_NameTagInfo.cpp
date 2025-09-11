#include "Info/cltMoF_NameTagInfo.h"

int cltMoF_NameTagInfo::Initialize(char* filename)
{
    Free();

    const char* Delim = "\t\n";
    FILE* fp = g_clTextFileManager.fopen(filename);
    if (!fp) return 0;

    // 跳過前三行（表頭/說明），與反編譯一致
    char header[10240] = { 0 };
    if (!std::fgets(header, sizeof(header), fp) ||
        !std::fgets(header, sizeof(header), fp) ||
        !std::fgets(header, sizeof(header), fp)) {
        g_clTextFileManager.fclose(fp);
        return 0;
    }

    // 記錄位置，先行數統計（反編譯用 4095，但實質無差異）
    fpos_t pos{};
    std::fgetpos(fp, &pos);

    char line[4096] = { 0 };
    count_ = 0;
    while (std::fgets(line, sizeof(line), fp)) {
        ++count_;
    }

    // 配置（每筆 20 bytes）
    if (count_ > 0) {
        table_ = static_cast<strNameTagInfo*>(std::malloc(sizeof(strNameTagInfo) * count_));
        if (!table_) {
            g_clTextFileManager.fclose(fp);
            count_ = 0;
            return 0;
        }
        std::memset(table_, 0, sizeof(strNameTagInfo) * count_);
    }

    // 回到資料起點
    std::fsetpos(fp, &pos);

    // 逐行解析（反編譯逐筆成功到 EOF 才回傳 1）
    uint16_t idx = 0;
    char buf[2048] = { 0 };   // 反編譯讀單筆時使用 2048
    while (idx < count_ && std::fgets(buf, sizeof(buf), fp)) {
        // 以 WORD 指針對應反編譯 v7 的寫入行為
        uint16_t* p = reinterpret_cast<uint16_t*>(&table_[idx]);

        // 1) NameTag ID（N****）
        char* tok = std::strtok(buf, Delim);
        if (!tok) { g_clTextFileManager.fclose(fp); Free(); return 0; }
        p[0] = TranslateItemKindCode(tok);

        // 2) 기획자용이름（僅跳過）
        if (!std::strtok(nullptr, Delim)) { g_clTextFileManager.fclose(fp); Free(); return 0; }

        // 3) 이미지 ID（HEX → 寫入 [2..3] 兩個 WORD 組成的 DWORD）
        tok = std::strtok(nullptr, Delim);
        if (!tok) { g_clTextFileManager.fclose(fp); Free(); return 0; }
        // 與反編譯一致：以 "%x" 直接寫入位於 p+2 的 32-bit 區域
        {
            unsigned int v = 0;
            if (std::sscanf(tok, "%x", &v) != 1) { g_clTextFileManager.fclose(fp); Free(); return 0; }
            *reinterpret_cast<uint32_t*>(&p[2]) = v;
        }

        // 4) 이미지 블록ID（WORD → p[4]）
        tok = std::strtok(nullptr, Delim);
        if (!tok) { g_clTextFileManager.fclose(fp); Free(); return 0; }
        p[4] = static_cast<uint16_t>(std::atoi(tok));

        // 5) PC방 사용자 이미지 ID（HEX → 寫入 [6..7]）
        tok = std::strtok(nullptr, Delim);
        if (!tok) { g_clTextFileManager.fclose(fp); Free(); return 0; }
        {
            unsigned int v = 0;
            if (std::sscanf(tok, "%x", &v) != 1) { g_clTextFileManager.fclose(fp); Free(); return 0; }
            *reinterpret_cast<uint32_t*>(&p[6]) = v;
        }

        // 6) PC방 사용자 블록ID（WORD → p[8]）
        tok = std::strtok(nullptr, Delim);
        if (!tok) { g_clTextFileManager.fclose(fp); Free(); return 0; }
        p[8] = static_cast<uint16_t>(std::atoi(tok));

        ++idx;
    }

    g_clTextFileManager.fclose(fp);

    // 若成功讀至 EOF，回傳 1；否則（中途失敗）已在上面 Free 並回傳 0
    return 1;
}

void cltMoF_NameTagInfo::Free()
{
    if (table_) {
        std::free(table_);
        table_ = nullptr;
    }
    count_ = 0;
}

strNameTagInfo* cltMoF_NameTagInfo::GetNameTagInfoByKind(uint16_t kind)
{
    if (!table_ || count_ == 0) return nullptr;
    for (uint16_t i = 0; i < count_; ++i) {
        if (table_[i].NameTagKind == kind) return &table_[i];
    }
    return nullptr;
}