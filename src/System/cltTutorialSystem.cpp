#include "System/cltTutorialSystem.h"

#include <cstdlib>
#include <cstdio>
#include <windows.h>  // timeGetTime, PtInRect, RECT, POINT

#include "Character/ClientCharacter.h"
#include "Character/ClientCharacterManager.h"
#include "Info/cltCharKindInfo.h"
#include "Info/cltItemKindInfo.h"
#include "Logic/Map.h"
#include "Logic/CMessageBoxManager.h"
#include "Logic/CObjectManager.h"
#include "Logic/CShortKey.h"
#include "Logic/cltBaseInventory.h"
#include "Logic/cltConfiguration.h"
#include "Logic/cltFieldItemManager.h"
#include "Logic/cltHelpMessage.h"
#include "Logic/cltItemList.h"
#include "Logic/cltMyCharData.h"
#include "Logic/cltNPCManager.h"
#include "Logic/cltSystemMessage.h"
#include "Util/cltTimer.h"
#include "Util/ScriptParameter.h"
#include "Util/StringStream.h"
#include "System/CMeritoriousSystem.h"
#include "System/cltQuestSystem.h"
#include "Text/DCTTextManager.h"
#include "UI/CUIManager.h"
#include "UI/CUITutorial.h"
#include "global.h"
#include <map>
#include <string>

// g_nTutorialState – global, NOT a member
// Declared extern in global.h; defined in global.cpp.

// ---------------------------------------------------------------------------
// Constructor / Destructor
// ---------------------------------------------------------------------------

cltTutorialSystem::cltTutorialSystem()
    : m_nTimerID(0)
{
}

cltTutorialSystem::~cltTutorialSystem() {
}

// ---------------------------------------------------------------------------
// InitalizeTutorialSystem
// ---------------------------------------------------------------------------

int cltTutorialSystem::InitalizeTutorialSystem(std::uint8_t tutorialType) {
    // 1. Key / message-box setup
    CShortKey::SetAllDefaultKey(g_pShortKeyList);
    CShortKey::SaveKeySetting(g_pShortKeyList);
    CMessageBoxManager::AddOK(g_pMsgBoxMgr, 8208, 0, 0, 0, -1);

    // 2. System message
    const char* msg = g_DCTTextManager.GetText(8208);
    cltSystemMessage::SetSystemMessage(&g_clSysemMessage, msg, 0, 0, 0);

    // 3. My-char account
    cltMyCharData::SetMyAccount(&g_clMyCharData, 10);

    // 4. Local item list (stack)
    cltItemList itemList;
    itemList.Initialize(7);

    // 5. Equip arrays (zero-initialised)
    stEquipItemInfo equip1[11] = {};
    stEquipItemInfo equip2[11] = {};

    // 6. Compute byte stride into global slot arrays (matches GT: v11 = 96 * a2)
    const int stride = 96 * tutorialType;
    const unsigned short* eq1data = reinterpret_cast<const unsigned short*>(
        reinterpret_cast<const char*>(word_23158F8) + stride);
    const unsigned short* eq2data = reinterpret_cast<const unsigned short*>(
        reinterpret_cast<const char*>(word_231590E) + stride);

    // 7. Fill equip arrays from slot data
    for (int i = 0; i < 11; ++i) {
        equip1[i].itemKind = eq1data[i];
    }
    for (int i = 0; i < 11; ++i) {
        equip2[i].itemKind = eq2data[i];
    }

    // 8. cltMyCharData::Initialize
    cltMyCharData::Initialize(
        &g_clMyCharData,
        word_23158EC[48 * tutorialType],
        static_cast<std::uint8_t>(byte_23158F4[stride]),
        /*a3*/0LL,
        /*hp*/100, /*maxHp*/100,
        0,0,0,0,0,
        /*teamKind*/2,
        &itemList,
        0,0,0,
        equip1, equip2,
        0,0,0,0,0,0,0,0,0,0,0,0,
        static_cast<std::uint8_t>(byte_23158EA[stride]),
        static_cast<std::uint8_t>(byte_23158EE[stride]),
        static_cast<std::uint8_t>(byte_23158EF[stride]),
        dword_23158F0[24 * tutorialType],
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    );

    // 9. Free quest / meritorious systems
    g_clQuestSystem.Free();
    g_clMeritoriousSystem.Free();

    // 10. AddCharacter for the player
    g_ClientCharMgr.AddCharacter(
        reinterpret_cast<ClientCharacter*>(10),
        642, 507,
        static_cast<std::uint16_t>(word_23158E8[48 * tutorialType]), 0x5001,
        25, &byte_23158C8[stride],
        "", 0,
        "", "",
        0,0,0,0,0,0,
        0,0,0,0,
        "", "",
        0, 0,0,0,0,
        static_cast<std::uint8_t>(byte_23158EA[stride]),
        static_cast<std::uint8_t>(byte_23158EE[stride]),
        static_cast<std::uint8_t>(byte_23158EF[stride]),
        0, 0
    );

    // 11. SetMyAccount on manager
    g_ClientCharMgr.SetMyAccount(10);

    // 12. Cache pointer to my character
    m_pMyCharacter = g_ClientCharMgr.GetMyCharacterPtr();

    // 13. Set byte at offset 11524 = 2
    *(reinterpret_cast<char*>(m_pMyCharacter) + 11524) = 2;

    // 14. Set equipped items on character manager (iterate raw slot data, matches GT)
    for (int i = 0; i < 11; ++i) {
        if (eq1data[i])
            g_ClientCharMgr.SetItem(10, eq1data[i], 1);
    }
    for (int i = 0; i < 11; ++i) {
        if (eq2data[i])
            g_ClientCharMgr.SetItem(10, eq2data[i], 1);
    }

    // 15. Disable help overlay
    g_clHelpMessage.SetDisable();

    // 16. Clear scene objects
    g_ObjectManager.DelAllObject();

    // 17. Spawn NPCs
    g_clNPCManager.AddAllNPC(0x5001);

    // 18. Create map
    {
        ClientCharacter* v22 = g_ClientCharMgr.GetMyCharacterPtr();
        g_Map.CreateMap(0x5001u, v22);
    }

    // 19. Set map ID on character
    *(reinterpret_cast<std::uint16_t*>(
        reinterpret_cast<char*>(m_pMyCharacter) + 592)) = 0x5001;

    // 20. Clear LR flag
    *(reinterpret_cast<int*>(
        reinterpret_cast<char*>(m_pMyCharacter) + 572)) = 0;

    // 21. Set current HP raw field and call SetHP
    *(reinterpret_cast<int*>(
        reinterpret_cast<char*>(m_pMyCharacter) + 11256)) = 25;
    m_pMyCharacter->SetHP();

    // 22. Set destination position fields
    *(reinterpret_cast<int*>(
        reinterpret_cast<char*>(m_pMyCharacter) + 556)) = 642;
    *(reinterpret_cast<int*>(
        reinterpret_cast<char*>(m_pMyCharacter) + 560)) = 507;

    // 23. SetCurPosition
    ClientCharacter::SetCurPosition(m_pMyCharacter, 642, 507);

    // 24. SetMapID on my-char data
    cltMyCharData::SetMapID(&g_clMyCharData, 0x5001);

    // 25. Clone CA
    g_ClientCharMgr.SetMyCAClone();

    // 26. Store char name in my-char data
    {
        const char* charName = g_ClientCharMgr.GetMyCharName();
        cltMyCharData::SetMyCharName(&g_clMyCharData, charName);
    }

    // 27. Reset tutorial state globals
    g_nTutorialState = 0;

    // 28. Member initialisation
    m_nMonsterHP = 35;

    m_fStartX = static_cast<float>(
        *(reinterpret_cast<int*>(
            reinterpret_cast<char*>(m_pMyCharacter) + 4384)));
    m_fStartY = static_cast<float>(
        *(reinterpret_cast<int*>(
            reinterpret_cast<char*>(m_pMyCharacter) + 4388)));

    m_nLastAttackTime = 0;
    m_nWaitingUseItemResult = 1;

    // 29. Advance tutorial UI to step 1 (no state increment)
    g_pUITutorial->AddTutorial(1);

    // 30. Clear timer ID
    m_nTimerID = 0;

    return 1;
}

// ---------------------------------------------------------------------------
// Poll
// ---------------------------------------------------------------------------

int cltTutorialSystem::Poll() {
    switch (g_nTutorialState) {
    case 1:
        MoveCharacterMission(0);
        return ExitTutorialMap();
    case 2:
        MoveCharacterMission(1);
        return ExitTutorialMap();
    case 3:
        MoveCharacterMission(2);
        return ExitTutorialMap();
    case 4:
        MoveCharacterMission(3);
        return ExitTutorialMap();
    case 5:
        AttackMonster();
        return ExitTutorialMap();
    case 6:
        PickUpItem();
        return ExitTutorialMap();
    case 7:
        UseItem();
        /* fall through */
    default:
        return ExitTutorialMap();
    }
}

// ---------------------------------------------------------------------------
// AttackMonster
// ---------------------------------------------------------------------------

void cltTutorialSystem::AttackMonster() {
    if (::timeGetTime() - m_nLastAttackTime < 0x320U) {
        return;
    }

    unsigned int targetAccount = m_pMyCharacter->GetSearchMonster();

    if (CUIManager::IsCharActionKey(g_UIMgr, 51)) {
        if (*(reinterpret_cast<std::uint32_t*>(
                reinterpret_cast<char*>(m_pMyCharacter) + 9684)) != 3
            && m_pMyCharacter->GetLastOrder() != 3)
        {
            if (targetAccount != 0) {
                ClientCharacter* pTarget =
                    g_ClientCharMgr.GetCharByAccount(targetAccount);
                if (pTarget) {
                    // Set attack flag
                    *(reinterpret_cast<int*>(
                        reinterpret_cast<char*>(m_pMyCharacter) + 616)) = 1;

                    int dx = std::abs(
                        *(reinterpret_cast<int*>(
                            reinterpret_cast<char*>(m_pMyCharacter) + 4384))
                        - *(reinterpret_cast<int*>(
                            reinterpret_cast<char*>(pTarget) + 4384)));

                    if (dx <= 70) {
                        int dy = std::abs(
                            *(reinterpret_cast<int*>(
                                reinterpret_cast<char*>(m_pMyCharacter) + 4388))
                            - *(reinterpret_cast<int*>(
                                reinterpret_cast<char*>(pTarget) + 4388)));

                        if (dy <= 20
                            && *(reinterpret_cast<std::uint32_t*>(
                                    reinterpret_cast<char*>(pTarget) + 9684)) != 2)
                        {
                            m_nLastAttackTime = ::timeGetTime();

                            int dmg = std::rand() % 5 + 5;
                            if (m_nMonsterHP <= dmg) dmg = m_nMonsterHP;
                            m_nMonsterHP -= dmg;

                            stCharOrder order = {};
                            m_pMyCharacter->SetOrderAttack(
                                &order, targetAccount, 0, dmg, m_nMonsterHP, 0, 0);
                            m_pMyCharacter->PushOrder(&order);

                            *(reinterpret_cast<int*>(
                                reinterpret_cast<char*>(pTarget) + 11256)) = m_nMonsterHP;
                            pTarget->SetHP();

                            if (m_nMonsterHP == 0) {
                                g_clFieldItemMgr.PushBuffer(
                                    targetAccount, 0x64, 0, 0x292E, 1, 0);
                                g_pUITutorial->AddTutorial(8);
                                ++g_nTutorialState;
                            }
                        }
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// PickUpItem
// ---------------------------------------------------------------------------

void cltTutorialSystem::PickUpItem() {
    if (CUIManager::IsCharActionKey(g_UIMgr, 50)) {
        float px = static_cast<float>(
            *(reinterpret_cast<int*>(
                reinterpret_cast<char*>(m_pMyCharacter) + 4384)));
        float py = static_cast<float>(
            *(reinterpret_cast<int*>(
                reinterpret_cast<char*>(m_pMyCharacter) + 4388)));

        std::uint16_t nearKind = 0;
        float nearX = py;
        float nearY = px;

        g_clFieldItemMgr.GetNearItemInfo(px, py, &nearKind, &nearX, &nearY, 0, 0, 0);

        if (nearKind != 0) {
            // Build parameter map for script substitution
            std::map<std::string, std::string> paramMap;

            stItemKindInfo* pItemInfo = g_clItemKindInfo.GetItemKindInfo(0x292Eu);
            const char* itemNameText = g_DCTTextManager.GetText(pItemInfo->m_wTextCode);
            paramMap["Parameter0"] = itemNameText ? itemNameText : "";

            StringStream ss;
            ss << 1;
            paramMap["Parameter1"] = ss.c_str();

            // Substitute parameters into message template (DCT 4462)
            const char* msgTemplate = g_DCTTextManager.GetText(4462);
            std::string substituted = getScriptParameter(msgTemplate, paramMap);

            char Buffer[1024];
            if (substituted == (msgTemplate ? msgTemplate : "")) {
                // No substitution occurred
                sprintf(Buffer, msgTemplate);
            } else {
                std::strcpy(Buffer, substituted.c_str());
            }

            cltSystemMessage::SetSystemMessage(&g_clSysemMessage, Buffer, 0, 0, 0);

            strInventoryItem inv;
            inv.itemKind = 10542;
            inv.itemQty  = 1;
            inv.value0   = 0;
            inv.value1   = 0;
            g_clMyInventory.AddInventoryItem(&inv, nullptr, nullptr);

            g_nFieldItemPickupFlag = 1;

            g_clFieldItemMgr.DelItem(10, 0x64, 1, 0);

            g_pUITutorial->AddTutorial(10);
            // Note: NO ++g_nTutorialState here (matches ground truth).
        }
    }
}

// ---------------------------------------------------------------------------
// UseItem
// ---------------------------------------------------------------------------

void cltTutorialSystem::UseItem() {
    if (m_nWaitingUseItemResult) {
        if (m_pMyCharacter->GetHP() > 25) {
            m_nWaitingUseItemResult = 0;
            m_nTimerID = g_clTimerManager.CreateTimer(
                0x3E8u,
                reinterpret_cast<unsigned int>(this),
                0, 1,
                nullptr, nullptr,
                reinterpret_cast<cltTimer::TimerCallback>(cltTutorialSystem::OnTimer_EndUseItem),
                nullptr, nullptr);
        }
    }
}

// ---------------------------------------------------------------------------
// ExitTutorialMap
// ---------------------------------------------------------------------------

int cltTutorialSystem::ExitTutorialMap() {
    if (!m_pMyCharacter) return 0;

    int px = *(reinterpret_cast<int*>(
        reinterpret_cast<char*>(m_pMyCharacter) + 4384));
    int py = *(reinterpret_cast<int*>(
        reinterpret_cast<char*>(m_pMyCharacter) + 4388));

    RECT rc  = { 1420, 488, 1490, 535 };
    POINT pt = { px, py };

    if (!::PtInRect(&rc, pt)) return 0;

    if (m_nTimerID) {
        g_clTimerManager.ReleaseTimer(m_nTimerID);
    }

    g_ClientCharMgr.DeleteAllChar();

    // ground truth: qmemcpy zeroes v6 before call, so exitX/exitY are 0
    g_pUITutorial->OnCancel(-1, 0, 0, 0, 0, 0);
    g_pUITutorial = nullptr;

    g_clMyInventory.EmptyInventoryItem(0);
    g_clMyInventory.EmptyInventoryItem(1);

    int helpState = 0;
    g_clConfig->GetHelpState(reinterpret_cast<LPCSTR>(g_clConfig), &helpState);
    g_clHelpMessage.IsShow(helpState);

    return 1;
}

// ---------------------------------------------------------------------------
// SendTutorialMsg
// ---------------------------------------------------------------------------

void cltTutorialSystem::SendTutorialMsg(std::uint8_t msgType) {
    switch (msgType) {
    case 7: {
        stCharKindInfo* ki = g_clCharKindInfo.GetMonsterNameByKind(0x4801);
        const char* nameFmt = g_DCTTextManager.GetText(reinterpret_cast<int>(ki));
        char monName[128];
        wsprintfA(monName, nameFmt, 100);

        g_ClientCharMgr.AddCharacter(
            reinterpret_cast<ClientCharacter*>(100),
            1000, 510,
            0x4801, 0x5001,
            m_nMonsterHP, monName,
            "", 0,
            "", "",
            0,0,0,0,0,0,
            0,0,0,0,
            "", "",
            2, 0,0,0,0,
            0, 0, 0,
            0, 0
        );
        ++g_nTutorialState;
        break;
    }
    case 0x0B: {
        strInventoryItem inv;
        inv.itemKind = 16396;
        inv.itemQty  = 1;
        inv.value0   = 0;
        inv.value1   = 0;
        g_clMyInventory.AddInventoryItem(&inv, nullptr, nullptr);
        // fall through to ++g_nTutorialState
        [[fallthrough]];
    }
    case 2:
    case 0x0D:
        ++g_nTutorialState;
        break;

    case 0x0E:
        m_nTimerID = g_clTimerManager.CreateTimer(
            0x7D0u,
            reinterpret_cast<unsigned int>(this),
            0, 1,
            nullptr, nullptr,
            reinterpret_cast<cltTimer::TimerCallback>(cltTutorialSystem::OnTimer_StartExitMap),
            nullptr, nullptr);
        break;

    default:
        return;
    }
}

// ---------------------------------------------------------------------------
// MoveCharacterMission
// ---------------------------------------------------------------------------

void cltTutorialSystem::MoveCharacterMission(std::uint8_t missionType) {
    switch (missionType) {
    case 0:
        if (m_fStartX - (double)(*(reinterpret_cast<int*>(
                    reinterpret_cast<char*>(m_pMyCharacter) + 4384)))
            > 240.0)
        {
            g_pUITutorial->AddTutorial(4);
            ++g_nTutorialState;
        }
        break;

    case 1:
        if ((double)(*(reinterpret_cast<int*>(
                    reinterpret_cast<char*>(m_pMyCharacter) + 4384)))
            - m_fStartX > 240.0)
        {
            g_pUITutorial->AddTutorial(3);
            ++g_nTutorialState;
        }
        break;

    case 2:
        if (m_fStartY - (double)(*(reinterpret_cast<int*>(
                    reinterpret_cast<char*>(m_pMyCharacter) + 4388)))
            > 140.0)
        {
            g_pUITutorial->AddTutorial(5);
            ++g_nTutorialState;
        }
        break;

    case 3:
        if ((double)(*(reinterpret_cast<int*>(
                    reinterpret_cast<char*>(m_pMyCharacter) + 4388)))
            - m_fStartY > 140.0)
        {
            g_pUITutorial->AddTutorial(6);
        }
        break;

    default:
        return;
    }
}

// ---------------------------------------------------------------------------
// Static timer callbacks
// ---------------------------------------------------------------------------

void cltTutorialSystem::OnTimer_EndUseItem() {
    g_pUITutorial->AddTutorial(12);
}

void cltTutorialSystem::OnTimer_StartExitMap() {
    g_pUITutorial->AddTutorial(15);
    ++g_nTutorialState;
}
