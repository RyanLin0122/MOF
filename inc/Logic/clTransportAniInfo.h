#pragma once
#include <cstdint>
class clTransportAniInfo {
public:
    void GetFrameInfo(unsigned int action, std::uint16_t curFrame, unsigned int* outRes, std::uint16_t* outFrame);
    std::uint16_t GetTotalFrameNum(unsigned int action);
};
