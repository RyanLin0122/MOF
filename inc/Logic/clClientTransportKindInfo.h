#pragma once
#include <cstdint>
class clTransportAniInfo;
class clClientTransportKindInfo {
public:
    clTransportAniInfo* GetTransportAniInfoUp(std::uint16_t transportKind);
    clTransportAniInfo* GetTransportAniInfoDown(std::uint16_t transportKind);
};
