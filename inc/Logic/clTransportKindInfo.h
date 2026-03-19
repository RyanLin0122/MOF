#pragma once
#include <cstdint>
struct stTransportKindInfo {};
class clTransportKindInfo {
public:
    stTransportKindInfo* GetTransportKindInfo(std::uint16_t transportKind);
};
