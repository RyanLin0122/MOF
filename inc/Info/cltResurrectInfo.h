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
// 對應 resurrectinfo.txt 欄位順序（檔案標題：부활 위치）：
//   id | map id | map xpos | map ypos
struct strResurrectInfo {
    uint16_t ResurrectID;  // 韓: id                              中: 復活點ID(R****→16位代碼)             offset: 0x00
    uint16_t MapID;        // 韓: map id                          中: 地圖ID(T/F/M****→16位代碼)           offset: 0x02
    int32_t  X;            // 韓: map xpos                        中: 地圖座標X                            offset: 0x04
    int32_t  Y;            // 韓: map ypos                        中: 地圖座標Y                            offset: 0x08
};
static_assert(sizeof(strResurrectInfo) == 12, "strResurrectInfo must be 12 bytes");
#pragma pack(pop)

class cltMapInfo;

class cltResurrectInfo
{
public:
    // mofclient.c:329654 — memset(this, 0, 0x4B0u); count_ = 0
    cltResurrectInfo();

    // mofclient.c:329647 — 設置靜態 m_pclMapInfo（GT set-but-never-read）
    static void InitializeStaticVariable(cltMapInfo* a1) { m_pclMapInfo = a1; }

    // mofclient.c:329662 — 載入 resurrectinfo.txt；EOF 或無資料行回 1，其他失敗回 0
    int Initialize(char* filename);

    // mofclient.c:329744 — 線性掃描 [0..count_)；步距 12 bytes
    strResurrectInfo* GetResurrectInfo(uint16_t id);

    int Count() const { return count_; }

    // mofclient.c:329763 — 5 碼 KindCode：(toupper+31)<<11 | atoi(rest)
    static uint16_t TranslateKindCode(char* s) {
        if (!s || std::strlen(s) != 5) return 0;
        int hi = (std::toupper(static_cast<unsigned char>(s[0])) + 31) << 11;
        int lo = std::atoi(s + 1);
        if (lo < 0x800) return static_cast<uint16_t>(hi | lo);
        return 0;
    }

private:
    // 與全域 ::IsDigit (mofclient.c:342909) 行為等價：strtok 切片下永遠是
    // 非空、無正負號的純數字串，所以以下嚴格判斷與 GT 在本檔案上輸出一致。
    static bool IsDigitString(const char* s) {
        if (!s || !*s) return false;
        for (const unsigned char* p = reinterpret_cast<const unsigned char*>(s); *p; ++p)
            if (!std::isdigit(*p)) return false;
        return true;
    }

private:
    // GT 佈局：offset 0..1199 = table_ (100 × 12 bytes)；offset 1200 = count_
    strResurrectInfo table_[100];
    int              count_;

    // mofclient.c:25417 — 靜態指標，GT 中由 InitializeStaticVariable 寫入但未被讀取
    static cltMapInfo* m_pclMapInfo;
};