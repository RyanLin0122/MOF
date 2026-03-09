#include "Network/CMofMsg.h"
#include "Logic/cltShopInfo.h"

int CMofMsg::Get_WORD(unsigned __int16* outValue) {
    if (outValue) {
        *outValue = 0;
    }
    return 1;
}

void CMofMsg::Put_WORD(unsigned __int16 value) {}

int CMofMsg::Get_BYTE(unsigned __int8* outValue) {
    if (outValue) {
        *outValue = 0;
    }
    return 1;
}

int CMofMsg::Get_DWORD(unsigned int* outValue) {
    if (outValue) {
        *outValue = 0;
    }
    return 1;
}

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
