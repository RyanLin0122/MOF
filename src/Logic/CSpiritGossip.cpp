#include "Logic/CSpiritGossip.h"

#include <cstdlib>

#include "global.h"
#include "Logic/SpiritGossipParser.h"
#include "System/cltPetSystem.h"

CSpiritGossip::CSpiritGossip()
{
}

CSpiritGossip::~CSpiritGossip()
{
}

char* CSpiritGossip::CheckSpiritGossip()
{
    int r = std::rand();
    if (r % 4)
        return nullptr;

    std::uint16_t spiritId = g_clPetSystem.GetPetKind();

    // If rand is odd, use the fixed "P0011" code instead
    if ((r & 1) != 0)
        spiritId = g_clSpiritGossipParser.TranslateKindCode("P0011");

    if (!g_clSpiritGossipParser.IsExistSpiritID(spiritId))
        return nullptr;

    CSpiritGossipParser localParser;
    g_clSpiritGossipParser.GetSpiritGossipInfo(localParser);

    const auto& gossipMap = localParser.GetMap();
    auto it = gossipMap.find(spiritId);
    if (it == gossipMap.end())
        return nullptr;

    const stGossipInfo& info = it->second;
    if (info.count == 0)
        return nullptr;

    int textIdx = std::rand() % info.count;
    char* text = g_DCTTextManager.GetText(info.textIds[textIdx]);
    return text;
}
