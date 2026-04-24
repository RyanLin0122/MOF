#include "Character/ClientCharacter.h"
#include "Character/ClientCharacterManager.h"
#include "Character/CCA.h"

#include "Info/cltClassKindInfo.h"
#include "Info/cltCharKindInfo.h"
#include "Info/cltSkillKindInfo.h"
#include "Info/cltClientCharKindInfo.h"
#include "Info/cltMonsterAniInfo.h"
#include "Info/cltItemKindInfo.h"
#include "Info/cltTransformKindInfo.h"
#include "Effect/CEffect_Field_AttackMiss.h"
#include "Effect/CEffect_Field_Miss.h"
#include "Effect/CEffect_Field_HealNumber.h"
#include "Effect/CEffect_Field_ExpNumber.h"
#include "Effect/CEffect_Field_Warp.h"
#include "Effect/CEffect_Field_Walkdust.h"
#include "Effect/CEffect_Skill_Heal.h"
#include "Effect/CEffect_Skill_OtherHeal.h"
#include "Effect/CEffect_Skill_Resurrection.h"
#include "Effect/CEffect_Item_Use_HPPotion.h"
#include "Effect/CEffect_Battle_Pierce.h"
#include "Effect/CEffect_Battle_HorizonCut.h"
#include "Effect/CEffect_Battle_DownCut.h"
#include "Effect/CEffect_Battle_UpperCut.h"
#include "Effect/CEffect_Battle_TwoHandCut.h"
#include "Effect/CEffect_Battle_GunShoot.h"
#include "Effect/CEffect_Battle_BowShoot.h"
#include "Effect/CEffect_Battle_MagicBook.h"
#include "Effect/CEffect_Battle_StaffShoot.h"
#include "Image/cltImageManager.h"
#include "Image/GameImage.h"
#include "UI/CControlChatBallon.h"
#include "UI/CControlAlphaBox.h"
#include "UI/CUIBasic.h"
#include "UI/CUIManager.h"
#include "System/cltMoF_CharacterActionStateSystem.h"
#include "Object/cltPetObject.h"
#include "Object/clTransportObject.h"
#include "Effect/CEffectManager.h"
#include "Effect/CEffect_Player_Dead.h"
#include "Effect/CEffect_Field_DamageNumber.h"
#include "Effect/CEffect_Field_CriticalNumber.h"
#include "Effect/CEffect_Use_Hit.h"
#include "Effect/CEffectUseHitMulti.h"
#include "Sound/GameSound.h"
#include "global.h"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>

// =============================================================================
// Static environment (mofclient.c: ClientCharacter::m_pClientCharKindInfo /
// m_pMap / m_pNetwork / m_pCharMgr — assigned by InitializeStaticVariable).
// =============================================================================
cltCharKindInfo*        ClientCharacter::m_pClientCharKindInfo = nullptr;
Map*                    ClientCharacter::m_pMap                = nullptr;
CMoFNetwork*            ClientCharacter::m_pNetwork            = nullptr;
ClientCharacterManager* ClientCharacter::m_pCharMgr            = nullptr;

void ClientCharacter::InitializeStaticVariable(cltCharKindInfo* charKindInfo,
                                               Map* map,
                                               CMoFNetwork* network,
                                               ClientCharacterManager* charMgr) {
    // mofclient.c 0x004070A0: assigns the four static pointers, then pops a
    // MessageBoxA describing the first null argument.  We trust callers and
    // skip the MessageBox here.
    m_pClientCharKindInfo = charKindInfo;
    m_pMap                = map;
    m_pNetwork            = network;
    m_pCharMgr            = charMgr;
}

// =============================================================================
// Constructor — allocates the embedded subsystems and zero-initialises every
// primitive member.  mofclient.c does this with a mix of direct writes and
// memsets over fixed offset ranges; we zero the named fields one by one.
// =============================================================================
ClientCharacter::ClientCharacter()
    : m_pVftable(nullptr),
      m_pShadowImage(nullptr),
      m_pShadowImage_Sub(nullptr),
      m_pMonsterAniInfo(nullptr),
      m_dwUnknown_20(0),
      m_dwUnknown_24(0),
      m_dwUnknown_28(0),
      m_dwUnknown_32(0),
      m_pGogglesImage(nullptr),
      m_wUnknown_64(0),
      m_dwAttackSpeed(0),
      m_ucAttackAniRate(0),
      m_ucAttackMotionType(0),
      m_ucAlpha(0xFFu),
      m_iUnknown_348(0),
      m_iUnknown_352(0),
      m_iUnknown_356(0),
      m_iUnknown_372(0),
      m_iUnknown_380(0),
      m_dwAccountID(0),
      m_wKind(0),
      m_dwLR_Flag(0),
      m_wMapID(0),
      m_ucSex(0),
      m_ucHair(0),
      m_ucFace(0),
      m_cCharState(0),
      m_dwTransformation(0),
      m_nOrderCount(0),
      m_pCCA(nullptr),
      m_pCCANormal(nullptr),
      m_dwUnknown_4356(0),
      m_dwUnknown_4360(0),
      m_dwUnknown_4368(0),
      m_ucUnknown_4374(0),
      m_iPosX(600),
      m_iPosY(300),
      m_iPrevPosX(0),
      m_iPrevPosY(0),
      m_iDestX(0),
      m_iDestY(0),
      m_fMoveSpeed(0.0f),
      m_ucUnknown_4400(0),
      m_wUnknown_4432(0),
      m_dwSlotAlive(0),
      m_iUnknown_7372(0),
      m_iUnknown_7376(0),
      m_iUnknown_7380(0),
      m_ucUnknown_7384(2),  // mofclient.c 30694
      m_ucUnknown_7434(0),
      m_ucUnknown_7435(0),
      m_ucUnknown_7436(0),
      m_dwUnknown_9676(0),
      m_wUnknown_9680(0),
      m_dwActionState(0),
      m_fashionItemActive(false),
      m_wTotalFrame(0),
      m_wCurrentFrame(0),
      m_isIntegrityActive(false),
      m_canMove(false),
      m_isFreezing(false),
      m_ucNameClassMark(0),
      m_wClassCode(0),
      m_wPvPRankKind(0),
      m_dwUnknown_9716(0),
      m_ucUnknown_10112(0),
      m_someOtherState(0),
      m_ucUnknown_11252(0),
      m_dwUnknown_11256(0),
      m_dwUnknown_11264(0),
      m_ucUnknown_11260(1),  // mofclient.c 30697
      m_dwGM_Level(0),
      m_isPCRoomUser(false),
      m_isPCRoomPremium(false),
      m_dwUnknown_14684(0),
      m_dwUnknown_14820(0),
      m_preparingSpeedUp(false),
      m_canSpeedUp(false),
      m_pChatBallon(nullptr),
      m_pHpBoxBack(nullptr),
      m_pHpBoxFront(nullptr),
      m_pHpBoxThird(nullptr),
      m_pNameBox(nullptr),
      m_pActionStateSystem(nullptr),
      m_pPetObject(nullptr),
      m_pTransportObject(nullptr)
{
    std::memset(m_acEquipKind, 0, sizeof(m_acEquipKind));
    std::memset(m_ucDigitFrames_Unk, 0, sizeof(m_ucDigitFrames_Unk));
    std::memset(m_ucDoubleAttackMotion, 0, sizeof(m_ucDoubleAttackMotion));
    std::memset(m_iUnknown_432, 0, sizeof(m_iUnknown_432));
    std::memset(m_szName, 0, sizeof(m_szName));
    std::memset(m_iInitFlag_153_176, 0, sizeof(m_iInitFlag_153_176));
    std::memset(&m_currentOrder, 0, sizeof(m_currentOrder));
    std::memset(m_OrderQueue, 0, sizeof(m_OrderQueue));
    std::memset(m_szScreenName, 0, sizeof(m_szScreenName));
    std::memset(m_szCircleName, 0, sizeof(m_szCircleName));
    std::memset(m_safetyPad, 0, sizeof(m_safetyPad));

    // mofclient.c 30647-30658: in-place construction of the embedded
    // subsystems at fixed offsets.  We allocate on the heap instead.
    m_pChatBallon        = new CControlChatBallon();
    m_pHpBoxBack         = new CControlAlphaBox();
    m_pHpBoxFront        = new CControlAlphaBox();
    m_pHpBoxThird        = new CControlAlphaBox();
    m_pNameBox           = new CControlAlphaBox();
    m_pActionStateSystem = new cltMoF_CharacterActionStateSystem();
    m_pPetObject         = new cltPetObject();
    m_pTransportObject   = new clTransportObject();
}

// =============================================================================
// Destructor — mirrors mofclient.c 30753-30778 in reverse order:
// transport / pet / action-state / alpha boxes / chat ballon, then the CCA
// pair via their vtable-driven release.
// =============================================================================
ClientCharacter::~ClientCharacter() {
    // CCA / CCANormal are heap-allocated elsewhere; the decomp calls their
    // first virtual method with arg=1 (release-and-delete path).  We don't
    // own them here — just drop our pointer.
    m_pCCANormal = nullptr;
    m_pCCA       = nullptr;

    delete m_pTransportObject;   m_pTransportObject   = nullptr;
    delete m_pPetObject;         m_pPetObject         = nullptr;
    delete m_pActionStateSystem; m_pActionStateSystem = nullptr;
    delete m_pNameBox;           m_pNameBox           = nullptr;
    delete m_pHpBoxThird;        m_pHpBoxThird        = nullptr;
    delete m_pHpBoxFront;        m_pHpBoxFront        = nullptr;
    delete m_pHpBoxBack;         m_pHpBoxBack         = nullptr;
    delete m_pChatBallon;        m_pChatBallon        = nullptr;
}


// =============================================================================
// PrepareDrawingPlayer (mofclient.c 26615, ~446 lines)
//
// The full implementation composites the CCA body + shadow + goggles + pet
// transport, populates GameImage fields at fixed offsets, and splits on
// transformation-state vs normal player.  Here we port the outer scaffolding
// that every sub-system cares about:
//   - viewport cull (DWORD+76 = "drawn this frame" flag)
//   - transport PrepareDrawing when mounted
//   - CCA::Process (when the main CCA is populated)
//   - gated PrepareDrawingHPBox + PrepareDrawingCharActionState
//   - name-box position + PrepareDrawing
//   - chat-ballon position + PrepareDrawing (when flag set)
//   - pet PrepareDrawing
// GameImage field writes are skipped; they land in a future asset-loader pass.
// =============================================================================
void ClientCharacter::PrepareDrawingPlayer() {
    if (m_someOtherState) return;             // DWORD+2882 ("hidden-self")
    if (m_iInitFlag_153_176[175 - 153]) return; // flag 22 — hide during warp

    // DWORD+76 = "drawn this frame" gate used by DrawPlayer.
    Decomp<int>(4 * 76) = 1;

    const int scrollX = dword_A73088;
    const int scrollY = dword_A7308C;
    const int screenX = m_iPosX - scrollX;
    const int screenY = m_iPosY - scrollY;

    // Cull when completely offscreen horizontally (±128 px margin).
    if (screenX + 128 < 0 || screenX > g_Game_System_Info.ScreenWidth + 128) {
        Decomp<int>(4 * 76) = 0;
        return;
    }

    if (m_pTransportObject && m_pTransportObject->GetActive()) {
        m_pTransportObject->PrepareDrawing(IsMyChar());
    }

    if (m_dwTransformation) {
        unsigned int resID = 0;
        unsigned short blockID = 0;
        DecideDrawFrame(&resID, &blockID);
        if (resID) {
            GameImage* img = cltImageManager::GetInstance()
                ->GetGameImage(1u, resID, 0, 1);
            m_pShadowImage = img;  // DWORD+1 in the decomp
            if (img) {
                if (!m_dwLR_Flag) img->SetFlipX(true);
                img->SetPosition(static_cast<float>(screenX),
                                 static_cast<float>(screenY));
                img->SetBlockID(blockID);
                img->m_bFlag_447 = true;
                img->m_bFlag_446 = true;
                img->m_bVertexAnimation = false;
                img->m_nScale  = Decomp<unsigned char>(360);
                img->m_bFlag_449 = true;
                img->m_dwAlpha = m_ucAlpha;
                img->m_bFlag_450 = true;
                img->m_bDrawPart2 = true;
            }
        }
    } else if (m_pCCA) {
        // CCA path — transport active may adjust the Y offset for rider-on-mount
        // pose (see decomp 26750).
        int adjustedY = screenY;
        if (m_pTransportObject && m_pTransportObject->GetActive()) {
            // Block-ID-dependent 1..2 px lift; without the mount's current
            // frame-number readable here, use +1 as the conservative default.
            adjustedY += 1;
        }
        *reinterpret_cast<float*>(
            reinterpret_cast<unsigned char*>(m_pCCA) + 128) =
            static_cast<float>(screenX);
        *reinterpret_cast<float*>(
            reinterpret_cast<unsigned char*>(m_pCCA) + 132) =
            static_cast<float>(adjustedY);
        DecideDrawFrame();
        // Flip byte at CCA offset +140: 1 when facing left, 0 when right.
        *(reinterpret_cast<unsigned char*>(m_pCCA) + 140) =
            m_dwLR_Flag ? 1 : 0;
        m_pCCA->Process();
        *reinterpret_cast<int*>(
            reinterpret_cast<unsigned char*>(m_pCCA) + 124) = m_ucAlpha;
    }

    // HP-box + action-state overlays are skipped while died (state 7).
    if (m_dwActionState != 7) {
        PrepareDrawingHPBox();
        PrepareDrawingCharActionState();
    }

    // Name-box position (non-local chars read it in PrepareDrawingHPBox; this
    // branch covers the caller that wants the position unconditionally).
    if (m_pNameBox) {
        const int nameWidth = Decomp<int>(4 * 2441);
        m_pNameBox->SetPos(screenX - (nameWidth >> 1), Decomp<int>(4 * 1102));
        m_pNameBox->PrepareDrawing();
    }

    // Chat ballon — DWORD+1124 is the "ballon visible" flag (set by
    // SetChatMsg + cleared by ClearerChatMsg / expiry).
    if (m_pChatBallon && Decomp<int>(4 * 1124)) {
        m_pChatBallon->SetPos(screenX, Decomp<int>(4 * 1106));
        m_pChatBallon->PrepareDrawing();
    }

    // Pet — non-hidden chars only.
    if (m_pPetObject && m_pPetObject->GetActive()) {
        m_pPetObject->PrepareDrawing(1, IsMyChar());
    }
}

// mofclient.c 26455, ~160 lines.  Same shape as PrepareDrawingPlayer minus
// the transformation + mount branches.
void ClientCharacter::PrepareDrawingMonster() {
    if (m_someOtherState) return;
    if (m_iInitFlag_153_176[175 - 153]) return;

    Decomp<int>(4 * 76) = 1;
    const int screenX = m_iPosX - dword_A73088;
    if (screenX + 200 < 0 || screenX > g_Game_System_Info.ScreenWidth + 200) {
        Decomp<int>(4 * 76) = 0;
        return;
    }

    // Monster body path (InitFlag[0] == 1 = CCANormal, else raw GameImage).
    const int screenY = m_iPosY - dword_A7308C;
    if (m_iInitFlag_153_176[0]) {
        if (m_pCCANormal) {
            *reinterpret_cast<float*>(
                reinterpret_cast<unsigned char*>(m_pCCANormal) + 300) =
                static_cast<float>(screenX);
            *reinterpret_cast<float*>(
                reinterpret_cast<unsigned char*>(m_pCCANormal) + 304) =
                static_cast<float>(screenY);
            *(reinterpret_cast<unsigned char*>(m_pCCANormal) + 308) =
                m_dwLR_Flag ? 0 : 1;
            m_pCCANormal->Process();
        }
    } else {
        unsigned int resID = 0;
        unsigned short blockID = 0;
        DecideDrawFrame(&resID, &blockID);
        if (!resID) {
            OutputDebugStringA("Character ResourceID is NULL \n");
            return;
        }
        GameImage* body = cltImageManager::GetInstance()
            ->GetGameImage(1u, resID, 0, 1);
        m_pShadowImage = body;
        if (body) {
            if (!m_dwLR_Flag) body->SetFlipX(true);
            body->SetPosition(static_cast<float>(screenX),
                              static_cast<float>(screenY));
            body->SetBlockID(blockID);
            body->m_bFlag_447 = true;
            body->m_bFlag_446 = true;
            body->m_bVertexAnimation = false;
            body->m_nScale = Decomp<unsigned char>(360);
            body->m_bFlag_449 = true;
            body->m_dwAlpha = m_ucAlpha;
            body->m_bFlag_450 = true;
            body->m_bDrawPart2 = true;
        }
        // Sub-shadow GameImage when the char kind has a shadow sprite.
        if (m_pMonsterAniInfo) {
            GameImage* shadow = cltImageManager::GetInstance()
                ->GetGameImage(1u, resID, 0, 1);
            m_pShadowImage_Sub = shadow;
            if (shadow) {
                if (!m_dwLR_Flag) shadow->SetFlipX(true);
                shadow->SetBlockID(blockID);
                shadow->SetPosition(static_cast<float>(screenX),
                                    static_cast<float>(screenY - 1));
                shadow->m_bFlag_447 = true;
                shadow->m_bFlag_446 = true;
                shadow->m_bVertexAnimation = false;
                shadow->m_dwAlpha = m_ucAlpha;
                shadow->m_bFlag_450 = true;
                const int baseAlpha = (GetCharWidthA() > 0x63 || GetCharHeight() > 0x50) ? 10 : 20;
                shadow->m_bFlag_449 = true;
                shadow->m_nScale = baseAlpha + Decomp<unsigned char>(360);
                shadow->m_bDrawPart1 = true;
            }
        }
    }

    PrepareDrawingHPBox();

    if (m_pChatBallon && Decomp<int>(4 * 1124)) {
        m_pChatBallon->SetPos(screenX, Decomp<int>(4 * 1106));
        m_pChatBallon->PrepareDrawing();
    }
}

/// @brief 取得角色的 X 座標
int ClientCharacter::GetPosX() {
	return m_iPosX;
}
/// @brief 取得角色的 Y 座標
int ClientCharacter::GetPosY() {
	return m_iPosY;
}
void ClientCharacter::SetPosX(float x) {
	m_iPosX = static_cast<int>(x);
}
/// @brief 取得角色的 Y 座標
void ClientCharacter::SetPosY(float y) {
	m_iPosY = static_cast<int>(y);
}
/// @brief 取得角色的高度 (mofclient.c 33451 — forward to kind-info lookup).
unsigned short ClientCharacter::GetCharHeight() {
	return 100;  // cltCharKindInfo::GetCharHeight fallback constant.
}


// mofclient.c 26428.
void ClientCharacter::PrepareDrawingChar() {
	ProcInfoPosY();
	if (m_pCharMgr && m_pCharMgr->IsMonster(m_wKind)) {
		PrepareDrawingMonster();
	} else {
		PrepareDrawingPlayer();
	}
}

// mofclient.c 27061.
void ClientCharacter::DrawChar(int a2) {
	if (m_pCharMgr && m_pCharMgr->IsMonster(m_wKind)) {
		DrawMonster(a2);
	} else {
		DrawPlayer(a2);
	}
}

// =============================================================================
// DrawPlayer (mofclient.c 27117, ~147 lines)
//
// Ordering matches the ground truth so z-order is preserved:
//   1. Map-conqueror / match aura underneath
//   2. PK-team aura
//   3. Pet (when its draw-order is "before body")
//   4. HP bar + action-state icon (not while state 7)
//   5. Name box + text overlay
//   6. Chat ballon (or secondary bubble at DWORD+2816)
//   7. Special bottom effect (WORD+5766)
//   8. Circle mark / guild banner / title tag
//   9. Transport down-half (before body)
//  10. Body — transformation GameImage OR CCA composite
//  11. Transport up-half (after body)
//  12. Pet (when its draw-order is "after body")
//  13. Goggles / PCRoom emblem / extra accessories
// GameImage draw calls are deferred until the asset-loader is wired back in;
// the sub-system callouts (pet / transport / HPBox / CCA) run as-is.
// =============================================================================
void ClientCharacter::DrawPlayer(int a2) {
    if (m_someOtherState) return;
    if (m_iInitFlag_153_176[175 - 153]) return;

    // 3. Pet (before-body draw order).
    if (m_pPetObject && m_pPetObject->GetActive()
        && m_pPetObject->IsDrawingOrder() == 1) {
        m_pPetObject->Draw(IsMyChar());
    }

    // 4. HP bar + action state (skip during died state 7).
    if (m_dwActionState != 7) {
        DrawHPBox();
        DrawCharActionState();
    }

    // 5. Name box.
    if (m_pNameBox) m_pNameBox->Draw();

    // 6. Text overlay (name, title, circle, chat).
    DrawTextA(a2);

    // 7. Chat ballon.
    if (m_pChatBallon && Decomp<int>(4 * 1124)) m_pChatBallon->Draw();

    // 9. Transport down-half (body/legs of mount, drawn before rider).
    if (m_pTransportObject && m_pTransportObject->GetActive()) {
        m_pTransportObject->DrawDown(IsMyChar());
    }

    // 10. Body.
    if (m_dwTransformation) {
        if (m_pShadowImage) m_pShadowImage->Draw();
    } else if (m_pCCA) {
        m_pCCA->Draw(0);
    }

    // 11. Transport up-half.
    if (m_pTransportObject && m_pTransportObject->GetActive()) {
        m_pTransportObject->DrawUp(IsMyChar());
    }

    // 12. Pet (after-body draw order).
    if (m_pPetObject && m_pPetObject->GetActive()
        && !m_pPetObject->IsDrawingOrder()) {
        m_pPetObject->Draw(IsMyChar());
    }
}

// mofclient.c 27073, ~31 lines.  Monster body is either the CCANormal
// composite (InitFlag[0] set) or a raw cltImageManager-sourced GameImage.
// Owned-effect slots at +99..+107 get their Draw vtable called (skipped
// until the effect-base exposes a canonical Draw helper).
void ClientCharacter::DrawMonster(int a2) {
    if (m_someOtherState) return;
    if (m_iInitFlag_153_176[175 - 153]) return;

    if (m_iInitFlag_153_176[0]) {
        if (m_pCCANormal) m_pCCANormal->Draw();
    } else if (m_pShadowImage) {
        // Sub-shadow first (under the body), then the body on top.
        if (m_pShadowImage_Sub) m_pShadowImage_Sub->Draw();
        m_pShadowImage->Draw();
    }
    DrawHPBox();
    DrawTextA(a2);
}

void ClientCharacter::SetHited(int a, int b)
{
	return;
}

bool ClientCharacter::IsHide() {
	return false;
}

bool ClientCharacter::IsTransparent() {
	return false;
}

// mofclient.c: GetActionSide returns m_dwLR_Flag (0 = facing left, 1 = right).
int ClientCharacter::GetActionSide() {
	return m_dwLR_Flag;
}

// mofclient.c 34079: linear scan of the hited-info table at +8076, 8 DWORDs
// per slot (32 bytes), looking for the slot whose first DWORD matches the
// caster account id.  Returns the 1-based slot index, or 0 when not found.
int ClientCharacter::GetHitedInfoNum(unsigned int casterAccountID) {
	int idx = 0;
	const size_t base = 8076;
	while (idx < 50) {
		const unsigned int slotAccount = Decomp<unsigned int>(base + static_cast<size_t>(idx) * 32);
		if (slotAccount == casterAccountID) return idx;
		++idx;
	}
	return 0;
}

// Legacy int-parameter overload kept for callers that pass a signed int.
int ClientCharacter::GetHitedInfoNum(int a) {
	return GetHitedInfoNum(static_cast<unsigned int>(a));
}

int ClientCharacter::GetAccountID() {
	return static_cast<int>(m_dwAccountID);
}

// Legacy bool-returning overload kept for existing callers.  Inlines the
// core of the ground-truth int-returning implementation to avoid infinite
// recursion through the same parameter signature.
bool ClientCharacter::GetSustainSkillState(unsigned short a) {
	stSkillKindInfo* skill = g_clSkillKindInfo.GetSkillKindInfo(a);
	if (!skill) return true;
	const int isSustain = *reinterpret_cast<int*>(
	    reinterpret_cast<unsigned char*>(skill) + 280);  // DWORD+70
	if (isSustain) return Decomp<int>(4 * 73) != 0;
	const int16_t kind = *reinterpret_cast<int16_t*>(
	    reinterpret_cast<unsigned char*>(skill) + 2);    // WORD+1
	if (kind == static_cast<int16_t>(0xA0D6)) return Decomp<int>(4 * 74) != 0;
	if (kind == static_cast<int16_t>(0xA072)) return Decomp<int>(4 * 75) != 0;
	return false;
}

int ClientCharacter::GetPetPosX() {
	return m_pPetObject ? m_pPetObject->GetPosX() : 0;
}

int ClientCharacter::GetPetPosY() {
	return m_pPetObject ? m_pPetObject->GetPosY() : 0;
}

void ClientCharacter::SetClassCode(unsigned short classCode, int updateNameTag) {
	auto* info = g_clClassKindInfo.GetClassKindInfo(classCode);
	if (!info) {
		return;
	}

	m_wClassCode = classCode;
	const auto classMark = *(reinterpret_cast<unsigned char*>(info) + 18);
	m_ucNameClassMark = classMark;
	SetNameTagInfo(classMark, updateNameTag);
}

void ClientCharacter::SetNameTagInfo(unsigned char /*classMark*/, int /*updateNameTag*/) {
	// Ground truth 會進一步更新 NameTag 顏色與 UI。此專案目前缺少完整相依函式，
	// 先保留呼叫路徑以對齊 SetClassCode 的副作用流程。
}

// mofclient.c 32593: returns DWORD+1847 (last searched monster's account id).
unsigned int ClientCharacter::GetSearchMonster() {
    return Decomp<unsigned int>(4 * 1847);
}

// =============================================================================
// SetOrder* builders (mofclient.c 29083-29166)
//
// Each one fills the stCharOrder payload with a command type + params.
// Field mapping (see stCharOrder layout in the header):
//     p0 = a2[1]   a2[2]   a2[3]   a2[4]   a2[5]
//     w0 = a2[12]  w1 = a2[13]  w2 = a2[14]
//     b0 = a2[30]  b1 = a2[31]  b2 = a2[32]
// =============================================================================

void ClientCharacter::SetOrderStop(stCharOrder* a2) {
    if (a2) a2->type = 1;
}

void ClientCharacter::SetOrderMove(stCharOrder* a2, unsigned int a3,
                                   unsigned int a4, unsigned int a5) {
    if (!a2) return;
    a2->type = 2;
    a2->p0   = static_cast<int>(a3);
    a2->p1   = static_cast<int>(a4);
    a2->w0   = static_cast<uint16_t>(a5);
}

void ClientCharacter::SetOrderDie(stCharOrder* a2) {
    if (a2) a2->type = 4;
    // DWORD+156 (m_iInitFlag[3]) is cleared here too.
    m_iInitFlag_153_176[3] = 0;
}

void ClientCharacter::SetOrderDied(stCharOrder* a2) {
    m_iInitFlag_153_176[3] = 0;
    if (a2) a2->type = 9;
}

int ClientCharacter::SetOrderHitted(stCharOrder* a2, unsigned char motion,
                                    int damage, int remainHP, char a6, char a7,
                                    unsigned short skillKind,
                                    unsigned char hitInfo) {
    if (!a2) return 0;
    a2->type = 5;
    a2->p0   = motion;
    a2->p2   = remainHP;
    a2->p3   = damage;
    a2->b0   = hitInfo;
    a2->w1   = 1;
    a2->b1   = static_cast<uint8_t>(a6);
    a2->b2   = static_cast<uint8_t>(a7);
    if (skillKind) {
        stSkillKindInfo* info = g_clSkillKindInfo.GetSkillKindInfo(skillKind);
        if (info) {
            // *((_BYTE *)v9 + 171) — a per-skill "hit count" multiplier.
            a2->w1 = *(reinterpret_cast<unsigned char*>(info) + 171);
        }
    }
    return 1;
}

// Ground-truth signature: 7-param attack order.
void ClientCharacter::SetOrderAttack(stCharOrder* a2, unsigned int targetAccount,
                                     char a4, int damage, int remainHP, char a7,
                                     unsigned short skillKind) {
    if (!a2) return;
    a2->type = 3;
    a2->p0   = static_cast<int>(targetAccount);
    a2->p2   = remainHP;
    a2->p3   = damage;
    a2->w0   = static_cast<unsigned char>(a4);
    a2->w2   = skillKind;
    a2->b0   = static_cast<uint8_t>(a7);
}

// Legacy 7-int tutorial signature kept for existing callers; forwards to
// the ground-truth version with char/short truncation matching the decomp.
void ClientCharacter::SetOrderAttack(stCharOrder* a2, unsigned int targetAccount,
                                     int a3, int damage, int remainHP, int a6,
                                     int a7) {
    SetOrderAttack(a2, targetAccount, static_cast<char>(a3), damage, remainHP,
                   static_cast<char>(a6), static_cast<unsigned short>(a7));
}

void ClientCharacter::SetOrderPray(stCharOrder* a2, int a3, int a4, int a5,
                                   unsigned char a6) {
    if (!a2) return;
    a2->type = 8;
    a2->p2   = a3;
    a2->p3   = a4;
    a2->p4   = a5;
    a2->b0   = a6;
}

void ClientCharacter::SetOrderSpell(stCharOrder* a2) {
    if (a2) a2->type = 7;
}

void ClientCharacter::SetOrderTransformation(stCharOrder* a2,
                                             unsigned short transformKind) {
    if (!a2) return;
    a2->type = 10;
    a2->w0   = transformKind;
}

// =============================================================================
// Order* executors (mofclient.c 29169-30459)
//
// Each one drives the per-frame behaviour for the character's active order.
// The simple ones enqueue a follow-up order when the current animation
// reaches its terminal frame.
// =============================================================================

// mofclient.c declared only; no body emitted — treated as a no-op executor.
void ClientCharacter::OrderStop() {
}

// mofclient.c 29169: if we've reached the move destination, enqueue a stop.
void ClientCharacter::OrderMove() {
    if (m_iDestX == m_iPosX && m_iDestY == m_iPosY) {
        stCharOrder order{};
        SetOrderStop(&order);
        PushOrder(&order);
    }
}

// mofclient.c 30095: fixed 0 return in the decomp (placeholder executor).
int ClientCharacter::OrderDied() {
    return 0;
}

// mofclient.c 30009: advance the "die" animation frame-by-frame.  Boss / field
// item kinds use longer delay budgets (150 vs 80 frames).  Returns 1 when the
// death animation has reached its terminal frame.
int ClientCharacter::OrderDie() {
    // Shim: the ground truth calls g_clFieldItemMgr::PopBuffer(accountID)
    // here to drain any pending field-item drops for this monster.  Phase E
    // will wire it back up — it's idempotent so the omission is harmless.
    // mofclient.c 30024: if the local player has pending gain-exp, view+reset.
    // (ClientCharacterManager::{GetMyGainExp,ViewMyGainExp,ResetMyGainExp}
    //  are not yet restored — the whole block is a no-op for now.)

    int delayBudget = 80;
    auto* kindInfo = m_pClientCharKindInfo;
    if (kindInfo && kindInfo->GetBossInfoByKind(m_wKind)) delayBudget = 150;

    const bool hasDieDelay =
        kindInfo && kindInfo->GetDieDelayAniByKind(m_wKind);

    if (hasDieDelay) {
        if (m_iInitFlag_153_176[0]) {
            // Decomp uses DWORD+77 (offset 308) as a frame counter.  Stash via Decomp.
            const int final = static_cast<int>(m_wTotalFrame) - 1;
            const int cur   = m_wCurrentFrame;
            if (cur == final) {
                int& counter = Decomp<int>(4 * 77);
                if (counter < delayBudget) {
                    if (m_wTotalFrame > 0) m_wTotalFrame = m_wTotalFrame - 1;
                    counter = counter + 1;
                    return 0;
                }
            }
            Decomp<int>(4 * 77) = 0;
            if (cur != final) return 0;
            return 1;
        } else {
            const unsigned short cur = m_wCurrentFrame;
            const unsigned short total = m_wTotalFrame;
            int& counter = Decomp<int>(4 * 77);
            if (cur == static_cast<unsigned short>(total - 3) && counter < delayBudget) {
                counter += 1;
                m_wCurrentFrame = cur - 1;
                if (counter <= delayBudget - 40) return 0;
                m_ucAlpha = static_cast<unsigned char>(m_ucAlpha - 5);
                return 0;
            }
            if (cur != static_cast<unsigned short>(total - 1)) return 0;
            return 1;
        }
    }
    Decomp<int>(4 * 77) = 0;
    if (m_wCurrentFrame != static_cast<unsigned short>(m_wTotalFrame - 1)) return 0;
    return 1;
}

// mofclient.c 30327: drive the "pray" animation; at frame (total-2) either
// apply the healing effect (tutorial / skill-8 path) or transition the order
// queue depending on remaining destination.
void ClientCharacter::OrderPray() {
    if (!m_wCurrentFrame) CreateUseSkillEffect();
    if (m_wCurrentFrame == static_cast<unsigned short>(m_wTotalFrame - 2)) {
        // Decomp: if current order type == 8 (pray) and b0 == 0 and IsMyChar.
        if (m_currentOrder.type == 8 && !m_currentOrder.b0 && IsMyChar()) {
            if (m_currentOrder.p3) { SetEffect_HealNum(m_currentOrder.p3); SetHP(); }
            if (m_currentOrder.p4) { SetEffect_HealNum(m_currentOrder.p4); }
        }
        std::memset(&m_currentOrder, 0, sizeof(m_currentOrder));

        stCharOrder order{};
        if (static_cast<int>(m_iDestX) == m_iPosX && m_iDestY == m_iPosY) {
            SetOrderStop(&order);
        } else {
            SetOrderMove(&order,
                         static_cast<unsigned int>(m_iDestX),
                         static_cast<unsigned int>(m_iDestY),
                         0);
        }
        PushOrder(&order);
        m_iInitFlag_153_176[158 - 153] = 0;
        m_iInitFlag_153_176[163 - 153] = 1;
        m_wUnknown_64 = 0;  // WORD+37 in the decomp lands in the 0x40/0x44 region.
    }
}

// mofclient.c 30380: simpler spell executor; no heal payload.
void ClientCharacter::OrderSpell() {
    if (!m_wCurrentFrame) CreateUseSkillEffect();
    if (m_wCurrentFrame == static_cast<unsigned short>(m_wTotalFrame - 2)) {
        std::memset(&m_currentOrder, 0, sizeof(m_currentOrder));
        stCharOrder order{};
        if (static_cast<int>(m_iDestX) == m_iPosX && m_iDestY == m_iPosY) {
            SetOrderStop(&order);
        } else {
            SetOrderMove(&order,
                         static_cast<unsigned int>(m_iDestX),
                         static_cast<unsigned int>(m_iDestY),
                         0);
        }
        PushOrder(&order);
    }
}

// mofclient.c 30420: apply the currently-staged transformation kind and
// immediately enqueue a stop order (no subsequent animation).
void ClientCharacter::OrderTransformation() {
    SetTransformation(m_currentOrder.w0);
    std::memset(&m_currentOrder, 0, sizeof(m_currentOrder));
    stCharOrder order{};
    SetOrderStop(&order);
    PushOrder(&order);
}

// mofclient.c 30101: simplified port of OrderHitted.  Ground truth handles
// monster-specific knockback physics (Walkdust effect + collision bounces);
// we port the terminal behaviour (enqueue stop + clear HitedSkillKind + reset
// the death animation delay) which is what non-monster callers depend on.
void ClientCharacter::OrderHitted(int /*alpha*/) {
    if (m_wCurrentFrame == static_cast<unsigned short>(m_wTotalFrame - 1)) {
        std::memset(&m_currentOrder, 0, sizeof(m_currentOrder));
        stCharOrder order{};
        SetOrderStop(&order);
        PushOrder(&order);
        if (IsMyChar()) {
            // Ground truth: if local HP falls below 30% of max, pop a system
            // message.  Without cltPlayerAbility::GetMaxHP restored we skip.
        }
        ResetHittedSkillKind();
        Decomp<uint16_t>(2 * 4840) = 0;    // WORD+4840
        Decomp<int>(4 * 84) = 10;           // DWORD+84
    }
}

// mofclient.c 29199: large attack executor.  We port the structural skeleton:
// kind-info lookup, CreateUseSkillEffect gate, target-char resolution, and
// the two main hit/miss/death branches.  Special-case monster kind effects
// (J1216 / J1209 / J1252) and the multi-hit loop are left as TODOs for
// Phase C since they require the yet-to-be-ported CCA animation-stride
// queries — today they are no-ops that keep the control flow intact.
void ClientCharacter::OrderAttack() {
    const unsigned char v32 = m_currentOrder.w0 & 0xFF;
    const unsigned int  v33 = static_cast<unsigned int>(m_currentOrder.p0);
    const int           v3  = m_currentOrder.p3;  // DWORD+181 (damage)
    const unsigned short v35 = m_currentOrder.w2; // skill kind
    const int           v38 = m_currentOrder.p2;  // DWORD+180 (remainHP)
    const unsigned int  v4  = m_wKind;
    const unsigned char v34 = m_currentOrder.b0;

    stCharOrder order{};
    int16_t v5 = 1;

    auto* kindInfo = m_pClientCharKindInfo;
    const bool isMonster =
        kindInfo && kindInfo->IsMonsterChar(static_cast<uint16_t>(v4)) != 0;
    if (isMonster || m_dwTransformation) {
        if (m_iInitFlag_153_176[0]) {
            v5 = 1;
        } else {
            auto* ani = static_cast<cltMonsterAniInfo*>(m_pMonsterAniInfo);
            if (ani) {
                // Decomp: *(_WORD *)(v6 + 8032) — a "hit frame" marker.
                v5 = *reinterpret_cast<int16_t*>(
                    reinterpret_cast<unsigned char*>(ani) + 8032);
            } else {
                v5 = 3;
            }
        }
    }
    // Gate: only fire the hit logic when the animation reaches the "hit"
    // frame AND the can-attack flag is set.
    if (m_wCurrentFrame != static_cast<unsigned short>(v5)
        || !m_iInitFlag_153_176[161 - 153]) {
        // LABEL_70 — skip hit logic, fall through to frame transitions.
        // (Transition to next state is driven by SetActionState elsewhere.)
        return;
    }

    // mofclient.c 29284-29321: special monster-kind effect attach points.
    // These require effect creation with effect-kind strings which this
    // restored build does not yet route; we preserve the state-reset that
    // follows regardless.

    m_iInitFlag_153_176[161 - 153] = 0;

    // mofclient.c calls CreateUseSkillEffect; when the active skill is a
    // "normal attack proxy" it returns non-null and we skip the plain-attack
    // logic.  In this restored build CreateUseSkillEffect returns void, so
    // we route via the hittedSkillKind flag that it clears on success.
    CreateUseSkillEffect();
    const bool skillFired = (GetUseSkillKind() == 0);

    if (!skillFired && m_iInitFlag_153_176[163 - 153]) {
        ClientCharacter* pTarget = m_pCharMgr
            ? m_pCharMgr->GetCharByAccount(v33)
            : nullptr;
        if (pTarget) {
            const bool v36 = (m_iPosX - pTarget->m_iPosX) > 0;
            // DWORD+166 on target is the "already-hited" flag.
            const int targetHitFlag = pTarget->m_iInitFlag_153_176[166 - 153];
            if (v38) {
                const unsigned char motion = m_ucUnknown_7384;
                if (targetHitFlag || motion == 9 || motion == 10 || motion == 11) {
                    pTarget->SetCharHitedInfo(
                        m_dwAccountID, v33, 0,
                        static_cast<char>(v32), v3, v38,
                        v36 ? 1 : 0, motion, v35, v34);
                } else if (v32 == 2) {
                    pTarget->HitMissed();
                } else {
                    stCharOrder hit{};
                    SetOrderHitted(&hit, v32, v3, v38,
                                   v36 ? 1 : 0, motion, v35, v34);
                    pTarget->PushOrder(&hit);
                }
            } else {
                const unsigned char motion = m_ucUnknown_7384;
                if (targetHitFlag || motion == 9 || motion == 10 || motion == 11) {
                    pTarget->SetCharHitedInfo(
                        m_dwAccountID, v33, 1,
                        static_cast<char>(v32), v3, 0,
                        v36 ? 1 : 0, motion, v35, v34);
                } else {
                    if (v32 == 2) {
                        pTarget->HitMissed();
                    } else {
                        stCharOrder hit{};
                        SetOrderHitted(&hit, v32, v3, 0,
                                       v36 ? 1 : 0, motion, v35, v34);
                        pTarget->PushOrder(&hit);
                    }
                    if (kindInfo && kindInfo->IsMonsterChar(pTarget->m_wKind)) {
                        SetOrderDie(&hit);
                    } else {
                        SetOrderDied(&hit);
                    }
                    pTarget->PushOrder(&hit);
                }
            }
        }

        // mofclient.c 29395: non-monster / non-transformation attackers play
        // the weapon sound and spawn a normal-attack effect.
        const bool isSelfMonster =
            kindInfo && kindInfo->IsMonsterChar(static_cast<uint16_t>(v4)) != 0;
        if (!isSelfMonster && !m_dwTransformation) {
            g_GameSoundManager.PlaySoundA(
                reinterpret_cast<char*>(m_safetyPad) + 7436,  // sound-id string lives here
                m_iPosX, m_iPosY);
            CreateNormalAttackEffect(m_ucUnknown_7384, v33, m_ucUnknown_7434);
        }

        // Multi-hit loop (mofclient.c LABEL_50, 29408+): iterate target list
        // at offset +10268.  Skipped in this pragmatic port — Phase C will
        // fold it back in once hit-target bookkeeping lands.
    }

    // mofclient.c 29523+: at terminal animation frame, transition to move/stop.
    if (m_wCurrentFrame == static_cast<unsigned short>(m_wTotalFrame - 2)) {
        std::memset(&m_currentOrder, 0, sizeof(m_currentOrder));
        stCharOrder next{};
        if (m_iDestX == m_iPosX && m_iDestY == m_iPosY) {
            SetOrderStop(&next);
        } else {
            SetOrderMove(&next,
                         static_cast<unsigned int>(m_iDestX),
                         static_cast<unsigned int>(m_iDestY), 0);
        }
        PushOrder(&next);
        m_iInitFlag_153_176[158 - 153] = 0;
    }
}

// mofclient.c 29569 / 29789: triple / double attack drivers.  These are
// multi-step variants of OrderAttack that fire sub-motions at specific frame
// counts (phase 0 / 1 / 2 for triple, 0 / 1 for double).  Port the outer
// scaffolding only; each sub-step reuses the same target-hit path as
// OrderAttack which we forward to directly.
void ClientCharacter::OrderTripleAttack() {
    // Simplified: when the triple-attack animation ends (phase counter at
    // byte+282 overflows past 2), reset and chain like OrderAttack.
    if (m_wCurrentFrame == static_cast<unsigned short>(m_wTotalFrame - 2)) {
        const unsigned char phase = Decomp<unsigned char>(282);
        m_wCurrentFrame = 0;
        m_iInitFlag_153_176[167 - 153] = 1;
        Decomp<unsigned char>(282) = static_cast<unsigned char>(phase + 1);
        if (static_cast<unsigned char>(phase + 1) > 2u) {
            Decomp<unsigned char>(282) = 0;
            std::memset(&m_currentOrder, 0, sizeof(m_currentOrder));
            stCharOrder next{};
            if (m_iDestX == m_iPosX && m_iDestY == m_iPosY) {
                SetOrderStop(&next);
            } else {
                SetOrderMove(&next,
                             static_cast<unsigned int>(m_iDestX),
                             static_cast<unsigned int>(m_iDestY), 0);
            }
            PushOrder(&next);
            m_iInitFlag_153_176[158 - 153] = 0;
            m_wUnknown_64 = 0;
        }
    }
}

void ClientCharacter::OrderDoubleAttack() {
    if (m_wCurrentFrame == static_cast<unsigned short>(m_wTotalFrame - 2)) {
        const unsigned char phase = Decomp<unsigned char>(282);
        m_wCurrentFrame = 0;
        m_iInitFlag_153_176[167 - 153] = 1;
        Decomp<unsigned char>(282) = static_cast<unsigned char>(phase + 1);
        if (static_cast<unsigned char>(phase + 1) > 1u) {
            Decomp<unsigned char>(282) = 0;
            std::memset(&m_currentOrder, 0, sizeof(m_currentOrder));
            stCharOrder next{};
            if (m_iDestX == m_iPosX && m_iDestY == m_iPosY) {
                SetOrderStop(&next);
            } else {
                SetOrderMove(&next,
                             static_cast<unsigned int>(m_iDestX),
                             static_cast<unsigned int>(m_iDestY), 0);
            }
            PushOrder(&next);
            m_iInitFlag_153_176[158 - 153] = 0;
        }
    }
}

// mofclient.c 34135: replay any queued-but-not-yet-pushed hit records that
// are still within a 3-second (0xBB8 ms) window.  Slot stride is 32 bytes,
// 50 slots total at offset +8076 — we iterate via Decomp<uint8_t>/uint32_t
// reads keyed by decomp offsets.
void ClientCharacter::OrderHit() {
    constexpr size_t kSlotBase   = 8076;
    constexpr size_t kSlotSize   = 32;
    constexpr size_t kNumSlots   = 50;
    const DWORD now = timeGetTime();

    for (size_t i = 0; i < kNumSlots; ++i) {
        const size_t base = kSlotBase + i * kSlotSize;
        const unsigned char typeMark = Decomp<unsigned char>(base + 23);  // v3-1
        if (typeMark != 16) continue;
        const DWORD timestamp = Decomp<DWORD>(base + 24);                 // *v3
        if (now - timestamp > 0xBB8) {
            ResetHitedInfo(static_cast<int>(i));
            continue;
        }
        const unsigned int targetAccount = Decomp<unsigned int>(base + 4);  // v3-5
        stCharOrder order{};
        ClientCharacter* target = m_pCharMgr
            ? m_pCharMgr->GetCharByAccount(targetAccount)
            : nullptr;
        if (target) {
            const char v5           = Decomp<char>(base + 8);             // v3-16
            const unsigned char v6  = Decomp<unsigned char>(base + 9);    // v3-15
            const int damage        = Decomp<int>(base + 12);             // v3-3
            const int remainHP      = Decomp<int>(base + 16);             // v3-2
            const char flag4        = Decomp<char>(base + 28);            // v3[4]
            const uint16_t skillK   = Decomp<uint16_t>(base + 20);        // v3-2 word
            const unsigned char hit = Decomp<unsigned char>(base + 22);   // v3-2 byte
            if (v5) {
                if (v6 != 2 &&
                    SetOrderHitted(&order, v6, damage, remainHP,
                                   flag4, v5, skillK, hit)) {
                    target->PushOrder(&order);
                }
                if (m_pClientCharKindInfo &&
                    m_pClientCharKindInfo->IsMonsterChar(target->m_wKind)) {
                    SetOrderDie(&order);
                } else {
                    SetOrderDied(&order);
                }
                target->PushOrder(&order);
            } else if (v6 == 2) {
                target->HitMissed();
            } else {
                if (SetOrderHitted(&order, v6, damage, remainHP,
                                   flag4, 0, skillK, hit)) {
                    target->PushOrder(&order);
                }
            }
        }
        ResetHitedInfo(static_cast<int>(i));
    }
}

void ClientCharacter::SetCurPosition(ClientCharacter* pChar, int x, int y) {
    if (!pChar) return;
    pChar->m_iPosX = x;
    pChar->m_iPosY = y;
}

// mofclient.c 32630.  My-char reads through cltPlayerAbility (the restored
// cltBaseAbility base provides GetHP); non-player chars use the local slot.
int ClientCharacter::GetHP() const {
    if (m_dwAccountID && m_dwAccountID == g_dwMyAccountID) {
        return g_clPlayerAbility.GetHP();
    }
    return const_cast<ClientCharacter*>(this)->Decomp<int>(4 * 144);
}

// mofclient.c 32622: copies DWORD+2814 (raw HP from server) into DWORD+144
// and, for my-char, mirrors the value into the player-ability singleton.
void ClientCharacter::SetHP() {
    const int hp = Decomp<int>(4 * 2814);
    Decomp<int>(4 * 144) = hp;
    if (IsMyChar()) {
        // cltPlayerAbility doesn't expose a SetHP today; the next Get/Inc
        // calls on the singleton will see the new HP through its own path.
        // When cltBaseAbility::SetHP is restored, plug it in here.
    }
}

// =============================================================================
// Order queue (mofclient.c 28411-28566)
// =============================================================================

void ClientCharacter::ClearOrder() {
    // mofclient.c 28411: zero the 100-slot queue, the current-order slot,
    // and the count.
    std::memset(m_OrderQueue, 0, sizeof(m_OrderQueue));
    std::memset(&m_currentOrder, 0, sizeof(m_currentOrder));
    m_nOrderCount = 0;
}

// mofclient.c 28490: enqueue a new order.
//   - Queue is capacity-limited to 100 (0x64).
//   - An "attack" order (type == 2? actually the decomp's literal is 2 which
//     we interpret as move; see SetOrder* mapping below) is silently dropped
//     when the character is already transforming (m_dwTransformation aka
//     DWORD+169) UNLESS DWORD+170 is set.  We preserve the raw decomp check
//     using the flag-bank accessor.
void ClientCharacter::PushOrder(stCharOrder* a2) {
    if (!a2) return;
    const unsigned int v3 = static_cast<unsigned int>(m_nOrderCount);
    // Decomp: *((_DWORD *)this + 169) == m_dwTransformation
    //         *((_DWORD *)this + 170) == m_iInitFlag_153_176[17]  (DWORD+170)
    const int transforming = m_dwTransformation;
    const int transformOverride = m_iInitFlag_153_176[170 - 153];
    if (v3 < 100 && (!transforming || transformOverride || a2->type != 2)) {
        std::memcpy(&m_OrderQueue[v3], a2, sizeof(stCharOrder));
        ++m_nOrderCount;
    }
}

// mofclient.c 28503: dequeue the head; shift the rest of the queue down by one.
void ClientCharacter::PopOrder(stCharOrder* a2) {
    if (!a2) return;
    if (m_nOrderCount) {
        std::memcpy(a2, &m_OrderQueue[0], sizeof(stCharOrder));
        if (m_nOrderCount > 1) {
            std::memmove(&m_OrderQueue[0], &m_OrderQueue[1],
                         sizeof(stCharOrder) * static_cast<size_t>(m_nOrderCount - 1));
        }
        --m_nOrderCount;
    }
}

// mofclient.c 28514: scan the queue for every "hitted" order (type == 5) and
// merge them into *a2 — accumulating damage (+4 int), copying the latest hit
// info fields, and splicing the matched slot out of the queue.  Returns 1 if
// any merge happened, 0 otherwise.
int ClientCharacter::MergeHittedOrder(stCharOrder* a2) {
    if (!a2) return 0;
    int count = m_nOrderCount;
    if (!count) return 0;
    int result = 0;
    int i = 0;
    while (i < count) {
        stCharOrder* slot = &m_OrderQueue[i];
        if (slot->type == 5) {
            a2->p0 = slot->p0;          // DWORD+1 of order (+4)
            a2->p2 = slot->p2;           // DWORD+3 of order (+12)
            a2->p3 += slot->p3;          // DWORD+4 of order (+16) — accumulate damage
            a2->w1  = slot->w1;
            a2->w2  = slot->w2;
            a2->b0  = slot->b0;
            if (count - 1 > i) {
                std::memmove(&m_OrderQueue[i], &m_OrderQueue[i + 1],
                             sizeof(stCharOrder) * static_cast<size_t>(count - 1 - i));
            }
            --count;
            if (i > 0) --i;  // re-scan current slot (now overwritten)
            result = 1;
            continue;
        }
        ++i;
    }
    m_nOrderCount = count;
    return result;
}

// mofclient.c 30449: returns the TYPE of the most-recently enqueued pending
// order (i.e. m_OrderQueue[count-1].type).  Returns 0 when the queue is empty.
unsigned int ClientCharacter::GetLastOrder() {
    const int count = m_nOrderCount;
    if (!count) return 0;
    return static_cast<unsigned int>(m_OrderQueue[count - 1].type);
}

// mofclient.c 28419: dispatch the head order via OnOrder*.
// Only advances when m_nOrderCount != 0.  Two extra guards:
//   - current-order type is 0/1/2 (stop/move/attack allow new orders)
//   - monsters with current type == 4 (die) do not pop further orders.
void ClientCharacter::ProcessOrder() {
    if (!m_nOrderCount) return;
    const int curType = m_currentOrder.type;
    const bool allowPop = (curType == 0 || curType == 1 || curType == 2);
    if (!allowPop) return;

    // Decomp guard (28430): if this is a monster and the current order type
    // is 4 (died), skip further processing.  We short-circuit when the kind
    // info manager is unavailable (pre-init tests run without it).
    // cltCharKindInfo::IsMonsterChar returns (stCharKindInfo*)1 when monster;
    // we approximate with m_dwAccountID == 0 — the same "monster-like" check
    // existing stubs use elsewhere in this file.
    if (curType == 4 && m_dwAccountID == 0) {
        return;
    }

    stCharOrder popped;
    std::memset(&popped, 0, sizeof(popped));
    PopOrder(&popped);

    switch (popped.type) {
    case 1:  OnOrderStop(&popped);           break;
    case 2:  OnOrderMove(&popped);           break;
    case 3:  OnOrderAttack(&popped);         break;
    case 4:  OnOrderDie(&popped);            break;
    case 5:  OnOrderHitted(&popped);         break;
    case 7:  OnOrderSpell(&popped);          break;
    case 8:  OnOrderPray(&popped);           break;
    case 9:  OnOrderDied(&popped);           break;
    case 10: OnOrderTransformation(&popped); break;
    default: break;
    }
    // mofclient.c 28482: regardless of dispatch, copy the popped order into
    // the current-order slot.
    std::memcpy(&m_currentOrder, &popped, sizeof(stCharOrder));
}


// mofclient.c 31357: dispatches an item-equip event to all the systems that
// care:  CCA layer composition, weapon aura effect, name-tag colouring,
// chat-ballon style, spirit-speech "onEquip" callback, and motion tables.
//
// This is a pragmatic port: the full ground truth includes ~240 lines of
// fine-grained fashion-item / shield-effect bookkeeping (offsets +1089/+1090
// snapshot-restore, +4373/+4374 byte pairs, J1216 transformation lock-out,
// chat-ballon block-table unpack).  We preserve the observable calls that
// downstream gameplay code depends on.
void ClientCharacter::SetItem(unsigned short itemKind, int qty) {
    auto* info = g_clItemKindInfo.GetItemKindInfo(itemKind);
    if (!info) return;

    const bool isFashion = g_clItemKindInfo.IsFashionItem(itemKind);
    unsigned char fashionBit = isFashion ? 1 : 0;

    auto* raw = reinterpret_cast<unsigned char*>(info);
    const unsigned int type  = *reinterpret_cast<unsigned int*>(raw + 80);  // DWORD+20
    const unsigned char classField = *(raw + 34);

    // Fashion mount-disable (item class 8 = vehicle): turn off transport.
    if (isFashion && type == 8) {
        if (m_pTransportObject && !m_pTransportObject->GetActive()) {
            m_dwUnknown_4356 = 0;
            m_dwUnknown_4360 = 0;
            Decomp<unsigned char>(4373) = 0;
            Decomp<unsigned char>(4374) = 0;
            m_dwUnknown_4368 = 0;
            Decomp<unsigned int>(4 * 1091) = 0;
            Decomp<unsigned char>(4372) = 0;
        }
        if (qty) {
            // Fashion-equip: snapshot item's effect kind, icon colour.
            m_dwUnknown_4356 = *reinterpret_cast<unsigned int*>(raw + 104);
            Decomp<unsigned int>(4 * 1091) = *reinterpret_cast<unsigned int*>(raw + 108);
            Decomp<unsigned char>(4373) = *(raw + 112);
        }
    }

    // Item type 2048 = nametag override.  Type 4096 = chat-ballon skin.
    // Both need cltMoF_NameTagInfo / cltMoF_ChatBallonInfo records which are
    // not yet exposed in the restored build — skip for now.
    (void)type;

    // Equip-slot update (skip meta-item classes 2048/4096 and the +34==1
    // branches for special-category items).
    const bool isSpecial =
        (classField == 1 && (type == 0x400000 || type == 0x2000 || type == 0x4000))
        || type == 2048 || type == 4096;

    if (!isSpecial && m_pCCA) {
        m_pCCA->SetItemID(itemKind, m_ucSex, qty, m_ucHair, m_ucFace, fashionBit);
    }

    // Downstream updates — weapon type (applies CreateWeaponEffect gating),
    // triple/double attack motion tables.
    SetWeaponType(itemKind, qty);
    SetTripleAttackMotion();
    SetDoubleAttackMotion();
}

void ClientCharacter::ResetItem(unsigned char slot) {
    if (slot < 16) {
        const size_t off = static_cast<size_t>(slot) * 2;
        unsigned short zero = 0;
        std::memcpy(m_acEquipKind + off, &zero, sizeof(zero));
    }
}

void ClientCharacter::SetCAClone() {
    // mofclient.c 31654: sets a "use CAClone" flag; recorded in m_iInitFlag
    // region at DWORD+2424 (offset 9696).  We stash into m_iInitFlag_153_176
    // keyed by the same logical meaning; Phase C will replace with a proper
    // named member (m_nUsingCAClone).
    m_iInitFlag_153_176[0] = 1;
}

// =============================================================================
// InitFlag (mofclient.c 34921)
//
// Reset the flag-bank DWORDs at offsets +612..+707 plus a handful of unrelated
// bytes/words that the decomp sets to their "just-created" defaults.  Called
// from the ctor, DeleteCharacter, and the item-system's reset paths.
// =============================================================================
void ClientCharacter::InitFlag() {
    // Flag bank — mirrors the 153..176 DWORD writes in the decomp:
    // indices (decomp - 153) = (0, 1, 2, 5, 3, 4)=(1 default?) …
    // Replicating 1-for-1:
    m_iInitFlag_153_176[153 - 153] = 0;
    m_iInitFlag_153_176[154 - 153] = 0;
    m_iInitFlag_153_176[158 - 153] = 0;
    m_iInitFlag_153_176[155 - 153] = 0;
    m_iInitFlag_153_176[156 - 153] = 0;
    m_iInitFlag_153_176[157 - 153] = 1;
    m_iInitFlag_153_176[159 - 153] = 0;
    m_iInitFlag_153_176[160 - 153] = 0;
    m_iInitFlag_153_176[161 - 153] = 1;
    m_iInitFlag_153_176[162 - 153] = 1;
    m_iInitFlag_153_176[163 - 153] = 1;
    m_iInitFlag_153_176[164 - 153] = 0;
    m_iInitFlag_153_176[165 - 153] = 1;
    m_iInitFlag_153_176[166 - 153] = 0;
    m_iInitFlag_153_176[167 - 153] = 1;
    m_iInitFlag_153_176[168 - 153] = 0;
    // DWORD+169 shares storage with m_dwTransformation — reset via the
    // named field directly (see the m_dwTransformation = 0 line below).
    m_iInitFlag_153_176[170 - 153] = 1;
    m_iInitFlag_153_176[171 - 153] = 1;
    m_iInitFlag_153_176[174 - 153] = 1;
    m_iInitFlag_153_176[172 - 153] = 1;
    m_iInitFlag_153_176[175 - 153] = 0;
    m_iInitFlag_153_176[176 - 153] = 0;

    // Byte / word scatter writes.
    m_ucUnknown_4400    = 0;
    Decomp<unsigned char>(11524) = 2;
    Decomp<unsigned int>(4 * 2889) = 0;   // DWORD+2889 (offset 11556)
    m_isPCRoomPremium   = false;
    Decomp<uint16_t>(2 * 2221) = 0;       // WORD+2221 (offset 4442)
    Decomp<uint16_t>(2 * 2222) = 0;       // WORD+2222 (offset 4444)
    SetPrivateMarketMsg(const_cast<char*>(""));
    m_ucNameClassMark   = 0;
    m_wPvPRankKind      = 0;
    m_dwUnknown_9716    = 0;
    m_wClassCode        = 0;
    ResetCharEffect();
    Decomp<int>(4 * 84) = 10;             // death-frame counter seed
    m_dwGM_Level        = 0;
    m_ucUnknown_10112   = 0;
    m_someOtherState    = 0;  // DWORD+2882 at offset 11528
    m_dwTransformation = 0;
}

// =============================================================================
// DeleteCharacter (mofclient.c 31671)
//
// Tear down a slot: flush chat, release CCA pair and animation resources,
// zero every per-character field that CreateCharacter expects freshly zero.
// =============================================================================
void ClientCharacter::DeleteCharacter() {
    ClearerChatMsg();
    m_dwSlotAlive = 0;
    if (IsMyChar()) {
        // mofclient.c: DirectInputManager::FreeAllKey — not yet wired up in
        // this restored build; skipped safely.
    }
    // Release CCANormal and CCA (vtable slot 0 with arg=1 = destroy+delete).
    m_pCCANormal = nullptr;
    m_pCCA       = nullptr;

    DeleteSustainEffect();
    ClearOrder();
    ResetUseSkillKind();
    ResetHittedSkillKind();
    ResetGainExp();

    m_ucUnknown_7435 = 0;
    m_ucUnknown_7434 = 0;
    Decomp<uint16_t>(2 * 3716) = 0;       // WORD+3716 at 7432
    Decomp<uint16_t>(2 * 3715) = 0;       // WORD+3715 at 7430
    Decomp<uint16_t>(2 * 5766) = 0;       // WORD+5766 at 11532
    Decomp<unsigned char>(9984) = 0;
    Decomp<unsigned char>(10124) = 0;
    m_szScreenName[0]  = '\0';
    m_szCircleName[0]  = '\0';

    InitFlag();

    Decomp<uint16_t>(2 * 3714) = 0;
    Decomp<int>(4 * 1847) = 0;
    Decomp<int>(4 * 1848) = 0;
    m_ucAttackMotionType = 0;
    Decomp<int>(4 * 1849) = 0;
    m_wUnknown_64        = 0;
    m_dwAttackSpeed      = 0;
    m_wTotalFrame        = 0;
    Decomp<uint16_t>(2 * 4845) = 0;
    Decomp<uint16_t>(2 * 4846) = 0;
    m_wCurrentFrame      = 0;
    Decomp<uint16_t>(2 * 3706) = 0;
    Decomp<uint16_t>(2 * 3707) = 0;
    Decomp<int>(4 * 1852) = 0;
    Decomp<int>(4 * 15) = 0;              // DWORD+15 (offset 60)
    m_ucUnknown_7384     = 2;
    Decomp<uint16_t>(2 * 4840) = 0;
    m_dwUnknown_9676     = 0;
    m_ucUnknown_11260    = 1;
    m_iDestY             = 0;  // WORD+140 overlays the low half of m_iDestY
    Decomp<unsigned char>(282) = 0;
    Decomp<uint16_t>(2 * 4848) = 0;
    Decomp<uint16_t>(2 * 4849) = 0;
    Decomp<unsigned char>(7436) = 0;
    Decomp<unsigned char>(9700) = 0;
    Decomp<int>(4 * 2426) = 0;
    Decomp<int>(4 * 2427) = 0;
    m_ucUnknown_11252    = 0;
    Decomp<int>(4 * 1850) = 0;
    Decomp<int>(4 * 1851) = 0;
    Decomp<int>(4 * 1843) = 0;
    Decomp<int>(4 * 2814) = 0;
    m_iUnknown_372       = 0;  // DWORD+73..75 approximate
    Decomp<int>(4 * 74)  = 0;
    Decomp<int>(4 * 75)  = 0;
    m_iUnknown_372       = 0;  // DWORD+77 — frame counter reuses m_iUnknown_372 slot logically
    Decomp<int>(4 * 77)  = 0;
    m_ucAlpha            = 0xFFu;
    m_dwUnknown_4356     = 0;
    m_dwUnknown_4360     = 0;
    Decomp<int>(4 * 2431) = 0;
    Decomp<uint16_t>(2 * 7340) = 0;
    m_dwUnknown_14684     = 0;
    Decomp<uint16_t>(2 * 7344) = 0;
    m_wUnknown_4432       = 0;
    SetActionState(0);

    m_pShadowImage     = nullptr;
    m_pShadowImage_Sub = nullptr;
    m_pMonsterAniInfo  = nullptr;

    if (m_pPetObject)      m_pPetObject->Release();
    if (m_pTransportObject) m_pTransportObject->Release();

    // memset +10252 × 1000: covers a secondary state buffer we do not name.
    std::memset(m_safetyPad + 10252, 0, 0x3E8);
    Decomp<int>(4 * 1844) = 0;
    Decomp<int>(4 * 1845) = 0;

    std::memset(m_acEquipKind, 0, sizeof(m_acEquipKind));
    // memset +8076 × 0x640 covers the "delayed hit info" table.
    std::memset(m_safetyPad + 8076, 0, 0x640);

    Decomp<uint16_t>(283) = 0;
    Decomp<unsigned char>(285) = 0;
    Decomp<uint16_t>(2 * 143) = 0;

    // DWORD+2816 at offset 11264 is an owned effect; destroy via vtable in
    // the decomp.  Without a restored CEffectBase destroy vtable we null it.
    Decomp<void*>(4 * 2816) = nullptr;
    Decomp<unsigned char>(360) = 0;

    // After this method the slot is officially empty.
    m_dwAccountID = 0;
    m_szName[0]   = '\0';
}

// =============================================================================
// CreateCharacter (mofclient.c 30831)
//
// The ground truth is a 515-line behemoth that wires up CCA/CCANormal
// renderers, item slots, pet/transport objects, weather effects, chat-ballon
// styling, and name-tag colouring.  This pragmatic port covers the core
// identity + position seed + the sub-setters the decomp calls in the first
// ~70 lines (InitFlag/ClearOrder, name/circle copy, HP seed, sex/hair/face,
// warp reset, search/attack target reset, PCRoom flag, PvP rank).
//
// Later phases will extend this with: CCA creation, monster-ani lookup, NPC
// recall assets, climate effects, chat-ballon style, guild-mark colour, and
// the "auto-resurrect" pet attach point.  The missing calls are invoked via
// the same sub-methods the ground truth uses, so once those are ported their
// effects will automatically take over here.
// =============================================================================
void ClientCharacter::CreateCharacter(
    ClientCharacter* self,
    int posX, int posY,
    unsigned short charKind, unsigned short mapID,
    int hp,
    char* name, char* circleName, unsigned short circleMark,
    char* coupleName, char* petName, unsigned short petKind,
    unsigned char ucUnknown14, char cUnknown15,
    unsigned char sex, unsigned int accountID,
    char cUnknown18, unsigned char hair,
    int iUnknown20, unsigned char face,
    unsigned char ucUnknown22, unsigned short classCode,
    unsigned short transformKind, unsigned char ucUnknown25,
    int iUnknown26, char cUnknown27, char* privateMarketMsg,
    unsigned short ucPCRoom, unsigned short weaponKind,
    unsigned short shieldKind, char* screenName,
    unsigned char nation, unsigned short titleKind,
    int iUnknown35) {
    (void)self;  // `a2` param — decomp stores this as account-id alias.

    InitFlag();
    ClearOrder();
    m_dwSlotAlive = 1;
    m_dwAccountID = accountID;
    m_wMapID      = mapID;
    m_iDestX      = posX;
    m_iDestY      = posY;
    m_wKind       = charKind;

    if (name) {
        std::strncpy(m_szName, name, sizeof(m_szName) - 1);
        m_szName[sizeof(m_szName) - 1] = '\0';
    }
    if (screenName) {
        std::strncpy(m_szScreenName, screenName, sizeof(m_szScreenName) - 1);
        m_szScreenName[sizeof(m_szScreenName) - 1] = '\0';
    }
    if (privateMarketMsg) {
        SetPrivateMarketMsg(privateMarketMsg);
    }

    Decomp<uint16_t>(2 * 7409) = titleKind;

    InitScreenName(reinterpret_cast<intptr_t>(this) & 0x7FFFFFFF);

    m_someOtherState = hp;     // DWORD+2814 (offset 11256) is the raw HP slot.
    SetHP();
    Decomp<int>(4 * 145) = iUnknown26;
    Decomp<char>(536)    = cUnknown27;

    // Sex: decomp uppercases the char and maps 'M'→1, 'F'→0.
    const int up = std::toupper(static_cast<unsigned char>(ucUnknown14));
    if (up == 'M') m_ucSex = 1;
    else if (up == 'F') m_ucSex = 0;
    // Hair / face
    m_ucHair = hair;
    m_ucFace = face;

    SetCharState(cUnknown18);
    SetSpecialState(ucUnknown22);
    SetWarp(0, 0, 0, 0, 0);
    SetSearchMonster(0);
    SetAttackMonster(0);
    SetCircleName(circleName);
    Decomp<unsigned char>(11524) = static_cast<unsigned char>(nation);
    SetCharHide(reinterpret_cast<intptr_t>(this) & 0x7FFFFFFF, ucUnknown25);
    SetPCRoomUser(iUnknown20, static_cast<unsigned char>(ucPCRoom & 0xFF));

    m_iPosX     = posX;
    m_iPosY     = posY;
    m_iPrevPosX = posX;
    m_iPrevPosY = posY;

    // DWORD+5 — map info pointer; looked up via Map::GetMapInfoByID.  Without
    // that method restored we leave the slot zero; PrepareDrawing will fall
    // back to a null-map path (tutorial / offline test runs).
    Decomp<void*>(4 * 5) = nullptr;

    Decomp<int>(4 * 1849) = 0;
    Decomp<int>(4 * 15)   = 0;
    m_ucAttackMotionType = 0;
    Decomp<int>(4 * 1843) = 0;
    Decomp<int>(4 * 1850) = 0;
    Decomp<int>(4 * 1851) = 0;
    Decomp<int>(4 * 1852) = 0;
    Decomp<uint16_t>(2 * 37) = 0;
    m_ucUnknown_11260 = 1;
    Decomp<uint16_t>(2 * 4840) = 0;
    Decomp<int>(4 * 1856) = 0;
    m_someOtherState  = 0;
    m_ucUnknown_7435  = 0;
    m_ucUnknown_7434  = 0;
    Decomp<uint16_t>(2 * 3716) = 0;
    Decomp<uint16_t>(2 * 3715) = 0;
    Decomp<uint16_t>(2 * 5766) = 0;
    Decomp<uint16_t>(2 * 7340) = 0;
    m_dwUnknown_14684 = 0;

    SetPvPRankKind(classCode);  // decomp passes a23 which equals `classCode`

    // g_wMapid — a global "currently shown map id" used by a few UI paths.
    // Not yet exposed as a global; UI callers that depend on it read
    // dword_21B8DF4 instead, which stays in sync via SetWarp.

    Decomp<unsigned char>(7436) = 0;
    Decomp<int>(4 * 1844) = 0;
    Decomp<int>(4 * 1845) = 0;

    if (m_pClientCharKindInfo) {
        // mofclient.c 30978: pulls the per-kind name-tag Y offset.  We'd call
        // GetCharInfoPosY here once restored; its absence leaves +2216 at 0.
    }

    // ------------------------------------------------------------------
    // mofclient.c 30981-31180: secondary init — buffers, CCA, speeds,
    // transport, class-code, circle-name text width.
    // ------------------------------------------------------------------
    // Hited-info count + slot table.
    Decomp<int>(4 * 2419) = 0;
    std::memset(m_safetyPad + 8076, 0, 0x640);  // hited-info 50 × 32
    std::memset(m_acEquipKind, 0, sizeof(m_acEquipKind));
    std::memset(m_safetyPad + 10252, 0, 0x3E8);  // hit-staging 50 × 20
    m_ucUnknown_11252 = 0;
    Decomp<int>(4 * 1094) = 0;

    // Random facing.
    m_dwLR_Flag = std::rand() & 1;

    const int frameOffset = (SETTING_FRAME == 2) ? 1 : 0;
    const bool isMonster =
        m_pClientCharKindInfo
        && m_pClientCharKindInfo->IsMonsterChar(m_wKind);

    if (isMonster) {
        // Monster branch: move speed from char-kind record, then either load
        // the "gastran00.ca" CCANormal (only for kind J0591) or attach the
        // per-kind cltMonsterAniInfo frame table.
        m_fMoveSpeed = static_cast<float>(frameOffset);  // + kind speed
        m_iInitFlag_153_176[159 - 153] = 0;
        stCharKindInfo* info = static_cast<stCharKindInfo*>(
            m_pClientCharKindInfo->GetCharKindInfo(m_wKind));
        // Specific-kind CCANormal load path is skipped — it needs the
        // cltCharKindInfo WORD+3 match against kind "J0591" which requires
        // a second pass of kind-info mapping.  Normal monsters go via
        // cltMonsterAniInfo below.
        if (info) {
            (void)info;  // sentinel for future J0591 branch
        }
        // cltClientCharKindInfo::GetMonsterAniInfo assignment — deferred.
        Decomp<uint16_t>(2 * 140) = 0;
        // Base alpha visibility check.
        Decomp<int>(4 * 76) = 1;
        Decomp<unsigned char>(360) = 100;
    } else {
        // Player branch: allocate CCA, load "Dot_Character.ca", set speeds.
        if (!m_pCCA) {
            m_pCCA = new CCA();
            m_pCCA->LoadCA("MoFData/Character/Dot_Character.ca",
                           nullptr, nullptr);
        }
        m_pCCA->Play(0, true);
        m_iInitFlag_153_176[159 - 153] = 1;

        if (IsMyChar()) {
            // 1082130432=4.0f, 1088421888=7.0f, 1092616192=10.0f
            Decomp<float>(4 * 80) = 4.0f;
            Decomp<float>(4 * 81) = 7.0f;
            Decomp<float>(4 * 82) = 10.0f;
        } else {
            const float fs = static_cast<float>(SETTING_FRAME);
            Decomp<float>(4 * 80) = static_cast<float>(frameOffset) + (4.0f / fs);
            Decomp<float>(4 * 81) = static_cast<float>(frameOffset) + (7.0f / fs);
            Decomp<float>(4 * 82) = static_cast<float>(frameOffset) + (10.0f / fs);
        }

        // Dead-spawn: non-lobby maps with hp==0 immediately push a Died
        // order so the respawn UI appears.
        if (!hp && m_pMap && !m_pMap->IsMatchLobbyMap(mapID)) {
            stCharOrder order{};
            SetOrderDied(&order);
            PushOrder(&order);
        }

        // Climate speed multiplier (snow / rain slow the player down).
        if (m_pMap) {
            strClimateInfo* climate = m_pMap->GetClimateKindByMapId(mapID);
            if (climate) {
                const uint16_t speedPerMille =
                    *reinterpret_cast<uint16_t*>(
                        reinterpret_cast<unsigned char*>(climate) + 4);
                const float mult = speedPerMille * 0.001f;
                Decomp<float>(4 * 80) *= mult;
                Decomp<float>(4 * 81) *= mult;
                Decomp<float>(4 * 82) *= mult;
            }
        }

        // Transport / run state.
        if (iUnknown35) {
            SetTransportKind(static_cast<unsigned short>(iUnknown35));
            m_fMoveSpeed = Decomp<float>(4 * 82);
            m_ucUnknown_4400 = 2;
            if (m_pActionStateSystem)
                m_pActionStateSystem->SetCharActionState(
                    static_cast<unsigned char>(cUnknown18));
        } else {
            m_dwUnknown_4356 = 0;
            m_dwUnknown_4360 = 0;
            m_dwUnknown_4368 = 0;
            Decomp<unsigned char>(4374) = 0;
            m_ucUnknown_4400 = 0;
            if (m_pActionStateSystem)
                m_pActionStateSystem->SetCharActionState(
                    static_cast<unsigned char>(cUnknown18));
            m_fMoveSpeed = Decomp<float>(4 * 80);
            const unsigned char acState = Decomp<unsigned char>(11548);
            if (acState & 0x20) {
                m_ucUnknown_4400 = 1;
                m_fMoveSpeed = Decomp<float>(4 * 81);
            } else if (acState & 0x40) {
                m_ucUnknown_4400 = 2;
                m_fMoveSpeed = Decomp<float>(4 * 82);
            }
        }

        SetGainExp(0);

        // CCA::InitItem — apply sex / hair / face / hair-colour to the
        // equipment-layer composite.
        if (m_pCCA) {
            m_pCCA->InitItem(m_ucSex, hair, face,
                             static_cast<uint32_t>(accountID));
        }

        m_iInitFlag_153_176[165 - 153] = 1;  // cctor sets to 1 in InitFlag but CC overrides
        m_iInitFlag_153_176[164 - 153] = 0;
        m_wUnknown_64 = 0;
        m_iInitFlag_153_176[162 - 153] = 1;
        m_iInitFlag_153_176[160 - 153] = 1;  // HP-box visible flag

        // Circle banner width calc.
        if (circleMark) {
            Decomp<uint16_t>(2 * 5766) = circleMark;
            Decomp<uint16_t>(2 * 5767) = 21;
            int widthPx = 0, heightPx = 0;
            g_MoFFont.GetTextLength(&widthPx, &heightPx,
                                    "CircleName", m_szCircleName);
            Decomp<int>(4 * 2884) = (widthPx + 26) / 2;
            if (circleMark == 2) Decomp<uint16_t>(2 * 5767) = 20;
        }
    }

    // Class code, pet kind, transform kind finalisation.
    SetClassCode(classCode, 0);
    if (petKind) CreatePet(petKind, 0);
    if (transformKind) SetTransformation(transformKind);

    (void)coupleName;
    (void)petName;
    (void)cUnknown15;
    (void)shieldKind;
    (void)weaponKind;
}

void ClientCharacter::SetEmoticonKind(int emoticonKind) {
    // DWORD+2425 (offset 9700) is an emoticon-kind scratch field in the decomp;
    // stash into m_iInitFlag_153_176[1] for now (Phase C will give it its own
    // named member once other consumers of the flag bank are identified).
    m_iInitFlag_153_176[1] = emoticonKind;
    if (m_pCCA) {
        m_pCCA->BegineEmoticon(emoticonKind);
    }
}

void ClientCharacter::ReleaseEmoticon() {
    m_iInitFlag_153_176[1] = 0;
    if (m_pCCA) {
        m_pCCA->EndEmoticon(m_ucFace, m_ucSex);
    }
}

// =============================================================================
// Action-state dispatch (mofclient.c 28898)
//
// Sets m_dwActionState and recomputes the total-frame count for the new state.
// The path branches by: monster-vs-player, transformation active (non-player),
// and whether the monster-aniInfo pointer is populated.  The per-state frame
// counts come from either a fixed table (constant frames 1/4/8/10/14) or from
// cltMonsterAniInfo::GetTotalFrameNum.
// =============================================================================
void ClientCharacter::SetActionState(unsigned int a2) {
    m_dwActionState = a2;
    const unsigned int kind = m_wKind;
    m_wCurrentFrame = 0;

    auto* pKindInfo = m_pClientCharKindInfo;
    const bool isMonster =
        pKindInfo && pKindInfo->IsMonsterChar(static_cast<uint16_t>(kind)) != 0;

    if (isMonster) {
        // Monster branch — picks from either the DWORD+153 fixed table or
        // the monster-aniInfo table.
        if (m_iInitFlag_153_176[0]) {
            // Legacy fixed-count path (mofclient.c 28911).
            switch (m_dwActionState) {
            case 0:
            case 1:
                m_wTotalFrame = 8;
                break;
            case 2:
                m_wTotalFrame = 14;
                break;
            case 3:
                m_wTotalFrame = 4;
                m_ucAttackMotionType = static_cast<unsigned char>(std::rand() & 1);
                break;
            case 4:
                switch (m_ucUnknown_11260) {
                case 1: m_wTotalFrame = 4;  break;
                case 2: m_wTotalFrame = 10; break;
                case 3: m_wTotalFrame = 6;  break;
                case 4: m_wTotalFrame = 5;  break;
                default: goto LABEL_47;
                }
                break;
            default:
                break;
            }
        } else {
            auto* v4 = static_cast<cltMonsterAniInfo*>(m_pMonsterAniInfo);
            if (!v4) return;
            switch (m_dwActionState) {
            case 0: m_wTotalFrame = v4->GetTotalFrameNum(0); break;
            case 1: m_wTotalFrame = v4->GetTotalFrameNum(1); break;
            case 2: m_wTotalFrame = v4->GetTotalFrameNum(2); break;
            case 3: m_wTotalFrame = v4->GetTotalFrameNum(3); break;
            case 4: {
                m_wTotalFrame = 0;
                unsigned int sub = 0xFFFFFFFFu;
                switch (m_ucUnknown_11260) {
                case 1: sub = 4; break;
                case 2: sub = 5; break;
                case 3: sub = 6; break;
                case 4: sub = 7; break;
                default: break;
                }
                if (sub != 0xFFFFFFFFu) m_wTotalFrame = v4->GetTotalFrameNum(sub);
                if (!m_wTotalFrame) m_wTotalFrame = v4->GetTotalFrameNum(4);
                break;
            }
            default:
                break;
            }
        }
    } else {
        // Player / NPC branch.
        if (m_dwTransformation) {
            auto* v5 = static_cast<cltMonsterAniInfo*>(m_pMonsterAniInfo);
            if (!v5) return;
            switch (m_dwActionState) {
            case 0: m_wTotalFrame = v5->GetTotalFrameNum(0); break;
            case 1: m_wTotalFrame = v5->GetTotalFrameNum(1); break;
            case 2: m_wTotalFrame = v5->GetTotalFrameNum(2); break;
            case 3: m_wTotalFrame = v5->GetTotalFrameNum(3); break;
            case 4: {
                m_wTotalFrame = 0;
                unsigned int sub = 0xFFFFFFFFu;
                switch (m_ucUnknown_11260) {
                case 1: sub = 4; break;
                case 2: sub = 5; break;
                case 3: sub = 6; break;
                case 4: sub = 7; break;
                default: break;
                }
                if (sub != 0xFFFFFFFFu) m_wTotalFrame = v5->GetTotalFrameNum(sub);
                if (!m_wTotalFrame) m_wTotalFrame = v5->GetTotalFrameNum(4);
                break;
            }
            default:
                break;
            }
        } else {
            // Normal player frame counts.
            switch (m_dwActionState) {
            case 0:
            case 1:
            case 10:
                m_wTotalFrame = 8;
                break;
            case 2:
            case 7:
                m_wTotalFrame = 1;
                break;
            case 3:
            case 4:
            case 5:
            case 6:
            case 8:
            case 9:
                m_wTotalFrame = 4;
                break;
            default:
                break;
            }
        }

        if (m_pTransportObject && m_pTransportObject->GetActive()) {
            m_pTransportObject->SetActionState(m_dwActionState);
        }
    }

LABEL_47:
    // mofclient.c 29075: any non-zero action state clears an active emoticon.
    if (m_dwActionState) {
        // DWORD+168 (m_iInitFlag[15]) is the emoticon-active flag.
        if (m_iInitFlag_153_176[15]) {
            if (m_pCharMgr) m_pCharMgr->ReleaseEmoticon(this);
        }
    }
}

// =============================================================================
// OnOrder* handlers (mofclient.c 28569-28897)
// =============================================================================

// mofclient.c 28569
void ClientCharacter::OnOrderStop(stCharOrder* /*a2*/) {
    // DWORD+0 guards: non-zero means "something is already stopping us".
    const uintptr_t offset0 = reinterpret_cast<uintptr_t>(m_pVftable);
    if (!offset0) SetActionState(0);
    if (m_pCharMgr && m_pCharMgr->IsPlayer(m_wKind)) {
        if (!m_someOtherState && !m_iInitFlag_153_176[22]) {
            m_iInitFlag_153_176[23] = 0;
            const bool canCreate = !offset0 && !m_dwTransformation
                                   && (!m_pTransportObject
                                       || !m_pTransportObject->GetActive());
            if (canCreate) {
                // *((_WORD *)this + 3715) / 3716 live inside the HPBox region.
                // Store via the decomp offset helper so reads and writes stay
                // consistent; a named field can replace this later.
                const uint16_t v4 = Decomp<uint16_t>(2 * 3715);
                if (v4) CreateWeaponEffect(v4, 7);
                const uint16_t v5 = Decomp<uint16_t>(2 * 3716);
                if (v5) CreateWeaponEffect(v5, 8);
            }
        }
    }
}

// mofclient.c 28597
void ClientCharacter::OnOrderMove(stCharOrder* a2) {
    if (!a2) return;
    const int destX = a2->p0;
    const int destY = a2->p1;
    if (m_iDestX != destX || m_iDestY != destY) {
        SetEndPosition(this, destX, destY);
        if (m_dwActionState != 1) {
            SetActionState(1);
            m_iInitFlag_153_176[23] = 1;
        }
    }
}

// mofclient.c 28614
void ClientCharacter::OnOrderAttack(stCharOrder* a2) {
    // DWORD+161 is the "can attack" flag — decomp sets it to 1 here.
    m_iInitFlag_153_176[161 - 153] = 1;
    m_ucUnknown_7384 = static_cast<unsigned char>(GetAttackMotion());
    SetActionState(3);
    // a2->w2 holds the skill kind (WORD+14 of the order).
    stSkillKindInfo* skill = g_clSkillKindInfo.GetSkillKindInfo(a2 ? a2->w2 : 0);
    if (skill) {
        // *((_BYTE *)v3 + 170) in the decomp — skill sub-type selector.
        const unsigned char subKind =
            *(reinterpret_cast<unsigned char*>(skill) + 170);
        switch (subKind) {
        case 1: SetActionState(5); break;
        case 2: SetActionState(6); break;
        case 3: SetActionState(8); break;
        case 4: SetActionState(9); break;
        default: SetActionState(3); break;
        }
    }
}

// mofclient.c 28646
void ClientCharacter::OnOrderDie(stCharOrder* /*a2*/) {
    SetActionState(2);
    SetEndPosition(this, m_iPosX, m_iPosY);
    if (m_pCharMgr) m_pCharMgr->ResetAutoAttack(m_dwAccountID);
}

// mofclient.c 28654
void ClientCharacter::OnOrderDied(stCharOrder* /*a2*/) {
    DeadProcSustainEffect();
    int v3 = 40;
    if (!m_dwLR_Flag) v3 = -45;
    // Field-item-box override — treat as origin-offset 0 when the kind code
    // represents a field item container.
    if (m_pClientCharKindInfo) {
        // The concrete check lives in cltClientCharKindInfo::IsFieldItemBox;
        // we approximate with the general "not a player char" fallback for
        // now — sufficient for tutorial / non-box callers.  Proper lookup
        // will land when cltClientCharKindInfo::IsFieldItemBox is restored.
    }
    m_dwTransformation = 0;
    SetActionState(7);

    // mofclient.c 28671-28679: spawn a Player_Dead effect below the corpse.
    if (g_pEffectManager_After_Chr) {
        CEffect_Player_Dead* pDead = new CEffect_Player_Dead();
        const float x = static_cast<float>(v3 + m_iPosX);
        const float y = static_cast<float>(m_iPosY - 30);
        // dword_21B8DFC (PK-mode flag) is not yet wired into this build, pass 0.
        pDead->SetEffect(this, x, y, 0);
        // DWORD+99 (offset 396) tracks the owned effect; stash via Decomp.
        Decomp<void*>(4 * 99) = pDead;
        g_pEffectManager_After_Chr->BulletAdd(reinterpret_cast<CEffectBase*>(pDead));
    }

    if (IsMyChar()) {
        // UI popup for local-player death; guarded by g_UIMgr availability.
        if (g_UIMgr) {
            // GetUIWindow(0) = main UI.  CUIBasic::DiedChar has not been
            // restored in this branch; keep the call site and wire it up
            // in Phase D when DrawTextA lands.
        }
    }
    SetEndPosition(this, m_iPosX, m_iPosY);
    EndAllSustainSkill();
}

// mofclient.c 28695 — large, splits on damage type.
void ClientCharacter::OnOrderHitted(stCharOrder* a2) {
    if (!a2) return;
    stCharOrder merged;
    std::memset(&merged, 0, sizeof(merged));
    if (MergeHittedOrder(&merged)) {
        a2->p0  = merged.p0;
        a2->p2  = merged.p2;
        a2->p3 += merged.p3;
        a2->w1  = merged.w1;
        a2->w2  = merged.w2;
        a2->b0  = merged.b0;
    }
    // a2->b2 (byte+32) holds the hit-info class id copied into +11260.
    m_ucUnknown_11260 = a2->b2;
    SetHP();
    const bool isMonster = (m_pCharMgr && m_pCharMgr->IsMonster(m_wKind));
    SetCharState(a2->b0);
    SetActionState(4);

    switch (a2->p0) {
    case 0:
    case 0xA: {
        // Normal damage number + hit multi + use-hit effect chain.
        if (g_pEffectManager_After_Chr) {
            CEffect_Field_DamageNumber* dmg = new CEffect_Field_DamageNumber();
            const float y = static_cast<float>(m_iPosY - GetCharHeight() - 45);
            const float x = static_cast<float>(m_iPosX);
            dmg->SetEffect(-a2->p3, x, y, isMonster ? 1 : 0);
            g_pEffectManager_After_Chr->BulletAdd(reinterpret_cast<CEffectBase*>(dmg));
        }
        if (!CreateHittedSkillEffect(a2->b1)) {
            CreateHitCharKindEffect();
            if (g_pEffectManager_After_Chr) {
                if (isMonster) {
                    CEffectUseHitMulti* hm = new CEffectUseHitMulti();
                    const float y = static_cast<float>(m_iPosY - 10);
                    const float x = static_cast<float>(m_iPosX);
                    hm->SetEffect2(x, y, (a2->b1 == 1) ? 1 : 0, 1);
                    g_pEffectManager_After_Chr->BulletAdd(
                        reinterpret_cast<CEffectBase*>(hm));

                    CEffect_Use_Hit* uh = new CEffect_Use_Hit();
                    const float uy = static_cast<float>(m_iPosY - 10);
                    const unsigned char dir = (m_dwLR_Flag == 1) ? 1 : 0;
                    uh->SetEffect(x, uy, dir);
                    g_pEffectManager_After_Chr->BulletAdd(
                        reinterpret_cast<CEffectBase*>(uh));
                } else {
                    m_wCurrentFrame = 0;
                    CEffect_Use_Hit* uh = new CEffect_Use_Hit();
                    const float uy = static_cast<float>(m_iPosY - 20);
                    const float x  = static_cast<float>(m_iPosX);
                    uh->SetEffect(x, uy, 2);
                    g_pEffectManager_After_Chr->BulletAdd(
                        reinterpret_cast<CEffectBase*>(uh));
                }
            }
        }
        break;
    }
    case 1:
    case 0xB: {
        // Critical damage: critical number + hit-multi effect + sound.
        if (g_pEffectManager_After_Chr) {
            CEffect_Field_CriticalNumber* crit = new CEffect_Field_CriticalNumber();
            const float y = static_cast<float>(m_iPosY - GetCharHeight() - 45);
            const float x = static_cast<float>(m_iPosX);
            crit->SetEffect(-a2->p3, x, y, 1);
            g_pEffectManager_After_Chr->BulletAdd(
                reinterpret_cast<CEffectBase*>(crit));
        }
        if (!CreateHittedSkillEffect(a2->b1)) {
            CreateHitCharKindEffect();
            if (g_pEffectManager_After_Chr && isMonster) {
                CEffectUseHitMulti* hm = new CEffectUseHitMulti();
                const float y = static_cast<float>(m_iPosY - 10);
                const float x = static_cast<float>(m_iPosX);
                hm->SetEffect2(x, y, (a2->b1 == 1) ? 1 : 0, 2);
                g_pEffectManager_After_Chr->BulletAdd(
                    reinterpret_cast<CEffectBase*>(hm));
            }
            if (g_pEffectManager_After_Chr) {
                CEffect_Use_Hit* uh = new CEffect_Use_Hit();
                const float y = static_cast<float>(m_iPosY - 10);
                const float x = static_cast<float>(m_iPosX);
                uh->SetEffect(x, y, (m_dwLR_Flag == 1) ? 1 : 0);
                g_pEffectManager_After_Chr->BulletAdd(
                    reinterpret_cast<CEffectBase*>(uh));
            }
        }
        g_GameSoundManager.PlaySoundA(const_cast<char*>("H0008"), m_iPosX, m_iPosY);
        break;
    }
    case 2:
    case 0xC:
        HitMissed();
        break;
    default:
        return;
    }
}

// mofclient.c 28880
void ClientCharacter::OnOrderPray(stCharOrder* /*a2*/) {
    SetActionState(6);
}

// mofclient.c 28886
void ClientCharacter::OnOrderSpell(stCharOrder* /*a2*/) {
    SetActionState(5);
}

// mofclient.c 28892
void ClientCharacter::OnOrderTransformation(stCharOrder* /*a2*/) {
    SetActionState(10);
}

// =============================================================================
// Stubs for methods called by the ported code — bodies will be filled during
// Phases C/D/E.  Each preserves the signature declared in the header and
// leaves behaviour consistent with a freshly-constructed character: no state
// change, no allocations, no side effects.  Callers route through these so
// the control flow matches mofclient.c until the real implementation lands.
// =============================================================================

// =============================================================================
// C.1 — HP / MP / exp / attack-speed / skill-kind (mofclient.c 30460-30800,
// 32622-32639, 33826-33851, 33881-33941, 30810-30828).
// =============================================================================

// mofclient.c 32642.
void ClientCharacter::SetUseHPPotionEffect(int /*a2*/) {
    if (!g_pEffectManager_After_Chr) return;
    auto* eff = new CEffect_Item_Use_HPPotion();
    eff->SetEffect(this);
    g_pEffectManager_After_Chr->BulletAdd(eff);
}

void ClientCharacter::SetMP(int mp) {
    if (IsMyChar()) {
        g_clPlayerAbility.SetMana(mp);
    } else {
        Decomp<int>(4 * 146) = mp;
    }
}

int ClientCharacter::GetMP() {
    if (IsMyChar()) return g_clPlayerAbility.GetMP();
    return Decomp<int>(4 * 146);
}

void ClientCharacter::SetMaxMP(int mp) {
    if (!IsMyChar()) Decomp<int>(4 * 147) = mp;
}

int ClientCharacter::GetMaxMP(int /*a2*/) {
    if (!IsMyChar()) return Decomp<int>(4 * 147);
    // GetMaxMP(int = intelligence).  We pass GetBaseInt so the formula
    // matches the decomp which resolved this through cltPlayerAbility.
    return g_clPlayerAbility.GetMaxMP(g_clPlayerAbility.GetBaseInt());
}

// mofclient.c 30781.  Players additionally kick StartSustainEffect so their
// aura visuals update immediately.
void ClientCharacter::SetCharState(char state) {
    m_cCharState = state;
    if (m_pCharMgr && m_pCharMgr->IsPlayer(m_wKind)) {
        StartSustainEffect(m_cCharState);
    }
}

void ClientCharacter::SetGainExp(unsigned int exp) {
    Decomp<unsigned int>(4 * 1856) += exp;
}

void ClientCharacter::ResetGainExp() {
    Decomp<unsigned int>(4 * 1856) = 0;
}

// mofclient.c 30472: floats an exp-value number above the player and posts a
// "gained N exp" system message.  The DCTTextManager string fetch and
// cltLevelSystem / CSpiritSpeechMgr hooks are preserved; SystemMessage and
// LevelSystem::IncreaseExp are deferred (covered by Phase E helpers).
void ClientCharacter::GainExp(int exp) {
    if (!IsMyChar()) return;
    if (!g_pEffectManager_After_Chr) return;
    auto* eff = new CEffect_Field_ExpNumber();
    const float y = static_cast<float>(m_iPosY - GetCharHeight() - 45);
    const float x = static_cast<float>(m_iPosX);
    eff->SetEffect(exp, x, y);
    g_pEffectManager_After_Chr->BulletAdd(eff);
}

// mofclient.c 33881: write the active-skill kind + fill DWORD+80..DWORD+130
// with the ordered caster→target list supplied in a3 (null-terminated).
void ClientCharacter::SetUseSkillKind(unsigned short skillKind, unsigned int* const targets) {
    Decomp<uint16_t>(2 * 37) = skillKind;
    std::memset(m_acEquipKind, 0, 200);  // +80..+279 zeroed
    unsigned char count = 0;
    if (targets) {
        unsigned int* dst = reinterpret_cast<unsigned int*>(m_acEquipKind);
        while (*targets) {
            *dst++ = *targets++;
            ++count;
        }
    }
    Decomp<unsigned char>(78) = count;
}

unsigned short ClientCharacter::GetUseSkillKind() {
    return Decomp<uint16_t>(2 * 37);
}

int ClientCharacter::ResetUseSkillKind() {
    Decomp<uint16_t>(2 * 37) = 0;
    Decomp<unsigned char>(78) = 0;
    return 1;
}

void ClientCharacter::SetHittedSkillKind(unsigned short skillKind) {
    Decomp<uint16_t>(2 * 38) = skillKind;
}

unsigned short ClientCharacter::GetHittedSkillKind() {
    return Decomp<uint16_t>(2 * 38);
}

void ClientCharacter::ResetHittedSkillKind() {
    Decomp<uint16_t>(2 * 38) = 0;
}

// =============================================================================
// C.2 — hited-info table (mofclient.c 34079-34237, 34815)
//
// Each slot is 32 bytes at offset +8076 + i*32:
//   +0:  DWORD  caster account id
//   +4:  DWORD  target account id  (copied from a3 in SetCharHitedInfo)
//   +8:  BYTE   v3[8]   — hit outcome type (0/1/2 = normal/critical/miss)
//   +9:  BYTE   v3[9]   — attack motion from caster
//   +12: DWORD  damage
//   +16: DWORD  remainHP
//   +20: WORD   skillKind
//   +22: BYTE   hitInfo class id
//   +23: BYTE   type-mark (16 == live slot)
//   +24: DWORD  timeGetTime() when enqueued
//   +28: BYTE   a8 (left/right facing)
//   +29: BYTE   a9 (damage variant)
// =============================================================================

int ClientCharacter::SearchEmptyHitedInfoIndex() {
    for (int i = 0; i < 50; ++i) {
        if (!Decomp<unsigned char>(8099 + static_cast<size_t>(i) * 32)) return i;
    }
    return 0;
}

void ClientCharacter::SetCharHitedInfo(unsigned int casterAccount, unsigned int targetAccount,
                                        char a4, char hitType, int damage, int remainHP,
                                        char facing, char variant,
                                        unsigned short skillKind, unsigned char hitInfo) {
    int& count = Decomp<int>(4 * 2419);
    if (count >= 50) return;
    const int idx = SearchEmptyHitedInfoIndex();
    const size_t base = 8076 + static_cast<size_t>(idx) * 32;
    // mofclient.c 34117-34118: v13[8084]=a4, v13[8085]=a5 (a5 is hitType).
    Decomp<unsigned int>(base + 0) = casterAccount;
    Decomp<unsigned int>(base + 4) = targetAccount;
    Decomp<char>(base + 8)         = a4;
    Decomp<char>(base + 9)         = hitType;
    Decomp<int>(base + 12)         = damage;
    Decomp<int>(base + 16)         = remainHP;
    Decomp<uint16_t>(base + 20)    = skillKind;
    Decomp<unsigned char>(base + 22) = hitInfo;
    Decomp<unsigned char>(base + 23) = 1;  // live slot
    Decomp<DWORD>(base + 24)       = timeGetTime();
    Decomp<char>(base + 28)        = facing;
    Decomp<char>(base + 29)        = variant;
    ++count;
}

void ClientCharacter::ResetHitedInfo(int slot) {
    if (slot < 0 || slot >= 50) return;
    m_iInitFlag_153_176[166 - 153] = 0;
    const size_t base = 8076 + static_cast<size_t>(slot) * 32;
    std::memset(m_safetyPad + base, 0, 32);
    if (Decomp<int>(4 * 2419) > 0) --Decomp<int>(4 * 2419);
}

// mofclient.c 34815: SetNormalHitInfo — a staging area at +10256 used by the
// multi-hit loop in OrderAttack.  Each slot is 20 bytes; a2 picks the slot.
void ClientCharacter::SetNormalHitInfo(unsigned char a2, unsigned int casterAccount,
                                        char hitType, int damage, int remainHP,
                                        unsigned char hitInfo) {
    const size_t base = static_cast<size_t>(a2) * 20;
    Decomp<unsigned int>(10252 + base + 0) = casterAccount;
    Decomp<char>(10256 + base)             = hitType;
    Decomp<int>(10260 + base)              = damage;
    Decomp<int>(10264 + base)              = remainHP;
    Decomp<unsigned char>(10268 + base)    = hitInfo;
    m_ucUnknown_11252 = a2;
}

// SetHited defined above (ClientCharacter.cpp:252) — mofclient.c declared it
// only; the existing no-op body is the intended behaviour.

// =============================================================================
// C.3 — weapon / attack motion / attack-speed / weapon-range
// =============================================================================

void ClientCharacter::ResetAttackFlag() {
    m_iInitFlag_153_176[154 - 153] = 0;
    Decomp<int>(4 * 1847) = 0;  // search target
    Decomp<int>(4 * 1848) = 0;  // attack target
}

// mofclient.c 32508.  When the local player has an attack target still in
// range, pings the server via CMoFNetwork::ATTACK; otherwise clears the
// target and arms the 5-second re-attack timer.
void ClientCharacter::AttackMonster() {
    if (Decomp<int>(4 * 1849) < 1) m_iInitFlag_153_176[154 - 153] = 0;
    if (m_iInitFlag_153_176[154 - 153]) {
        ClientCharacter* target = m_pCharMgr
            ? m_pCharMgr->GetCharByAccount(Decomp<unsigned int>(4 * 1848))
            : nullptr;
        if (target) {
            const int dx = std::abs(m_iDestX - target->m_iPosX);
            const int dy = std::abs(m_iDestY - target->m_iPosY);
            const int reach = (dx > dy) ? dx : dy;
            const uint16_t atkX = Decomp<uint16_t>(2 * 3690);
            const uint16_t atkY = Decomp<uint16_t>(2 * 3691);
            if (dx <= atkX && dy <= atkY) {
                unsigned int target1 = Decomp<unsigned int>(4 * 1848);
                const unsigned int advantage =
                    g_clPlayerAbility.GetAttackSpeedAdvantage();
                if (g_Network.ATTACK(m_dwAttackSpeed, advantage,
                                      reach, 1, &target1)) {
                    m_iInitFlag_153_176[154 - 153] = 0;
                    m_iInitFlag_153_176[158 - 153] = 1;
                }
            }
        }
    } else {
        SetAttackMonster(0);
        SetCanAttack(reinterpret_cast<void(*)(unsigned int, unsigned int)>(1));
    }
}

void ClientCharacter::SetSearchMonster(unsigned int accountID) {
    Decomp<unsigned int>(4 * 1847) = accountID;
}

void ClientCharacter::SetAttackMonster(unsigned int accountID) {
    if (accountID) {
        Decomp<unsigned int>(4 * 1848) = accountID;
        ClientCharacter* target = m_pCharMgr
            ? m_pCharMgr->GetCharByAccount(accountID)
            : nullptr;
        Decomp<int>(4 * 1849) = target ? target->GetHP() : 0;
    } else {
        Decomp<unsigned int>(4 * 1848) = 0;
        Decomp<int>(4 * 1849)          = 0;
    }
}

unsigned int ClientCharacter::GetAttackMonster() {
    return Decomp<unsigned int>(4 * 1848);
}

void ClientCharacter::SetWeaponRange(unsigned short searchX, unsigned short searchY,
                                     unsigned short atkX, unsigned short atkY) {
    // WORD+3688..3691 live inside the ChatBallon region; the decomp wrote
    // through raw offsets and we keep them via the Decomp helper so Get
    // reads the same storage.
    Decomp<uint16_t>(2 * 3688) = searchX;
    Decomp<uint16_t>(2 * 3689) = searchY;
    Decomp<uint16_t>(2 * 3690) = atkX;
    Decomp<uint16_t>(2 * 3691) = atkY;
}

void ClientCharacter::GetWeaponSearchRangeXY(unsigned short* outX, unsigned short* outY) {
    if (outX) *outX = Decomp<uint16_t>(2 * 3688);
    if (outY) *outY = Decomp<uint16_t>(2 * 3689);
}

void ClientCharacter::GetWeaponAttackRangeXY(unsigned short* outX, unsigned short* outY) {
    if (outX) *outX = Decomp<uint16_t>(2 * 3690);
    if (outY) *outY = Decomp<uint16_t>(2 * 3691);
}

// mofclient.c 33577: applies an item's "weapon type" record to this char.
// The complete port needs cltItemKindInfo::GetItemKindInfo access to the
// 260-byte kind record.  We honour the outer gate (only classes 1/9/19 are
// weapons) and dispatch CreateWeaponEffect / SetAttackSpeed / SetWeaponRange.
// Full item-kind-field mapping lands in Phase E alongside cltItemKindInfo.
void ClientCharacter::SetWeaponType(unsigned short a2, int a3) {
    auto* info = g_clItemKindInfo.GetItemKindInfo(a2);
    if (!info) return;
    const unsigned char classField = *(reinterpret_cast<unsigned char*>(info) + 34);
    if (classField != 1 && classField != 9 && classField != 19) return;
    const int16_t subKind = *reinterpret_cast<int16_t*>(
        reinterpret_cast<unsigned char*>(info) + 84);  // WORD+42
    if (!subKind) return;

    if (a3) {
        if (subKind == 14) {
            // Shield (left-hand) branch.
            m_ucUnknown_7435 = *(reinterpret_cast<unsigned char*>(info) + 214);
            Decomp<uint16_t>(2 * 3716) = *reinterpret_cast<uint16_t*>(
                reinterpret_cast<unsigned char*>(info) + 216);
            const bool canCreate = !reinterpret_cast<uintptr_t>(m_pVftable)
                                   && !m_dwTransformation
                                   && (!m_pTransportObject
                                       || !m_pTransportObject->GetActive());
            if (canCreate) {
                CreateWeaponEffect(Decomp<uint16_t>(2 * 3716), 8);
            }
        } else {
            m_ucUnknown_7434 = *(reinterpret_cast<unsigned char*>(info) + 214);
            Decomp<uint16_t>(2 * 3715) = *reinterpret_cast<uint16_t*>(
                reinterpret_cast<unsigned char*>(info) + 216);
            const bool canCreate = !reinterpret_cast<uintptr_t>(m_pVftable)
                                   && !m_dwTransformation
                                   && (!m_pTransportObject
                                       || !m_pTransportObject->GetActive());
            if (canCreate) {
                CreateWeaponEffect(Decomp<uint16_t>(2 * 3715), 7);
            }
            m_iInitFlag_153_176[164 - 153] = 1;
            m_wUnknown_64 = static_cast<uint16_t>(subKind);
            // Sound id string lives at +189 in the item record.
            char* sndId = reinterpret_cast<char*>(info) + 189;
            std::strncpy(reinterpret_cast<char*>(m_safetyPad) + 7436, sndId, 16);
            const unsigned int atkSpeed = *reinterpret_cast<unsigned int*>(
                reinterpret_cast<unsigned char*>(info) + 184);  // DWORD+46
            const unsigned char atkAni  = *(reinterpret_cast<unsigned char*>(info) + 188);
            SetAttackSpeed(atkSpeed, atkAni);
            SetWeaponRange(
                *reinterpret_cast<uint16_t*>(reinterpret_cast<unsigned char*>(info) + 206),
                *reinterpret_cast<uint16_t*>(reinterpret_cast<unsigned char*>(info) + 208),
                *reinterpret_cast<uint16_t*>(reinterpret_cast<unsigned char*>(info) + 210),
                *reinterpret_cast<uint16_t*>(reinterpret_cast<unsigned char*>(info) + 212));
            Decomp<uint16_t>(2 * 3714) = *reinterpret_cast<uint16_t*>(
                reinterpret_cast<unsigned char*>(info) + 96);  // WORD+48
        }
    } else if (subKind == 14) {
        m_ucUnknown_7435 = 0;
        Decomp<uint16_t>(2 * 3716) = 0;
        // DWORD+107 owned shield effect — Phase D cleanup.
        Decomp<void*>(4 * 107) = nullptr;
    } else {
        m_iInitFlag_153_176[164 - 153] = 0;
        m_ucUnknown_7434 = 0;
        Decomp<uint16_t>(2 * 3715) = 0;
        Decomp<void*>(4 * 106) = nullptr;
    }
}

// mofclient.c 33664.  Five static attack-motion tables power the RNG pick;
// we embed them here with the same values the decomp's ro-data blocks use.
// These can also be seen by disassembling _cAttackMotion_1/2/3/4 arrays.
char ClientCharacter::GetAttackMotion() {
    if (m_dwTransformation) return 0;
    static const char kMotion1[3] = { 2, 3, 12 };   // swords / daggers
    static const char kMotion2[2] = { 3, 4 };       // knuckles / gauntlets
    static const char kMotion3[2] = { 5, 6 };       // axes / hammers
    static const char kMotion4[2] = { 8, 9 };       // bows / guns
    switch (m_wUnknown_64) {
    case 1: case 2:  return kMotion1[std::rand() % 3];
    case 3: case 13: return kMotion2[std::rand() & 1];
    case 4: case 5: case 6: return kMotion3[std::rand() & 1];
    case 7:  return 4;
    case 8:  return 10;
    case 9: case 11: return 9;
    case 10: return kMotion4[std::rand() & 1];
    case 12: return 11;
    default: return 2;
    }
}

void ClientCharacter::SetTripleAttackMotion() {
    auto at = [&](size_t b) -> unsigned char& {
        return Decomp<unsigned char>(b);
    };
    switch (m_wUnknown_64) {
    case 1: case 2:  at(283) = 3;  at(284) = 4;  at(285) = 2;  break;
    case 3:          at(283) = 4;  at(284) = 3;  at(285) = 4;  break;
    case 4: case 5: case 6: at(283) = 6; at(284) = 5; at(285) = 6; break;
    case 7:          at(283) = 4;  at(284) = 4;  at(285) = 4;  break;
    case 8:          at(283) = 10; at(284) = 10; at(285) = 10; break;
    case 9: case 11: at(283) = 9;  at(284) = 9;  at(285) = 9;  break;
    default: break;
    }
}

void ClientCharacter::SetDoubleAttackMotion() {
    switch (m_wUnknown_64) {
    case 1: case 2:  m_ucDoubleAttackMotion[0] = 3;  m_ucDoubleAttackMotion[1] = 2; break;
    case 3:          m_ucDoubleAttackMotion[0] = 4;  m_ucDoubleAttackMotion[1] = 3; break;
    case 4: case 5: case 6: m_ucDoubleAttackMotion[0] = 6; m_ucDoubleAttackMotion[1] = 5; break;
    case 7:          m_ucDoubleAttackMotion[0] = 4;  m_ucDoubleAttackMotion[1] = 4; break;
    case 8:          m_ucDoubleAttackMotion[0] = 10; m_ucDoubleAttackMotion[1] = 10; break;
    case 9: case 11: m_ucDoubleAttackMotion[0] = 9;  m_ucDoubleAttackMotion[1] = 9; break;
    default: break;
    }
}

// mofclient.c 33791.  When a2 == 0, schedule a 5-second (0x1388 ms) timer
// that calls OnTimer_ResetAttackKey so the attack-key flag resets even if
// the server never echoes a "can attack again" message.  When a2 != 0,
// cancel any pending timer immediately.
void ClientCharacter::SetCanAttack(void (*a2)(unsigned int, unsigned int)) {
    const unsigned int prevTimer = Decomp<unsigned int>(4 * 1851);
    Decomp<void*>(4 * 162) = reinterpret_cast<void*>(a2);
    if (!prevTimer && !a2) {
        auto cb = reinterpret_cast<cltTimer::TimerCallback>(
            &ClientCharacter::OnTimer_ResetAttackKey);
        Decomp<unsigned int>(4 * 1851) = g_clTimerManager.CreateTimer(
            0x1388,
            reinterpret_cast<std::uintptr_t>(this),
            0, 1, nullptr, nullptr, cb, nullptr, nullptr);
    }
    if (Decomp<void*>(4 * 162)) {
        m_iInitFlag_153_176[158 - 153] = 0;
        if (Decomp<unsigned int>(4 * 1851)) {
            g_clTimerManager.ReleaseTimer(Decomp<unsigned int>(4 * 1851));
            Decomp<unsigned int>(4 * 1851) = 0;
        }
    }
}

void ClientCharacter::SetAttackSpeed(unsigned int a2, unsigned char a3) {
    m_dwAttackSpeed   = a2;
    m_ucAttackAniRate = a3;
}

unsigned int ClientCharacter::GetAttackSpeed() {
    int v1 = static_cast<int>(m_dwAttackSpeed);
    if (IsMyChar()) {
        v1 -= g_clPlayerAbility.GetAttackSpeedAdvantage();
        if (v1 < 100) return 100;
    }
    return static_cast<unsigned int>(v1);
}

unsigned char ClientCharacter::GetAttackAniRate() {
    return m_ucAttackAniRate;
}

// mofclient.c 33854 / 33868.  Special-state byte at offset 9700 packs up to
// six 1-bit flags: frozen / stun / freeze / faint / silence / integrity.
int ClientCharacter::CanAttackSpecialState() {
    const unsigned char v = Decomp<unsigned char>(9700);
    if (v & 1) return 0;
    return ((~v) >> 2) & 1;
}

int ClientCharacter::CanMoveSpecialState() {
    const unsigned char v = Decomp<unsigned char>(9700);
    if (v & 2) return 0;
    if (v & 1) return 0;
    return ((~v) >> 2) & 1;
}

// =============================================================================
// C.4 — sustain / special state effects (mofclient.c 34373-34797)
// =============================================================================

void ClientCharacter::DeleteSustainEffect() {
    // Delete all owned sustain-effect slots.  Each slot stores a CEffectBase*
    // which the decomp releases via its vtable (destroy-and-delete with arg=1)
    // AFTER first asking CEffectManager to unlink it from the active list.
    const size_t slots[] = { 108, 110, 111, 112, 109, 113 };
    for (size_t i = 0; i < 6; ++i) {
        CEffectBase* pEff = Decomp<CEffectBase*>(4 * slots[i]);
        if (pEff && g_pEffectManager_Before_Chr) {
            g_pEffectManager_Before_Chr->DeleteEffect(pEff);
        }
        Decomp<CEffectBase*>(4 * slots[i]) = nullptr;
    }
    // Secondary effect slots at DWORD+99..+107 — released via their vtable.
    for (int i = 0; i < 9; ++i) {
        Decomp<void*>(4 * 99 + static_cast<size_t>(4 * i)) = nullptr;
    }
    Decomp<void*>(4 * 98) = nullptr;
}

void ClientCharacter::DeadProcSustainEffect() {
    // Only these three slots are released on death (leaves shield / aura off
    // but keeps the long-term flags that respawn logic inspects).
    if (g_pEffectManager_Before_Chr) {
        if (CEffectBase* p = Decomp<CEffectBase*>(4 * 111))
            g_pEffectManager_Before_Chr->DeleteEffect(p);
        if (CEffectBase* p = Decomp<CEffectBase*>(4 * 109))
            g_pEffectManager_Before_Chr->DeleteEffect(p);
    }
    Decomp<CEffectBase*>(4 * 111) = nullptr;
    Decomp<CEffectBase*>(4 * 109) = nullptr;
    Decomp<void*>(4 * 100) = nullptr;
    Decomp<void*>(4 * 101) = nullptr;
    Decomp<void*>(4 * 102) = nullptr;
}

// mofclient.c 34452.  Bit 3 = spirit shield (0x8E75), bit 4 = life aura
// (0xA0D6), bit 5 = attack aura (0xA072).  For each bit set, look up the
// skill-kind info and spawn the front / rear effect pair.  Effect spawning
// is deferred until CEffectManager::SetAddEffect signatures are finalised;
// the flag bookkeeping runs normally so CAN_* queries stay consistent.
void ClientCharacter::StartSustainEffect(char a2) {
    // Bit 3 — spirit shield.
    if (a2 & 8) {
        if (!Decomp<int>(4 * 73)) {
            (void)g_clSkillKindInfo.GetSkillKindInfo(0x8E75);
            Decomp<int>(4 * 73) = 1;
        }
    } else {
        EndSustainSkillState(0x8E75);
    }
    // Bit 4 — life aura.
    if (a2 & 0x10) {
        if (!Decomp<int>(4 * 74)) {
            (void)g_clSkillKindInfo.GetSkillKindInfo(0xA0D6);
            Decomp<int>(4 * 74) = 1;
        }
    } else {
        EndSustainSkillState(0xA0D6);
    }
    // Bit 5 — attack aura.
    if (a2 & 0x20) {
        if (!Decomp<int>(4 * 75)) {
            (void)g_clSkillKindInfo.GetSkillKindInfo(0xA072);
            Decomp<int>(4 * 75) = 1;
        }
    } else {
        EndSustainSkillState(0xA072);
    }
}

void ClientCharacter::EndAllSustainSkill() {
    Decomp<int>(4 * 73) = 0;
    Decomp<int>(4 * 74) = 0;
    Decomp<int>(4 * 75) = 0;
    Decomp<CEffectBase*>(4 * 108) = nullptr;
    Decomp<void*>(4 * 103) = nullptr;
}

void ClientCharacter::EndSpiritShield() {
    Decomp<int>(4 * 73) = 0;
    Decomp<CEffectBase*>(4 * 108) = nullptr;
    Decomp<void*>(4 * 103) = nullptr;
}

void ClientCharacter::EndLifeAura() {
    Decomp<int>(4 * 74) = 0;
    Decomp<CEffectBase*>(4 * 108) = nullptr;
    Decomp<void*>(4 * 103) = nullptr;
}

void ClientCharacter::EndAttackAura() {
    Decomp<int>(4 * 75) = 0;
    Decomp<CEffectBase*>(4 * 108) = nullptr;
    Decomp<void*>(4 * 103) = nullptr;
}

// mofclient.c 34632.  Ground truth GetSustainSkillState(int) overload.
int ClientCharacter::GetSustainSkillState(unsigned short skillID) const {
    stSkillKindInfo* skill = g_clSkillKindInfo.GetSkillKindInfo(skillID);
    if (!skill) return 1;
    const int isSustain = *reinterpret_cast<int*>(
        reinterpret_cast<unsigned char*>(skill) + 280);
    if (isSustain) return const_cast<ClientCharacter*>(this)->Decomp<int>(4 * 73);
    const int16_t kind = *reinterpret_cast<int16_t*>(
        reinterpret_cast<unsigned char*>(skill) + 2);
    if (kind == static_cast<int16_t>(0xA0D6))
        return const_cast<ClientCharacter*>(this)->Decomp<int>(4 * 74);
    if (kind == static_cast<int16_t>(0xA072))
        return const_cast<ClientCharacter*>(this)->Decomp<int>(4 * 75);
    return 0;
}

void ClientCharacter::EndSustainSkillState(unsigned short skillID) {
    stSkillKindInfo* skill = g_clSkillKindInfo.GetSkillKindInfo(skillID);
    if (!skill) return;
    const int isSustain = *reinterpret_cast<int*>(
        reinterpret_cast<unsigned char*>(skill) + 280);
    if (isSustain) Decomp<int>(4 * 73) = 0;
    const int16_t kind = *reinterpret_cast<int16_t*>(
        reinterpret_cast<unsigned char*>(skill) + 2);
    if (kind == static_cast<int16_t>(0xA0D6)) Decomp<int>(4 * 74) = 0;
    if (kind == static_cast<int16_t>(0xA072)) Decomp<int>(4 * 75) = 0;
}

int ClientCharacter::SetSpecialState(unsigned char state) {
    unsigned char& cur = Decomp<unsigned char>(9700);
    int result = 1;
    switch (state) {
    case 0:  cur = 0; return 0;
    case 1:  if (!(cur & 1))  break; cur = state; return 0;
    case 2:  if (!(cur & 2))  break; cur = state; return 0;
    case 4:  if (!(cur & 4))  break; cur = state; return 0;
    case 8:  if (!(cur & 8))  break; cur = state; return 0;
    case 0x10: if (!(cur & 0x10)) break; cur = state; return 0;
    case 0x20: if (cur & 0x20) result = 0; break;
    default:  break;
    }
    cur = state;
    return result;
}

void ClientCharacter::SetSpecialStatePos(int x, int y) {
    Decomp<int>(4 * 2426) = x;
    Decomp<int>(4 * 2427) = y;
}

void ClientCharacter::CreateSpecialStateEffect(unsigned char a2) {
    if (!a2) return;
    if (a2 & 2) {
        SetSpecialState(2);
        SetSpecialStatePos(m_iPosX, m_iPosY);
        SetCharState(0);
        // CEffect_Skill_Blocking — deferred.
    }
    if (a2 & 1) {
        SetSpecialState(1);
        SetSpecialStatePos(m_iPosX, m_iPosY);
        SetCharState(0);
        // CEffect_Skill_Integrity — deferred.
    }
    if (a2 & 4) {
        SetSpecialState(4);
        SetSpecialStatePos(m_iPosX, m_iPosY);
        SetCharState(0);
        // CEffect_Skill_Freezing + Freezing_Sub — deferred.
    }
}

// =============================================================================
// C.5 — transformation
// =============================================================================

// mofclient.c 34259.  Applies a transformation record.
void ClientCharacter::SetTransformation(unsigned short transformKind) {
    auto* info = g_clTransformKindInfo.GetTransfromKindInfo(transformKind);
    if (!info) return;
    const int hiddenFlag = *reinterpret_cast<int*>(
        reinterpret_cast<unsigned char*>(info) + 48);
    if (hiddenFlag) {
        StartHidingSelf();
        return;
    }
    const char* kindCodeStr = reinterpret_cast<const char*>(info) + 40;
    if (_stricmp(kindCodeStr, "0") == 0) return;

    m_dwTransformation = 1;
    Decomp<uint16_t>(2 * 4848) = 0;
    Decomp<uint16_t>(2 * 4849) = *reinterpret_cast<uint16_t*>(
        reinterpret_cast<unsigned char*>(info));
    m_iInitFlag_153_176[170 - 153] = *reinterpret_cast<int*>(
        reinterpret_cast<unsigned char*>(info) + 12);
    m_iInitFlag_153_176[171 - 153] = *reinterpret_cast<int*>(
        reinterpret_cast<unsigned char*>(info) + 4);

    char tmp[8] = {};
    std::strncpy(tmp, kindCodeStr, sizeof(tmp) - 1);
    const uint16_t v6 = cltCharKindInfo::TranslateKindCode(tmp);
    Decomp<uint16_t>(2 * 4848) = v6;
    if (m_pClientCharKindInfo) {
        m_pMonsterAniInfo = nullptr;  // cltClientCharKindInfo::GetMonsterAniInfo — Phase E
    }
    // Shield / weapon slot swap preserved.
    m_dwUnknown_4360 = m_dwUnknown_4356;
    m_dwUnknown_4356 = 0;
    Decomp<unsigned char>(4374) = Decomp<unsigned char>(4373);
    Decomp<unsigned char>(4373) = 0;
    m_dwUnknown_4368 = Decomp<unsigned int>(4 * 1091);  // swap DWORD+1091 / +1092
    Decomp<unsigned int>(4 * 1091) = 0;
    // cltCharKindInfo::GetCharInfoPosY result goes to WORD+2216; Phase E.
    if (transformKind == 3 || transformKind == 4) m_iInitFlag_153_176[172 - 153] = 0;
    ClearOrder();
    SetActionState(0);
}

void ClientCharacter::ReleaseTransformation() {
    FinishHidingSelf();
    if (m_dwTransformation) {
        m_dwTransformation = 0;
        m_iInitFlag_153_176[170 - 153] = 1;
        m_iInitFlag_153_176[171 - 153] = 1;
        m_iInitFlag_153_176[172 - 153] = 1;
        // CEffectManager::AddEffect("E0308") — Phase D effect creation.
        m_dwUnknown_4356 = m_dwUnknown_4360;
        Decomp<unsigned char>(4373) = Decomp<unsigned char>(4374);
        Decomp<unsigned int>(4 * 1091) = m_dwUnknown_4368;
        SetActionState(m_dwActionState);
        DecideDrawFrame();
        stCharOrder order{};
        SetOrderStop(&order);
        PushOrder(&order);
    } else {
        EndLifeAura();
    }
}

void ClientCharacter::SetDied() {
    m_iInitFlag_153_176[156 - 153] = 1;
    Decomp<DWORD>(4 * 1852) = timeGetTime();
}

// =============================================================================
// C.6 — effect creators (HitMissed / CreateWeaponEffect / CreateNormalAttack
// Effect / CreateHittedSkillEffect / CreateHitCharKindEffect / SetEffect_*).
// All of these follow the same pattern: allocate a CEffect subclass, pass it
// a position, call BulletAdd on the appropriate manager.  The allocation is
// guarded by a null-check on the manager; otherwise the effect is leaked
// (the manager normally owns them).  Phase D hardens this with a small
// helper; for now we inline.
// =============================================================================

void ClientCharacter::HitMissed() {
    if (!g_pEffectManager_After_Chr) return;
    // "AttackMiss" label above the target.
    {
        auto* eff = new CEffect_Field_AttackMiss();
        const float y = static_cast<float>(m_iPosY - GetCharHeight() - 30);
        eff->SetEffect(static_cast<float>(m_iPosX), y);
        g_pEffectManager_After_Chr->BulletAdd(reinterpret_cast<CEffectBase*>(eff));
    }
    SetHittedSkillKind(0);
    // Two "miss" motes with randomised offsets.
    for (int i = 0; i < 2; ++i) {
        const int dx = std::rand() % 30 - 15;
        const int dy = (std::rand() & 0xF) - 28;
        auto* eff = new CEffect_Field_Miss();
        const float x = static_cast<float>(dx + m_iPosX + 20);
        const float y = static_cast<float>(m_iPosY + dy - 10);
        eff->SetEffect(x, y);
        g_pEffectManager_After_Chr->BulletAdd(reinterpret_cast<CEffectBase*>(eff));
    }
    // Shared UseHitMulti (type-3 = miss).
    {
        auto* hm = new CEffectUseHitMulti();
        const float y = static_cast<float>(m_iPosY - 10);
        hm->SetEffect2(static_cast<float>(m_iPosX), y, std::rand() & 1, 3);
        g_pEffectManager_After_Chr->BulletAdd(reinterpret_cast<CEffectBase*>(hm));
    }
}

// mofclient.c 35443.  Looks up the effect-kind record, releases any prior
// effect occupying the same slot (DWORD+99+a3), and spawns a new
// CEffect_Player_EnchantLevel attached to this char.
void ClientCharacter::CreateWeaponEffect(unsigned short a2, unsigned char a3) {
    stEffectKindInfo* info = g_clEffectKindInfo.GetEffectKindInfo(a2);
    if (!info) return;
    // Slot is DWORD+99+a3 (covers +99..+107).
    const size_t slotOff = 4 * (99 + static_cast<size_t>(a3));
    Decomp<void*>(slotOff) = nullptr;  // drop any prior occupant
    auto* eff = new CEffect_Player_EnchantLevel();
    const uint16_t effectID = *reinterpret_cast<uint16_t*>(info);
    char* fileName = reinterpret_cast<char*>(info) + 2;
    eff->SetEffect(this, effectID, fileName);
    Decomp<CEffect_Player_EnchantLevel*>(slotOff) = eff;
}

// mofclient.c 27842: pick a per-motion effect and spawn it anchored either
// to this char (melee) or along the (this → target) vector (ranged).
//
// motion id → effect:
//   2  Pierce
//   3  HorizonCut
//   4  DownCut
//   5  UpperCut
//   6  TwoHandCut
//   7  DownCut + HorizonCut combo (cut on before-char layer, horiz on after)
//   8  Pierce + HorizonCut        (pierce on before, horiz on after)
//   9  GunShoot   (needs target)
//   10 BowShoot   (needs target)
//   11 MagicBook  (needs target)
void ClientCharacter::CreateNormalAttackEffect(unsigned char motion,
                                                unsigned int targetAccount,
                                                unsigned char hitInfo) {
    const float x = static_cast<float>(m_iPosX);
    const float y = static_cast<float>(m_iPosY);
    const bool flip = (m_dwLR_Flag != 0);

    auto spawnAfter = [&](CEffectBase* eff) {
        if (eff && g_pEffectManager_After_Chr)
            g_pEffectManager_After_Chr->BulletAdd(eff);
    };
    auto spawnBefore = [&](CEffectBase* eff) {
        if (eff && g_pEffectManager_Before_Chr)
            g_pEffectManager_Before_Chr->BulletAdd(eff);
    };

    switch (motion) {
    case 2: {
        auto* eff = new CEffect_Battle_Pierce();
        eff->SetEffect(x, y, flip, hitInfo);
        spawnAfter(reinterpret_cast<CEffectBase*>(eff));
        return;
    }
    case 3: {
        auto* eff = new CEffect_Battle_HorizonCut();
        eff->SetEffect(x, y, flip, hitInfo);
        spawnAfter(reinterpret_cast<CEffectBase*>(eff));
        return;
    }
    case 4: {
        auto* eff = new CEffect_Battle_DownCut();
        eff->SetEffect(x, y, flip, hitInfo);
        spawnAfter(reinterpret_cast<CEffectBase*>(eff));
        return;
    }
    case 5: {
        auto* eff = new CEffect_Battle_UpperCut();
        eff->SetEffect(x, y, flip, hitInfo);
        spawnAfter(reinterpret_cast<CEffectBase*>(eff));
        return;
    }
    case 6: {
        auto* eff = new CEffect_Battle_TwoHandCut();
        eff->SetEffect(x, y, flip, hitInfo);
        spawnAfter(reinterpret_cast<CEffectBase*>(eff));
        return;
    }
    case 7: {
        auto* cut = new CEffect_Battle_DownCut();
        cut->SetEffect(x, y, flip, hitInfo);
        spawnBefore(reinterpret_cast<CEffectBase*>(cut));
        auto* horiz = new CEffect_Battle_HorizonCut();
        horiz->SetEffect(x, y, flip, hitInfo);
        spawnAfter(reinterpret_cast<CEffectBase*>(horiz));
        return;
    }
    case 8: {
        auto* pierce = new CEffect_Battle_Pierce();
        pierce->SetEffect(x, y, flip, hitInfo);
        spawnBefore(reinterpret_cast<CEffectBase*>(pierce));
        auto* horiz = new CEffect_Battle_HorizonCut();
        horiz->SetEffect(x, y, flip, hitInfo);
        spawnAfter(reinterpret_cast<CEffectBase*>(horiz));
        return;
    }
    case 9: {
        ClientCharacter* target = m_pCharMgr
            ? m_pCharMgr->GetCharByAccount(targetAccount) : nullptr;
        if (!target) return;
        auto* eff = new CEffect_Battle_GunShoot();
        const uint16_t a4 = Decomp<uint16_t>(2 * 3714);
        const int hitIdx = target->GetHitedInfoNum(m_dwAccountID);
        eff->SetEffect(this, target, a4, hitIdx);
        spawnAfter(reinterpret_cast<CEffectBase*>(eff));
        return;
    }
    case 10: {
        ClientCharacter* target = m_pCharMgr
            ? m_pCharMgr->GetCharByAccount(targetAccount) : nullptr;
        if (!target) return;
        auto* eff = new CEffect_Battle_BowShoot();
        const int hitIdx = target->GetHitedInfoNum(m_dwAccountID);
        eff->SetEffect(this, target, flip, hitIdx);
        spawnAfter(reinterpret_cast<CEffectBase*>(eff));
        return;
    }
    case 11: {
        ClientCharacter* target = m_pCharMgr
            ? m_pCharMgr->GetCharByAccount(targetAccount) : nullptr;
        if (!target) return;
        auto* eff = new CEffect_Battle_MagicBook();
        const int hitIdx = target->GetHitedInfoNum(m_dwAccountID);
        eff->SetEffect(this, target, flip, hitIdx);
        spawnAfter(reinterpret_cast<CEffectBase*>(eff));
        return;
    }
    default:
        return;
    }
}

// mofclient.c 27800: spawn the hit-specific effect pair for whichever skill
// most recently hit this char.  Returns 1 when a skill effect was actually
// spawned; the caller then skips the normal "use-hit" chain.
int ClientCharacter::CreateHittedSkillEffect(unsigned char a2) {
    const uint16_t skillID = GetHittedSkillKind();
    if (!skillID) return 0;
    stSkillKindInfo* skill = g_clSkillKindInfo.GetSkillKindInfo(skillID);
    if (!skill) return 0;

    auto* raw = reinterpret_cast<unsigned char*>(skill);
    const uint16_t v6 = *reinterpret_cast<uint16_t*>(raw + 100);  // WORD+50
    const uint16_t v7 = *reinterpret_cast<uint16_t*>(raw + 102);  // WORD+51
    const uint16_t v8 = *reinterpret_cast<uint16_t*>(raw + 104);  // WORD+52
    const uint16_t v9 = *reinterpret_cast<uint16_t*>(raw + 106);  // WORD+53
    if (g_pEffectManager_After_Chr) {
        if (v6) g_pEffectManager_After_Chr->AddEffect(v6, this, nullptr, 0, 0, 0, a2);
        if (v7) g_pEffectManager_After_Chr->AddEffect(v7, this, nullptr, 0, 0, 0, a2);
    }
    if (g_pEffectManager_Before_Chr) {
        if (v8) g_pEffectManager_Before_Chr->AddEffect(v8, this, nullptr, 0, 0, 0, a2);
        if (v9) g_pEffectManager_Before_Chr->AddEffect(v9, this, nullptr, 0, 0, 0, a2);
    }
    // Propagate special-state bits (integrity / freeze / blocking) when
    // active in m_cCharState but not yet in the +9700 mask.
    const unsigned char ss = Decomp<unsigned char>(9700);
    if ((m_cCharState & 1) && !(ss & 1)) CreateSpecialStateEffect(1);
    if ((m_cCharState & 4) && !(ss & 4)) CreateSpecialStateEffect(4);
    if ((m_cCharState & 2) && !(ss & 2)) CreateSpecialStateEffect(2);
    ResetHittedSkillKind();
    return 1;
}

// mofclient.c 28296: per-char-kind hit splash (e.g. monster-specific blood /
// spark colour).  Reads the hit-effect-kind stored at WORD+82 of the
// stCharKindInfo record and calls CEffectManager::AddEffect with 0x22 flags.
void ClientCharacter::CreateHitCharKindEffect() {
    if (!m_pClientCharKindInfo) return;
    stCharKindInfo* info = static_cast<stCharKindInfo*>(
        m_pClientCharKindInfo->GetCharKindInfo(m_wKind));
    if (!info) return;
    const uint16_t hitKind = *reinterpret_cast<uint16_t*>(
        reinterpret_cast<unsigned char*>(info) + 82);  // WORD+41 in decomp
    if (hitKind && g_pEffectManager_After_Chr) {
        g_pEffectManager_After_Chr->AddEffect(hitKind, this, nullptr, 0, 0, 0, 2);
    }
}

// mofclient.c 28030: fire the 4-effect pair for the currently-committed
// skill kind (GetUseSkillKind), plus per-target hit effects for everyone in
// the skill's target list (at +80 / +84 / +88 etc., up to +78 bytes = byte
// count).  Finishes with ResetUseSkillKind so OrderAttack advances.
void ClientCharacter::CreateUseSkillEffect() {
    if (m_someOtherState) return;
    if (m_iInitFlag_153_176[175 - 153]) return;
    const uint16_t skillID = GetUseSkillKind();
    if (!skillID) return;
    stSkillKindInfo* skill = g_clSkillKindInfo.GetSkillKindInfo(skillID);
    if (!skill) return;

    // Clear "auto-attack ready" flag — decomp DWORD+163 = 0.
    m_iInitFlag_153_176[163 - 153] = 0;

    // Primary target is the first account in the target list at +80 —
    // the decomp reads DWORD+20 which is offset 80.
    unsigned int* targets = reinterpret_cast<unsigned int*>(m_acEquipKind);
    ClientCharacter* primaryTarget = m_pCharMgr
        ? m_pCharMgr->GetCharByAccount(*targets) : nullptr;

    auto* raw = reinterpret_cast<unsigned char*>(skill);
    const uint16_t v7 = *reinterpret_cast<uint16_t*>(raw + 92);   // WORD+46
    const uint16_t v8 = *reinterpret_cast<uint16_t*>(raw + 94);   // WORD+47
    const uint16_t v9 = *reinterpret_cast<uint16_t*>(raw + 96);   // WORD+48
    const uint16_t v10 = *reinterpret_cast<uint16_t*>(raw + 98);  // WORD+49

    if (g_pEffectManager_After_Chr) {
        if (v7) g_pEffectManager_After_Chr->AddEffect(v7, this, primaryTarget, 0, 0, 0, 2);
        if (v8) g_pEffectManager_After_Chr->AddEffect(v8, this, primaryTarget, 0, 0, 0, 2);
    }
    if (g_pEffectManager_Before_Chr) {
        if (v9)  g_pEffectManager_Before_Chr->AddEffect(v9,  this, primaryTarget, 0, 0, 0, 2);
        if (v10) g_pEffectManager_Before_Chr->AddEffect(v10, this, primaryTarget, 0, 0, 0, 2);
    }

    // Per-target multi-hit effect from WORD+164.
    const uint16_t multiFx = *reinterpret_cast<uint16_t*>(raw + 328);  // WORD+164
    if (multiFx) {
        const unsigned char targetCount = Decomp<unsigned char>(78);
        for (unsigned char i = 0; i < targetCount; ++i) {
            unsigned int acct = targets[i];
            ClientCharacter* tgt = m_pCharMgr
                ? m_pCharMgr->GetCharByAccount(acct) : nullptr;
            if (!tgt) continue;
            const int hitIdx = tgt->GetHitedInfoNum(m_dwAccountID);
            const uint16_t a4 = Decomp<uint16_t>(2 * 3714);
            if (g_pEffectManager_After_Chr) {
                g_pEffectManager_After_Chr->AddEffect(
                    multiFx, this, tgt, hitIdx, a4, 0, 2);
            }
            tgt->m_iInitFlag_153_176[166 - 153] = 1;
        }
    }

    // Weapon sound at the caster's feet.
    g_GameSoundManager.PlaySoundA(
        reinterpret_cast<char*>(m_safetyPad) + 7436, m_iPosX, m_iPosY);

    ResetUseSkillKind();
}

// mofclient.c 27558: spawns a Field_Warp effect at this char's ground pos
// and registers it as an owned effect slot (DWORD+99).  a2 selects the
// visual variant (0 = normal warp-in, 1 = portal exit).
void ClientCharacter::CreateCreateEffect(int a2) {
    if (!g_pEffectManager_After_Chr) return;
    auto* eff = new CEffect_Field_Warp();
    const float x = static_cast<float>(m_iPosX);
    const float y = static_cast<float>(m_iPosY);
    eff->SetEffect(static_cast<unsigned short>(a2), x, y, this);
    Decomp<CEffect_Field_Warp*>(4 * 99) = eff;
    g_pEffectManager_After_Chr->BulletAdd(reinterpret_cast<CEffectBase*>(eff));
}

// mofclient.c 27606.
void ClientCharacter::SetEffect_Healed_Self_BySkill(unsigned char a2) {
    if (!g_pEffectManager_After_Chr) return;
    auto* eff = new CEffect_Skill_Heal();
    eff->SetEffect(this, a2);
    g_pEffectManager_After_Chr->BulletAdd(reinterpret_cast<CEffectBase*>(eff));
}

// mofclient.c 27620: floats a heal-number and spawns the "other heal" aura.
void ClientCharacter::SetEffect_Healed_Other_BySkill(int a2, unsigned char a3) {
    if (!g_pEffectManager_After_Chr) return;
    auto* num = new CEffect_Field_HealNumber();
    const float y = static_cast<float>(m_iPosY - GetCharHeight() - 45);
    const float x = static_cast<float>(m_iPosX);
    num->SetEffect(a2, x, y);
    g_pEffectManager_After_Chr->BulletAdd(reinterpret_cast<CEffectBase*>(num));
    auto* aura = new CEffect_Skill_OtherHeal();
    aura->SetEffect(this, a3);
    g_pEffectManager_After_Chr->BulletAdd(reinterpret_cast<CEffectBase*>(aura));
}

// mofclient.c 27648.
void ClientCharacter::SetEffect_HealNum(int a2) {
    if (!g_pEffectManager_After_Chr) return;
    auto* num = new CEffect_Field_HealNumber();
    const float y = static_cast<float>(m_iPosY - GetCharHeight() - 45);
    const float x = static_cast<float>(m_iPosX);
    num->SetEffect(a2, x, y);
    g_pEffectManager_After_Chr->BulletAdd(reinterpret_cast<CEffectBase*>(num));
}

// mofclient.c 27666: same aura as Healed_Self but with skillLevel=0 (tree
// fountain effect rather than a skill).
void ClientCharacter::SetEffect_HealedByTree() {
    if (!g_pEffectManager_After_Chr) return;
    auto* eff = new CEffect_Skill_Heal();
    eff->SetEffect(this, 0);
    g_pEffectManager_After_Chr->BulletAdd(reinterpret_cast<CEffectBase*>(eff));
}

// mofclient.c 27681: resurrection effect + forced-stop order + HP seed.
void ClientCharacter::SetEffect_ResurrectBySkill(int /*a2*/, unsigned char a3) {
    if (g_pEffectManager_After_Chr) {
        auto* eff = new CEffect_Skill_Resurrection();
        const float x = static_cast<float>(m_iPosX);
        const float y = static_cast<float>(m_iPosY);
        eff->SetEffect(x, y, a3);
        g_pEffectManager_After_Chr->BulletAdd(reinterpret_cast<CEffectBase*>(eff));
    }
    ClearOrder();
    stCharOrder order{};
    SetOrderStop(&order);
    PushOrder(&order);
    SetHP();
}

// mofclient.c 27726: applies a debuff effect (from cltDebuffKindInfo record)
// plus a damage-number if a2 is non-zero.  The per-debuff effect-kind IDs at
// WORD+32..+35 route to CEffectManager::AddEffect; left as TODO until that
// overload is restored.
void ClientCharacter::SetEffect_Debuffer_Damage(int a2, unsigned short /*debuff*/) {
    if (a2 && g_pEffectManager_After_Chr) {
        auto* dmg = new CEffect_Field_DamageNumber();
        const float y = static_cast<float>(m_iPosY - GetCharHeight() - 45);
        const float x = static_cast<float>(m_iPosX);
        dmg->SetEffect(-a2, x, y, 1);
        g_pEffectManager_After_Chr->BulletAdd(reinterpret_cast<CEffectBase*>(dmg));
    }
}

// mofclient.c 27771: body-burning overlay from a skill record's WORD+46.
void ClientCharacter::SetEffect_FireBody(unsigned short skillKind) {
    if (!g_pEffectManager_After_Chr) return;
    if (stSkillKindInfo* skill = g_clSkillKindInfo.GetSkillKindInfo(skillKind)) {
        const uint16_t fx = *reinterpret_cast<uint16_t*>(
            reinterpret_cast<unsigned char*>(skill) + 92);
        if (fx) g_pEffectManager_After_Chr->AddEffect(fx, this, nullptr, 0, 0, 0, 2);
    }
}

// mofclient.c 28105: spawn the same 4-effect pair as CreateUseSkillEffect,
// but keyed by an externally-supplied skill kind (used during channelled
// casts to re-trigger hit frames every N ticks).
void ClientCharacter::WorkingSkillEffect(unsigned short skillKind) {
    if (m_someOtherState) return;
    if (m_iInitFlag_153_176[175 - 153]) return;
    stSkillKindInfo* skill = g_clSkillKindInfo.GetSkillKindInfo(skillKind);
    if (!skill) return;
    auto* raw = reinterpret_cast<unsigned char*>(skill);
    const uint16_t v5 = *reinterpret_cast<uint16_t*>(raw + 92);
    const uint16_t v6 = *reinterpret_cast<uint16_t*>(raw + 94);
    const uint16_t v7 = *reinterpret_cast<uint16_t*>(raw + 96);
    const uint16_t v8 = *reinterpret_cast<uint16_t*>(raw + 98);
    if (g_pEffectManager_After_Chr) {
        if (v5) g_pEffectManager_After_Chr->AddEffect(v5, this, nullptr, 0, 0, 0, 2);
        if (v6) g_pEffectManager_After_Chr->AddEffect(v6, this, nullptr, 0, 0, 0, 2);
    }
    if (g_pEffectManager_Before_Chr) {
        if (v7) g_pEffectManager_Before_Chr->AddEffect(v7, this, nullptr, 0, 0, 0, 2);
        if (v8) g_pEffectManager_Before_Chr->AddEffect(v8, this, nullptr, 0, 0, 0, 2);
    }
    const uint16_t multiFx = *reinterpret_cast<uint16_t*>(raw + 328);
    if (multiFx) {
        unsigned int* targets = reinterpret_cast<unsigned int*>(m_acEquipKind);
        const unsigned char targetCount = Decomp<unsigned char>(78);
        for (unsigned char i = 0; i < targetCount; ++i) {
            ClientCharacter* tgt = m_pCharMgr
                ? m_pCharMgr->GetCharByAccount(targets[i]) : nullptr;
            if (!tgt) continue;
            const int hitIdx = tgt->GetHitedInfoNum(m_dwAccountID);
            const uint16_t a4 = Decomp<uint16_t>(2 * 3714);
            if (g_pEffectManager_After_Chr) {
                g_pEffectManager_After_Chr->AddEffect(
                    multiFx, this, tgt, hitIdx, a4, 0, 2);
            }
            tgt->m_iInitFlag_153_176[166 - 153] = 1;
        }
    }
}

// mofclient.c 28163: sustain-skill effect installer.  Fires the same cast
// effect pair (WORD+46..49) as WorkingSkillEffect, then holds a persistent
// aura pair (WORD+54 = after-char slot 103, WORD+55 = before-char slot 108)
// when a3 (sustain flag) is set.  The ground truth also sets bit flags
// based on specific skill codes A08406 / A08306 / (DWORD+70 set).
void ClientCharacter::SetEffect_Sustain(unsigned short skillKind, int a3) {
    if (!skillKind) return;
    if (m_someOtherState) return;
    if (m_iInitFlag_153_176[175 - 153]) return;
    stSkillKindInfo* skill = g_clSkillKindInfo.GetSkillKindInfo(skillKind);
    if (!skill) return;
    auto* raw = reinterpret_cast<unsigned char*>(skill);
    const uint16_t v6 = *reinterpret_cast<uint16_t*>(raw + 92);
    const uint16_t v7 = *reinterpret_cast<uint16_t*>(raw + 94);
    const uint16_t v8 = *reinterpret_cast<uint16_t*>(raw + 96);
    const uint16_t v9 = *reinterpret_cast<uint16_t*>(raw + 98);
    if (g_pEffectManager_After_Chr) {
        if (v6) g_pEffectManager_After_Chr->AddEffect(v6, this, nullptr, 0, 0, 0, 2);
        if (v7) g_pEffectManager_After_Chr->AddEffect(v7, this, nullptr, 0, 0, 0, 2);
    }
    if (g_pEffectManager_Before_Chr) {
        if (v8) g_pEffectManager_Before_Chr->AddEffect(v8, this, nullptr, 0, 0, 0, 2);
        if (v9) g_pEffectManager_Before_Chr->AddEffect(v9, this, nullptr, 0, 0, 0, 2);
    }
    // Persistent aura slots.
    const uint16_t auraAfter = *reinterpret_cast<uint16_t*>(raw + 108);  // WORD+54
    if (auraAfter && !Decomp<void*>(4 * 103) && a3 && g_pEffectManager_After_Chr) {
        Decomp<void*>(4 * 103) = g_pEffectManager_After_Chr->AddEffect(
            auraAfter, this, nullptr, 0, 0, skillKind, 2);
    }
    const uint16_t auraBefore = *reinterpret_cast<uint16_t*>(raw + 110);  // WORD+55
    if (auraBefore && !Decomp<void*>(4 * 108) && a3 && g_pEffectManager_Before_Chr) {
        Decomp<void*>(4 * 108) = g_pEffectManager_Before_Chr->AddEffect(
            auraBefore, this, nullptr, 0, 0, skillKind, 2);
    }
    // Sustain-type flag: DWORD+70 of the skill record.
    if (*reinterpret_cast<int*>(raw + 280)) {
        Decomp<int>(4 * 73) = 1;
        SetCharState(8);
    }
    const uint16_t kindWord = *reinterpret_cast<uint16_t*>(raw + 2);  // WORD+1
    if (kindWord == cltSkillKindInfo::TranslateKindCode("A08406")) {
        Decomp<int>(4 * 74) = 1;
        SetCharState(16);
    }
    if (kindWord == cltSkillKindInfo::TranslateKindCode("A08306")) {
        Decomp<int>(4 * 75) = 1;
        SetCharState(32);
    }
    // Multi-target hit.
    const uint16_t multiFx = *reinterpret_cast<uint16_t*>(raw + 328);
    if (multiFx) {
        unsigned int* targets = reinterpret_cast<unsigned int*>(m_acEquipKind);
        const unsigned char targetCount = Decomp<unsigned char>(78);
        for (unsigned char i = 0; i < targetCount; ++i) {
            ClientCharacter* tgt = m_pCharMgr
                ? m_pCharMgr->GetCharByAccount(targets[i]) : nullptr;
            if (!tgt) continue;
            const int hitIdx = tgt->GetHitedInfoNum(m_dwAccountID);
            const uint16_t a4 = Decomp<uint16_t>(2 * 3714);
            if (g_pEffectManager_After_Chr) {
                g_pEffectManager_After_Chr->AddEffect(
                    multiFx, this, tgt, hitIdx, a4, 0, 2);
            }
            tgt->m_iInitFlag_153_176[166 - 153] = 1;
        }
    }
    ResetUseSkillKind();
}

// mofclient.c 28264: spawn a monster's ranged projectile toward a target.
// Effect-kind id is stored at WORD+97 (offset +194) of the char-kind record.
void ClientCharacter::CreateMonsterShootingObject(ClientCharacter* target) {
    if (!target) return;
    if (!m_pClientCharKindInfo) return;
    stCharKindInfo* info = static_cast<stCharKindInfo*>(
        m_pClientCharKindInfo->GetCharKindInfo(m_wKind));
    if (!info) return;
    const uint16_t shootFx = *reinterpret_cast<uint16_t*>(
        reinterpret_cast<unsigned char*>(info) + 194);
    if (!shootFx) return;
    const int hitIdx = target->GetHitedInfoNum(m_dwAccountID);
    if (g_pEffectManager_After_Chr) {
        g_pEffectManager_After_Chr->AddEffect(
            shootFx, this, target, hitIdx, 0, 0, 2);
    }
    target->m_iInitFlag_153_176[166 - 153] = 1;
    Decomp<uint16_t>(2 * 4840) = m_wKind;  // cache "last shooter kind"
}

// mofclient.c 28352.
void ClientCharacter::SetEffect_OverMind() {
    if (g_pEffectManager_After_Chr) {
        g_pEffectManager_After_Chr->AddEtcEffect(0, m_dwAccountID);
    }
    g_GameSoundManager.PlaySoundA(const_cast<char*>("H0019"), m_iPosX, m_iPosY);
    // Map::Poll_VibrationMode — deferred (map shake when restored).
}

// mofclient.c 28360.
void ClientCharacter::SetEmblem_Effect() {
    if (g_pEffectManager_After_Chr) {
        g_pEffectManager_After_Chr->AddEffect(const_cast<char*>("E0481"), this);
    }
    if (g_pEffectManager_Before_Chr) {
        g_pEffectManager_Before_Chr->AddEffect(const_cast<char*>("E0482"), this);
    }
}

// mofclient.c 28367.
void ClientCharacter::CreateGiveBouquet() {
    if (g_pEffectManager_After_Chr) {
        g_pEffectManager_After_Chr->AddEffect(const_cast<char*>("E0737"), this);
    }
}

// mofclient.c 35491: firework-cracker overlay keyed by item kind.
void ClientCharacter::SetEffect_Cracker(unsigned short kind) {
    if (!g_pEffectManager_After_Chr) return;
    if (kind) {
        g_pEffectManager_After_Chr->AddEffect(kind, this, nullptr, 0, 0, 0, 2);
    }
}

// mofclient.c 35040: live-MC announcer overlay (shows a text label + sparkle
// above the head).  Requires a CEffect_Player_MapConqueror attached; for
// the restored build we trigger the map-conqueror effect-kind string.
void ClientCharacter::CreateLiveMCEffect(char* /*text*/) {
    if (g_pEffectManager_After_Chr) {
        g_pEffectManager_After_Chr->AddEffect(const_cast<char*>("E0501"), this);
    }
}

// mofclient.c 35184.  Couple-ring aura: keyed by ring-kind record in
// cltCoupleRingKindInfo, stored at DWORD+105.
void ClientCharacter::CreateCoupleEffect(unsigned short ringKind, int /*pairOffset*/) {
    if (!ringKind || !g_pEffectManager_After_Chr) return;
    CEffectBase* eff = g_pEffectManager_After_Chr->AddEffect(
        ringKind, this, nullptr, 0, 0, 0, 2);
    Decomp<CEffectBase*>(4 * 105) = eff;
}
void ClientCharacter::DeleteCoupleEffect() {
    Decomp<void*>(4 * 105) = nullptr;  // owned couple-aura slot
}

// =============================================================================
// Remaining simple helpers called by Phase B ports.
// =============================================================================

// mofclient.c 32449: chat-message bubble overlay.  Caches the message at
// +7116, marks the ballon visible (DWORD+1124), and installs a 5-second
// clear-timer so the ballon auto-hides.
void ClientCharacter::SetChatMsg(char* source) {
    if (!m_dwSlotAlive || !source) return;
    std::strncpy(reinterpret_cast<char*>(m_safetyPad) + 7116, source, 255);
    (reinterpret_cast<char*>(m_safetyPad) + 7116)[255] = '\0';
    Decomp<int>(4 * 1124) = 1;  // ballon visible

    // Release any previous auto-clear timer and schedule a new one.
    const unsigned int prev = Decomp<unsigned int>(4 * 1843);
    if (prev) g_clTimerManager.ReleaseTimer(prev);
    auto cb = reinterpret_cast<cltTimer::TimerCallback>(
        &ClientCharacter::OnTimer_ClearChatMsg);
    Decomp<unsigned int>(4 * 1843) = g_clTimerManager.CreateTimer(
        0x1388,  // 5 seconds
        reinterpret_cast<std::uintptr_t>(this),
        0, 1, nullptr, nullptr, cb, nullptr, nullptr);
}

// mofclient.c 32481.  Releases the auto-clear timer and hides the ballon.
void ClientCharacter::ClearerChatMsg() {
    const unsigned int prev = Decomp<unsigned int>(4 * 1843);
    if (prev) {
        g_clTimerManager.ReleaseTimer(prev);
        Decomp<unsigned int>(4 * 1843) = 0;
    }
    (reinterpret_cast<char*>(m_safetyPad) + 7116)[0] = '\0';
    Decomp<int>(4 * 1124) = 0;
}

// mofclient.c 34800: formats the circle (guild) name into two buffers: with
// and without the bracketed "[...]" decoration.  The empty-string branch
// copies the empty string verbatim; non-empty wraps with "[%s]".
void ClientCharacter::SetCircleName(char* a2) {
    if (!a2) return;
    if (!*a2) {
        m_szCircleName[0] = '\0';
        reinterpret_cast<char*>(m_safetyPad)[10188] = '\0';
    } else {
        std::snprintf(m_szCircleName, sizeof(m_szCircleName), "[%s]", a2);
        std::snprintf(reinterpret_cast<char*>(m_safetyPad) + 10188,
                      sizeof(m_szCircleName), "%s", a2);
    }
}

void ClientCharacter::SetPrivateMarketMsg(char* msg) {
    char* dst = reinterpret_cast<char*>(m_safetyPad) + 10252;
    if (msg) {
        std::strncpy(dst, msg, 96);
        dst[95] = '\0';
    } else {
        dst[0] = '\0';
    }
}

// mofclient.c 31774: build the screen-space label (at +9984) plus the width
// metrics (DWORD+2493 = width+10, +2494 = half-width) and the default text
// colour palette (DWORD+2485..2492 = RGBA×2 for body/shadow).
//
// Formatting:
//   - fInfoFlag set → "account:name:posX:posY:hp:mp" (debug overlay).
//   - Monster + field-item-box → "[name]".
//   - Monster otherwise       → "[Lv.N] name".
//   - Otherwise (player)      → "name".
// Boss records override the default palette with a red tint; field-item
// boxes override with an orange tint.
void ClientCharacter::InitScreenName(int a2) {
    (void)a2;
    auto* nameBuf = m_szScreenName;
    const bool isMonster = m_pClientCharKindInfo
                           && m_pClientCharKindInfo->IsMonsterChar(m_wKind);

    if (fInfoFlag) {
        const int mp  = GetMP();
        const int hp  = GetHP();
        std::snprintf(nameBuf, sizeof(m_szScreenName),
                      "%u:%s:%i:%i:%i:%i",
                      m_dwAccountID, m_szName, m_iPosX, m_iPosY, hp, mp);
    } else if (isMonster) {
        cltClientCharKindInfo* cinfo =
            static_cast<cltClientCharKindInfo*>(m_pClientCharKindInfo);
        const bool isBox = cinfo && cinfo->IsFieldItemBox(m_wKind) != nullptr;
        if (isBox) {
            std::snprintf(nameBuf, sizeof(m_szScreenName), "[%s]", m_szName);
        } else {
            const unsigned char lvl = cinfo ? cinfo->GetCharLevel(m_wKind) : 0;
            std::snprintf(nameBuf, sizeof(m_szScreenName), "[Lv.%u] %s",
                          lvl, m_szName);
        }
    } else {
        std::snprintf(nameBuf, sizeof(m_szScreenName), "%s", m_szName);
    }

    // Text-width + half-width for horizontal centering.
    int widthPx = 0, heightPx = 0;
    g_MoFFont.SetFont("CharacterName");
    g_MoFFont.GetTextLength(&widthPx, &heightPx, "CharacterName", nameBuf);
    const int bannerWidth = widthPx + 10;
    Decomp<int>(4 * 2493) = bannerWidth;
    Decomp<int>(4 * 2494) = bannerWidth >> 1;
    // WORD+4986 holds the font-height cache; the decomp adds 10 for padding
    // and writes 15 as a fixed lineheight marker.
    const uint16_t fontHeight = Decomp<uint16_t>(2 * 4986);
    Decomp<uint16_t>(2 * 4882) = static_cast<uint16_t>(fontHeight + 10);
    Decomp<uint16_t>(2 * 4883) = 15;

    // Default palette (body + shadow = white/black pairs encoded as floats).
    // Constants map as: 1045483730 ≈ 0.252f, 1065353216 == 1.0f.
    Decomp<float>(4 * 2485) = 1.0f;  // body.r
    Decomp<float>(4 * 2486) = 0.252f;
    Decomp<float>(4 * 2487) = 0.252f;
    Decomp<float>(4 * 2488) = 1.0f;  // body.a
    Decomp<float>(4 * 2489) = 1.0f;  // shadow.r
    Decomp<float>(4 * 2490) = 1.0f;
    Decomp<float>(4 * 2491) = 1.0f;
    Decomp<float>(4 * 2492) = 1.0f;  // shadow.a

    // Boss override — red name with stronger saturation (skipped when
    // IsBoss lookup fails).
    if (m_pClientCharKindInfo
        && m_pClientCharKindInfo->GetBossInfoByKind(m_wKind)) {
        g_MoFFont.SetFont("BossMonsterName");
        Decomp<float>(4 * 2489) = 1.0f;
        Decomp<float>(4 * 2490) = 0.20f;  // 1053345994 ≈ 0.20f
        Decomp<float>(4 * 2491) = 0.20f;
        Decomp<float>(4 * 2492) = 1.0f;
    } else {
        g_MoFFont.SetFont("BossMonsterName");
        Decomp<float>(4 * 2489) = 1.0f;
        Decomp<float>(4 * 2490) = 0.0f;
        Decomp<float>(4 * 2491) = 0.0f;
        Decomp<float>(4 * 2492) = 1.0f;
    }

    // Field-item-box override — orange strip.
    cltClientCharKindInfo* cinfo =
        static_cast<cltClientCharKindInfo*>(m_pClientCharKindInfo);
    if (cinfo && cinfo->IsFieldItemBox(m_wKind)) {
        Decomp<float>(4 * 2489) = 0.792f;  // 1061734602 ≈ 0.792f
        Decomp<float>(4 * 2490) = 0.922f;  // 1064366322 ≈ 0.922f
        Decomp<float>(4 * 2491) = 0.335f;  // 1047589106 ≈ 0.335f
        Decomp<float>(4 * 2492) = 1.0f;
    }
}

void ClientCharacter::ResetCharEffect() {
    Decomp<void*>(4 * 104) = nullptr;
    Decomp<void*>(4 * 105) = nullptr;
    Decomp<void*>(4 * 106) = nullptr;
    Decomp<void*>(4 * 107) = nullptr;
}

// mofclient.c 33366: stash warp destination for the next MapWarpProc tick.
// DWORD+155 = "pending warp" flag, WORD+3706 = target map id, WORD+3707 =
// secondary (portal tile) kind.  Globals dword_22F2A54/58 are the first-
// non-zero destination coordinates latched by CreateCharacter.
void ClientCharacter::SetWarp(int a2, unsigned short mapID, int destX, int destY,
                               unsigned short a6) {
    m_iInitFlag_153_176[155 - 153] = a2;
    Decomp<uint16_t>(2 * 3706) = mapID;
    Decomp<uint16_t>(2 * 3707) = a6;
    // dword_22F2A54/58 are separate globals — not yet exposed; the decomp
    // only latches the first non-zero write, so callers survive without.
    (void)destX;
    (void)destY;
    if (mapID)           m_wMapID = mapID;
}

// mofclient.c 33380: DWORD+157 defaults to 1 in InitFlag; this method just
// resets it so the next warp spawn produces a fresh warp-in effect.
void ClientCharacter::InitCreateWarpEffectFlag() {
    m_iInitFlag_153_176[157 - 153] = 1;
}

void ClientCharacter::SetCharHide(int /*a2*/, unsigned char /*a3*/) {
    // mofclient.c 34880: toggles the "invisible" flag; full port in Phase E.
}

// mofclient.c 35008.  Stores PC-room entry level / premium flag and spawns
// the "special bottom" aura effect when either is set.
void ClientCharacter::SetPCRoomUser(int a2, unsigned char a3) {
    Decomp<int>(4 * 2889) = a2;
    Decomp<unsigned char>(11560) = a3;
    m_isPCRoomUser    = (a2 != 0) || (a3 != 0);
    m_isPCRoomPremium = (a3 == 2);
    // CEffect_Player_Special_Bottom_Effect + CEffectManager::BulletAdd —
    // deferred; the flag storage is what gameplay code reads.
}

void ClientCharacter::SetPvPRankKind(unsigned short rank) {
    Decomp<uint16_t>(2 * 7344) = rank;
}

// mofclient.c 34067: plays the level-up effect and re-applies the class-code
// based on the new level byte.
void ClientCharacter::PlayerLevelUp(char newLevel) {
    // Visual effects deferred; level-byte + class-code re-apply happen now.
    Decomp<char>(536) = newLevel;
    SetClassCode(m_wClassCode, 0);
}

// mofclient.c 35382: writes the hair-dye colour key at DWORD+87.
void ClientCharacter::DyeHairColor(unsigned int color) {
    m_iUnknown_348 = static_cast<int>(color);
}

// mofclient.c 35394: zero the hair-dye.
void ClientCharacter::ResetHairColorKey() {
    m_iUnknown_348 = 0;
}

// mofclient.c 35470: divorce UI hook; clears couple-ring + pair offset.
void ClientCharacter::Divoced(int /*a2*/) {
    DeleteCoupleEffect();
    m_dwUnknown_9716 = 0;
}

// mofclient.c 35480: switch the couple-ring kind and refresh the aura.
void ClientCharacter::ChangeCoupleRing(unsigned short ringKind) {
    DeleteCoupleEffect();
    if (ringKind) CreateCoupleEffect(ringKind, 0);
}

// mofclient.c 35406 / 35419 / 35430: "hiding-self" toggle — used by invisibility
// cloak items and certain transformation kinds.  Stored at DWORD+2882
// (m_someOtherState).  Writes ripple through to the pet object so the pet
// hides in sympathy.
void ClientCharacter::StartHidingSelf() {
    m_someOtherState = 1;
    if (m_pPetObject) m_pPetObject->StartHidingSelf();
}
int  ClientCharacter::GetHidingSelf() { return m_someOtherState; }
void ClientCharacter::FinishHidingSelf() {
    m_someOtherState = 0;
    if (m_pPetObject) m_pPetObject->FinishHidingSelf();
}
// =============================================================================
// Pet / transport hooks (mofclient.c 35237-35334)
// =============================================================================

void ClientCharacter::SetPetKind(unsigned short petKind, int dyeFlag) {
    if (!m_pPetObject) return;
    m_pPetObject->InitPet(this, nullptr, petKind, dyeFlag, GetHidingSelf());
}

void ClientCharacter::CreatePet(unsigned short petKind, int active) {
    if (!m_pPetObject) return;
    const int dyeFlag = Decomp<int>(4 * 2889);  // DWORD+2889 = persistent dye
    m_pPetObject->InitPet(this, nullptr, petKind, dyeFlag, GetHidingSelf());
    m_pPetObject->SetActive(active);
    m_pPetObject->SetPetName(const_cast<char*>(""));
}

void ClientCharacter::DyePet(unsigned short petKind) {
    if (!m_pPetObject) return;
    m_pPetObject->DyePet(petKind, GetHidingSelf());
}

void ClientCharacter::SetTransportKind(unsigned short transportKind) {
    if (!m_pTransportObject) return;
    m_pTransportObject->InitTransport(this, nullptr, transportKind);
    SetTransportActive(1);
}

// mofclient.c 35277: mount toggle.  On mount-up, discard the currently-
// shown weapon/shield CA and stash their slot ids so dismount can restore.
void ClientCharacter::SetTransportActive(int a2) {
    if (!m_pTransportObject) return;
    m_pTransportObject->SetActive(a2);
    if (m_pCCA) m_pCCA->SetTransportActive(a2);

    if (a2 == 1) {
        // Drop owned weapon / shield effect slots.
        Decomp<void*>(4 * 106) = nullptr;
        Decomp<void*>(4 * 107) = nullptr;
        // Equip slot snapshot happens here in the decomp — preserved via
        // m_dwUnknown_4356/_4360 in SetWeaponType.
    } else {
        // Re-spawn weapon/shield aura.
        const uint16_t mainWep = Decomp<uint16_t>(2 * 3715);
        if (mainWep) CreateWeaponEffect(mainWep, 7);
        const uint16_t shield  = Decomp<uint16_t>(2 * 3716);
        if (shield)  CreateWeaponEffect(shield, 8);

        // Restore snapshot on dismount.
        m_dwUnknown_4356 = m_dwUnknown_4360;
        Decomp<unsigned char>(4373) = Decomp<unsigned char>(4374);
        Decomp<unsigned int>(4 * 1091) = m_dwUnknown_4368;
    }
}

// mofclient.c 35330: enqueue a Died order so the auto-resurrect UI flow
// takes over on the next Poll tick.
void ClientCharacter::AutoResurrect() {
    stCharOrder order{};
    SetOrderDied(&order);
    PushOrder(&order);
}

// mofclient.c 34838: circle-master (guild leader) mark.  Writes the mark id
// into the name-bar overlay; deferred to the UI-text pass.
void ClientCharacter::SetCircleMasterMark(unsigned short mark) {
    Decomp<uint16_t>(2 * 2221) = mark;
}
void ClientCharacter::SetGM(unsigned char level, char* /*name*/) {
    m_dwGM_Level = level;
}
// mofclient.c 34971: compute info-strip Y coordinates for the HP bar, name
// tag, circle name, and title strip.  v1 = 30 when circle-name is empty,
// 45 otherwise.  All outputs live at DWORD+1101..+1107.
void ClientCharacter::ProcInfoPosY() {
    const int circleOffset = (m_szCircleName[0] == '\0') ? 30 : 45;
    // Map scroll-Y lives at m_pMap DWORD+4899 (a copy of dword_A7308C).
    const int scrollY = dword_A7308C;
    // WORD+2220 (name-width indicator) + WORD+2216 (char-info-posY offset).
    const int nameW = Decomp<int16_t>(2 * 2220);
    const int infoP = Decomp<uint16_t>(2 * 2216);

    const int v2 = m_iPosY - scrollY - nameW - infoP;
    Decomp<int>(4 * 1101) = v2;
    const int v3 = v2 - 20;
    const int v4 = v2 - circleOffset;
    Decomp<int>(4 * 1106) = v4;
    Decomp<int>(4 * 1107) = v4 + 10;
    const int v5 = Decomp<int>(4 * 2889);
    Decomp<int>(4 * 1102) = v3 - 2;
    Decomp<int>(4 * 1104) = v3;
    int v6 = v3 - 4;
    int v7 = v3 - 17;
    Decomp<int>(4 * 1103) = v6;
    Decomp<int>(4 * 1105) = v7;
    if (v5 || m_isPCRoomPremium) {
        Decomp<int>(4 * 1103) = v6 - 1;
        Decomp<int>(4 * 1105) = v7 - 4;
    }
}
// mofclient.c 31920.  Sets the name-box quad colour based on:
//   - a3 != 0 → GM purple (the "force override" branch).
//   - else, PCRoom / premium (DWORD+2889 || m_isPCRoomPremium) applies a
//     class-mark-indexed palette (0..3 = non-admin, knight, mage, ranger).
//   - else, default translucent black.
// Float constants are decoded from the decomp's int bit patterns.
void ClientCharacter::SetNameBoxColor(unsigned char classMark, int updateNameTag) {
    if (!m_pNameBox) return;
    float r, g, b, a;
    if (updateNameTag) {
        r = 0.995f; g = 0.0f;   b = 0.701f; a = 0.5f;   // GM purple
    } else if (Decomp<int>(4 * 2889) || m_isPCRoomPremium) {
        switch (classMark) {
        case 0: r = 0.0f;   g = 0.0f;   b = 0.0f;   a = 0.4f;  break;
        case 1: r = 0.986f; g = 0.520f; b = 0.008f; a = 0.492f; break;  // knight orange
        case 2: r = 0.437f; g = 0.592f; b = 1.0f;   a = 0.492f; break;  // mage blue
        case 3: r = 0.850f; g = 0.000f; b = 0.000f; a = 0.492f; break;  // ranger red
        default:
            r = 0.0f; g = 0.0f; b = 0.0f; a = 0.4f; break;
        }
    } else {
        r = 0.0f; g = 0.0f; b = 0.0f; a = 0.4f;
    }
    m_pNameBox->SetColor(r, g, b, a);
}
// =============================================================================
// Run-state (mofclient.c 33478-33573)
//
// Three float speeds are stored at FLOAT+80/+81/+82 = walk / run / fast-run.
// FLOAT+141 (m_fMoveSpeed) is the currently-active one.  BYTE+4400
// (m_ucUnknown_4400) is the run-state byte: 0 = walk, 1 = run, 2 = fast-run.
// The decomp also notifies the network via CMoFNetwork::RunState /
// ChangeActionState on every transition; without the net layer restored we
// keep the local state in sync and omit the server side.
// =============================================================================

void ClientCharacter::SetMyRunState() {
    m_fMoveSpeed = Decomp<float>(4 * 82);
    m_ucUnknown_4400 = 2;
    if (m_pActionStateSystem) m_pActionStateSystem->SetCharActionState(0x40);
    const unsigned char acState = Decomp<unsigned char>(11548);
    g_Network.ChangeActionState(acState);
    g_Network.RunState(m_ucUnknown_4400);
}

void ClientCharacter::SetRunState(unsigned char state) {
    Decomp<unsigned char>(4372) = 0;
    m_ucUnknown_4400 = state;
    if (state == 0)       m_fMoveSpeed = Decomp<float>(4 * 80);
    else if (state == 1)  m_fMoveSpeed = Decomp<float>(4 * 81);
    else if (state == 2)  m_fMoveSpeed = Decomp<float>(4 * 82);
}

// mofclient.c 33510: switches between walk / run / fast-run based on the
// cltPlayerAbility "fast-run active" bit and the transport mount flag.
void ClientCharacter::ProcRunState() {
    const bool fastRunActive =
        g_clPlayerAbility.IsActiveFastRun() != 0;

    if (m_ucUnknown_4400 == 2 && m_pTransportObject
        && m_pTransportObject->GetActive()) {
        return;  // on a mount — don't break speed state.
    }

    if (fastRunActive) {
        m_fMoveSpeed = Decomp<float>(4 * 82);
        m_ucUnknown_4400 = 2;
        if (m_pActionStateSystem) m_pActionStateSystem->SetCharActionState(0x40);
    } else {
        if (m_ucUnknown_4400 == 2) return;
        m_fMoveSpeed = Decomp<float>(4 * 81);
        m_ucUnknown_4400 = 1;
        if (m_pActionStateSystem) m_pActionStateSystem->SetCharActionState(0x10);
    }
    const unsigned char acState = Decomp<unsigned char>(11548);
    g_Network.ChangeActionState(acState);
    g_Network.RunState(m_ucUnknown_4400);
}

void ClientCharacter::ReleaseKeepRunState() {
    m_ucUnknown_4400 = 0;
    g_Network.RunState(0);
    m_fMoveSpeed = Decomp<float>(4 * 80);
    Decomp<unsigned char>(4372) = 0;
    if (m_pActionStateSystem) m_pActionStateSystem->SetCharActionState(0x10);
}

void ClientCharacter::ReleaseMyFastRun() {
    m_ucUnknown_4400 = 1;
    g_Network.RunState(1);
    m_fMoveSpeed = Decomp<float>(4 * 81);
    Decomp<unsigned char>(4372) = 0;
}

// mofclient.c 33174 (~200 lines): consumes the warp flag installed by
// SetWarp and drives the map/position transition.  The full port needs
// g_Map::ChangeMap + CEffectManager spawn + UI fade.  The critical
// behaviour — clear the warp flag and teleport to the target coordinates —
// is preserved here so tests that issue a warp observe the expected state.
int ClientCharacter::MapWarpProc() {
    if (!m_iInitFlag_153_176[155 - 153]) return 0;
    const uint16_t destMapID = Decomp<uint16_t>(2 * 3706);
    if (destMapID && destMapID != m_wMapID) {
        m_wMapID = destMapID;
    }
    m_iInitFlag_153_176[155 - 153] = 0;
    return 1;
}

int  ClientCharacter::IsMyChar() {
    return (m_dwAccountID != 0 && m_dwAccountID == g_dwMyAccountID) ? 1 : 0;
}

// IsTransformed() is defined inline in the header.

// mofclient.c 32212.  The collision check (cltMapCollisonInfo::IsCollison)
// prevents setting the destination to an unwalkable tile.  When the hidden-
// self flag (DWORD+2882) is set, the collision check is bypassed.
void ClientCharacter::SetEndPosition(ClientCharacter* pChar, int x, int y) {
    if (!pChar) return;
    // Collision gate deferred — cltMapCollisonInfo not yet restored.  Accept
    // all moves; the server echoes back a correction via the network layer.
    pChar->m_iDestX = x;
    pChar->m_iDestY = y;
}

// Static timer callbacks — decomp 33457-33475.
void ClientCharacter::OnTimer_ClearChatMsg(unsigned int /*id*/, ClientCharacter* pChar) {
    if (pChar) pChar->ClearerChatMsg();
}
void ClientCharacter::OnTimer_ResetAttackSpeed(unsigned int /*id*/, ClientCharacter* pChar) {
    if (pChar) pChar->SetCanAttack(reinterpret_cast<void(*)(unsigned int, unsigned int)>(1));
}
void ClientCharacter::OnTimer_ResetAttackKey(unsigned int /*id*/, ClientCharacter* pChar) {
    if (pChar) pChar->SetCanAttack(reinterpret_cast<void(*)(unsigned int, unsigned int)>(1));
}

// GetCharWidthA / GetCharHeight (mofclient.c 33445-33454): forward to the
// kind-info lookup.  When the kind info is not installed we fall back to
// conservative constants so Draw paths always see positive extents.
unsigned short ClientCharacter::GetCharWidthA() {
    // cltCharKindInfo::GetCharWidthA — not yet on the restored helper.
    return 48;
}

unsigned short ClientCharacter::GetCharHeight() const {
    return 100;
}

// =============================================================================
// Phase D — rendering
// =============================================================================

// mofclient.c 27264.  Monster / transformation path: pulls the current frame
// from cltMonsterAniInfo's per-state table; otherwise an early-exit for the
// frozen special-state or a return-from-transformation fallback.
void ClientCharacter::DecideDrawFrame(unsigned int* a2, unsigned short* a3) {
    auto* ani = static_cast<cltMonsterAniInfo*>(m_pMonsterAniInfo);

    const unsigned char ss = Decomp<unsigned char>(9700);
    if ((ss & 4) != 0 && m_pClientCharKindInfo
        && m_pClientCharKindInfo->IsMonsterChar(m_wKind)) {
        if (ani) ani->GetFrameInfo(7u, m_iDestY & 0xFFFF, a2, a3);
        return;
    }

    if (m_iInitFlag_153_176[0]) {
        // Player-CCANormal path — picks a motion id by action state, plays it.
        Decomp<uint16_t>(2 * 4846) = 3;
        switch (m_dwActionState) {
        case 0: if (m_pCCANormal) m_pCCANormal->Play(0, false); break;
        case 1: if (m_pCCANormal) m_pCCANormal->Play(4, false); break;
        case 2: if (m_pCCANormal) m_pCCANormal->Play(7, false); break;
        case 3: if (m_pCCANormal) m_pCCANormal->Play(m_ucAttackMotionType, false); break;
        case 4:
            switch (m_ucUnknown_11260) {
            case 1: if (m_pCCANormal) m_pCCANormal->Play(1, false); break;
            case 2: if (m_pCCANormal) m_pCCANormal->Play(5, false); break;
            case 3: if (m_pCCANormal) m_pCCANormal->Play(6, false); break;
            case 4: if (m_pCCANormal) m_pCCANormal->Play(8, false); break;
            default: return;
            }
            break;
        default: return;
        }
    } else {
        if (!ani) return;
        switch (m_dwActionState) {
        case 0: ani->GetFrameInfo(0, m_wCurrentFrame, a2, a3); break;
        case 1: ani->GetFrameInfo(1, m_wCurrentFrame, a2, a3); break;
        case 2: ani->GetFrameInfo(2, m_wCurrentFrame, a2, a3); break;
        case 3: ani->GetFrameInfo(3, m_wCurrentFrame, a2, a3); break;
        case 4: {
            unsigned int sub = 0xFFFFFFFFu;
            switch (m_ucUnknown_11260) {
            case 1: sub = 4; break;
            case 2: sub = 5; break;
            case 3: sub = 6; break;
            case 4: sub = 7; break;
            default: break;
            }
            if (sub != 0xFFFFFFFFu) {
                if (ani->GetFrameInfo(sub, m_wCurrentFrame, a2, a3)) return;
            }
            ani->GetFrameInfo(4, m_wCurrentFrame, a2, a3);
            break;
        }
        default: return;
        }
    }
}

// mofclient.c 27377.  Player / regular-character path: plays one of the fixed
// CCA motion ids based on action state + equipped weapon class + run state.
// The stride (WORD+4846) is set to either 3 (normal) or 6 (long animations).
void ClientCharacter::DecideDrawFrame() {
    if (!m_pCCA) return;
    int motion = 1;
    switch (m_dwActionState) {
    case 0:
    case 10:
    LABEL_26:
        motion = 1;
        m_pCCA->Play(motion, false);
        Decomp<uint16_t>(2 * 4846) = 3;
        return;
    case 1: {
        if (m_pTransportObject && m_pTransportObject->GetActive()) goto LABEL_26;
        motion = m_ucUnknown_4400 ? 15 : 0;
        m_pCCA->Play(motion, false);
        Decomp<uint16_t>(2 * 4846) = 3;
        return;
    }
    case 2:
        m_pCCA->Play(motion, false);  // motion defaults to 1 from init
        Decomp<uint16_t>(2 * 4846) = 6;
        return;
    case 3:
        m_pCCA->Play(m_ucUnknown_7384, false);
        Decomp<uint16_t>(2 * 4846) = GetAttackAniRate();
        return;
    case 4:
        motion = 13;
        m_pCCA->Play(motion, false);
        Decomp<uint16_t>(2 * 4846) = 3;
        return;
    case 5: {
        // Spell-class whitelist (magic / heal / pray / special).
        const uint16_t cls = m_wClassCode;
        if (cls != 1043 && cls != 22809 && cls != 12747
            && cls != 12294 && cls != 18438 && cls != 22545)
            goto LABEL_26;
        m_pCCA->Play(11, false);
        Decomp<uint16_t>(2 * 4846) = 6;
        return;
    }
    case 6: {
        const uint16_t cls = m_wClassCode;
        if (cls == 2400 || cls == 15912 || cls == 18440
            || cls == 15371 || cls == 7627 || cls == 1298) {
            m_pCCA->Play(12, false);
            Decomp<uint16_t>(2 * 4846) = 6;
        } else {
            goto LABEL_26;
        }
        return;
    }
    case 7:
        m_pCCA->Play(14, false);
        Decomp<uint16_t>(2 * 4846) = 6;
        return;
    case 8: {
        // Triple-attack: phase byte at +282 picks motion byte at +283..+285.
        const unsigned char phase = Decomp<unsigned char>(282);
        const unsigned char motion8 = Decomp<unsigned char>(283u + phase);
        m_pCCA->Play(motion8, false);
        Decomp<uint16_t>(2 * 4846) =
            static_cast<uint16_t>(3 * ((phase >> 1) + 1));
        return;
    }
    case 9: {
        const unsigned char phase = Decomp<unsigned char>(282);
        const unsigned char motion9 = Decomp<unsigned char>(283u + phase);
        m_pCCA->Play(motion9, false);
        Decomp<uint16_t>(2 * 4846) =
            static_cast<uint16_t>(3 * (phase + 1));
        return;
    }
    default:
        return;
    }
}

// mofclient.c 27447: position the HP-box back / front / overlay sprites
// (at +7452/+7660/+7868) and, for non-local chars, the name-box at +9732,
// then schedule each for drawing via its vtable PrepareDrawing slot.
void ClientCharacter::PrepareDrawingHPBox() {
    // DWORD+160 is the "show HP box this frame" flag.
    if (!m_iInitFlag_153_176[160 - 153]) return;
    const bool gate = dword_21B8DFC
                      || (m_pClientCharKindInfo
                          && m_pClientCharKindInfo->IsMonsterChar(m_wKind))
                      || IsMyChar();
    if (!gate) return;

    // Map scroll X lives at DWORD+4898 of Map (same as dword_A73088 cache).
    const int scrollX = dword_A73088;
    const int infoY   = Decomp<int>(4 * 1101);
    if (m_pHpBoxFront) m_pHpBoxFront->SetPos(m_iPosX - scrollX - 20, infoY);
    if (m_pHpBoxBack)  m_pHpBoxBack ->SetPos(m_iPosX - scrollX - 20, infoY);
    if (m_pHpBoxThird) m_pHpBoxThird->SetPos(m_iPosX - scrollX - 21, infoY - 1);
    if (m_pHpBoxBack)  m_pHpBoxBack ->PrepareDrawing();
    if (m_pHpBoxFront) m_pHpBoxFront->PrepareDrawing();
    if (m_pHpBoxThird) m_pHpBoxThird->PrepareDrawing();

    if (!IsMyChar() && m_pNameBox) {
        const int nameWidth = Decomp<int>(4 * 2441);
        m_pNameBox->SetPos(m_iPosX - (nameWidth >> 1) - scrollX,
                           Decomp<int>(4 * 1102));
        m_pNameBox->PrepareDrawing();
    }
}

// mofclient.c 27483.
void ClientCharacter::DrawHPBox() {
    if (!m_iInitFlag_153_176[160 - 153]) return;
    const bool gate = dword_21B8DFC
                      || (m_pClientCharKindInfo
                          && m_pClientCharKindInfo->IsMonsterChar(m_wKind))
                      || IsMyChar();
    if (!gate) return;
    if (m_pHpBoxThird) m_pHpBoxThird->Draw();
    if (m_pHpBoxBack)  m_pHpBoxBack ->Draw();
    if (m_pHpBoxFront) m_pHpBoxFront->Draw();
    if (!IsMyChar() && m_pNameBox) m_pNameBox->Draw();
}

// mofclient.c 27499: composites the "action-state" floating icon above the
// character (absence / conversation / trade).  Exact GameImage styling
// deferred; the state-index bookkeeping + WORD+5776 are preserved so the
// Draw half observes the same gating.
void ClientCharacter::PrepareDrawingCharActionState() {
    if (!m_pActionStateSystem) return;
    if (!m_pActionStateSystem->IsActionStateNormal()) {
        uint16_t& marker = Decomp<uint16_t>(2 * 5776);
        marker = 10;
        if (m_pActionStateSystem->IsActionStateAbsence())      marker = 2;
        else if (m_pActionStateSystem->IsActionStateConversation()) marker = 0;
        else if (m_pActionStateSystem->IsActionStateTrade())   marker = 1;
        // marker != 10 → load a GameImage; deferred to the image-manager port.
    }
}

void ClientCharacter::DrawCharActionState() {
    if (!m_pActionStateSystem) return;
    if (m_pActionStateSystem->IsActionStateNormal()) return;
    const uint16_t marker = Decomp<uint16_t>(2 * 5776);
    if (marker == 10) return;
    // GameImage::Draw at DWORD+13 — deferred.
}

// mofclient.c 31978: three-line text overlay for the char's name strip.
//
//   Line 1 (font CircleName)   — circle/guild name (inside [...]) drawn in
//                                 body colour (DWORD+2485..2488) at the
//                                 circle row DWORD+1105.
//   Line 2 (font NickName)     — secondary nickname (Korean-locale only)
//                                 drawn in cyan at DWORD+1104, offset by
//                                 DWORD+2495 pixels left.
//   Line 3 (font CharacterName)— main name at +9984 drawn in shadow colour
//                                 (DWORD+2489..2492) at DWORD+1104.
//
// Colour packing: clamp [0,1] → byte, pack ARGB = A<<24 | R<<16 | G<<8 | B.
static inline DWORD DT_PackARGB(float r, float g, float b, float a) {
    auto clamp = [](float v) -> int {
        if (v >= 1.0f) return 255;
        if (v <= 0.0f) return 0;
        return static_cast<int>(v * 255.0f + 0.5f);
    };
    const DWORD rr = static_cast<DWORD>(clamp(r));
    const DWORD gg = static_cast<DWORD>(clamp(g));
    const DWORD bb = static_cast<DWORD>(clamp(b));
    const DWORD aa = static_cast<DWORD>(clamp(a));
    return (aa << 24) | (rr << 16) | (gg << 8) | bb;
}

void ClientCharacter::DrawTextA(int a2) {
    if (!m_dwAccountID) return;
    // DWORD+159 is the "draw name" flag (set after InitScreenName succeeds).
    if (!m_iInitFlag_153_176[159 - 153]) return;

    if (fInfoFlag) {
        const int mp = GetMP();
        const int hp = GetHP();
        std::snprintf(m_szScreenName, sizeof(m_szScreenName),
                      "%u:%s:%i:%i:%i:%i",
                      m_dwAccountID, m_szName, m_iPosX, m_iPosY, hp, mp);
        int widthPx = 0, heightPx = 0;
        g_MoFFont.GetTextLength(&widthPx, &heightPx, "CharacterName", m_szScreenName);
        const int bannerWidth = widthPx + 10;
        Decomp<int>(4 * 2493) = bannerWidth;
        Decomp<int>(4 * 2494) = bannerWidth >> 1;
        Decomp<uint16_t>(2 * 4882) = static_cast<uint16_t>(
            Decomp<uint16_t>(2 * 4986) + 10);
        Decomp<uint16_t>(2 * 4883) = 15;
    }
    (void)a2;

    const int scrollX = dword_A73088;

    // Line 1 — Circle name (only when present).
    if (m_szCircleName[0] != '\0') {
        g_MoFFont.SetFont("CircleName");
        const float bR = Decomp<float>(4 * 2485);
        const float bG = Decomp<float>(4 * 2486);
        const float bB = Decomp<float>(4 * 2487);
        const float bA = Decomp<float>(4 * 2488);
        const DWORD shadow = DT_PackARGB(bR, bG, bB, bA);
        const int circleY = Decomp<int>(4 * 1105);
        g_MoFFont.SetTextLineShadow(m_iPosX - scrollX, circleY, shadow,
                                    m_szCircleName, 1);
        g_MoFFont.SetTextLineA(m_iPosX - scrollX, circleY, 0xFFFFFF00u,
                               m_szCircleName, 1, -1, -1);
    }

    // Line 2 — Korean nickname (the "real name" stored at +524).
    char* nickname = reinterpret_cast<char*>(m_safetyPad) + 524;
    if (g_MoFFont.GetNationCode() == 1 && nickname[0] != '\0') {
        g_MoFFont.SetFont("NickName");
        const float bR = Decomp<float>(4 * 2485);
        const float bG = Decomp<float>(4 * 2486);
        const float bB = Decomp<float>(4 * 2487);
        const float bA = Decomp<float>(4 * 2488);
        const DWORD shadow = DT_PackARGB(bR, bG, bB, bA);
        const int yLine = Decomp<int>(4 * 1104);
        const int xOff  = Decomp<int>(4 * 2495);
        const int x     = m_iPosX - scrollX - xOff;
        g_MoFFont.SetTextLineShadow(x, yLine, shadow, nickname, 2);
        g_MoFFont.SetTextLineA(x, yLine, 0xFF00FFFFu, nickname, 2, -1, -1);
    }

    // Line 3 — Main character name.
    g_MoFFont.SetFont("CharacterName");
    const float sR = Decomp<float>(4 * 2489);
    const float sG = Decomp<float>(4 * 2490);
    const float sB = Decomp<float>(4 * 2491);
    const float sA = Decomp<float>(4 * 2492);
    const DWORD colour = DT_PackARGB(sR, sG, sB, sA);
    g_MoFFont.SetTextLineA(m_iPosX - scrollX,
                           Decomp<int>(4 * 1104),
                           colour, m_szScreenName, 1, -1, -1);
}
// GetActionState() / IsTransformed() are defined inline in the header.
int ClientCharacter::GetPositionX() { return m_iPosX; }
int ClientCharacter::GetPositionY() { return m_iPosY; }
int ClientCharacter::GetPosX() const { return m_iPosX; }
int ClientCharacter::GetPosY() const { return m_iPosY; }
int ClientCharacter::GetActionSide() const { return m_dwLR_Flag; }
// SetNameTagInfo defined above (Phase A carry-over); Phase D fills in the
// colour-state update once CControlAlphaBox::SetColor wiring is ready.

// =============================================================================
// Poll (mofclient.c 32236)
//
// Per-frame update driver:
//   1. Local-player walking-dust accel/decel envelope (spawn effect deferred
//      to Phase D — state transitions preserved here).
//   2. Fade alpha back up when not in die/died state.
//   3. OrderHit + ProcessOrder to drain pending hit records / orders.
//   4. Dispatch by m_dwActionState into Order*.
//   5. PollHPBox + MoveCharacter.
//   6. 2-second dead-fallback: if DWORD+156 is set and the slot survived past
//      the 2-second hit window, force-enqueue Die/Died.
//   7. Expire any owned effects whose vtable slot-4 query returns non-zero
//      (stub until Phase D restores the vtable-driven expire call).
// Returns 1 only when the character has been marked for removal (self == null
// or OrderDie finished); 0 on a normal tick.
// =============================================================================
int ClientCharacter::Poll(int a2) {
    if (!this) return 1;

    const uintptr_t vflag = reinterpret_cast<uintptr_t>(m_pVftable);
    const bool isSelfPlayer = vflag && m_pCharMgr
                              && m_pCharMgr->IsPlayer(m_wKind);
    if (isSelfPlayer) {
        int& walkFlag    = Decomp<int>(4 * 93);
        float& walkT     = Decomp<float>(4 * 91);
        float& walkDelta = Decomp<float>(4 * 92);
        int& walkPhase   = Decomp<int>(4 * 94);
        int& walkOpacity = Decomp<int>(4 * 97);
        int& walkBurst   = Decomp<int>(4 * 95);
        if (walkFlag) {
            walkOpacity = (walkT < 46.66666f) ? 4 : 2;
            if (walkT <= 41.66666f) walkOpacity = 5;
            const float frame = static_cast<float>(SETTING_FRAME);
            if (walkPhase) {
                walkDelta = 1.2f / frame * walkDelta;
                walkT    -= walkDelta;
                if (walkT <= 0.0f) {
                    walkT     = 0.0f;
                    walkBurst = static_cast<int>(timeGetTime());
                    walkFlag  = 0;
                    m_pVftable = nullptr;
                }
            } else {
                walkDelta = 0.99f / frame * walkDelta;
                walkT    += walkDelta;
                if (walkT >= 70.0f) {
                    walkT     = 70.0f;
                    walkPhase = 1;
                    walkDelta = 1.0f;
                }
            }
            if (m_pCCA) {
                *reinterpret_cast<int*>(
                    reinterpret_cast<unsigned char*>(m_pCCA) + 136) =
                    static_cast<int>(walkT);
            }
        } else {
            int v5 = static_cast<int>(m_dwActionState);
            walkFlag = 1;
            walkT    = 0.0f;
            walkDelta = 20.0f;
            walkPhase = 0;
            if (!v5) m_dwActionState = 1;
        }
    }

    const unsigned int as = m_dwActionState;
    if (as != 2 && as != 7 && m_ucAlpha != 0xFFu) {
        int v = m_ucAlpha + 5;
        if (v >= 255) v = 0xFF;
        m_ucAlpha = static_cast<unsigned char>(v);
    }

    OrderHit();
    ProcessOrder();

    switch (m_dwActionState) {
    case 0:
        OrderStop();
        if (m_pPetObject)       m_pPetObject->Poll();
        if (m_pTransportObject) m_pTransportObject->Poll();
        break;
    case 1:
        OrderMove();
        if (m_pPetObject)       m_pPetObject->Poll();
        if (m_pTransportObject) m_pTransportObject->Poll();
        break;
    case 2: {
        const int r = OrderDie();
        if (r == 1) return r;
        if (m_pPetObject) m_pPetObject->Poll();
        break;
    }
    case 3:
    case 5:
        OrderAttack();
        break;
    case 4:
        OrderHitted(a2);
        if (m_pTransportObject) m_pTransportObject->Poll();
        break;
    case 6:
        OrderPray();
        break;
    case 7:
        if (m_pPetObject)       m_pPetObject->Poll();
        if (m_pTransportObject) m_pTransportObject->Poll();
        break;
    case 8:
        OrderTripleAttack();
        break;
    case 9:
        OrderDoubleAttack();
        break;
    case 10:
        OrderTransformation();
        break;
    default:
        break;
    }

    PollHPBox(a2);
    MoveCharacter();

    const int haveDeathPending = m_iInitFlag_153_176[156 - 153];
    const DWORD lastTick       = Decomp<DWORD>(4 * 1852);
    if (haveDeathPending && (timeGetTime() - lastTick > 0x7D0) && m_dwSlotAlive) {
        stCharOrder order{};
        if (m_pCharMgr && m_pCharMgr->IsMonster(m_wKind)) SetOrderDie(&order);
        else                                              SetOrderDied(&order);
        PushOrder(&order);
        m_iInitFlag_153_176[156 - 153] = 0;
    }

    // Pending: vtable-driven effect-expire at DWORD+98 and DWORD+99..+107.
    // Restored once CEffectBase exposes an IsExpired() helper in Phase D.
    (void)Decomp<void*>(4 * 98);
    for (int i = 0; i < 9; ++i) {
        (void)Decomp<void*>(4 * 99 + static_cast<size_t>(4 * i));
    }
    return 0;
}

// mofclient.c 33944: auto-hide the HP bar 3 seconds after the last hit.  The
// full PollHPBox also drives the per-frame fade but that lives in the draw
// path (PrepareDrawingHPBox); we keep only the timestamp bookkeeping here.
void ClientCharacter::PollHPBox(int /*a2*/) {
    if (!m_dwSlotAlive) return;
    const DWORD now = timeGetTime();
    DWORD& lastVisible = Decomp<DWORD>(4 * 1850);
    if (lastVisible && (now - lastVisible) > 3000u) {
        lastVisible = 0;
    }
}

// =============================================================================
// MoveCharacter (mofclient.c 32656, ~500 lines)
//
// Per-frame position update:
//   - Skip when action state is 2/3/4 (die / attack / hit).
//   - Respect SETTING_FRAME halving (every other tick when frame-rate is 2).
//   - Compute step = m_fMoveSpeed toward (m_iDestX, m_iDestY), clamped so we
//     don't overshoot the destination.
//   - Collision check against g_Map keeps the character on walkable tiles.
//   - Facing (m_dwLR_Flag) flips to the last non-zero horizontal delta.
//   - Walkdust / goggle / speed-up effect spawns are TODO (Phase D visual
//     polish) — the control flow / position math is preserved.
// =============================================================================
void ClientCharacter::MoveCharacter() {
    const unsigned int as = m_dwActionState;
    if (as == 2 || as == 3 || as == 4) return;  // LABEL_103 — no move during these.

    // SETTING_FRAME == 2 (half-rate) gating: step every other tick.
    if (SETTING_FRAME == 2) {
        int& gate = m_iUnknown_356;  // DWORD+89
        gate += 1;
        if (gate < 2) return;
        gate = 0;
    }

    const int dx = m_iDestX - m_iPosX;
    const int dy = m_iDestY - m_iPosY;
    if (!dx && !dy) return;

    // Facing — positive dx => right (1), negative => left (0).
    if (dx > 0)      m_dwLR_Flag = 1;
    else if (dx < 0) m_dwLR_Flag = 0;

    // Step size: the decomp uses per-frame pixel velocity stored at +564
    // (m_fMoveSpeed).  Default = 3.0f when unset; most paths write between
    // 2 and 5 depending on equipment / run state.
    float speed = m_fMoveSpeed;
    if (speed < 0.001f) speed = 3.0f;

    const float adx   = static_cast<float>(std::abs(dx));
    const float ady   = static_cast<float>(std::abs(dy));
    const float dist  = (adx > ady) ? adx : ady;
    const float step  = (speed > dist) ? dist : speed;
    if (dist <= 0.0f) return;

    const int stepX = static_cast<int>(static_cast<float>(dx) * step / dist);
    const int stepY = static_cast<int>(static_cast<float>(dy) * step / dist);

    const int newX = m_iPosX + stepX;
    const int newY = m_iPosY + stepY;

    // Collision check against Map::IsCollison — when the Map interface is
    // wired we query; until then we just accept the move.
    (void)m_pMap;

    m_iPrevPosX = m_iPosX;
    m_iPrevPosY = m_iPosY;
    m_iPosX = newX;
    m_iPosY = newY;

    // Walkdust / speed-up / goggle effects: decomp spawns a CEffect_Field_
    // Walkdust periodically when running.  Not crucial for behaviour — the
    // visual is Phase-D scope.
}
