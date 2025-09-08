#pragma once
#include <cstdint>

// 每筆 4 bytes，僅第一個 WORD 會被設定，第二個保留供位移對齊
#pragma pack(push, 1)
struct strBasicAppearKindInfo {
    uint16_t code;     // 3186/3187/3188 或 3189/3190/3191
    uint16_t reserved; // 未使用；僅為了符合 4-byte 間距
};
#pragma pack(pop)

static_assert(sizeof(strBasicAppearKindInfo) == 4, "strBasicAppearKindInfo must be 4 bytes");

class cltBasicAppearKindInfo {
public:
    cltBasicAppearKindInfo();  // 依反編譯：填入 3186..3191

    // 索引 0..2；越界回傳 nullptr
    strBasicAppearKindInfo* GetHair(int index);
    strBasicAppearKindInfo* GetFace(int index);

private:
    // 連續記憶體布局：hair[0..2] 接著 face[0..2]
    strBasicAppearKindInfo hair_[3];
    strBasicAppearKindInfo face_[3];
};
