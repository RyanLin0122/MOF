#include "Logic/cltFieldItemManager.h"

void cltFieldItemManager::PushBuffer(unsigned int /*account*/, unsigned int /*dropId*/, int /*a3*/,
                                     unsigned short /*itemKind*/, int /*itemQty*/, int /*a6*/) {
    // Stub: real implementation queues a field-item drop packet.
}

void cltFieldItemManager::GetNearItemInfo(float /*x*/, float /*y*/,
                                          std::uint16_t* outKind, float* /*outX*/, float* /*outY*/,
                                          int /*a5*/, int /*a6*/, int /*a7*/) {
    // Stub: real implementation searches the field-item list.
    if (outKind) *outKind = 0;
}

void cltFieldItemManager::DelItem(unsigned int /*account*/, unsigned int /*dropId*/,
                                  int /*a3*/, int /*a4*/) {
    // Stub: real implementation removes the item from the field-item list.
}
