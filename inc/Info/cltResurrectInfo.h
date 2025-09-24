#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include "Info/cltMapInfo.h"
#include "global.h"

#pragma pack(push, 1)
// 單筆復活資訊（12 bytes）：與反編譯之步距 12 完全一致
struct strResurrectInfo {
    uint16_t ResurrectID;  // id（R**** → 16位代碼）
    uint16_t MapID;        // map id（T****/F****… → 16位代碼）
    int32_t  X;            // map xpos
    int32_t  Y;            // map ypos
};
static_assert(sizeof(strResurrectInfo) == 12, "strResurrectInfo must be 12 bytes");
#pragma pack(pop)

class cltResurrectInfo
{
public:
    // 建構：清零 0x4B0（=1200）bytes 的表區，並將 count 清 0
    cltResurrectInfo();

    // 載入 resurrectinfo.txt；成功（完整到 EOF 或無資料行）回傳 1；失敗回傳 0
    int Initialize(char* filename);

    // 依 R**** 轉碼後的 id 取得一筆資料；找不到回傳 nullptr
    strResurrectInfo* GetResurrectInfo(uint16_t id);

    // 取得已載入筆數
    int Count() const { return count_; }

    // 與反編譯一致的 5 碼 KindCode 轉碼（字首 + 4位數 → 16位代碼）
    static uint16_t TranslateKindCode(char* s) {
        if (!s || std::strlen(s) != 5) return 0;
        int hi = (std::toupper(static_cast<unsigned char>(s[0])) + 31) << 11;
        int lo = std::atoi(s + 1);
        if (lo < 0x800) return static_cast<uint16_t>(hi | lo);
        return 0;
    }

private:
    static bool IsDigitString(const char* s) {
        if (!s || !*s) return false;
        for (const unsigned char* p = reinterpret_cast<const unsigned char*>(s); *p; ++p)
            if (!std::isdigit(*p)) return false;
        return true;
    }

private:
    // 佈局對齊反編譯：前 1200 bytes 為 100 筆表格，接著一個 DWORD 計數
    strResurrectInfo table_[100]; // 100 × 12 = 1200 bytes
    int              count_;      // 放在 offset +1200
};