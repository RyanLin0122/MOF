#include "Logic/clTransportAniInfo.h"
void clTransportAniInfo::GetFrameInfo(unsigned int action, std::uint16_t curFrame, unsigned int* outRes, std::uint16_t* outFrame) {
    if (outRes) *outRes = action;
    if (outFrame) *outFrame = curFrame;
}
std::uint16_t clTransportAniInfo::GetTotalFrameNum(unsigned int action) {
    return action == 4 ? 1 : 1;
}
