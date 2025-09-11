#include "Info/cltResurrectInfo.h"

cltResurrectInfo::cltResurrectInfo()
{
    // memset(this, 0, 0x4B0); *((DWORD*)this + 300) = 0;
    std::memset(table_, 0, sizeof(table_)); // 1200 bytes
    count_ = 0;
}

int cltResurrectInfo::Initialize(char* filename)
{
    // 清空既有內容
    std::memset(table_, 0, sizeof(table_));
    count_ = 0;

    const char* Delim = "\t\n";
    FILE* fp = g_clTextFileManager.fopen(filename);
    if (!fp) return 0;

    int ok = 0; // v17：僅在讀到 EOF（或無資料行）時設為 1

    char buf[1024] = { 0 };

    // 跳過三行表頭
    if (!std::fgets(buf, sizeof(buf), fp) ||
        !std::fgets(buf, sizeof(buf), fp) ||
        !std::fgets(buf, sizeof(buf), fp)) {
        g_clTextFileManager.fclose(fp);
        return 0;
    }

    // 嘗試讀第一筆資料行；若沒有任何資料行，也算成功（與反編譯一致）
    if (!std::fgets(buf, sizeof(buf), fp)) {
        ok = 1;
        g_clTextFileManager.fclose(fp);
        return ok;
    }

    // 逐行解析，最多 100 筆
    do {
        if (count_ >= 100) break;

        // 1) id（R****）
        char* tok = std::strtok(buf, Delim);
        if (!tok) { ok = 0; break; }
        uint16_t id = TranslateKindCode(tok);
        if (!id) { ok = 0; break; }

        // 2) map id（T****/F****/...）
        tok = std::strtok(nullptr, Delim);
        if (!tok) { ok = 0; break; }
        uint16_t map = cltMapInfo::TranslateKindCode(tok);
        if (!map) { ok = 0; break; }

        // 3) map xpos（純數字）
        tok = std::strtok(nullptr, Delim);
        if (!tok || !IsDigitString(tok)) { ok = 0; break; }
        int x = std::atoi(tok);

        // 4) map ypos（純數字）
        tok = std::strtok(nullptr, Delim);
        if (!tok || !IsDigitString(tok)) { ok = 0; break; }
        int y = std::atoi(tok);

        // 寫入一筆
        table_[count_].ResurrectID = id;
        table_[count_].MapID = map;
        table_[count_].X = x;
        table_[count_].Y = y;
        ++count_;

        // 讀下一行；到 EOF 視為成功
        if (!std::fgets(buf, sizeof(buf), fp)) { ok = 1; break; }

    } while (true);

    g_clTextFileManager.fclose(fp);
    return ok;
}

strResurrectInfo* cltResurrectInfo::GetResurrectInfo(uint16_t a2)
{
    // 線性掃描：每筆步距 12 bytes（與反編譯 i += 12 完全一致）
    for (int i = 0; i < count_; ++i) {
        if (table_[i].ResurrectID == a2) return &table_[i];
    }
    return nullptr;
}