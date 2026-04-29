// 還原自 mofclient.c 內的 ClientCharacterManager。
// 本檔以全域 unk_1409D80[] 為 300 個角色 slot 儲存體，與反編譯
// 在 manager 內嵌 300 份 ClientCharacter 的設計語意等價。
// 所有方法皆對應 mofclient.c 35545..39636 之間的同名函式；
// 呼叫的 helper 函式維持與反編譯相同 (cltMyCharData / Map / CMoFNetwork
// / cltSkillKindInfo / cltPKManager 等)，呼叫面差異僅在於本還原將
// IDA 標記為 static 的呼叫改寫為 g_xxx.method() 形式。
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// winsock2.h must precede any indirect inclusion of <windows.h>; otherwise
// the older <winsock.h> bundled by windows.h produces duplicate sockaddr/
// fd_set declarations.  CMoFNetwork.h is the only header in this TU that
// pulls in winsock2.h, so include it first.
#include "Network/CMoFNetwork.h"

#include "global.h"
#include "Character/ClientCharacterManager.h"

#include "Character/ClientCharacter.h"
#include "Character/CCA.h"
#include "Info/cltCharKindInfo.h"
#include "Info/cltClientCharKindInfo.h"
#include "Info/cltMonsterAniInfo.h"
#include "Info/cltSkillKindInfo.h"
#include "Info/cltCoupleRingKindInfo.h"
#include "Info/cltClientPortalInfo.h"
#include "Info/cltMapInfo.h"
#include "Logic/Map.h"
#include "Character/cltMyCharData.h"
#include "Logic/cltSystemMessage.h"
#include "Logic/cltPKManager.h"
#include "Logic/cltMoF_MineManager.h"
#include "Logic/cltChattingMgr.h"
#include "System/cltLevelSystem.h"
#include "System/cltPlayerAbility.h"
#include "System/cltSexSystem.h"
#include "System/cltBasicAppearSystem.h"
#include "System/cltEquipmentSystem.h"
#include "System/cltTradeSystem.h"
#include "System/cltSkillSystem.h"
#include "System/cltMoF_CharacterActionStateSystem.h"
#include "System/cltHelpSystem.h"
#include "UI/CUIManager.h"
#include "UI/CUIBasic.h"
#include "Effect/CEffectManager.h"
#include "Effect/CEffect_Player_MapConqueror.h"
#include "Object/clTransportObject.h"
#include "Sound/GameSound.h"
#include "Util/cltTimer.h"

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cmath>

#ifndef _WIN32
#  include <strings.h>
#  define _stricmp strcasecmp
#  define _strnicmp strncasecmp
#  define _toupper(c) toupper(c)
#endif

// =========================================================================
// 本檔需要、但 global.h 尚未匯出的外部符號。
// 大部分 mofclient.c 雜項旗標已搬到 global.h（具有意義名稱 + 原始 IDA
// 命名於註解），這裡只列出尚未具備全域別名者。
// =========================================================================

// g_clBasicAppearSystem 已移至 global.h / global.cpp（mofclient.c 0x21BA7B4）。

namespace {

constexpr int kFirstSlot = 1;
constexpr int kLastSlot  = 299;
constexpr int kSlotCount = 300;

inline ClientCharacter* SlotPtr(int slot) {
    return &unk_1409D80[slot];
}
inline bool SlotAlive(const ClientCharacter& slot) {
    return slot.m_dwSlotAlive != 0;
}

// mofclient.c 經常透過 *((_DWORD *)v + N) 取讀 ClientCharacter 內未具名的
// 欄位.  以下 helper 將最常用的索引對應到既有名字 / safety pad。
inline int& Decomp_AttackProcGate(ClientCharacter* p) {
    return p->m_iInitFlag_153_176[154 - 153];
}
inline int& Decomp_158(ClientCharacter* p) { return p->m_iInitFlag_153_176[158 - 153]; }
inline int& Decomp_159(ClientCharacter* p) { return p->m_iInitFlag_153_176[159 - 153]; }
inline int& Decomp_160(ClientCharacter* p) { return p->m_iInitFlag_153_176[160 - 153]; }
inline int& Decomp_162(ClientCharacter* p) { return p->m_iInitFlag_153_176[162 - 153]; }
inline int& Decomp_164(ClientCharacter* p) { return p->m_iInitFlag_153_176[164 - 153]; }
inline int& Decomp_169(ClientCharacter* p) { return p->m_iInitFlag_153_176[169 - 153]; }
inline int& Decomp_170(ClientCharacter* p) { return p->m_iInitFlag_153_176[170 - 153]; }
inline int& Decomp_171(ClientCharacter* p) { return p->m_iInitFlag_153_176[171 - 153]; }
inline int& Decomp_175(ClientCharacter* p) { return p->m_iInitFlag_153_176[175 - 153]; }

// Read/write to an unnamed decomp byte offset — routed through ClientCharacter's
// safety pad so every callsite touching the same offset shares storage.
template <typename T>
inline T& At(ClientCharacter* p, std::size_t byteOffset) {
    return p->Decomp<T>(byteOffset);
}

}  // namespace

// =========================================================================
// 內嵌的 helper 子系統 stubs
// =========================================================================

cltTargetMark::cltTargetMark() = default;
cltTargetMark::~cltTargetMark() = default;
void cltTargetMark::SetActive(int active) { m_iActive = active; }
void cltTargetMark::SetPos(float x, float y) { m_fPosX = x; m_fPosY = y; }
void cltTargetMark::Poll() { if (m_iActive) ++m_iCounter; }
void cltTargetMark::PrepareDrawing() {}
void cltTargetMark::Draw() {}

cltAutoMoveTarget::cltAutoMoveTarget() = default;
cltAutoMoveTarget::~cltAutoMoveTarget() = default;
void cltAutoMoveTarget::SetActive(int active) { m_iActive = active; }
void cltAutoMoveTarget::InitNewTarget(int x, int y) {
    m_iActive = 1; m_iX = x; m_iY = y; m_iCounter = 0;
}
void cltAutoMoveTarget::Poll() { if (m_iActive) ++m_iCounter; }
void cltAutoMoveTarget::PrepareDrawing() {}
void cltAutoMoveTarget::Draw() {}

cltKeepRunState::cltKeepRunState() = default;
cltKeepRunState::~cltKeepRunState() = default;
void cltKeepRunState::SetActive(ClientCharacter* pChar, int active) {
    m_iActive = active; m_pChar = pChar;
}
void cltKeepRunState::ReleaseViewKeepRunState() { m_iActive = 0; }
void cltKeepRunState::OnTimer_ReleaseViewKeepRunState(unsigned int /*timerID*/,
                                                      cltKeepRunState* self) {
    if (self) self->ReleaseViewKeepRunState();
}
void cltKeepRunState::Poll() {}
void cltKeepRunState::PrepareDrawing() {}
void cltKeepRunState::Draw() {}

cltInstansDungeonPortal::cltInstansDungeonPortal() = default;
cltInstansDungeonPortal::~cltInstansDungeonPortal() = default;
void cltInstansDungeonPortal::Init() { m_count = 0; }
int  cltInstansDungeonPortal::AddPortal(unsigned short mapID) {
    if (m_count >= kMaxPortals) return 0;
    m_mapIDs[m_count++] = mapID;
    return 1;
}
unsigned char cltInstansDungeonPortal::IsPortalAction(int /*x*/, int /*y*/,
                                                     int* outMapID) {
    if (outMapID) *outMapID = 0;
    return 0;
}

// =========================================================================
// ClientCharacterManager
// =========================================================================

ClientCharacterManager::ClientCharacterManager() {
    m_pClientCharKindInfo = new cltClientCharKindInfo();
    m_dwSearchAttackMonAccount = 0;
    m_dwSearchMonsterAccount   = 0;
    m_dwSearchPlayerAccount    = 0;
    m_dwSearchDiedPlayerAccount= 0;
    m_iKeepRunStateFlag        = 1;
    m_iCreateClientEffectFlag  = 0;
    m_iAttackReady             = 1;
    m_dwLastWarpFailTime       = 0;
    m_pMapConquerorEffect      = nullptr;
    m_pBeginningEffect         = nullptr;
    m_iIsAttackProc            = 0;
    m_dwAutoAttackTimer        = 0;
    ResetMoveTarget();
}

ClientCharacterManager::~ClientCharacterManager() {
    delete m_pClientCharKindInfo;
    m_pClientCharKindInfo = nullptr;
}

int ClientCharacterManager::Initialize() {
    ResetMoveTarget();
    if (m_pClientCharKindInfo)
        return m_pClientCharKindInfo->Initialize(const_cast<char*>("charkindinfo.txt"));
    return 0;
}

void ClientCharacterManager::Free() {
    if (m_pClientCharKindInfo) m_pClientCharKindInfo->Free();
}

// -------------------------------------------------------------------------
// Slot 查找 / 計數
// -------------------------------------------------------------------------

short ClientCharacterManager::GetMonsterCount() {
    short cnt = 0;
    for (int i = kFirstSlot; i < kSlotCount; ++i) {
        ClientCharacter* p = SlotPtr(i);
        if (SlotAlive(*p) && IsMonster(p->m_wKind)) ++cnt;
    }
    return cnt;
}

short ClientCharacterManager::GetPlayerCount() {
    short cnt = 0;
    for (int i = kFirstSlot; i < kSlotCount; ++i) {
        ClientCharacter* p = SlotPtr(i);
        if (SlotAlive(*p) && !IsMonster(p->m_wKind)) ++cnt;
    }
    return cnt;
}

ClientCharacter* ClientCharacterManager::GetMyCharacterPtr() {
    if (m_dwMyAccountSlot == 0) return nullptr;
    ClientCharacter* p = SlotPtr(static_cast<int>(m_dwMyAccountSlot));
    return SlotAlive(*p) ? p : nullptr;
}

ClientCharacter* ClientCharacterManager::GetCharByAccount(unsigned int account) {
    for (int i = kFirstSlot; i < kSlotCount; ++i) {
        ClientCharacter* p = SlotPtr(i);
        if (SlotAlive(*p) && p->m_dwAccountID == account) return p;
    }
    return nullptr;
}

ClientCharacter* ClientCharacterManager::GetCharByName(char* name) {
    if (!name) return nullptr;
    for (int i = kFirstSlot; i < kSlotCount; ++i) {
        ClientCharacter* p = SlotPtr(i);
        if (!SlotAlive(*p)) continue;
        if (IsMonster(p->m_wKind)) continue;
        if (_stricmp(p->m_szName, name) == 0) return p;
    }
    return nullptr;
}

ClientCharacter* ClientCharacterManager::GetCharByIndex(int index) {
    if (index < 0 || index >= kSlotCount) return nullptr;
    ClientCharacter* p = SlotPtr(index);
    return SlotAlive(*p) ? p : nullptr;
}

// mofclient.c 0x0040E520：找到帳號對應的 slot，更新 manager 內部指標 / 索引，
// 然後啟動角色預設動作狀態 (0x20 = 站立)，並通知伺服器目前 actionState。
void ClientCharacterManager::SetMyAccount(unsigned int account) {
    m_dwMyAccount = account;
    for (int i = kFirstSlot; i < kSlotCount; ++i) {
        ClientCharacter* p = SlotPtr(i);
        if (SlotAlive(*p) && p->m_dwAccountID == account) {
            m_dwMyAccountSlot   = static_cast<unsigned int>(i);
            m_iKeepRunStateFlag = 1;
            m_pMyCharacter      = p;

            // mofclient.c 35952：啟動角色 ActionState 系統的預設站立狀態。
            if (p->m_pActionStateSystem) {
                p->m_pActionStateSystem->SetCharActionState(0x20u);
            }

            // mofclient.c 35953：非 Tutorial 子狀態時通報伺服器 actionState。
            // (BYTE+11548 = ActionStateSystem 內首個 byte，對應目前 acState ID)
            if (g_dwSubGameState != 12) {
                unsigned char acState = p->Decomp<unsigned char>(11548);
                g_Network.ChangeActionState(acState);
            }
            return;
        }
    }
}

// -------------------------------------------------------------------------
// AddCharacter / DeleteCharacter / DeleteAllChar / DeleteAllMonster
// -------------------------------------------------------------------------

void ClientCharacterManager::AddCharacter(
    ClientCharacter* a2, int a3, int a4,
    unsigned short a5, unsigned short a6, int a7,
    const char* a8, const char* a9, int a10,
    const char* a11, const char* a12,
    int a13, int a14, int a15, int a16, int a17,
    int a18, int a19, int a20, int a21, int a22,
    const char* a23, const char* a24,
    unsigned int a25, int a26, int a27, int a28, int a29,
    unsigned char a30, unsigned char a31, unsigned char a32,
    int a33, int a34) {
    // mofclient.c 35605：找空 slot；找不到就放棄。
    int slot = kFirstSlot;
    for (; slot < kSlotCount; ++slot) {
        if (!SlotAlive(*SlotPtr(slot))) break;
    }
    if (slot >= kSlotCount) return;

    // mofclient.c 35617：以 permute 順序前向至 CreateCharacter。
    ClientCharacter* dst = SlotPtr(slot);
    dst->CreateCharacter(
        /*self        */ a2,
        /*posX        */ a3,
        /*posY        */ a4,
        /*charKind    */ a5,
        /*mapID       */ a6,
        /*hp          */ a7,
        /*name        */ const_cast<char*>(a8),
        /*coupleName  */ const_cast<char*>(a9),
        /*coupleRing  */ static_cast<unsigned short>(a10),
        /*petName     */ const_cast<char*>(a11),
        /*circleName  */ const_cast<char*>(a12),
        /*petKind     */ static_cast<unsigned short>(a27),
        /*sexLetter   */ static_cast<unsigned char>(a30),
        /*cUnknown15  */ static_cast<char>(a31),
        /*face        */ static_cast<unsigned char>(a32),
        /*accountID   */ static_cast<unsigned int>(a33),
        /*cUnknown18  */ static_cast<char>(a13),
        /*hair        */ static_cast<unsigned char>(a14),
        /*iUnknown20  */ a15,
        /*ucUnknown21 */ static_cast<unsigned char>(a16),
        /*ucUnknown22 */ static_cast<unsigned char>(a17),
        /*classCode   */ static_cast<unsigned short>(a18),
        /*transformK  */ static_cast<unsigned short>(a19),
        /*ucUnknown25 */ static_cast<unsigned char>(a20),
        /*iUnknown26  */ a21,
        /*cUnknown27  */ static_cast<char>(a22),
        /*marketMsg   */ const_cast<char*>(a23),
        /*ucPCRoom    */ static_cast<unsigned short>(a29),
        /*weaponKind  */ static_cast<unsigned short>(a28),
        /*shieldKind  */ static_cast<unsigned short>(a25),
        /*screenName  */ const_cast<char*>(a24),
        /*nation      */ static_cast<unsigned char>(a26),
        /*titleKind   */ static_cast<unsigned short>(a34),
        /*iUnknown35  */ m_iCreateClientEffectFlag);
    dst->m_dwSlotAlive = 1;
    // mofclient.c 35653：CUIBasic::InitWeddingMap — 新角色加入後重新整理婚禮地圖 UI。
    if (g_UIMgr) {
        if (auto* w = static_cast<CUIBasic*>(g_UIMgr->GetUIWindow(0)))
            w->InitWeddingMap();
    }
    m_iCreateClientEffectFlag = 0;
}

void ClientCharacterManager::DeleteCharacter(unsigned int account, int slotHint) {
    if (m_dwViewCharAccount == account) {
        ClientCharacter* my = GetMyCharacterPtr();
        g_Map.SetViewChar(my);
    }

    ClientCharacter* p = nullptr;
    if (slotHint == 0) {
        DeleteCoupleEffect(account);
        for (int i = kFirstSlot; i < kSlotCount; ++i) {
            ClientCharacter* q = SlotPtr(i);
            if (SlotAlive(*q) && q->m_dwAccountID == account) { p = q; break; }
        }
    } else {
        p = SlotPtr(slotHint);
        if (!SlotAlive(*p)) return;
        DeleteCoupleEffect(p->m_dwAccountID);
        if (p->m_dwAccountID != account) return;
    }
    if (!p) return;

    if (account == m_dwSearchMonsterAccount) m_dwSearchMonsterAccount = 0;
    if (!IsMonster(p->m_wKind)) p->CreateCreateEffect(0);

    {
        // mofclient.c 35742: 若刪掉的角色是當前交易對象，就取消交易。
        unsigned int myAcc = 0, otherAcc = 0;
        g_clTradeSystem.GetAccounts(&myAcc, &otherAcc);
        if (account == myAcc || account == otherAcc) {
            cltMyCharData::DelCharCancelTrade(&g_clMyCharData);
        }
    }

    p->DeleteCharacter();
    p->m_dwSlotAlive = 0;

    // mofclient.c 35767：刪除完同樣呼叫 InitWeddingMap。
    if (g_UIMgr) {
        if (auto* w = static_cast<CUIBasic*>(g_UIMgr->GetUIWindow(0)))
            w->InitWeddingMap();
    }
}

void ClientCharacterManager::DeleteAllChar() {
    g_Map.SetViewChar(GetMyCharacterPtr());
    for (int i = kFirstSlot; i < kSlotCount; ++i) {
        ClientCharacter* p = SlotPtr(i);
        if (SlotAlive(*p)) {
            p->m_dwSlotAlive = 0;
            p->DeleteCharacter();
        }
    }
    m_dwSearchMonsterAccount = 0;
    m_iCharLiveCount         = 0;
    m_pBeginningEffect       = nullptr;
    m_dwViewCharAccount      = 0;
    ResetMoveTarget();
}

void ClientCharacterManager::DeleteAllMonster() {
    for (int i = kFirstSlot; i < kSlotCount; ++i) {
        ClientCharacter* p = SlotPtr(i);
        if (SlotAlive(*p) && IsMonster(p->m_wKind)) p->DeleteCharacter();
    }
    m_dwSearchMonsterAccount = 0;
}

void ClientCharacterManager::AllDieMonster() {
    for (int i = kFirstSlot; i < kSlotCount; ++i) {
        ClientCharacter* p = SlotPtr(i);
        if (SlotAlive(*p) && IsMonster(p->m_wKind)) {
            stCharOrder ord{};
            p->SetOrderDie(&ord);
            p->PushOrder(&ord);
        }
    }
    m_dwSearchMonsterAccount = 0;
}

void ClientCharacterManager::SetUse(unsigned int account, int use) {
    for (int i = kFirstSlot; i < kSlotCount; ++i) {
        ClientCharacter* p = SlotPtr(i);
        if (SlotAlive(*p) && p->m_dwAccountID == account) {
            p->m_dwSlotAlive = static_cast<unsigned int>(use);
            return;
        }
    }
}

void ClientCharacterManager::ResetAutoAttack(unsigned int account) {
    if (account != m_dwSearchMonsterAccount) return;
    cltMyCharData::SetAutoAttack(&g_clMyCharData, 0);
    if (ClientCharacter* my = GetMyCharacterPtr()) {
        my->m_dwUnknown_14820 = 0;
    }
}

// -------------------------------------------------------------------------
// Poll / ProcAutoAttack / SetContinueAutoAttack / MoveForAttack
// -------------------------------------------------------------------------

void ClientCharacterManager::Poll() {
    m_iCharLiveCount = 0;
    m_clTargetMark.SetActive(0);
    CharKeyInputProcess();

    if (m_wMoveForAttackTarget) MoveForAttack(m_wMoveForAttackTarget);
    m_iAttackReady = (ProcAutoAttack() == 0) ? 1 : 0;

    for (int i = kFirstSlot; i < kSlotCount; ++i) {
        ClientCharacter* p = SlotPtr(i);
        if (!SlotAlive(*p)) continue;
        // mofclient.c 35982：把 m_clTargetMark 的位址傳給 ClientCharacter::Poll，
        // 在反編譯中是 (int)v2 透過 ebp 傳進去的 cltTargetMark*。
        if (p->Poll(static_cast<void*>(&m_clTargetMark)) == 1) {
            DeleteCharacter(p->m_dwAccountID, i);
        } else {
            ++m_iCharLiveCount;
        }
    }

    if (g_dwSubGameState != 12) IsInPortralArea(dword_21B8DF4);

    if (ClientCharacter* my = GetMyCharacterPtr()) {
        if (At<unsigned int>(my, 11528) == 0) {
            if (m_iAttackReady) SearchAttackMonster();
            MouseTargetMonster();
        }
    }

    g_clMineMgr.Poll();
    m_clTargetMark.Poll();
    m_clKeepRunState.Poll();
    m_clAutoMoveTarget.Poll();
}

int ClientCharacterManager::ProcAutoAttack() {
    ClientCharacter* my = m_pMyCharacter;
    if (!my) return 0;
#ifdef _WIN32
    if (!::GetFocus()) {
        cltMyCharData::SetAutoAttack(&g_clMyCharData, 0);
        my->m_dwAttackSearchTarget = 0;
        my->m_dwUnknown_14820 = 0;
        return 0;
    }
#endif
    if (!g_iAutoAttackEnabled) return 0;
    if (!Decomp_171(my)) {
        my->m_dwAttackSearchTarget = 0;
        Decomp_AttackProcGate(my) = 0;
        my->m_dwUnknown_14820 = 0;
        return 0;
    }
    if (g_dwSubGameState == 12) return 0;
    int actState = static_cast<int>(my->m_dwActionState);
    if (actState == 3 || actState == 4) return 0;
    if (my->GetLastOrder() == 3) return 0;
    if (!Decomp_162(my)) return 0;
    if (!Decomp_164(my)) return 0;
    if ( Decomp_158(my)) return 0;

    unsigned int searchTarget = my->m_dwAttackSearchTarget;
    ClientCharacter* tgt = GetCharByAccount(searchTarget);
    if (!tgt) return 0;

    Decomp_AttackProcGate(my) = 1;
    my->SetAttackMonster(searchTarget);
    my->AttackMonster();
    // mofclient.c +660 / DWORD+165 — m_iInitFlag_153_176 array slot 12
    my->m_iInitFlag_153_176[165 - 153] = 0;
    SetSearchMonster(searchTarget);
    Decomp_160(tgt) = 1;
    Decomp_159(tgt) = 1;
    return 1;
}

// mofclient.c 36087：當持續攻擊鎖定中、但目標已脫出武器攻擊範圍時，
// 關閉自動攻擊狀態避免空轉。
void ClientCharacterManager::SetContinueAutoAttack() {
    if (!g_iAutoAttackEnabled) return;
    ClientCharacter* my = m_pMyCharacter;
    if (!my) return;
    unsigned short rangeX[2]{}, rangeY[2]{};
    my->GetWeaponAttackRangeXY(rangeX, rangeY);
    ClientCharacter* tgt = GetCharByAccount(my->m_dwAttackTargetAccount);
    if (!tgt
        || std::llabs(static_cast<long long>(my->m_iPosX - tgt->m_iPosX)) > rangeX[0]
        || std::llabs(static_cast<long long>(my->m_iPosY - tgt->m_iPosY)) > rangeY[0]) {
        cltMyCharData::SetAutoAttack(&g_clMyCharData, 0);
    }
}

void ClientCharacterManager::MoveForAttack(unsigned short skillKind) {
    ClientCharacter* my = m_pMyCharacter;
    if (!my) return;
    if (!Decomp_AttackProcGate(my)) return;
    unsigned int target = my->m_dwAttackTargetAccount;
    if (!target) return;
    if (my->GetLastOrder() == 1) return;

    unsigned short rangeX[2]{}, rangeY[2]{};
    my->GetWeaponAttackRangeXY(rangeX, rangeY);
    if (skillKind) {
        if (auto* sk = g_clSkillKindInfo.GetSkillKindInfo(skillKind)) {
            rangeX[0] = *((unsigned short*)sk + 83);
            rangeY[0] = *((unsigned short*)sk + 84);
        }
    }
    ClientCharacter* tgt = GetCharByAccount(target);
    if (!tgt) return;
    int dx = my->m_iDestX - tgt->m_iPosX;
    int dy = my->m_iDestY - tgt->m_iPosY;
    bool needX = std::abs(dx) > rangeX[0];
    bool needY = std::abs(dy) > rangeY[0];
    if (!needX && !needY) return;
    int stepX = needX ? (dx >= 0 ? -1 : 1) * static_cast<int>(my->m_fMoveSpeed) : 0;
    int stepY = needY ? (dy >= 0 ? -1 : 1) * static_cast<int>(my->m_fMoveSpeed) : 0;

    // mofclient.c 36210：碰撞檢查；若新格不可行走且角色未處於隱身（DWORD+2882），
    // 中止 move-target、清除 m_dwUnknown_14820、解除搜怪鎖定。
    const int newX = my->m_iDestX + stepX;
    const int newY = my->m_iDestY + stepY;
    cltMapCollisonInfo* coll =
        ClientCharacter::m_pMap ? ClientCharacter::m_pMap->GetMapCollisonInfo() : nullptr;
    const bool collided = coll && coll->IsCollison(
        static_cast<unsigned short>(newX), static_cast<unsigned short>(newY)) == 1;
    if (!collided || my->m_someOtherState) {
        stCharOrder ord{};
        my->SetOrderMove(&ord, newX, newY, 0);
        my->PushOrder(&ord);
    } else {
        ResetMoveTarget();
        my->m_dwUnknown_14820 = 0;
        my->SetSearchMonster(0);
    }
}

// -------------------------------------------------------------------------
// Died / Resurrect
// -------------------------------------------------------------------------

void ClientCharacterManager::DiedPlayer(int a2) {
    g_Network.MyCharDead(a2);
    cltMyCharData::SetAutoAttack(&g_clMyCharData, 0);
}

void ClientCharacterManager::SetResurrectPlayer(unsigned int account, unsigned short mp) {
    if (ClientCharacter* p = GetCharByAccount(account)) p->AutoResurrect();
    if (account == m_dwMyAccount) {
        // mofclient.c 36261：自己復活 → 關閉死亡 UI、重置 mana。
        if (g_UIMgr) {
            if (auto* w = static_cast<CUIBasic*>(g_UIMgr->GetUIWindow(0)))
                w->ResurrectDiedChar();
        }
        g_clPlayerAbility.SetMana(mp);
    }
}

void ClientCharacterManager::AutoResurrect() {
    for (int i = kFirstSlot; i < kSlotCount; ++i) {
        ClientCharacter* p = SlotPtr(i);
        if (SlotAlive(*p)) p->AutoResurrect();
    }
}

// -------------------------------------------------------------------------
// 位置 / 速度 / Exp / Run / 名稱 / Chat
// -------------------------------------------------------------------------

void ClientCharacterManager::SetPosition(unsigned int account, unsigned int x,
                                         unsigned int y, int speed) {
    if (ClientCharacter* p = GetCharByAccount(account)) {
        stCharOrder ord{};
        p->SetOrderMove(&ord, x, y, static_cast<unsigned int>(speed));
        p->PushOrder(&ord);
    }
}

void ClientCharacterManager::SetCurPosition(unsigned int account, int x, int y) {
    ClientCharacter* p = GetCharByAccount(account);
    if (p && (p->m_iPosX != x || p->m_iPosY != y))
        ClientCharacter::SetCurPosition(p, x, y);
}

int ClientCharacterManager::GetPositionX(unsigned int account) {
    ClientCharacter* p = GetCharByAccount(account); return p ? p->m_iDestX : 0;
}
int ClientCharacterManager::GetPositionY(unsigned int account) {
    ClientCharacter* p = GetCharByAccount(account); return p ? p->m_iDestY : 0;
}
int ClientCharacterManager::GetCurPosX(unsigned int account) {
    ClientCharacter* p = GetCharByAccount(account); return p ? p->m_iPosX : 0;
}
int ClientCharacterManager::GetCurPosY(unsigned int account) {
    ClientCharacter* p = GetCharByAccount(account); return p ? p->m_iPosY : 0;
}
void ClientCharacterManager::SetSpeed(unsigned int account, float speed) {
    if (ClientCharacter* p = GetCharByAccount(account)) p->m_fMoveSpeed = speed;
}
double ClientCharacterManager::GetSpeed(unsigned int account) {
    ClientCharacter* p = GetCharByAccount(account); return p ? p->m_fMoveSpeed : 0.0;
}

void ClientCharacterManager::SetMyPosition(int x, int y) {
    if (ClientCharacter* my = GetMyCharacterPtr()) ClientCharacter::SetEndPosition(my, x, y);
}
int ClientCharacterManager::GetMyPositionX() {
    ClientCharacter* my = GetMyCharacterPtr(); return my ? my->m_iDestX : 0;
}
int ClientCharacterManager::GetMyPositionY() {
    ClientCharacter* my = GetMyCharacterPtr(); return my ? my->m_iDestY : 0;
}
int ClientCharacterManager::GetMyCurPosX() {
    ClientCharacter* my = GetMyCharacterPtr(); return my ? my->m_iPosX : 0;
}
int ClientCharacterManager::GetMyCurPosY() {
    ClientCharacter* my = GetMyCharacterPtr(); return my ? my->m_iPosY : 0;
}
void ClientCharacterManager::SetMySpeed(float speed) {
    if (ClientCharacter* my = GetMyCharacterPtr()) my->m_fMoveSpeed = speed;
}
double ClientCharacterManager::GetMySpeed() {
    ClientCharacter* my = GetMyCharacterPtr(); return my ? my->m_fMoveSpeed : 0.0;
}

unsigned int ClientCharacterManager::GetMyAttackMonster() {
    ClientCharacter* my = GetMyCharacterPtr(); return my ? my->GetAttackMonster() : 0;
}
unsigned int ClientCharacterManager::GetMyTargetMonster() {
    ClientCharacter* my = GetMyCharacterPtr(); return my ? my->GetSearchMonster() : 0;
}
void ClientCharacterManager::GetMySearchMonRange(unsigned short* a, unsigned short* b) {
    if (ClientCharacter* my = GetMyCharacterPtr()) my->GetWeaponSearchRangeXY(a, b);
}
char* ClientCharacterManager::GetMyCharName() {
    ClientCharacter* my = GetMyCharacterPtr(); return my ? my->m_szName : nullptr;
}

void ClientCharacterManager::SetMyGainExp(unsigned int exp) {
    if (ClientCharacter* my = GetMyCharacterPtr()) my->SetGainExp(exp);
}
unsigned int ClientCharacterManager::GetMyGainExp() {
    ClientCharacter* my = GetMyCharacterPtr();
    return my ? my->m_dwGainExpDisplay : 0;
}
void ClientCharacterManager::ViewMyGainExp(unsigned int exp) {
    if (ClientCharacter* my = GetMyCharacterPtr()) my->GainExp(static_cast<int>(exp));
}
void ClientCharacterManager::ResetMyGainExp() {
    if (ClientCharacter* my = GetMyCharacterPtr()) my->ResetGainExp();
}

void ClientCharacterManager::SetRunState(unsigned int account, int state) {
    if (ClientCharacter* p = GetCharByAccount(account))
        p->SetRunState(static_cast<unsigned char>(state));
}
void ClientCharacterManager::ReleaseMyFastRun() {
    if (ClientCharacter* my = GetMyCharacterPtr()) my->ReleaseMyFastRun();
}
void ClientCharacterManager::SetKeepRunState() {
    m_iKeepRunStateFlag = (m_iKeepRunStateFlag == 0) ? 1 : 0;
    ClientCharacter* my = GetMyCharacterPtr();
    m_clKeepRunState.SetActive(my, m_iKeepRunStateFlag);
    if (!m_iKeepRunStateFlag) ReleaseKeepRunState();
}
void ClientCharacterManager::ReleaseKeepRunState() {
    if (ClientCharacter* my = GetMyCharacterPtr()) my->ReleaseKeepRunState();
}

char* ClientCharacterManager::GetPlayerName(unsigned int account) {
    static char s_empty[1] = { '\0' };
    ClientCharacter* p = GetCharByAccount(account); return p ? p->m_szName : s_empty;
}
void ClientCharacterManager::SetCharName(unsigned int account, char* name) {
    ClientCharacter* p = GetCharByAccount(account); if (p && name) std::strcpy(p->m_szName, name);
}
char* ClientCharacterManager::GetCharName(unsigned int account) {
    static char s_empty[1] = { '\0' };
    ClientCharacter* p = GetCharByAccount(account); return p ? p->m_szName : s_empty;
}
void ClientCharacterManager::SetChatMsg(unsigned int account, char* source) {
    if (ClientCharacter* p = GetCharByAccount(account)) p->SetChatMsg(source);
}
void ClientCharacterManager::SetChatMsg(char* charName, char* source) {
    if (ClientCharacter* p = GetCharByName(charName)) p->SetChatMsg(source);
}
void ClientCharacterManager::ClearerChatMsg(unsigned int account) {
    if (ClientCharacter* p = GetCharByAccount(account)) p->ClearerChatMsg();
}
void ClientCharacterManager::ClearerChatMsg(char* charName) {
    if (ClientCharacter* p = GetCharByName(charName)) p->ClearerChatMsg();
}

// -------------------------------------------------------------------------
// CharKeyInputProcess (mofclient.c 0x0040F480) — 簡化骨幹
// -------------------------------------------------------------------------

void ClientCharacterManager::CharKeyInputProcess() {
    if (g_iNPCDialogActive) return;
    ClientCharacter* my = m_pMyCharacter;
    if (!my) return;
    if (my->MapWarpProc()) return;

    int destX = my->m_iDestX;
    int destY = my->m_iDestY;
    my->m_iMoveDirX = 0;
    my->m_iMoveDirY = 0;

    if (m_bMoveTargetActive) {
        int dx = destX - m_iMoveTargetX;
        int dy = destY - m_iMoveTargetY;
        if (my->m_fMoveSpeed > std::abs(dx)) dx = 0;
        if (my->m_fMoveSpeed > std::abs(dy)) dy = 0;
        if (dx == 0 && dy == 0) ResetMoveTarget();
    }

    // mofclient.c 36880：玩家按攻擊鍵但角色狀態不允許 → 觸發引導訊息。
    // 完整的「按鍵觸發」路徑屬於 A1 還原範圍；這裡只在 m_dwAttackTargetAccount
    // 被設定（代表上層 UI 嘗試發起攻擊）但角色狀態無法攻擊時呼叫一次。
    if (my->m_dwAttackTargetAccount) {
        const int actState = static_cast<int>(my->m_dwActionState);
        const bool blocked =
              (actState == 3 || actState == 4)
           || !my->CanAttackSpecialState()
           || my->GetLastOrder() == 3
           || !my->m_iInitFlag_153_176[162 - 153]
           || !my->m_iInitFlag_153_176[164 - 153]
           ||  my->m_iInitFlag_153_176[158 - 153];
        if (blocked) {
            g_clHelpSystem.OnAttack();
        }
    }

    SetContinueAutoAttack();

    // mofclient.c 37189：方向鍵調整後的 (destX, destY) 若落在碰撞格上，
    // 且角色不在隱身狀態 (DWORD+2882)：把目的還原成 m_iDestX/Y、解除
    // move target、清除 m_dwUnknown_14820 與搜怪鎖。
    cltMapCollisonInfo* coll =
        ClientCharacter::m_pMap ? ClientCharacter::m_pMap->GetMapCollisonInfo() : nullptr;
    if (coll && coll->IsCollison(static_cast<unsigned short>(destX),
                                 static_cast<unsigned short>(destY)) == 1
        && !my->m_someOtherState) {
        destX = my->m_iDestX;
        destY = my->m_iDestY;
        ResetMoveTarget();
        my->m_dwUnknown_14820 = 0;
        my->SetSearchMonster(0);
    }

    if (destX == my->m_iDestX && destY == my->m_iDestY) {
        MoveForAttack(0);
    } else if (my->GetLastOrder() != 1) {
        stCharOrder ord{};
        my->SetOrderMove(&ord,
                         static_cast<unsigned int>(destX),
                         static_cast<unsigned int>(destY), 0);
        my->PushOrder(&ord);
    }

    int sx = my->m_iPosX, sy = my->m_iPosY;
    if ((unsigned short)sx != g_wLastSentMoveX || (unsigned short)sy != g_wLastSentMoveY) {
        g_Network.Move(static_cast<unsigned short>(sx),
                       static_cast<unsigned short>(sy),
                       static_cast<unsigned char>(my->m_fMoveSpeed), 0);
        g_wLastSentMoveX = static_cast<unsigned short>(sx);
        g_wLastSentMoveY = static_cast<unsigned short>(sy);
    }

    // mofclient.c 37233：拾取按鍵觸發；隱身狀態 (DWORD+2882 / m_someOtherState) 時跳過。
    if (g_iPickupKeyHeld && my->m_someOtherState == 0) {
        cltMyCharData::RequestPickUpItem(&g_clMyCharData);
    }

    // mofclient.c 37240：方向 + Tab 鎖定附近玩家。我們沒有 IsCharActionKey 全套
    // 動作 ID，但有 g_iPickupKeyHeld / SearchPlayer 開關可由其它系統設定。
    // 這裡保留 ResetSearchPlayer 預設動作避免上一回殘留。
    ResetSearchPlayer();
}

// -------------------------------------------------------------------------
// 角色查詢: width / height / HP / map
// -------------------------------------------------------------------------

unsigned short ClientCharacterManager::GetCharWidthA(unsigned int account) {
    ClientCharacter* p = GetCharByAccount(account); return p ? p->GetCharWidthA() : 0;
}
unsigned short ClientCharacterManager::GetCharHeight(unsigned int account) {
    ClientCharacter* p = GetCharByAccount(account); return p ? p->GetCharHeight() : 0;
}
unsigned short ClientCharacterManager::GetMonsterInfoPosY(unsigned int account) {
    ClientCharacter* p = GetCharByAccount(account); return p ? p->m_wUnknown_4432 : 0;
}

void ClientCharacterManager::SetMyHP(int hp) {
    if (ClientCharacter* my = GetMyCharacterPtr()) {
        my->m_dwUnknown_11256 = static_cast<unsigned int>(hp);
        my->SetHP();
    }
}
void ClientCharacterManager::SetMyTempHP(int hp) {
    if (ClientCharacter* my = GetMyCharacterPtr()) my->m_dwUnknown_11256 = static_cast<unsigned int>(hp);
}
void ClientCharacterManager::UseHPPotion(int amount) {
    if (ClientCharacter* my = GetMyCharacterPtr()) my->SetUseHPPotionEffect(amount);
}
int ClientCharacterManager::GetMyHP() {
    ClientCharacter* my = GetMyCharacterPtr(); return my ? my->GetHP() : 0;
}
void ClientCharacterManager::SetHP(unsigned int account, int /*hp*/) {
    if (ClientCharacter* p = GetCharByAccount(account)) p->SetHP();
}
ClientCharacter* ClientCharacterManager::GetHP(unsigned int account) {
    ClientCharacter* p = GetCharByAccount(account); if (p) p->GetHP(); return p;
}
void ClientCharacterManager::SetTempHP(unsigned int account, int hp) {
    if (ClientCharacter* p = GetCharByAccount(account)) p->m_dwUnknown_11256 = static_cast<unsigned int>(hp);
}
unsigned short ClientCharacterManager::GetMapID(unsigned int account) {
    ClientCharacter* p = GetCharByAccount(account); return p ? p->m_wMapID : 0;
}

// -------------------------------------------------------------------------
// 搜尋／攻擊／LR
// -------------------------------------------------------------------------

void ClientCharacterManager::SetSearchMonster(unsigned int account) {
    if (ClientCharacter* my = GetMyCharacterPtr()) my->SetSearchMonster(account);
}

unsigned char
ClientCharacterManager::SearchMonsterInSkillRange(unsigned short skillKind) {
    ClientCharacter* my = GetMyCharacterPtr();
    if (!my) return 0;
    int rx = 50, ry = 50;
    int cx = my->m_iPosX, cy = my->m_iPosY;
    if (skillKind) {
        auto* sk = g_clSkillKindInfo.GetSkillKindInfo(skillKind);
        if (!sk) return 0;
        rx = *((unsigned short*)sk + 83);
        ry = *((unsigned short*)sk + 84);
        if (*((int*)sk + 81)) {
            unsigned int sm = my->GetSearchMonster();
            if (!sm) return 0;
            ClientCharacter* tgt = GetCharByAccount(sm);
            if (!tgt) return 0;
            cx = tgt->m_iPosX; cy = tgt->m_iPosY;
        }
    } else {
        unsigned int sm = my->GetSearchMonster();
        if (!sm) return 0;
        ClientCharacter* tgt = GetCharByAccount(sm);
        if (!tgt) return 0;
        cx = tgt->m_iPosX; cy = tgt->m_iPosY;
    }
    unsigned char count = 0;
    m_dwSearchMonInRangeCount = 0;
    for (int i = kFirstSlot; i < kSlotCount && count < 40; ++i) {
        if (i == static_cast<int>(m_dwMyAccountSlot)) continue;
        ClientCharacter* p = SlotPtr(i);
        if (!SlotAlive(*p)) continue;
        int actState = static_cast<int>(p->m_dwActionState);
        if (actState == 2 || actState == 7) continue;
        if (!Decomp_171(p)) continue;
        if (dword_21B8DFC) {
            int targetTeam = g_clPKManager.GetTeamByAccount(p->m_dwAccountID);
            int myTeam = g_clPKManager.GetTeamByAccount(m_dwMyAccount);
            if (targetTeam == myTeam) continue;
            if (IsMonster(p->m_wKind)) continue;
        } else if (!IsMonster(p->m_wKind)) {
            continue;
        }
        // mofclient.c 37604：分身(Clone)角色不納入技能命中範圍計算。
        if (m_pClientCharKindInfo &&
            m_pClientCharKindInfo->GetIsClone(p->m_wKind)) continue;
        int dx = p->m_iPosX - cx, dy = p->m_iPosY - cy;
        if (std::abs(dx) > rx || std::abs(dy) > ry) continue;
        m_aSearchMonsterInRange[count] = p->m_dwAccountID;
        ++count;
    }
    m_dwSearchMonInRangeCount = count;
    return count;
}

void ClientCharacterManager::SetMyCharLR(int lr) {
    if (ClientCharacter* my = GetMyCharacterPtr()) my->m_dwLR_Flag = lr;
}
int ClientCharacterManager::GetMyCharLR() {
    ClientCharacter* my = GetMyCharacterPtr(); return my ? my->m_dwLR_Flag : 0;
}
void ClientCharacterManager::SetCharLR(unsigned int account, int lr) {
    if (ClientCharacter* p = GetCharByAccount(account)) p->m_dwLR_Flag = lr;
}

void ClientCharacterManager::SearchAttackMonster() {
    if (g_iNPCDialogActive) return;
    m_dwSearchAttackMonAccount = 0;
    for (int i = kFirstSlot; i < kSlotCount; ++i) {
        if (i == static_cast<int>(m_dwMyAccountSlot)) continue;
        ClientCharacter* p = SlotPtr(i);
        if (!SlotAlive(*p)) continue;
        Decomp_159(p) = 0;
        Decomp_160(p) = 0;
    }
    ClientCharacter* my = GetMyCharacterPtr();
    if (!my) return;
    if (my->m_someOtherState) return;
    if (Decomp_175(my))     return;
    if (Decomp_169(my))     return;

    unsigned short rangeX[2]{}, rangeY[2]{};
    my->GetWeaponSearchRangeXY(rangeX, rangeY);
    int bestDist = 2400;
    unsigned int bestAcct = 0;
    for (int i = kFirstSlot; i < kSlotCount; ++i) {
        if (i == static_cast<int>(m_dwMyAccountSlot)) continue;
        ClientCharacter* p = SlotPtr(i);
        if (!SlotAlive(*p)) continue;
        if (!Decomp_171(p)) continue;
        if (!IsMonster(p->m_wKind)) continue;
        // mofclient.c 37604：分身(Clone)角色不算可攻擊目標。
        if (m_pClientCharKindInfo &&
            m_pClientCharKindInfo->GetIsClone(p->m_wKind)) continue;
        int dx = my->m_iPosX - p->m_iPosX;
        int dy = my->m_iPosY - p->m_iPosY;
        if (std::abs(dx) > rangeX[0] || std::abs(dy) > rangeY[0]) continue;
        int dist = std::abs(dx) + std::abs(dy);
        if (dist >= bestDist) continue;
        if (dx < 0 && my->m_dwLR_Flag != 0) continue;
        if (dx > 0 && my->m_dwLR_Flag != 1) continue;
        bestDist = dist;
        bestAcct = p->m_dwAccountID;
    }
    if (bestAcct) {
        SetSearchMonster(bestAcct);
        if (ClientCharacter* p = GetCharByAccount(bestAcct)) {
            Decomp_159(p) = 1;
            Decomp_160(p) = 1;
        }
        m_dwSearchAttackMonAccount = bestAcct;
    } else {
        // mofclient.c 37855：找不到任何可攻擊目標時關閉自動攻擊。
        cltMyCharData::SetAutoAttack(&g_clMyCharData, 0);
    }
}

unsigned int ClientCharacterManager::GetUseSkillAttackMonAccount(
        unsigned short rangeX, unsigned short rangeY) {
    ClientCharacter* my = GetMyCharacterPtr();
    if (!my) return 0;
    int bestDist = 2400;
    unsigned int bestAcct = 0;
    for (int i = kFirstSlot; i < kSlotCount; ++i) {
        if (i == static_cast<int>(m_dwMyAccountSlot)) continue;
        ClientCharacter* p = SlotPtr(i);
        if (!SlotAlive(*p)) continue;
        int actState = static_cast<int>(p->m_dwActionState);
        if (actState == 2 || actState == 7) continue;
        if (!Decomp_171(p)) continue;
        if (dword_21B8DFC) {
            int t = g_clPKManager.GetTeamByAccount(p->m_dwAccountID);
            int mn = g_clPKManager.GetTeamByAccount(m_dwMyAccount);
            if (t == mn) continue;
        } else if (IsPlayer(p->m_wKind)) {
            continue;
        }
        int dx = my->m_iPosX - p->m_iPosX;
        int dy = my->m_iPosY - p->m_iPosY;
        if (std::abs(dx) > rangeX || std::abs(dy) > rangeY) continue;
        int dist = std::abs(dx) + std::abs(dy);
        if (dist >= bestDist) continue;
        if (dx < 0 && my->m_dwLR_Flag != 0) continue;
        if (dx > 0 && my->m_dwLR_Flag != 1) continue;
        bestDist = dist;
        bestAcct = p->m_dwAccountID;
    }
    if (bestAcct) m_dwSearchMonsterAccount = bestAcct;
    return bestAcct;
}

void ClientCharacterManager::SetRealAttackMonAccount(unsigned int account) {
    if (account) {
        m_dwSearchMonsterAccount = account;
        if (ClientCharacter* p = GetCharByAccount(account)) Decomp_160(p) = 1;
        SetDrawInfoText(account, 1);
        SetSearchMonster(account);
    } else {
        m_dwSearchMonsterAccount = 0;
        SetSearchMonster(0);
    }
}

void ClientCharacterManager::Attack(unsigned int attacker, unsigned int target,
                                    char a4, int damage, int remainHP, char a7) {
    ClientCharacter* a = GetCharByAccount(attacker);
    ClientCharacter* t = GetCharByAccount(target);
    if (!a || !t) return;
    a->m_dwLR_Flag = (a->m_iPosX - t->m_iPosX) > 0 ? 1 : 0;
    stCharOrder ord{};
    a->SetOrderAttack(&ord, target, static_cast<int>(a4), damage, remainHP,
                      static_cast<int>(a7), 0);
    a->PushOrder(&ord);
    if (attacker == m_dwMyAccount) {
        SetRealAttackMonAccount(target);
        // mofclient.c 38118：自己的攻擊命中時，引導系統登錄擊殺事件。
        g_clHelpSystem.OnKillMonster(t->m_wKind);
    }
}

void ClientCharacterManager::AttackedBySkill(
        unsigned int attackerAccount, unsigned short skillKind,
        unsigned int* const targets,
        unsigned char (*const motions)[3], int (*const damages)[3],
        int* const remainHPs, unsigned char* const hitInfos) {
    auto* skill = g_clSkillKindInfo.GetSkillKindInfo(skillKind);
    ClientCharacter* attacker = GetCharByAccount(attackerAccount);
    if (!attacker) return;
    attacker->SetUseSkillKind(skillKind, targets);

    for (int i = 0; targets[i]; ++i) {
        ClientCharacter* tgt = GetCharByAccount(targets[i]);
        if (!tgt) continue;
        if (i == 0)
            attacker->m_dwLR_Flag = (attacker->m_iPosX - tgt->m_iPosX) > 0 ? 1 : 0;

        unsigned char skillType = skill ? *((unsigned char*)skill + 112) : 0;
        if (skillType != 1 && skillType != 2) continue;

        tgt->SetHittedSkillKind(skillKind);
        if (skillType == 1 || i == 0) {
            stCharOrder ord{};
            attacker->SetOrderAttack(&ord, targets[i],
                                     static_cast<char>(motions[i][0]),
                                     damages[i][0],
                                     remainHPs[i],
                                     static_cast<char>(hitInfos[i]),
                                     skillKind);
            attacker->PushOrder(&ord);
        }

        int hitCount = skill ? *((int*)skill + 52) : 0;
        for (int k = 0; k < hitCount; ++k) {
            unsigned char canCharHit = skill ? *((unsigned char*)skill + 164) : 0;
            tgt->m_iInitFlag_153_176[166 - 153] = canCharHit;
            if (canCharHit) {
                tgt->SetCharHitedInfo(attackerAccount, targets[i],
                                      static_cast<char>((k == hitCount - 1) ? 1 : 0),
                                      static_cast<char>(motions[i][k]),
                                      damages[i][k], remainHPs[i],
                                      static_cast<char>(attacker->m_iPosX - tgt->m_iPosX > 0),
                                      static_cast<char>(3),
                                      skillKind, hitInfos[i]);
            } else {
                unsigned char m = motions[i][k];
                if (m == 2) {
                    tgt->HitMissed();
                } else {
                    stCharOrder ord{};
                    if (tgt->SetOrderHitted(&ord, m, damages[i][k], remainHPs[i],
                                            static_cast<char>(attacker->m_iPosX - tgt->m_iPosX > 0),
                                            static_cast<char>(3), skillKind, hitInfos[i])) {
                        tgt->PushOrder(&ord);
                    }
                }
            }
            if (skillType == 2) tgt->SetDied();
        }

        if (skillType == 2 && tgt->m_iInitFlag_153_176[166 - 153] == 0) {
            stCharOrder ord{};
            if (IsMonster(tgt->m_wKind)) tgt->SetOrderDie(&ord);
            else                          tgt->SetOrderDied(&ord);
            tgt->PushOrder(&ord);
        }
    }
}

void ClientCharacterManager::AttackedByMine(
        unsigned int /*attackerAccount*/, unsigned short /*attackerKind*/,
        unsigned int mineHandle, unsigned short skillKind,
        unsigned int* const targets,
        unsigned char (*const motions)[3], int (*const damages)[3],
        int* const remainHPs, unsigned char* const hitInfos) {
    auto* skill = g_clSkillKindInfo.GetSkillKindInfo(skillKind);
    if (!skill) return;
    auto* mine = g_clMineMgr.GetMineByHandel(mineHandle);
    for (int i = 0; targets[i]; ++i) {
        ClientCharacter* tgt = GetCharByAccount(targets[i]);
        if (!tgt) continue;
        tgt->SetHittedSkillKind(skillKind);
        char lr = static_cast<char>(static_cast<unsigned char>(tgt->m_dwLR_Flag & 0xFF));
        stCharOrder ord{};
        if (remainHPs[i]) {
            if (motions[i][0] == 2) tgt->HitMissed();
            else if (tgt->SetOrderHitted(&ord, 1, damages[i][0], remainHPs[i],
                                         lr, 3, skillKind, hitInfos[i]))
                tgt->PushOrder(&ord);
        } else {
            if (mine) lr = (*((float*)mine + 7) - static_cast<double>(tgt->m_iPosX)) > 0.0f ? 1 : 0;
            if (tgt->SetOrderHitted(&ord, 1, damages[i][0], 0, lr, 3, skillKind, hitInfos[i]))
                tgt->PushOrder(&ord);
            if (IsMonster(tgt->m_wKind)) tgt->SetOrderDie(&ord);
            else                          tgt->SetOrderDied(&ord);
            tgt->PushOrder(&ord);
            tgt->SetDied();
        }
        char* sound = g_clSkillKindInfo.UseSkillSound(skillKind);
        g_GameSoundManager.PlaySoundA(sound, tgt->m_iPosX, tgt->m_iPosY);
        char h0008[] = "H0008";
        g_GameSoundManager.PlaySoundA(h0008, tgt->m_iPosX, tgt->m_iPosY);
    }
    g_clMineMgr.DeleteMineByHandle(mineHandle, *((unsigned char*)skill + 292));
}

// -------------------------------------------------------------------------
// SearchPlayer / Set/Get/Reset variants
// -------------------------------------------------------------------------

void ClientCharacterManager::SearchPlayer() {
    m_dwSearchPlayerAccount = 0;
    m_dwSearchDiedPlayerAccount = 0;
    if (g_iNPCDialogActive) return;
    ClientCharacter* my = GetMyCharacterPtr();
    if (!my) return;
    int bestDist = 2400;
    unsigned int bestAcct = 0;
    unsigned int diedAcct = 0;
    for (int i = kFirstSlot; i < kSlotCount; ++i) {
        ClientCharacter* p = SlotPtr(i);
        if (!SlotAlive(*p)) continue;
        if (IsMonster(p->m_wKind)) continue;
        if (p->m_dwAccountID == m_dwMyAccount) continue;
        if (dword_21B8DFC && !g_clPKManager.IsMyTeam(p->m_dwAccountID)) continue;
        int dx = my->m_iPosX - p->m_iPosX;
        int dy = my->m_iPosY - p->m_iPosY;
        int dist = std::abs(dx) + std::abs(dy);
        if (dist > bestDist || std::abs(dx) > 300 || std::abs(dy) > 200) continue;
        if (dx < 0 && my->m_dwLR_Flag != 0) continue;
        if (dx > 0 && my->m_dwLR_Flag != 1) continue;
        bestDist = dist;
        bestAcct = p->m_dwAccountID;
        int actState = static_cast<int>(p->m_dwActionState);
        if (actState == 2 || actState == 7) diedAcct = p->m_dwAccountID;
    }
    if (bestAcct) {
        SetSearchPlayer(bestAcct);
        SetDrawInfoText(bestAcct, 1);
        m_clTargetMark.SetActive(1);
        m_clTargetMark.SetPos(static_cast<float>(GetCurPosX(bestAcct)),
                              static_cast<float>(GetCurPosY(bestAcct) - 20));
    }
    if (diedAcct) {
        SetSearchDiedPlayer(diedAcct);
        SetDrawInfoText(bestAcct, 1);
        m_clTargetMark.SetActive(1);
        m_clTargetMark.SetPos(static_cast<float>(GetCurPosX(bestAcct)),
                              static_cast<float>(GetCurPosY(bestAcct) - 20));
    }
}

void ClientCharacterManager::SetSearchPlayer(unsigned int account)        { m_dwSearchPlayerAccount = account; }
unsigned int ClientCharacterManager::GetSearchPlayerAccount()             { return m_dwSearchPlayerAccount; }
void ClientCharacterManager::SetSearchDiedPlayer(unsigned int account)    { m_dwSearchDiedPlayerAccount = account; }
unsigned int ClientCharacterManager::GetSearchDiedPlayerAccount()         { return m_dwSearchDiedPlayerAccount; }
void ClientCharacterManager::ResetSearchPlayer() {
    m_dwSearchPlayerAccount = 0;
    m_dwSearchDiedPlayerAccount = 0;
}

void ClientCharacterManager::IsInPortralArea(unsigned short mapID) {
    if (g_iNPCDialogActive) return;
    if (g_wInstantDungeonMapKind) {
        int outMap = 0;
        unsigned char act = m_clInstansDungeonPortal.IsPortalAction(
            GetMyCurPosX(), GetMyCurPosY(), &outMap);
        if (act == 3) {
            SetMyWarp(1, static_cast<unsigned short>(outMap), 1, 0, 0);
            return;
        }
    }
    unsigned short outMapKind[2]{}, outWorldKind[2]{};
    unsigned char outFlag[4]{};
    int level = g_clLevelSystem.GetLevel();
    int act = g_clClientPortalInfo.IsPortalAction(
        GetMyCurPosX(), GetMyCurPosY(), mapID,
        outMapKind, outFlag, outWorldKind, level);
    switch (act) {
    case 1:
        SetMyWarp(0, 0, 0, 0, 0);
        m_dwLastWarpFailTime = 0;
        break;
    case 2: {
#ifdef _WIN32
        DWORD now = ::timeGetTime();
        if (now - m_dwLastWarpFailTime <= 0x2710) return;
        SetMyWarp(0, 0, 0, 0, 0);
        char buf[256] = {};
        const char* msg = g_DCTTextManager.GetText(3859);
        if (msg) wsprintfA(buf, msg, outFlag[0]);
        cltSystemMessage::SetSystemMessage(&g_clSysemMessage, buf, 0, 0, 0);
        m_dwLastWarpFailTime = now;
#endif
        break;
    }
    case 3:
        SetMyWarp(1, outMapKind[0], 0, 0, outWorldKind[0]);
        m_dwLastWarpFailTime = 0;
        break;
    case 4:
        if (g_iResurrectInstantWarp) { SetMyWarp(1, 0, 0, 0, 0); return; }
#ifdef _WIN32
        if (::timeGetTime() - m_dwLastWarpFailTime > 0x2710) {
            const char* m = g_DCTTextManager.GetText(4481);
            cltSystemMessage::SetSystemMessage(&g_clSysemMessage, m, 0, 0, 0);
            m_dwLastWarpFailTime = ::timeGetTime();
        }
#endif
        break;
    default: break;
    }
}

void ClientCharacterManager::SetMyWarp(int a2, unsigned short mapKind, int a4,
                                       int a5, unsigned short worldKind) {
    if (ClientCharacter* my = GetMyCharacterPtr()) my->SetWarp(a2, mapKind, a4, a5, worldKind);
}

void ClientCharacterManager::SetItem(unsigned int account,
                                     unsigned short itemKind, int qty) {
    if (ClientCharacter* p = GetCharByAccount(account)) p->SetItem(itemKind, qty);
}
void ClientCharacterManager::SetMyItem(unsigned short itemKind, int qty) {
    if (ClientCharacter* my = GetMyCharacterPtr()) my->SetItem(itemKind, qty);
}
void ClientCharacterManager::ResetMyItem(unsigned char slot) {
    if (ClientCharacter* my = GetMyCharacterPtr()) my->ResetItem(slot);
}
void ClientCharacterManager::ResetItem(unsigned int account, unsigned char slot) {
    if (ClientCharacter* p = GetCharByAccount(account)) p->ResetItem(slot);
}
void ClientCharacterManager::SetMyCAClone() {
    if (ClientCharacter* my = GetMyCharacterPtr()) my->SetCAClone();
}

// mofclient.c 0x00412370：找到滑鼠位於哪個角色的點擊框上。
//   a4 = 0：只接受玩家（hover-on-player）；a4 != 0：只接受怪物。
//   點擊框 = 角色頭頂往下到目前位置，左右各擴張 (charWidth/2 + 15) 像素。
//   自己（v17 == m_dwMyAccountSlot）的點擊框會略小（去掉 +15 / +10 padding）。
unsigned int ClientCharacterManager::MouseOverCharacter(int x, int y, int isMonster) {
    POINT pt;
    pt.x = x + g_Map.GetScrollX();
    pt.y = y + g_Map.GetScrollY();

    for (int v17 = kFirstSlot; v17 < kSlotCount; ++v17) {
        ClientCharacter* p = SlotPtr(v17);
        if (!SlotAlive(*p)) continue;

        // mofclient.c 38899：本 slot 已被 "currently using" 旗標佔用 (DWORD+175)，
        // 且帳號不是我的，就跳過。
        if (p->m_iInitFlag_153_176[175 - 153]
            && p->m_dwAccountID != m_dwMyAccount) {
            continue;
        }

        // 角色種類過濾：依照 a4 取怪物或玩家。
        bool match = isMonster ? IsMonster(p->m_wKind) : IsPlayer(p->m_wKind);
        if (!match) continue;

        // 38903：m_someOtherState (DWORD+2882) != 0 → hidden / 無法點選。
        if (p->m_someOtherState) continue;

        unsigned short halfW = static_cast<unsigned short>((p->GetCharWidthA() >> 1) + 15);
        unsigned short topPad = 10;
        if (v17 == static_cast<int>(m_dwMyAccountSlot)) {
            halfW = static_cast<unsigned short>(halfW - 15);
            topPad = 0;
        }

        RECT rc;
        rc.left   = p->m_iPosX - halfW;
        rc.right  = p->m_iPosX + halfW;
        rc.top    = p->m_iPosY - topPad - p->GetCharHeight();
        rc.bottom = p->m_iPosY + topPad;
        if (PtInRect(&rc, pt)) return p->m_dwAccountID;
    }
    return 0;
}
ClientCharacter* ClientCharacterManager::MouseOverCharacterPtr(int x, int y, int m) {
    return GetCharByAccount(MouseOverCharacter(x, y, m));
}
ClientCharacter* ClientCharacterManager::MouseTargetMonster() {
    if (!GetMyCharacterPtr()) return nullptr;
    if (!g_iMouseTargetEnabled) return nullptr;
    return MouseOverCharacterPtr(0, 0, 0);
}
ClientCharacter* ClientCharacterManager::MouseTargetMonsterPtr() {
    ClientCharacter* p = MouseTargetMonster();
    return p ? GetCharByAccount(p->m_dwAccountID) : nullptr;
}

// -------------------------------------------------------------------------
// IsMonster / IsPlayer / GetCharKind / GetMonsterName / EtcMark
// -------------------------------------------------------------------------

bool ClientCharacterManager::IsMonster(unsigned int account) {
    ClientCharacter* p = GetCharByAccount(account);
    unsigned short kind = p ? p->m_wKind : static_cast<unsigned short>(account);
    return IsMonster(kind);
}

bool ClientCharacterManager::IsMonster(unsigned short kindCode) {
    if (m_pClientCharKindInfo)
        return m_pClientCharKindInfo->IsMonsterChar(kindCode) != 0;
    if (ClientCharacter::m_pClientCharKindInfo)
        return ClientCharacter::m_pClientCharKindInfo->IsMonsterChar(kindCode) != 0;
    return false;
}

bool ClientCharacterManager::IsPlayer(unsigned short kindCode) {
    if (m_pClientCharKindInfo)
        return m_pClientCharKindInfo->IsPlayerChar(kindCode) != 0;
    if (ClientCharacter::m_pClientCharKindInfo)
        return ClientCharacter::m_pClientCharKindInfo->IsPlayerChar(kindCode) != 0;
    return false;
}

unsigned short ClientCharacterManager::GetCharKind(unsigned int account) {
    ClientCharacter* p = GetCharByAccount(account); return p ? p->m_wKind : 0;
}

unsigned short ClientCharacterManager::GetMonsterName(unsigned short kindCode) {
    if (!m_pClientCharKindInfo) return 0;
    void* info = m_pClientCharKindInfo->GetCharKindInfo(kindCode);
    return info ? *((unsigned short*)info + 1) : 0;
}

void ClientCharacterManager::PrepareDrawingEtcMark() {
    m_clTargetMark.PrepareDrawing();
    m_clKeepRunState.PrepareDrawing();
    m_clAutoMoveTarget.PrepareDrawing();
}
void ClientCharacterManager::DrawEtcMark() {
    m_clTargetMark.Draw();
    m_clKeepRunState.Draw();
    m_clAutoMoveTarget.Draw();
}
void ClientCharacterManager::SetDrawInfoText(unsigned int account, int active) {
    if (ClientCharacter* p = GetCharByAccount(account)) Decomp_159(p) = active;
}

CCA* ClientCharacterManager::GetCAData(unsigned int account,
                                       unsigned char* a3, int* a4, int* a5,
                                       unsigned int* a6, unsigned short* a7) {
    if (ClientCharacter* p = GetCharByAccount(account))
        return p->GetCAData(a3, a4, a5, a6, a7);
    return nullptr;
}

void ClientCharacterManager::SetCharState(unsigned int account, unsigned char state) {
    ClientCharacter* p = GetCharByAccount(account);
    if (!p) return;
    p->SetCharState(static_cast<char>(state));
    if (p->SetSpecialState(state)) p->CreateSpecialStateEffect(state);
}

stCharKindInfo* ClientCharacterManager::GetCharKindInfo(unsigned short kindCode) {
    if (m_pClientCharKindInfo)
        return static_cast<stCharKindInfo*>(m_pClientCharKindInfo->GetCharKindInfo(kindCode));
    return nullptr;
}
cltMonsterAniInfo*
ClientCharacterManager::GetMonsterAniInfo(unsigned short kindCode) {
    if (m_pClientCharKindInfo) return m_pClientCharKindInfo->GetMonsterAniInfo(kindCode);
    return nullptr;
}

void ClientCharacterManager::SetEmoticonKind(unsigned int account, int emoticonKind) {
    if (ClientCharacter* p = GetCharByAccount(account)) p->SetEmoticonKind(emoticonKind);
}
void ClientCharacterManager::SetEmoticonKind(char* charName, int emoticonKind) {
    if (ClientCharacter* p = GetCharByName(charName)) p->SetEmoticonKind(emoticonKind);
}
void ClientCharacterManager::ReleaseEmoticon(ClientCharacter* p) {
    if (p) p->ReleaseEmoticon();
}

ClientCharacter* ClientCharacterManager::GetMyTransformationed() {
    ClientCharacter* my = GetMyCharacterPtr();
    if (!my) return nullptr;
    int t = Decomp_169(my);
    return t ? reinterpret_cast<ClientCharacter*>(static_cast<intptr_t>(t)) : nullptr;
}

void ClientCharacterManager::SetSpiritSpeech(int a2) { m_iSpiritSpeech = a2; }

int ClientCharacterManager::CanCreatePrivateMarket() {
    ClientCharacter* my = GetMyCharacterPtr();
    if (!my) return 0;
    auto* mi = g_Map.GetMapInfoByID(my->m_wMapID);
    if (!mi) return 0;
    if (!*((int*)mi + 73)) return 0;
    int mx = my->m_iPosX, myc = my->m_iPosY;
    for (int i = kFirstSlot; i < kSlotCount; ++i) {
        ClientCharacter* p = SlotPtr(i);
        if (!SlotAlive(*p)) continue;
        if (p->m_dwAccountID == m_dwMyAccount) continue;
        if (std::abs(mx - p->m_iPosX) >= 100) continue;
        if (std::abs(myc - p->m_iPosY) >= 100) continue;
        unsigned char st = p->m_ucTradeState;
        if (st == 2 || st == 3) return 0;
    }
    return 1;
}

int ClientCharacterManager::CanCreatePrivateMarket(unsigned int account) {
    ClientCharacter* p = GetCharByAccount(account);
    if (!p) return 0;
    ClientCharacter* my = GetMyCharacterPtr();
    if (!my) return 0;
    auto* mi = g_Map.GetMapInfoByID(my->m_wMapID);
    if (!mi || !*((int*)mi + 73)) return 0;
    int px = p->m_iPosX, py = p->m_iPosY;
    for (int i = kFirstSlot; i < kSlotCount; ++i) {
        ClientCharacter* q = SlotPtr(i);
        if (!SlotAlive(*q)) continue;
        if (q->m_dwAccountID == account) continue;
        if (std::abs(px - q->m_iPosX) > 100) continue;
        if (std::abs(py - q->m_iPosY) > 100) continue;
        if (q->m_ucTradeState) return 0;
    }
    return 1;
}

ClientCharacter* ClientCharacterManager::CanJoinPrivateMarket(unsigned int account) {
    if (!account) return nullptr;
    ClientCharacter* my = GetMyCharacterPtr();
    if (!my) return nullptr;
    if (my->m_ucTradeState != 2) return nullptr;
    ClientCharacter* p = GetCharByAccount(account);
    if (!p) return nullptr;
    return (p->m_ucTradeState == 1) ? p : nullptr;
}

void ClientCharacterManager::SetCanNotChatting(int active) {
    cltMyCharData::SetCanNotChatting(&g_clMyCharData,
                                     static_cast<unsigned char>(active));
    if (active) g_clChattingMgr.SetBanChat();
}
int ClientCharacterManager::CanNotChatting() {
    return cltMyCharData::GetCanNotChatting(&g_clMyCharData);
}

void ClientCharacterManager::CreateClientEffect() { m_iCreateClientEffectFlag = 1; }

void ClientCharacterManager::ChangeMySex() {
    ClientCharacter* my = GetMyCharacterPtr();
    if (!my) return;
    unsigned char sex = g_clSexSystem.GetSex();
    if (_toupper(sex) == 'M') my->m_ucSex = 1;
    else if (_toupper(sex) == 'F') my->m_ucSex = 0;
    my->m_ucHair = g_clBasicAppearSystem.GetHair();
    my->m_ucFace = g_clBasicAppearSystem.GetFace();
    if (my->m_pCCA)
        my->m_pCCA->InitItem(my->m_ucSex, my->m_ucHair, my->m_ucFace,
                             my->m_iInitFlag_153_176[150 - 153]);
    if (dword_21BA32C) {
        unsigned short* eq1 = dword_21BA32C->GetEquipItemList(0);
        if (eq1) for (int i = 0; i < 11; ++i) if (eq1[i]) my->SetItem(eq1[i], 1);
        unsigned short* eq2 = dword_21BA32C->GetEquipItemList(1);
        if (eq2) for (int i = 0; i < 11; ++i) if (eq2[i]) my->SetItem(eq2[i], 1);
    }
    my->SetCAClone();
}

// mofclient.c 39326：紀錄地圖征服者名字並建立其專屬光環特效，掛到
// EffectManager_Before_Chr 上。重新呼叫時會覆蓋名稱與特效。
void ClientCharacterManager::SetMapConqueror(char* name) {
    if (name) {
        std::strncpy(m_szMapConquerorName, name, sizeof(m_szMapConquerorName) - 1);
        m_szMapConquerorName[sizeof(m_szMapConquerorName) - 1] = '\0';
    }
    if (ClientCharacter* p = GetCharByName(name)) {
        auto* eff = new CEffect_Player_MapConqueror();
        eff->SetEffect(p);
        if (g_pEffectManager_Before_Chr) {
            g_pEffectManager_Before_Chr->BulletAdd(eff);
        }
        m_pMapConquerorEffect = eff;
    }
}
bool ClientCharacterManager::IsMapConqueror(char* name) {
    if (!name) return false;
    return std::strcmp(m_szMapConquerorName, name) == 0;
}
// mofclient.c 39353：偵測角色名稱中是否包含 "[GM]" 標籤。
// 因角色名可能含中文 DBCS（leading byte 0x81-0xFE 後接 trail byte），掃描時
// 必須遇到 lead byte 跳過下一個 byte，避免把 trail byte 當成 '['。
int ClientCharacterManager::IsGM(char* name) {
    if (!name) return 0;
    const char* tag = "[GM]";
    const std::size_t tagLen = 4;
    std::size_t nameLen = std::strlen(name);
    if (nameLen < tagLen) return 0;

    const char* p = name;
    while (p + tagLen <= name + nameLen) {
#ifdef _WIN32
        if (::IsDBCSLeadByte(static_cast<BYTE>(*p))) {
            p += 2;
            continue;
        }
#else
        if (static_cast<unsigned char>(*p) >= 0x81) {
            p += 2;
            continue;
        }
#endif
        if (std::strncmp(p, tag, tagLen) == 0) return 1;
        ++p;
    }
    return 0;
}

void ClientCharacterManager::MoveMapFail() {
    g_bIsMoveMap = 1;
    SetMyWarp(0, 0, 0, 0, 0);
    if (ClientCharacter* my = GetMyCharacterPtr()) {
        int dx = my->m_dwLR_Flag ? -50 : 50;
        my->m_iDestX += dx;
        ClientCharacter::SetCurPosition(my, my->m_iDestX, my->m_iDestY);
        my->InitCreateWarpEffectFlag();
    }
}

void ClientCharacterManager::SetViewPartyMember(unsigned int account) {
    if (m_dwViewCharAccount) {
        g_Map.SetViewChar(GetMyCharacterPtr());
        m_dwViewCharAccount = 0;
    } else {
        ClientCharacter* p = GetCharByAccount(account);
        if (p) {
            m_dwViewCharAccount = account;
            g_Map.SetViewChar(p);
        } else {
            m_dwViewCharAccount = 0;
        }
    }
}

void ClientCharacterManager::CreateCoupleEffect() {
    for (int i = kFirstSlot; i < kSlotCount; ++i) {
        ClientCharacter* a = SlotPtr(i);
        if (!SlotAlive(*a) || IsMonster(a->m_wKind)) continue;
        for (int j = kFirstSlot; j < kSlotCount; ++j) {
            if (i == j) continue;
            ClientCharacter* b = SlotPtr(j);
            if (!SlotAlive(*b) || IsMonster(b->m_wKind)) continue;
            // mofclient.c：用對方 m_szCoupleName 對照本角色 m_szName，找出婚姻配對。
        if (std::strcmp(b->m_szCoupleName, a->m_szName) == 0) {
                auto* info = g_clCoupleRingKindInfo.GetCoupleRingKindInfo(
                    a->m_wCoupleRingKind);
                if (info) a->CreateCoupleEffect(*((unsigned short*)info + 9), 1);
            }
        }
    }
}

void ClientCharacterManager::CreateCoupleEffect(ClientCharacter* a2, int a3) {
    // mofclient.c: 32 位元下 a2 直接被當成 account ID 使用；x64 下 ClientCharacter*
    // 是 64 位元指標，這裡先轉成 uintptr_t 再截成 32 位元 account 以對齊原意。
    ClientCharacter* a = GetCharByAccount(
        static_cast<unsigned int>(reinterpret_cast<uintptr_t>(a2)));
    if (!a) return;
    unsigned short ring = a->m_wCoupleRingKind;
    if (!ring || IsMonster(a->m_wKind)) return;
    for (int j = kFirstSlot; j < kSlotCount; ++j) {
        ClientCharacter* b = SlotPtr(j);
        if (!SlotAlive(*b) || IsMonster(b->m_wKind)) continue;
        // mofclient.c：用對方 m_szCoupleName 對照本角色 m_szName，找出婚姻配對。
        if (std::strcmp(b->m_szCoupleName, a->m_szName) == 0) {
            auto* info = g_clCoupleRingKindInfo.GetCoupleRingKindInfo(ring);
            if (info) {
                b->CreateCoupleEffect(*((unsigned short*)info + 9), a3);
                auto* info2 = g_clCoupleRingKindInfo.GetCoupleRingKindInfo(
                    a->m_wCoupleRingKind);
                if (info2) a->CreateCoupleEffect(*((unsigned short*)info2 + 9), a3);
            }
            return;
        }
    }
}

void ClientCharacterManager::DeleteCoupleEffect(unsigned int account) {
    ClientCharacter* a = GetCharByAccount(account);
    if (!a) return;
    if (!a->m_wCoupleRingKind) return;
    if (IsMonster(a->m_wKind)) return;
    for (int j = kFirstSlot; j < kSlotCount; ++j) {
        ClientCharacter* b = SlotPtr(j);
        if (!SlotAlive(*b) || IsMonster(b->m_wKind)) continue;
        // mofclient.c：用對方 m_szCoupleName 對照本角色 m_szName，找出婚姻配對。
        if (std::strcmp(b->m_szCoupleName, a->m_szName) == 0) {
            b->DeleteCoupleEffect();
            return;
        }
    }
}

void ClientCharacterManager::SetMoveTarget(int x, int y) {
    m_bMoveTargetActive = true;
    m_iMoveTargetX = x;
    m_iMoveTargetY = y;
    m_clAutoMoveTarget.InitNewTarget(x, y);
}
void ClientCharacterManager::ResetMoveTarget() {
    m_bMoveTargetActive = false;
    m_iMoveTargetX = 0;
    m_iMoveTargetY = 0;
    m_clAutoMoveTarget.SetActive(0);
}

// mofclient.c 0x00413240：上下馬。流程：
//   1) 不論結果先送 E0308 mount/dismount 特效
//   2) 若還沒有 transport object，或當前 kind 與請求不同：呼叫
//      ClientCharacter::SetTransportKind 換騎 → 自己的話 LABEL_11
//   3) kind 相同時依 GetActive() toggle：當前是 active → SetTransportActive(0)
//      下馬並 return；當前 inactive → SetTransportActive(1) 上馬，自己的話
//      跳到 LABEL_11
//   LABEL_11：SetAutoAttack(0) + SetMyRunState()，重置動作狀態。
void ClientCharacterManager::SetTransport(unsigned int account,
                                          unsigned short transportKind,
                                          unsigned char /*a4*/) {
    ClientCharacter* p = GetCharByAccount(account);
    if (!p) return;

    if (g_pEffectManager_After_Chr) {
        char eff[] = "E0308";
        g_pEffectManager_After_Chr->AddEffect(eff, p);
    }

    auto labelEnsureMyState = [this, p, account]() {
        if (account == m_dwMyAccount) {
            cltMyCharData::SetAutoAttack(&g_clMyCharData, 0);
            p->SetMyRunState();
        }
    };

    if (!p->m_pTransportObject) {
        p->SetTransportKind(transportKind);
        labelEnsureMyState();
        return;
    }

    if (p->m_pTransportObject->GetTransportKind() != transportKind) {
        p->SetTransportKind(transportKind);
        labelEnsureMyState();
        return;
    }

    if (p->m_pTransportObject->GetActive()) {
        p->SetTransportActive(0);   // 同 kind 下馬
        return;
    }
    p->SetTransportActive(1);       // 同 kind 重新上馬
    labelEnsureMyState();
}
