#include "Logic/clClientTransportKindInfo.h"
#include "Logic/clTransportAniInfo.h"
clTransportAniInfo* clClientTransportKindInfo::GetTransportAniInfoUp(std::uint16_t transportKind) {
    static clTransportAniInfo info;
    (void)transportKind;
    return &info;
}
clTransportAniInfo* clClientTransportKindInfo::GetTransportAniInfoDown(std::uint16_t transportKind) {
    static clTransportAniInfo info;
    (void)transportKind;
    return &info;
}
