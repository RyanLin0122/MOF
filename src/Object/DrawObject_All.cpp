#include "Object/DrawObject_All.h"
#include "Object/CObjectManager.h"
#include "Object/CBaseObject.h"
#include "Character/ClientCharacter.h"
#include "Character/ClientCharacterManager.h"
#include "Effect/CEffectManager.h"
#include "Logic/cltFieldItem.h"
#include "Logic/cltFieldItemManager.h"
#include "Logic/cltHelpMessage.h"
#include "Logic/Map.h"
#include "Logic/cltMapTitle.h"
#include "Logic/cltMoF_MineManager.h"
#include "Logic/cltMatchManager.h"
#include "Logic/cltPKFlagManager.h"
#include "global.h"
#include <cstdlib>
#include <cstring>

namespace {
constexpr int kDrawObjectCapacity = 1000;
CBaseObject* pDrawObject[kDrawObjectCapacity] = {};
CBaseObject* pDrawPortal[kDrawObjectCapacity] = {};
}

int compareCharYpos(const void* a1, const void* a2)
{
    int y1 = (*(ClientCharacter* const*)a1)->m_iPosY;
    int y2 = (*(ClientCharacter* const*)a2)->m_iPosY;
    double diff = static_cast<double>(y1 - y2);
    if (diff == 0.0)
        return 1;
    return (diff > 0.0) ? 1 : -1;
}

int compareObjectYpos(const void* a1, const void* a2)
{
    int y1 = (*(CBaseObject* const*)a1)->m_nPosY;
    int y2 = (*(CBaseObject* const*)a2)->m_nPosY;
    double diff = static_cast<double>(y1 - y2);
    if (diff == 0.0)
        return 0;
    return (diff > 0.0) ? 1 : -1;
}

int compareItemYpos(const void* a1, const void* a2)
{
    int y1 = *(int*)((char*)(*(void* const*)a1) + 8);
    int y2 = *(int*)((char*)(*(void* const*)a2) + 8);
    double diff = static_cast<double>(y1 - y2);
    if (diff == 0.0)
        return 0;
    return (diff > 0.0) ? 1 : -1;
}

int compareMatchFlagYpos(const void* a1, const void* a2)
{
    int y1 = *(int*)((char*)(*(void* const*)a1) + 64);
    int y2 = *(int*)((char*)(*(void* const*)a2) + 64);
    double diff = static_cast<double>(y1 - y2);
    if (diff == 0.0)
        return 0;
    return (diff > 0.0) ? 1 : -1;
}

int comparePKPlagYpos(const void* a1, const void* a2)
{
    int y1 = *(int*)((char*)(*(void* const*)a1) + 8);
    int y2 = *(int*)((char*)(*(void* const*)a2) + 8);
    double diff = static_cast<double>(y1 - y2);
    if (diff == 0.0)
        return 0;
    return (diff > 0.0) ? 1 : -1;
}

DrawObject_All::DrawObject_All() {}
DrawObject_All::~DrawObject_All() {}

void DrawObject_All::PrepareDrawing()
{
    ClientCharacter* pChar = &unk_1409D80[0];
    while (pChar < reinterpret_cast<ClientCharacter*>(&dword_1843F78))
    {
        if (reinterpret_cast<unsigned int*>(pChar)[1109])
            pChar->PrepareDrawingChar();
        pChar = reinterpret_cast<ClientCharacter*>(reinterpret_cast<char*>(pChar) + 14824);
    }

    g_ObjectManager.Process();
    g_clFieldItemMgr.PrepareDrawing();
    g_ClientCharMgr.PrepareDrawingEtcMark();
    g_clMapTitle.PrepareDrawing();
    g_clHelpMessage.PrepareDrawing();
    g_clMineMgr.PrepareDrawing();
    g_Map.PrepareDrawingClimate();
    g_clMatchManager.PrepareDrawing();
    g_clPKFlagManager.PrepareDrawing();
}

void DrawObject_All::DrawObject()
{
    if (byte_21CB35D)
        return;

    g_clMineMgr.Draw();

    size_t charCount = NumOfElements;
    int objectIndex = 0;
    int matchIndex = 0;
    int charIndex = 0;
    int itemIndex = 0;
    int pkIndex = 0;
    int objectCount = g_ObjectManager.GetObjectCount();
    size_t drawObjectCount = 0;
    int portalCount = 0;
    const int fieldItemCount = static_cast<unsigned short>(dword_73D154);
    const int matchCount = g_clMatchManager.GetMatchRoomNum();
    const int pkCount = dword_B4BAB4;

    qsort(dword_1843F78, charCount, sizeof(ClientCharacter*), compareCharYpos);

    if (objectCount > 0)
    {
        CBaseObject** pObj = pDrawObject;
        CBaseObject** pPortal = pDrawPortal;
        do
        {
            if (g_ObjectManager.GetType(static_cast<unsigned short>(objectIndex)) == 1)
            {
                *pPortal++ = g_ObjectManager.GetObjectA(static_cast<unsigned short>(objectIndex));
                ++portalCount;
            }
            else
            {
                *pObj++ = g_ObjectManager.GetObjectA(static_cast<unsigned short>(objectIndex));
                ++drawObjectCount;
            }
            ++objectIndex;
        } while (objectIndex < objectCount);
        objectIndex = 0;
    }

    qsort(pDrawObject, drawObjectCount, sizeof(CBaseObject*), compareObjectYpos);
    qsort(unk_73D15C, fieldItemCount, sizeof(cltFieldItem*), compareItemYpos);
    qsort(unk_813AA8, matchCount, sizeof(void*), compareMatchFlagYpos);
    qsort(unk_B4B924, pkCount, sizeof(void*), comparePKPlagYpos);

    void** pkPtr = unk_B4B924;
    void** matchPtr = unk_813AA8;
    cltFieldItem** itemPtr = unk_73D15C;
    CBaseObject** objPtr = pDrawObject;

    while (true)
    {
        int charY;
        if (charIndex < static_cast<int>(charCount))
            charY = reinterpret_cast<int*>(dword_1843F78[charIndex])[1097];
        else if (objectIndex < static_cast<int>(drawObjectCount) || itemIndex < fieldItemCount || matchIndex < matchCount || pkIndex < pkCount)
            charY = 10000;
        else
        {
            g_ClientCharMgr.DrawEtcMark();
            g_clMapTitle.Draw();
            g_Map.DrawClimate();
            CEffectManager::GetInstance()->Draw();
            g_clHelpMessage.Draw();
            return;
        }

        int objY = (objectIndex >= static_cast<int>(drawObjectCount)) ? 10000 : (*objPtr)->m_nPosY;
        int itemY = (itemIndex >= fieldItemCount) ? 10000 : reinterpret_cast<int*>(*itemPtr)[2] - 32;
        int matchY = (matchIndex >= matchCount) ? 10000 : *reinterpret_cast<int*>((char*)*matchPtr + 64);
        int pkY = (pkIndex >= pkCount) ? 10000 : *reinterpret_cast<int*>((char*)*pkPtr + 8);

        if (charY < objY)
        {
            if (charY < itemY)
            {
                if (charY < matchY)
                {
                    if (charY < pkY)
                    {
                        int prevCharIndex = charIndex;
                        if (!charIndex)
                        {
                            if (g_pEffectManager_Before_Chr)
                                g_pEffectManager_Before_Chr->Draw();
                            for (int i = 0; i < portalCount; ++i)
                            {
                                if (pDrawPortal[i])
                                    pDrawPortal[i]->Draw();
                            }
                        }
                        dword_1843F78[charIndex++]->DrawChar(pkIndex);
                        if (prevCharIndex + 1 == static_cast<int>(charCount) && g_pEffectManager_After_Chr)
                            g_pEffectManager_After_Chr->Draw();
                    }
                    else if (*pkPtr)
                    {
                        g_clPKFlagManager.Draw(pkIndex++);
                        ++pkPtr;
                    }
                }
                else if (matchY < pkY)
                {
                    if (*matchPtr)
                    {
                        g_clMatchManager.Draw(matchIndex);
                        ++matchIndex;
                        ++matchPtr;
                    }
                }
                else if (*pkPtr)
                {
                    g_clPKFlagManager.Draw(pkIndex++);
                    ++pkPtr;
                }
            }
            else if (itemY < matchY)
            {
                if (itemY < pkY)
                {
                    if (*itemPtr)
                    {
                        (*itemPtr)->Draw();
                        ++itemIndex;
                        ++itemPtr;
                    }
                }
                else if (*pkPtr)
                {
                    g_clPKFlagManager.Draw(pkIndex++);
                    ++pkPtr;
                }
            }
            else if (matchY < pkY)
            {
                if (*matchPtr)
                {
                    g_clMatchManager.Draw(matchIndex);
                    ++matchIndex;
                    ++matchPtr;
                }
            }
            else if (*pkPtr)
            {
                g_clPKFlagManager.Draw(pkIndex++);
                ++pkPtr;
            }
        }
        else if (objY < itemY)
        {
            if (objY < matchY)
            {
                if (objY < pkY)
                {
                    if (*objPtr)
                    {
                        (*objPtr)->Draw();
                        ++objectIndex;
                        ++objPtr;
                        g_Map.DrawClip();
                    }
                }
                else if (*pkPtr)
                {
                    g_clPKFlagManager.Draw(pkIndex++);
                    ++pkPtr;
                }
            }
            else if (matchY < pkY)
            {
                if (*matchPtr)
                {
                    g_clMatchManager.Draw(matchIndex);
                    ++matchIndex;
                    ++matchPtr;
                }
            }
            else if (*pkPtr)
            {
                g_clPKFlagManager.Draw(pkIndex++);
                ++pkPtr;
            }
        }
        else if (itemY < matchY)
        {
            if (itemY < pkY)
            {
                if (*itemPtr)
                {
                    (*itemPtr)->Draw();
                    ++itemIndex;
                    ++itemPtr;
                }
            }
            else if (*pkPtr)
            {
                g_clPKFlagManager.Draw(pkIndex++);
                ++pkPtr;
            }
        }
        else if (matchY < pkY)
        {
            if (*matchPtr)
            {
                g_clMatchManager.Draw(matchIndex);
                ++matchIndex;
                ++matchPtr;
            }
        }
        else if (*pkPtr)
        {
            g_clPKFlagManager.Draw(pkIndex++);
            ++pkPtr;
        }

        if (charIndex == static_cast<int>(charCount)
            && objectIndex == static_cast<int>(drawObjectCount)
            && itemIndex == fieldItemCount
            && matchIndex == matchCount
            && pkIndex == pkCount)
        {
            g_ClientCharMgr.DrawEtcMark();
            g_clMapTitle.Draw();
            g_Map.DrawClimate();
            CEffectManager::GetInstance()->Draw();
            g_clHelpMessage.Draw();
            return;
        }
    }
}
