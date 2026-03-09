#pragma once

#include <cstdint>

struct stShopInfo;
struct strPackageShopInfo;

class cltShopInfo {
public:
    int GetShopInfoByID(std::uint16_t shopId, stShopInfo** outShopInfo, strPackageShopInfo** outPackageInfo);
    std::int64_t GetPackageItemPrice(std::uint16_t shopId, std::uint16_t itemKind, std::uint16_t itemQty);
};
