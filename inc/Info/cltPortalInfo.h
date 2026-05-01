#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include "global.h"

#pragma pack(push, 1)
// 每筆傳送門資訊（步距 = 40 bytes；對齊反編譯中 i+=20(WORD)）
// 對應 PortalList.txt 欄位順序：
//   Portal ID | MapID | 맵 이름(skip) | PosX_1 | PosY_1 | Portal type_1 |
//   도착 맵 ID | 맵 이름(skip) | PosX_2 | PosY_2 |
//   이미지 파일 | 블럭이미지 | 중간이미지(리소스ID) | 레벨 제한
struct stPortalInfo
{
    uint16_t PortalID;        // 韓: Portal ID                       中: 傳送門ID(P****→16位代碼)             offset: 0x00
    uint16_t MapID_1;         // 韓: MapID                           中: 出發地圖ID(M/T/F****)                offset: 0x02
    int32_t  PosX_1;          // 韓: PosX_1                          中: 出發座標X                            offset: 0x04
    int32_t  PosY_1;          // 韓: PosY_1                          中: 出發座標Y                            offset: 0x08
    uint16_t PortalType_1;    // 韓: Portal type_1                   中: 出發端類型(WORD)                     offset: 0x0C
    uint16_t MapID_2;         // 韓: 도착 맵 ID                       中: 到達地圖ID                           offset: 0x0E
    int32_t  PosX_2;          // 韓: PosX_2                          中: 到達座標X                            offset: 0x10
    int32_t  PosY_2;          // 韓: PosY_2                          中: 到達座標Y                            offset: 0x14
    uint32_t ImageFileHex;    // 韓: 이미지 파일 (%x)                  中: 圖像檔案(HEX 資源ID)                  offset: 0x18
    uint16_t BlockImage;      // 韓: 블럭이미지                       中: 方塊影像索引/ID(WORD)                offset: 0x1C
    uint16_t _pad0;           // 韓: (예약, 미기록)                    中: 保留對齊(反編譯未寫入)                offset: 0x1E
    uint32_t MiddleImageHex;  // 韓: 중간이미지(리소스ID) (%x)         中: 中間圖像資源ID(可含 0x 前綴)           offset: 0x20
    uint8_t  LevelLimit;      // 韓: 레벨 제한                        中: 等級限制(BYTE)                        offset: 0x24
    uint8_t  _pad1[3];        // 韓: (정렬용 패딩)                     中: 對齊填充至 40 bytes                   offset: 0x25~0x27
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