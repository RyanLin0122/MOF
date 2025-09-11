#include "Info/cltMoF_ChatBallonInfo.h"

int cltMoF_ChatBallonInfo::Initialize(char* filename)
{
    Free();

    const char* Delim = "\t\n";
    FILE* fp = g_clTextFileManager.fopen(filename);
    if (!fp) return 0;

    // 跳過前三行表頭/註解（反編譯：三次 _fgets(&Buffer, 10239, ...)）
    char header[10240] = { 0 };
    if (!std::fgets(header, sizeof(header), fp) ||
        !std::fgets(header, sizeof(header), fp) ||
        !std::fgets(header, sizeof(header), fp)) {
        g_clTextFileManager.fclose(fp);
        return 0;
    }

    // 記錄位置，先統計資料行數（反編譯：以 4095 bytes 緩衝）
    fpos_t pos{};
    std::fgetpos(fp, &pos);

    char countBuf[4096] = { 0 };
    count_ = 0;
    while (std::fgets(countBuf, sizeof(countBuf), fp)) {
        ++count_;
    }

    // 配置表（每筆 36 bytes）
    if (count_ > 0) {
        table_ = static_cast<strChatBallonInfo*>(std::malloc(sizeof(strChatBallonInfo) * count_));
        if (!table_) {
            g_clTextFileManager.fclose(fp);
            count_ = 0;
            return 0;
        }
        std::memset(table_, 0, sizeof(strChatBallonInfo) * count_);
    }

    // 回到資料起點
    std::fsetpos(fp, &pos);

    // 逐行解析（反編譯單筆解析時使用 2048 bytes 緩衝）
    uint16_t idx = 0;
    char line[2048] = { 0 };
    if (!std::fgets(line, sizeof(line), fp)) {
        // 無任何資料行也視為成功（反編譯會回傳 1）
        g_clTextFileManager.fclose(fp);
        return 1;
    }

    do {
        // 指到目前要寫入的記錄（以 WORD 指標行為模擬反編譯寫入）
        uint16_t* p = reinterpret_cast<uint16_t*>(&table_[idx]);

        // 1) ChatBallon ID（B****）
        char* tok = std::strtok(line, Delim);
        if (!tok) { g_clTextFileManager.fclose(fp); Free(); return 0; }
        p[0] = TranslateItemKindCode(tok);

        // 2) 기획자용이름（僅跳過）
        if (!std::strtok(nullptr, Delim)) { g_clTextFileManager.fclose(fp); Free(); return 0; }

        // 3) 이미지 ID（HEX → 寫入偏移 +4 的 32-bit 區塊）
        tok = std::strtok(nullptr, Delim);
        if (!tok) { g_clTextFileManager.fclose(fp); Free(); return 0; }
        {
            unsigned int hexv = 0;
            if (std::sscanf(tok, "%x", &hexv) != 1) { g_clTextFileManager.fclose(fp); Free(); return 0; }
            *reinterpret_cast<uint32_t*>(reinterpret_cast<uint8_t*>(p) + 4) = hexv;
        }

        // 4..14) 11 個區塊索引（WORD，從偏移 +8 開始連續存放）
        for (int i = 0; i < 11; ++i) {
            tok = std::strtok(nullptr, Delim);
            if (!tok) { g_clTextFileManager.fclose(fp); Free(); return 0; }
            // p 在 WORD 對齊下，+4 bytes 對應到 p[2]，再加 i → p[2 + i]
            p[2 + i] = static_cast<uint16_t>(std::atoi(tok));
        }

        // 15) R（BYTE，位於 record 偏移 +30）
        tok = std::strtok(nullptr, Delim);
        if (!tok) { g_clTextFileManager.fclose(fp); Free(); return 0; }
        *reinterpret_cast<uint8_t*>(reinterpret_cast<uint8_t*>(p) + 30) =
            static_cast<uint8_t>(std::atoi(tok));

        // 16) G（BYTE，偏移 +31）
        tok = std::strtok(nullptr, Delim);
        if (!tok) { g_clTextFileManager.fclose(fp); Free(); return 0; }
        *reinterpret_cast<uint8_t*>(reinterpret_cast<uint8_t*>(p) + 31) =
            static_cast<uint8_t>(std::atoi(tok));

        // 17) B（BYTE，偏移 +32）
        tok = std::strtok(nullptr, Delim);
        if (!tok) { g_clTextFileManager.fclose(fp); Free(); return 0; }
        *reinterpret_cast<uint8_t*>(reinterpret_cast<uint8_t*>(p) + 32) =
            static_cast<uint8_t>(std::atoi(tok));

        ++idx;

        // 讀入下一行；EOF 則結束並回傳成功
        if (!std::fgets(line, sizeof(line), fp)) {
            g_clTextFileManager.fclose(fp);
            return 1;
        }
    } while (idx < count_);

    g_clTextFileManager.fclose(fp);
    return 1;
}

void cltMoF_ChatBallonInfo::Free()
{
    if (table_) {
        std::free(table_);
        table_ = nullptr;
    }
    count_ = 0;
}

strChatBallonInfo* cltMoF_ChatBallonInfo::GetChatBallonInfoByKind(uint16_t kind)
{
    if (!table_ || count_ == 0) return nullptr;
    for (uint16_t i = 0; i < count_; ++i) {
        if (table_[i].ChatBallonKind == kind) return &table_[i];
    }
    return nullptr;
}