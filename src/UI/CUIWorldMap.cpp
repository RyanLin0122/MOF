#include "UI/CUIWorldMap.h"
#include "Info/cltMapInfo.h"
#include "Logic/Map.h"
#include "UI/CControlBase.h"
#include "global.h"
#include <windows.h>

// ----------------------------------------------------------------
// GetMyMapInfo  (ground truth: 004D6980)
// Returns the stMapInfo for the currently loaded map.
// ----------------------------------------------------------------
stMapInfo* CUIWorldMap::GetMyMapInfo()
{
    return g_Map.GetMapInfoByID(dword_21B8DF4);
}

// ----------------------------------------------------------------
// GetToolTipData  (ground truth: 004D6A10)
// Hit-tests (mouseX, mouseY) against world-map icon positions.
// Returns the map ID under the cursor, or 0 if none.
// ----------------------------------------------------------------
uint16_t CUIWorldMap::GetToolTipData(int mouseX, int mouseY)
{
    // v19[0]   = continent type of current map (byte, for filtering)
    // v19[1..4]= isDungeonMode flag (int, set to 1 in dungeon branch)
    uint8_t continentType = static_cast<uint8_t>(
        reinterpret_cast<uint8_t*>(GetMyMapInfo())[253]);   // m_byteContinentType

    // *((_DWORD *)this + 1246) — pointer to "select mode" object
    // *((_BYTE **)this + 1245) — pointer whose first byte overrides continent type
    if (*reinterpret_cast<uintptr_t*>(reinterpret_cast<char*>(this) + 4984))
        continentType = **reinterpret_cast<uint8_t**>(reinterpret_cast<char*>(this) + 4980);

    bool isDungeonMode = false;

    unsigned short totalMaps = g_Map.GetTotalMapNum();
    if (!totalMaps)
        return 0;

    // *((_WORD *)this + 2472) — current world-map kind (e.g. 7/8=E-continent dungeon)
    int16_t mapKind = *reinterpret_cast<int16_t*>(reinterpret_cast<char*>(this) + 4944);

    // The inner CControlBase panel sits at offset 260 (ground truth: (char*)this + 260)
    CControlBase* panel = reinterpret_cast<CControlBase*>(reinterpret_cast<char*>(this) + 260);

    int loopIdx = 0;
    while (loopIdx < totalMaps)
    {
        stMapInfo* info = g_Map.GetMapInfoByIndex(loopIdx);
        if (!info)
        {
            ++loopIdx;
            continue;
        }

        int offsetX = 0;
        int offsetY = 0;

        if (mapKind == 7 || mapKind == 8 || mapKind == 15 || mapKind == 16 ||
            mapKind == 18 || mapKind == 19)
        {
            // Dungeon continent view — map icon belongs to the parent continent entry
            const char* kindStr;
            switch (info->m_wRegionType)
            {
                case 7:  case 8:  kindStr = "E1000"; break;
                case 15: case 16: kindStr = "N0001"; break;
                case 18: case 19: kindStr = "X0001"; break;
                default:
                    ++loopIdx;
                    continue;
            }
            uint16_t parentKind = cltMapInfo::TranslateKindCode(kindStr);
            info = g_Map.GetMapInfoByID(parentKind);
            if (!info)
            {
                ++loopIdx;
                continue;
            }
            offsetX = -17;   // ground truth: v8 = -17
            offsetY = -16;   // ground truth: v7 = -16
            isDungeonMode = true;
        }
        else
        {
            // Normal view — skip entries that are themselves dungeon-continent kinds
            int16_t entryKind = static_cast<int16_t>(info->m_wRegionType);
            if (entryKind == 7 || entryKind == 8 || entryKind == 15 || entryKind == 16 ||
                entryKind == 18 || entryKind == 19)
            {
                ++loopIdx;
                continue;
            }
        }

        // Build hit-test rect around the map icon
        int iconX = static_cast<int>(info->m_dwWorldMapPosX) + panel->GetX() + offsetX;
        int iconY = static_cast<int>(info->m_dwWorldMapPosY) + panel->GetY() + offsetY;

        RECT rc;
        SetRect(&rc, iconX + 10, iconY + 3, iconX + 25, iconY + 23);

        POINT pt = { mouseX, mouseY };

        bool filtered = !isDungeonMode &&
            (mapKind == 2 || mapKind == 17 ||
             continentType != info->m_byteContinentType ||
             info->m_wRegionType == 2 ||
             (mapKind == 1 && info->m_wRegionType == 17));

        if (!PtInRect(&rc, pt) || filtered)
        {
            ++loopIdx;
            continue;
        }

        return info->m_wID;
    }

    return 0;
}
