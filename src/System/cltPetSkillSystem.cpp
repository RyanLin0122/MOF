#include "System/cltPetSkillSystem.h"

#include <cstring>

#include "Info/cltPetSkillKindInfo.h"
#include "Network/CMofMsg.h"

cltPetSkillKindInfo* cltPetSkillSystem::m_pclPetSkillKindInfo = nullptr;

// 反編譯對應：cltPetSkillSystem::InitializeStaticVariable
// 原始碼 mofclient.c:321391
void cltPetSkillSystem::InitializeStaticVariable(cltPetSkillKindInfo* petSkillKindInfo) {
    m_pclPetSkillKindInfo = petSkillKindInfo;
}

// 反編譯對應：cltPetSkillSystem::cltPetSkillSystem
// 原始碼 mofclient.c:321397
// GT: memset(this, 0, 0xC8u); *(DWORD+50)=0; *(DWORD+51)=0; *(DWORD+52)=0;
cltPetSkillSystem::cltPetSkillSystem() {
    std::memset(skillKinds_.data(), 0, 0xC8u);
    skillCount_ = 0;
    std::memset(usingSkills_.data(), 0, sizeof(usingSkills_));
}

// 反編譯對應：cltPetSkillSystem::~cltPetSkillSystem
// 原始碼 mofclient.c:321407
// GT: memset(this, 0, 0xC8u); *(DWORD+50)=0; *(DWORD+51)=0; *(DWORD+52)=0;
cltPetSkillSystem::~cltPetSkillSystem() {
    std::memset(skillKinds_.data(), 0, 0xC8u);
    skillCount_ = 0;
    std::memset(usingSkills_.data(), 0, sizeof(usingSkills_));
}

// 反編譯對應：cltPetSkillSystem::Initialize
// 原始碼 mofclient.c:321416
void cltPetSkillSystem::Initialize(CMofMsg* msg) {
    std::memset(skillKinds_.data(), 0, 0xC8u);
    int count = 0;
    msg->Get_LONG(&count);
    skillCount_ = count;
    msg->Get(reinterpret_cast<std::uint8_t*>(skillKinds_.data()), 2 * count);
    std::memset(usingSkills_.data(), 0, sizeof(usingSkills_));

    int usingCount = 0;
    msg->Get_LONG(&usingCount);
    for (int i = 0; i < usingCount; ++i) {
        std::uint8_t slot = 0;
        msg->Get_BYTE(&slot);
        if (slot < kMaxUsingSkills)
            msg->Get_WORD(&usingSkills_[slot]);
    }
}

// 反編譯對應：cltPetSkillSystem::Free
// 原始碼 mofclient.c:321444
void cltPetSkillSystem::Free() {
    std::memset(skillKinds_.data(), 0, 0xC8u);
    skillCount_ = 0;
    std::memset(usingSkills_.data(), 0, sizeof(usingSkills_));
}

namespace {

// 通用的 Advantage 計算輔助：
// 從 basePassiveSkill 取得 strPetSkillKindInfo 的指定 DWORD 偏移量，
// 再加上 4 個 usingSkills 的同一欄位。
int GetAdvantageByDwordOffset(
    cltPetSkillKindInfo* kindInfo,
    const std::array<std::uint16_t, 4>& usingSkills,
    std::uint16_t basePassiveSkill,
    int dwordOffset)
{
    int result = 0;
    if (basePassiveSkill) {
        auto* info = kindInfo->GetPetSkillKindInfo(basePassiveSkill);
        if (info)
            result = reinterpret_cast<const std::int32_t*>(info)[dwordOffset];
    }
    for (int i = 0; i < 4; ++i) {
        auto* info = kindInfo->GetPetSkillKindInfo(usingSkills[i]);
        if (info)
            result += reinterpret_cast<const std::int32_t*>(info)[dwordOffset];
    }
    return result;
}

} // namespace

// 反編譯對應：cltPetSkillSystem::GetSkillAPowerAdvantage
// strPetSkillKindInfo DWORD offset 4 = dwSkillAttackPowerIncrease (byte offset 16)
int cltPetSkillSystem::GetSkillAPowerAdvantage(std::uint16_t basePassiveSkill) {
    return GetAdvantageByDwordOffset(m_pclPetSkillKindInfo, usingSkills_, basePassiveSkill, 4);
}

// 反編譯對應：cltPetSkillSystem::GetAPowerAdvantage
// strPetSkillKindInfo DWORD offset 5 = dwAttackPowerIncreasePerThousand (byte offset 20)
int cltPetSkillSystem::GetAPowerAdvantage(std::uint16_t basePassiveSkill) {
    return GetAdvantageByDwordOffset(m_pclPetSkillKindInfo, usingSkills_, basePassiveSkill, 5);
}

// 反編譯對應：cltPetSkillSystem::GetDPowerAdvantage
// strPetSkillKindInfo DWORD offset 6 = dwDefensePowerIncreasePerThousand (byte offset 24)
int cltPetSkillSystem::GetDPowerAdvantage(std::uint16_t basePassiveSkill) {
    return GetAdvantageByDwordOffset(m_pclPetSkillKindInfo, usingSkills_, basePassiveSkill, 6);
}

// 反編譯對應：cltPetSkillSystem::GetHitRateAdvantage
// strPetSkillKindInfo DWORD offset 7 = dwHitRateIncreasePerThousand (byte offset 28)
int cltPetSkillSystem::GetHitRateAdvantage(std::uint16_t basePassiveSkill) {
    return GetAdvantageByDwordOffset(m_pclPetSkillKindInfo, usingSkills_, basePassiveSkill, 7);
}

// 反編譯對應：cltPetSkillSystem::GetAutoRecoverHPAdvantage
// strPetSkillKindInfo DWORD offset 8 = dwHpAutoRecoveryRateChangePerThousand (byte offset 32)
int cltPetSkillSystem::GetAutoRecoverHPAdvantage(std::uint16_t basePassiveSkill) {
    return GetAdvantageByDwordOffset(m_pclPetSkillKindInfo, usingSkills_, basePassiveSkill, 8);
}

// 反編譯對應：cltPetSkillSystem::GetAutoRecoverManaAdvantage
// strPetSkillKindInfo DWORD offset 9 = dwMpAutoRecoveryRateChangePerThousand (byte offset 36)
int cltPetSkillSystem::GetAutoRecoverManaAdvantage(std::uint16_t basePassiveSkill) {
    return GetAdvantageByDwordOffset(m_pclPetSkillKindInfo, usingSkills_, basePassiveSkill, 9);
}

// 反編譯對應：cltPetSkillSystem::CanPickupItem
// 原始碼 mofclient.c:321633
// 檢查 basePassiveSkill 或任一 usingSkill 的 wPickup (WORD offset 22) 是否非零
int cltPetSkillSystem::CanPickupItem(std::uint16_t basePassiveSkill) {
    if (basePassiveSkill) {
        auto* info = m_pclPetSkillKindInfo->GetPetSkillKindInfo(basePassiveSkill);
        if (info && info->wPickup)
            return 1;
    }
    for (int i = 0; i < kMaxUsingSkills; ++i) {
        auto* info = m_pclPetSkillKindInfo->GetPetSkillKindInfo(usingSkills_[i]);
        if (info && info->wPickup)
            return 1;
    }
    return 0;
}

// 反編譯對應：cltPetSkillSystem::GetDropRateAdvantage
// strPetSkillKindInfo DWORD offset 12 = dwItemDropRateIncreasePerThousand (byte offset 48)
int cltPetSkillSystem::GetDropRateAdvantage(std::uint16_t basePassiveSkill) {
    return GetAdvantageByDwordOffset(m_pclPetSkillKindInfo, usingSkills_, basePassiveSkill, 12);
}

// 反編譯對應：cltPetSkillSystem::GetSTRAdvantage
// strPetSkillKindInfo DWORD offset 13 = dwStr (byte offset 52)
int cltPetSkillSystem::GetSTRAdvantage(std::uint16_t basePassiveSkill) {
    return GetAdvantageByDwordOffset(m_pclPetSkillKindInfo, usingSkills_, basePassiveSkill, 13);
}

// 反編譯對應：cltPetSkillSystem::GetVITAdvantage
// strPetSkillKindInfo DWORD offset 15 = dwVit (byte offset 60)
int cltPetSkillSystem::GetVITAdvantage(std::uint16_t basePassiveSkill) {
    return GetAdvantageByDwordOffset(m_pclPetSkillKindInfo, usingSkills_, basePassiveSkill, 15);
}

// 反編譯對應：cltPetSkillSystem::GetDEXAdvantage
// strPetSkillKindInfo DWORD offset 14 = dwDex (byte offset 56)
int cltPetSkillSystem::GetDEXAdvantage(std::uint16_t basePassiveSkill) {
    return GetAdvantageByDwordOffset(m_pclPetSkillKindInfo, usingSkills_, basePassiveSkill, 14);
}

// 反編譯對應：cltPetSkillSystem::GetINTAdvantage
// strPetSkillKindInfo DWORD offset 16 = dwInt (byte offset 64)
int cltPetSkillSystem::GetINTAdvantage(std::uint16_t basePassiveSkill) {
    return GetAdvantageByDwordOffset(m_pclPetSkillKindInfo, usingSkills_, basePassiveSkill, 16);
}

// 反編譯對應：cltPetSkillSystem::GetAttackSpeedAdvantage
// strPetSkillKindInfo DWORD offset 10 = dwAttackSpeed (byte offset 40)
int cltPetSkillSystem::GetAttackSpeedAdvantage(std::uint16_t basePassiveSkill) {
    return GetAdvantageByDwordOffset(m_pclPetSkillKindInfo, usingSkills_, basePassiveSkill, 10);
}

// 反編譯對應：cltPetSkillSystem::CanSetPetUsingSkill
// 原始碼 mofclient.c:321841
// 如果 a3==0，檢查該 slot 是否有技能可以重設
// 如果 a3!=0，檢查該技能是否不在 usingSkills 中，且存在於 skillKinds 中
int cltPetSkillSystem::CanSetPetUsingSkill(std::uint8_t slot, std::uint16_t skillKind) {
    if (slot >= kMaxUsingSkills)
        return 0;

    if (skillKind) {
        // 確認該技能不在已裝備的 usingSkills 中
        for (int i = 0; i < kMaxUsingSkills; ++i) {
            if (usingSkills_[i] == skillKind)
                return 0;
        }
        // 確認該技能存在於已學會的 skillKinds 中
        for (int i = 0; i < skillCount_; ++i) {
            if (skillKinds_[i] == skillKind)
                return 1;
        }
        return 0;
    }

    // a3 == 0: 可以重設（清除）該 slot 嗎？
    return usingSkills_[slot] != 0;
}

// 反編譯對應：cltPetSkillSystem::SetPetUsingSkill
// 原始碼 mofclient.c:321877
void cltPetSkillSystem::SetPetUsingSkill(std::uint8_t slot, std::uint16_t skillKind) {
    usingSkills_[slot] = skillKind;
}

// 反編譯對應：cltPetSkillSystem::CanResetPetSkill
// 原始碼 mofclient.c:321883
int cltPetSkillSystem::CanResetPetSkill(std::uint8_t slot) {
    if (slot >= kMaxUsingSkills)
        return 0;
    return usingSkills_[slot] != 0;
}

// 反編譯對應：cltPetSkillSystem::ResetPetSkill
// 原始碼 mofclient.c:321895
void cltPetSkillSystem::ResetPetSkill(std::uint8_t slot) {
    usingSkills_[slot] = 0;
}

// 反編譯對應：cltPetSkillSystem::CanAddPetSkill
// 原始碼 mofclient.c:321901
int cltPetSkillSystem::CanAddPetSkill(std::uint16_t skillKind) {
    if (skillCount_ >= kMaxSkills)
        return 0;
    for (int i = 0; i < skillCount_; ++i) {
        if (skillKinds_[i] == skillKind)
            return 0;
    }
    return 1;
}

// 反編譯對應：cltPetSkillSystem::AddPetSkill
// 原始碼 mofclient.c:321925
void cltPetSkillSystem::AddPetSkill(std::uint16_t skillKind) {
    skillKinds_[skillCount_++] = skillKind;
}

// 反編譯對應：cltPetSkillSystem::OnPetCreated
// 原始碼 mofclient.c:321931
void cltPetSkillSystem::OnPetCreated() {
    std::memset(skillKinds_.data(), 0, 0xC8u);
    skillCount_ = 0;
    std::memset(usingSkills_.data(), 0, sizeof(usingSkills_));
}

// 反編譯對應：cltPetSkillSystem::OnPetDeleted
// 原始碼 mofclient.c:321940
void cltPetSkillSystem::OnPetDeleted() {
    std::memset(skillKinds_.data(), 0, 0xC8u);
    skillCount_ = 0;
    std::memset(usingSkills_.data(), 0, sizeof(usingSkills_));
}

// 反編譯對應：cltPetSkillSystem::GetPetUsingSkillKind
// 原始碼 mofclient.c:321949
std::uint16_t* cltPetSkillSystem::GetPetUsingSkillKind() {
    return usingSkills_.data();
}

// 反編譯對應：cltPetSkillSystem::GetPetSkillKind
// 原始碼 mofclient.c:321955
std::uint16_t* cltPetSkillSystem::GetPetSkillKind() {
    return skillKinds_.data();
}

// 反編譯對應：cltPetSkillSystem::GetPetSkillNum
// 原始碼 mofclient.c:321961
int cltPetSkillSystem::GetPetSkillNum() {
    return skillCount_;
}
