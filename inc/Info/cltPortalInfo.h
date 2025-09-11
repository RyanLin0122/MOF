#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include "global.h"

#pragma pack(push, 1)
// 每筆傳送門資訊（步距 = 40 bytes；對齊反編譯中 i+=20(WORD)）
struct stPortalInfo
{
    // 0x00
    uint16_t PortalID;        // Portal ID（P**** → 16 位代碼）
    // 0x02
    uint16_t MapID_1;         // MapID（出發地圖 ID，M/T/F****）
    // 0x04
    int32_t  PosX_1;          // PosX_1（出發座標X）
    // 0x08
    int32_t  PosY_1;          // PosY_1（出發座標Y）
    // 0x0C
    uint16_t PortalType_1;    // Portal type_1（出發端類型）
    // 0x0E
    uint16_t MapID_2;         // 도착 맵 ID（到達地圖 ID）
    // 0x10
    int32_t  PosX_2;          // PosX_2（到達座標X）
    // 0x14
    int32_t  PosY_2;          // PosY_2（到達座標Y）
    // 0x18
    uint32_t ImageFileHex;    // 이미지 파일（圖檔資源ID，%x）
    // 0x1C
    uint16_t BlockImage;      // 블럭이미지（方塊影像索引/ID）
    // 0x1E
    uint16_t _pad0;           // 對齊（反編譯未寫入）
    // 0x20
    uint32_t MiddleImageHex;  // 중간이미지(리소스ID)（中間圖資源ID，%x/可含0x）
    // 0x24
    uint8_t  LevelLimit;      // 레벨 제한（等級限制，BYTE）
    // 0x25~0x27
    uint8_t  _pad1[3];        // 對齊
};
static_assert(sizeof(stPortalInfo) == 40, "stPortalInfo size must be 40 bytes");
#pragma pack(pop)

class cltPortalInfo
{
public:
    cltPortalInfo() : table_(nullptr), count_(0) {}
    virtual ~cltPortalInfo() { Free(); }

    // 載入 PortalList.txt；成功回傳 1，失敗回傳 0
    int Initialize(char* filename);

    // 釋放配置
    void Free();

    // 依 PortalID 取得記錄；找不到回傳 nullptr
    stPortalInfo* GetPortalInfoByID(uint16_t id);

    // 依索引（0-based）取得記錄指標（不做邊界檢查，與反編譯一致）
    stPortalInfo* GetPortalInfoByIndex(int index) {
        return reinterpret_cast<stPortalInfo*>(
            reinterpret_cast<uint8_t*>(table_) + 40 * index);
    }

    // 回傳指定地圖中 portal 個數，並把各 portal 的索引寫入 a3[]（與反編譯行為一致）
    // 傳回值：計數
    int GetPortalCntInMap(uint16_t mapKind, int* outIndices);

    // 除錯/取用
    const stPortalInfo* data() const { return table_; }
    int count() const { return count_; }

    // KindCode 轉碼（5 碼：首字母 + 4 位數 → 16 位），與反編譯一致
    static uint16_t TranslateKindCode(const char* s) {
        if (!s || std::strlen(s) != 5) return 0;
        int hi = (std::toupper(static_cast<unsigned char>(s[0])) + 31) << 11;
        int lo = std::atoi(s + 1);
        if (lo < 0x800) return static_cast<uint16_t>(hi | lo);
        return 0;
    }

private:
    // 反編譯中的 IsDigit / IsAlphaNumeric
    static bool IsDigitString(const char* s) {
        if (!s || !*s) return false;
        for (const unsigned char* p = reinterpret_cast<const unsigned char*>(s); *p; ++p)
            if (!std::isdigit(*p)) return false;
        return true;
    }
    static bool IsAlphaNumeric(const char* s) {
        if (!s || !*s) return false;
        for (const unsigned char* p = reinterpret_cast<const unsigned char*>(s); *p; ++p)
            if (!std::isalnum(*p)) return false;
        return true;
    }

private:
    stPortalInfo* table_; // *((_DWORD*)this + 1)
    int           count_; // *((_DWORD*)this + 2)
};