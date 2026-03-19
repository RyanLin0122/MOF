#include "Logic/clTransportKindInfo.h"
stTransportKindInfo* clTransportKindInfo::GetTransportKindInfo(std::uint16_t transportKind) {
    static stTransportKindInfo info;
    (void)transportKind;
    return &info;
}
