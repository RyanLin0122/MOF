#include "Info/cltBasicAppearKindInfo.h"

cltBasicAppearKindInfo::cltBasicAppearKindInfo() {
    // 與反編譯一致：寫入 6 個 WORD，間距為 4 bytes
    hair_[0].code = 3186;
    hair_[1].code = 3187;
    hair_[2].code = 3188;

    face_[0].code = 3189;
    face_[1].code = 3190;
    face_[2].code = 3191;
}

strBasicAppearKindInfo* cltBasicAppearKindInfo::GetHair(int index) {
    if (index < 0 || index >= 3) return nullptr;
    return &hair_[index]; // 位移 0/4/8
}

strBasicAppearKindInfo* cltBasicAppearKindInfo::GetFace(int index) {
    if (index < 0 || index >= 3) return nullptr;
    return &face_[index]; // 起點位移 12，再加 0/4/8
}
