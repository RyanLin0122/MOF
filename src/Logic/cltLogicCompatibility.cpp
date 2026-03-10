#include "Logic/cltShopInfo.h"

int cltShopInfo::GetShopInfoByID(std::uint16_t shopId, stShopInfo** outShopInfo, strPackageShopInfo** outPackageInfo) {
    if (outShopInfo) {
        *outShopInfo = nullptr;
    }
    if (outPackageInfo) {
        *outPackageInfo = nullptr;
    }
    return 0;
}

std::int64_t cltShopInfo::GetPackageItemPrice(std::uint16_t shopId, std::uint16_t itemKind, std::uint16_t itemQty) {
    return 0;
}
