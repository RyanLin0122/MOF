#include "Network/CMoFNetwork.h"
#include "Object/cltPetObject.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <map>
#include <string>

#include "Character/ClientCharacter.h"
#include "Character/ClientCharacterManager.h"
#include "Effect/CEffect_Pet_Base.h"
#include "Image/CDeviceManager.h"
#include "Image/GameImage.h"
#include "Image/cltImageManager.h"
#include "Info/cltClientPetKindInfo.h"
#include "Info/cltItemKindInfo.h"
#include "Info/cltMoFC_EffectKindInfo.h"
#include "Info/cltPetAniInfo.h"
#include "Info/cltPetKindInfo.h"
#include "Logic/cltFieldItem.h"
#include "Logic/cltFieldItemManager.h"
#include "Logic/CSpiritSpeechMgr.h"
#include "Logic/cltMyCharData.h"
#include "Logic/cltSystemMessage.h"
#include "Object/clTransportObject.h"
#include "System/cltMoneySystem.h"
#include "System/cltPetInventorySystem.h"
#include "System/cltPetSystem.h"
#include "Text/DCTTextManager.h"
#include "Util/ScriptParameter.h"
#include "Util/StringStream.h"
#include "global.h"

namespace {

constexpr char kEmptyString[] = "";

inline int abs32(int value) {
    return value < 0 ? -value : value;
}

inline bool IsOwnerOnTransport(const ClientCharacter* owner) {
    if (!owner) return false;
    auto* transport = reinterpret_cast<const clTransportObject*>(reinterpret_cast<const char*>(owner) + 14616);
    return const_cast<clTransportObject*>(transport)->GetActive() != 0;
}

inline unsigned int ResolveMyAccount() {
    return cltMyCharData::GetMyAccount(&g_clMyCharData);
}

} // namespace

cltPetObject::cltPetObject()
    : m_nActive(0)
    , m_pOwnerChar(nullptr)
    , m_pPetKindInfo(nullptr)
    , m_pEffect(nullptr)
    , m_fTraceRatio(1.0f)
    , m_nTraceReady(1)
    , m_nAlpha(255) {
}

cltPetObject::~cltPetObject() {
    Release();
}

void cltPetObject::Release() {
    DeleteEffect();
    m_nActive = 0;
    m_pGameImage = nullptr;
    m_pOwnerChar = nullptr;
    m_pAniInfo = nullptr;
    m_pCCA = nullptr;
    m_pPetKindInfo = nullptr;
}

void cltPetObject::SetActive(int active) {
    m_nActive = active;
    m_nOffsetX = 0;
    m_nPatrolDir = 1;
    m_fTraceRatio = 1.0f;
    m_nTraceReady = 1;

    if (active) {
        m_wCurrentFrame = 0;
        m_nOffsetX = -100;
        m_nPatrolDir = 1;
        m_fTraceRatio = 1.0f;
        m_nFacing = (m_pOwnerChar->m_dwLR_Flag == 0);
        m_nTraceReady = 1;
        if (m_pCCA) {
            m_nPosX = static_cast<int>(*reinterpret_cast<float*>(reinterpret_cast<char*>(m_pCCA) + 128));
        } else {
            m_nPosX = m_pOwnerChar->m_iPosX;
        }
        m_dwLastPickupTime = 0;
        m_dwLastInvFullMsg = 0;
        m_nCanPickup = 1;
        CreatePetEffect();
    } else {
        DeleteEffect();
    }
}

int cltPetObject::GetActive() {
    return m_nActive;
}

void cltPetObject::PetLevelUP(uint16_t petKind) {
    m_pPetKindInfo = g_clPetKindInfoBase.GetPetKindInfo(petKind);
    if (!m_pPetKindInfo) {
        return;
    }

    m_pAniInfo = g_clClientPetKindInfo.GetPetAniInfo(petKind);
    m_dwLastPickupTime = 0;
    m_dwLastInvFullMsg = 0;
    m_nLastSatiety = g_clPetSystem.GetPetSatiety();
    m_wNameHeight = static_cast<std::uint16_t>(m_pPetKindInfo->wCharacterHeightPosY + 17);
}

void cltPetObject::InitPet(ClientCharacter* owner, CCA* cca, uint16_t petKind, int dyeFlag, int hiddenFlag) {
    m_pOwnerChar = owner;
    if (!owner) {
        return;
    }

    m_pCCA = cca;
    if (owner->m_isPCRoomPremium || owner->m_isPCRoomUser) {
        m_nPickupInterval = 250;
    } else {
        m_nPickupInterval = 500;
    }
    m_nPickupInterval = 500;

    m_pPetKindInfo = g_clPetKindInfoBase.GetPetKindInfo(petKind);
    if (!m_pPetKindInfo) {
        return;
    }

    m_pAniInfo = g_clClientPetKindInfo.GetPetAniInfo(petKind);
    m_bIsMyPet = (owner->m_dwAccountID == ResolveMyAccount());

    m_chatBallon.Create(nullptr, 0, static_cast<unsigned int>(-1), 0);

    m_wCurrentFrame = 0;
    m_nOffsetX = -100;
    m_nPatrolDir = 1;
    m_nFacing = (owner->m_dwLR_Flag == 0);
    m_fTraceRatio = 1.0f;
    m_nTraceReady = 1;

    if (m_pCCA) {
        m_nPosX = static_cast<int>(*reinterpret_cast<float*>(reinterpret_cast<char*>(m_pCCA) + 128));
    } else {
        m_nPosX = owner->m_iPosX;
    }

    const DWORD color = 0x60000000u;
    const float a = static_cast<float>((color >> 24) & 0xFF) / 255.0f;
    const float r = static_cast<float>((color >> 16) & 0xFF) / 255.0f;
    const float g = static_cast<float>((color >> 8) & 0xFF) / 255.0f;
    const float b = static_cast<float>(color & 0xFF) / 255.0f;
    m_alphaBox.Create(m_nPosX, m_nPosY, 0, 15, r, g, b, a, nullptr);

    m_dwLastPickupTime = 0;
    m_dwLastInvFullMsg = 0;
    m_nLastSatiety = g_clPetSystem.GetPetSatiety();
    m_wNameBoxW = 30;
    m_wNameHeight = static_cast<std::uint16_t>(m_pPetKindInfo->wCharacterHeightPosY + 17);
    m_nDyeFlag = dyeFlag;
    if (dyeFlag) {
        m_wNameBoxW = 33;
    }
    m_nAlpha = hiddenFlag ? 127 : 255;
    m_nCanPickup = 1;
    CreatePetEffect();
}

void cltPetObject::CreatePetEffect() {
    m_pEffect = nullptr;
    if (m_pPetKindInfo->wPetEffect == 0) {
        return;
    }

    stEffectKindInfo* effectInfo = g_clEffectKindInfo.GetEffectKindInfo(m_pPetKindInfo->wPetEffect);
    if (!effectInfo) {
        return;
    }

    auto* effect = new CEffect_Pet_Base();
    effect->SetEffect(this, m_pPetKindInfo->wPetEffect, effectInfo->eaFile);
    m_pEffect = effect;
}

void cltPetObject::DeleteEffect() {
    if (!m_pEffect) {
        return;
    }
    delete m_pEffect;
    m_pEffect = nullptr;
}

void cltPetObject::DyePet(uint16_t petKind, int hiddenFlag) {
    InitPet(m_pOwnerChar, m_pCCA, petKind, m_nDyeFlag, hiddenFlag);
}

void cltPetObject::SetPetName(char* name) {
    std::strcpy(m_szPetName, name);
    g_MoFFont.SetFont("CharacterName");
    int textWidth = 0;
    int textHeight = 0;
    g_MoFFont.GetTextLength(&textWidth, &textHeight, "CharacterName", m_szPetName);
    m_nNameWidth = textWidth + 8;
    m_nNameHalfWidth = m_nNameWidth >> 1;
    m_alphaBox.SetSize(static_cast<std::uint16_t>(m_nNameWidth), 15);
}

void cltPetObject::Poll() {
    if (!m_nActive) {
        return;
    }

    if (++m_wCurrentFrame >= m_wTotalFrames) {
        m_wCurrentFrame = 0;
    }

    if (m_pEffect) {
        m_pEffect->FrameProcess(0.04f);
    }

    if (m_bIsMyPet && g_clPetSystem.IsActivity()) {
        if (g_clPetSystem.CanPickupItem() && timeGetTime() - m_dwLastPickupTime > static_cast<DWORD>(m_nPickupInterval)) {
            m_dwLastPickupTime = timeGetTime();
            RequestPickUpItem();
        }

        char* speech = m_nSpeechFlag ? g_clSpiritSpeechMgr.GetSpiritSpeechScript() : const_cast<char*>(kEmptyString);
        m_chatBallon.SetString(speech, 0, 0, 0, 0, static_cast<Direction>(DirLeft | DirRight));

        if (m_pPetKindInfo) {
            const float satietyF = static_cast<float>(g_clPetSystem.GetPetSatiety());
            if (static_cast<double>(m_nLastSatiety) > static_cast<double>(satietyF)) {
                m_nLastSatiety = g_clPetSystem.GetPetSatiety();
                if (!m_pPetKindInfo->wSaturation) {
                    SetActive(0);
                    return;
                }

                const double ratio = static_cast<double>(satietyF) / static_cast<double>(m_pPetKindInfo->wSaturation);
                if (ratio <= 0.1) {
                    SetActive(1);
                    cltSystemMessage::SetSystemMessage(&g_clSysemMessage, g_DCTTextManager.GetText(6727), 0, 0xFFFFFFFF, 0xFFFF0000);
                } else if (ratio <= 0.3) {
                    SetActive(1);
                    cltSystemMessage::SetSystemMessage(&g_clSysemMessage, g_DCTTextManager.GetText(6726), 0, 0xFFFFFFFF, 0xFFFF0000);
                }
            }
        }
    }
}

void cltPetObject::PrepareDrawing(int speechFlag, int forceShow) {
    if (!m_nActive || !m_pOwnerChar || (!forceShow && IsBasePet())) {
        return;
    }

    m_nSpeechFlag = speechFlag;
    m_nDrawMode = static_cast<int>(m_pOwnerChar->GetActionState());
    if (m_nDrawMode == 1) {
        MoveTrace();
    } else {
        MovePatrol();
    }

    unsigned int resourceId = 0;
    std::uint16_t frame = 0;
    DecideDrawFrame(&resourceId, &frame);
    m_pGameImage = cltImageManager::GetInstance()->GetGameImage(1u, resourceId, 0, 1);
    if (!m_pGameImage) {
        return;
    }

    int drawX = m_nPosX;
    int drawY = m_nPosY;
    if (!m_pCCA) {
        drawX -= dword_A73088;
        drawY -= dword_A7308C;
    }

    m_pGameImage->SetBlockID(frame);
    m_pGameImage->m_bFlag_447 = true;
    m_pGameImage->m_bFlag_446 = true;
    m_pGameImage->m_bVertexAnimation = false;
    m_pGameImage->SetPosition(static_cast<float>(drawX), static_cast<float>(drawY + static_cast<int>(m_pPetKindInfo->dwPetPositionAdjustmentY)));
    m_pGameImage->m_bFlipX = (m_nFacing != 0);
    m_pGameImage->SetAlpha(static_cast<unsigned int>(m_nAlpha));
    m_pGameImage->m_bFlag_450 = true;

    if (!m_pCCA) {
        if (m_bIsMyPet && m_nSpeechFlag) {
            auto* pGIData = m_pGameImage->m_pGIData;
            if (!pGIData) {
                return;
            }
            int value = *reinterpret_cast<int*>(*reinterpret_cast<int*>(reinterpret_cast<char*>(pGIData) + 32) + 32);
            int textOffsetY = abs32(value) + 40;

            if (!m_pOwnerChar) {
                return;
            }

            int balloonX = m_nOffsetX + m_pOwnerChar->m_iPosX - dword_A73088;
            if (IsOwnerOnTransport(m_pOwnerChar) && !m_nDrawMode) {
                balloonX += m_pOwnerChar->m_dwLR_Flag ? 110 : -110;
            }
            const int balloonY = m_nPosY - dword_A7308C - textOffsetY;
            m_chatBallon.SetPos(balloonX, balloonY);
            m_chatBallon.Draw();
        }

        if (m_pEffect && m_nDrawMode != 1) {
            m_pEffect->Process();
        }
    }
}

void cltPetObject::Draw(int forceShow) {
    if (!m_nActive || (!forceShow && IsBasePet())) {
        return;
    }

    if (m_pGameImage) {
        if (m_pEffect && m_nDrawMode != 1) {
            m_pEffect->Draw();
            CDeviceManager::GetInstance()->ResetRenderState();
        }
        m_pGameImage->Draw();
    }

    if (std::strcmp(m_szPetName, kEmptyString) != 0) {
        m_alphaBox.Draw();
        if (m_pNameBgRight) m_pNameBgRight->Draw();
        if (m_pNameBgCenter) m_pNameBgCenter->Draw();
        if (m_pNameBgExtra) m_pNameBgExtra->Draw();

        g_MoFFont.SetFont("CharacterName");
        int textX = m_nPosX;
        int textY = m_nPosY;
        if (!m_pCCA) {
            textX -= dword_A73088;
            textY -= dword_A7308C;
        }
        g_MoFFont.SetTextLineA(textX, textY - m_wNameHeight + 4, 0xFFFFFFFF, m_szPetName, 1, -1, -1);
    }

    if (forceShow && m_nSpeechFlag) {
        m_chatBallon.Draw();
    }
}

void cltPetObject::MoveTrace() {
    if (m_pCCA) {
        return;
    }

    m_fPatrolRatio = 1.0f;
    m_nPatrolActive = 1;
    if (abs32(m_nOffsetX) < 65) {
        m_nPatrolState = 1;
    }

    if (m_nTraceReady) {
        const int deltaX = m_nPosX - m_pOwnerChar->m_iPosX;
        if (abs32(deltaX) < 65) {
            m_nPosY = m_pOwnerChar->m_iPosY;
            m_nOffsetX = deltaX;
            return;
        }
    }

    m_nTraceReady = 0;
    if (m_fTraceRatio > 0.0f) {
        m_nOffsetX = static_cast<int>(static_cast<double>(m_nPosX - m_pOwnerChar->m_iPosX) * m_fTraceRatio);
        m_fTraceRatio -= 0.02f;
    }

    m_nFacing = (m_pOwnerChar->m_dwLR_Flag == 0);
    if (m_pOwnerChar->m_dwLR_Flag) {
        if (m_nOffsetX < 65) {
            m_nOffsetX += m_nPatrolStep;
            if (m_nOffsetX >= 65) {
                m_nOffsetX = 65;
            }
        }
    } else if (m_nOffsetX > -65) {
        m_nOffsetX -= m_nPatrolStep;
        if (m_nOffsetX <= -65) {
            m_nOffsetX = -65;
        }
    }

    m_nPatrolStep += 2;
    m_nPosX = m_pOwnerChar->m_iPosX + m_nOffsetX;
    m_nPosY = m_pOwnerChar->m_iPosY;
    m_nTransportInit = 0;
}

void cltPetObject::MovePatrol() {
    int ownerX = m_pOwnerChar->m_iPosX;
    int ownerY = m_pOwnerChar->m_iPosY;
    if (m_pCCA) {
        ownerX = static_cast<int>(*reinterpret_cast<float*>(reinterpret_cast<char*>(m_pCCA) + 128));
        ownerY = static_cast<int>(*reinterpret_cast<float*>(reinterpret_cast<char*>(m_pCCA) + 132));
    }

    int extraRange = 55;
    if (IsOwnerOnTransport(m_pOwnerChar)) {
        extraRange = 0;
        if (!m_nTransportInit) {
            m_nOffsetX = m_pOwnerChar->m_dwLR_Flag ? -45 : 45;
            m_fPatrolRatio = 0.0f;
        }
        m_nTransportInit = 1;
    }

    if (m_nPatrolActive) {
        const int deltaX = m_nPosX - ownerX;
        if (abs32(deltaX) > extraRange + 45) {
            if (m_fPatrolRatio <= 0.0f) {
                m_nPatrolActive = 0;
            } else {
                m_nOffsetX = static_cast<int>(static_cast<double>(deltaX) * m_fPatrolRatio);
                m_nPosX = ownerX + m_nOffsetX;
                m_nPosY = ownerY;
                m_fPatrolRatio -= 0.02f;
                m_nFacing = (m_pOwnerChar->m_dwLR_Flag == 0);
            }
            return;
        }
    }

    m_nPatrolActive = 0;
    m_fTraceRatio = 1.0f;
    m_nTraceReady = 1;
    m_nFacing = 1;
    m_nOffsetX += m_nPatrolDir;

    if (m_nSpeechFlag) {
        if (abs32(m_nOffsetX) > extraRange + 45) {
            m_nOffsetX = (m_nOffsetX <= 0) ? (-45 - extraRange) : (extraRange + 45);
            m_nPatrolDir = -m_nPatrolDir;
            m_nPatrolState = 0;
        }
        if (!IsOwnerOnTransport(m_pOwnerChar) && abs32(m_nOffsetX) < 65 && !m_nPatrolState) {
            m_nOffsetX = (m_nOffsetX <= 0) ? -65 : 65;
            m_nPatrolDir = -m_nPatrolDir;
        }
        if (m_nPatrolDir < 0) {
            m_nFacing = 0;
        }
        m_nPosX = ownerX + m_nOffsetX;
        m_nPatrolStep = 1;
        m_nPosY = ownerY;
        if (IsOwnerOnTransport(m_pOwnerChar)) {
            m_nPosX += m_pOwnerChar->m_dwLR_Flag ? 110 : -110;
        }
    } else {
        m_nPatrolState = 1;
        if (abs32(m_nOffsetX) > extraRange + 45) {
            m_nOffsetX = (m_nOffsetX <= 0) ? (-45 - extraRange) : (extraRange + 45);
            m_nPatrolDir = -m_nPatrolDir;
        }
        if (m_nPatrolDir < 0) {
            m_nFacing = 0;
        }
        m_nPosX = ownerX + m_nOffsetX;
        m_nPatrolStep = 1;
        m_nPosY = ownerY;
        if (IsOwnerOnTransport(m_pOwnerChar)) {
            m_nPosX += m_pOwnerChar->m_dwLR_Flag ? 110 : -110;
        }
    }
}

void cltPetObject::DecideDrawFrame(unsigned int* outResId, uint16_t* outFrame) {
    const std::uint8_t motionFlags = *reinterpret_cast<std::uint8_t*>(reinterpret_cast<char*>(m_pOwnerChar) + 11548);
    m_wTotalFrames = m_pAniInfo->GetTotalFrameNum(0);
    m_pAniInfo->GetFrameInfo(0, m_wCurrentFrame, outResId, outFrame);

    switch (m_pOwnerChar->GetActionState()) {
    case 0:
        m_wTotalFrames = m_pAniInfo->GetTotalFrameNum(0);
        m_pAniInfo->GetFrameInfo(0, m_wCurrentFrame, outResId, outFrame);
        break;
    case 1:
        if ((motionFlags & 0x60) != 0) {
            m_wTotalFrames = m_pAniInfo->GetTotalFrameNum(2u);
            if (m_wCurrentFrame >= m_wTotalFrames) {
                m_wCurrentFrame = 0;
            }
            m_pAniInfo->GetFrameInfo(2u, m_wCurrentFrame, outResId, outFrame);
        } else if ((motionFlags & 0x10) != 0) {
            m_wTotalFrames = m_pAniInfo->GetTotalFrameNum(1u);
            if (m_wCurrentFrame >= m_wTotalFrames) {
                m_wCurrentFrame = 0;
            }
            m_pAniInfo->GetFrameInfo(1u, m_wCurrentFrame, outResId, outFrame);
        }
        break;
    default:
        break;
    }
}

void cltPetObject::SetNearItemInfo(int itemCount, uint16_t itemKind, uint16_t itemQty) {
    m_nNearItemCount = itemCount;
    m_wNearItemKind = itemKind;
    m_wNearItemQty = itemQty;
}

void cltPetObject::RequestPickUpItem() {
    if (!m_nCanPickup) {
        return;
    }

    const int satiety = g_clPetSystem.GetPetSatiety();
    const std::uint16_t maxSatiety = m_pPetKindInfo->wSaturation;
    if (static_cast<double>(satiety) / static_cast<double>(maxSatiety) < 0.1) {
        return;
    }

    ClientCharacter* myChar = g_ClientCharMgr.GetMyCharacterPtr();
    if (!myChar || myChar->IsTransformed()) {
        return;
    }

    std::uint16_t fieldItemId = 0;
    float itemX = 0.0f;
    float itemY = 0.0f;
    if (!g_clFieldItemMgr.GetNearItemInfo(static_cast<float>(myChar->m_iPosX), static_cast<float>(myChar->m_iPosY), &fieldItemId, &itemX, &itemY, 1, 0, 0)) {
        return;
    }

    cltFieldItem* fieldItem = g_clFieldItemMgr.GetFieldItem(fieldItemId);
    if (!fieldItem) {
        return;
    }

    int itemCount = 0;
    std::uint16_t itemKind = 0;
    std::uint16_t itemInfo = 0;
    fieldItem->GetItemInfo(&itemCount, &itemKind, &itemInfo);

    if (!itemKind) {
        SetNearItemInfo(itemCount, 0, itemInfo);
        g_Network.PetPickUpItem(fieldItemId, 0);
        m_nCanPickup = 0;
        return;
    }

    cltPetInventorySystem* petInventory = g_clPetSystem.GetPetInventorySystem();
    const int canAdd = petInventory->CanAddItem(itemKind, itemInfo);
    if (canAdd == 0) {
        SetNearItemInfo(itemCount, itemKind, itemInfo);
        g_Network.PetPickUpItem(fieldItemId, itemInfo);
        m_nCanPickup = 0;
        return;
    }

    if (timeGetTime() - m_dwLastInvFullMsg > 0xFA0) {
        m_dwLastInvFullMsg = timeGetTime();
        const unsigned int color = (canAdd == 1904) ? 0 : static_cast<unsigned int>(-71168);
        const int textId = (canAdd == 1904) ? 6834 : 6800;
        cltSystemMessage::SetSystemMessage(&g_clSysemMessage, g_DCTTextManager.GetText(textId), 0, color, 0);
    }
}

void cltPetObject::PickUpItem() {
    char buffer[1024]{};

    if (m_wNearItemKind) {
        std::map<std::string, std::string> params;
        StringStream ss;
        ss << m_wNearItemQty;

        stItemKindInfo* itemInfo = g_clItemKindInfo.GetItemKindInfo(m_wNearItemKind);
        const char* itemName = g_DCTTextManager.GetText(itemInfo->m_wTextCode);
        params.emplace("Parameter0", itemName);
        params.emplace("Parameter1", ss.c_str());

        const char* tmpl = g_DCTTextManager.GetText(6735);
        std::string msg = getScriptParameter(tmpl, params);
        if (msg == tmpl) {
            std::snprintf(buffer, sizeof(buffer), tmpl, itemName, m_wNearItemQty);
        } else {
            std::snprintf(buffer, sizeof(buffer), "%s", msg.c_str());
        }

        cltPetInventorySystem* petInventory = g_clPetSystem.GetPetInventorySystem();
        petInventory->AddItem(m_wNearItemKind, static_cast<int>(m_wNearItemQty), nullptr);
        g_clSpiritSpeechMgr.UpdateQuestCollection(m_wNearItemKind);
    } else {
        std::snprintf(buffer, sizeof(buffer), g_DCTTextManager.GetText(6736), m_nNearItemCount);
        g_clMoneySystem.IncreaseMoney(m_nNearItemCount);
    }

    m_nCanPickup = 1;
    cltSystemMessage::SetSystemMessage(&g_clSysemMessage, buffer, 0, 0, 0);
}

void cltPetObject::StartHidingSelf() {
    m_nAlpha = 127;
}

void cltPetObject::FinishHidingSelf() {
    m_nAlpha = 255;
}

uint8_t cltPetObject::IsDrawingOrder() {
    return m_pPetKindInfo ? m_pPetKindInfo->bCharacterFrontBack : 0;
}

bool cltPetObject::IsBasePet() {
    return m_pPetKindInfo ? (m_pPetKindInfo->dwSkillAcquisitionStatus == 0) : true;
}
