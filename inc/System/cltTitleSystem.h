#pragma once

#include <cstdint>

class cltTitleSystem {
public:
    void OnEvent_acquireskill(std::uint16_t skillKind);
    void OnEvent_setemblem(std::uint16_t emblemKind); // TODO
};

