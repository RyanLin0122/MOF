// CA_CACHE_DATA — 1:1 還原 mofclient.c 0x527050
#include "Image/CA_CACHE_DATA.h"

#include <cstring>

//----- (00527050) --------------------------------------------------------
CA_CACHE_DATA::CA_CACHE_DATA()
    : GameImage()
    , m_wTagAt476(0)
    , m_wAt510(0)
    , m_wAt512(0)
{
    std::memset(m_aZero478, 0, sizeof(m_aZero478));
}
