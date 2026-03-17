#pragma once

#include <array>
#include <cstdint>

class CMofMsg;
class cltPetSkillKindInfo;

// 反編譯對應：cltPetSkillSystem
// 記憶體佈局：
//   offset 0-199   (0xC8 bytes): skillKinds_[100] (uint16 * 100 = 200 bytes)
//   offset 200     (DWORD +50):  skillCount_
//   offset 204-211 (WORD +102~+105): usingSkills_[4]
class cltPetSkillSystem {
public:
    static void InitializeStaticVariable(cltPetSkillKindInfo* petSkillKindInfo);

    cltPetSkillSystem();
    ~cltPetSkillSystem();

    void Initialize(CMofMsg* msg);
    void Free();
    void OnPetCreated();
    void OnPetDeleted();

    int GetSkillAPowerAdvantage(std::uint16_t basePassiveSkill);
    int GetAPowerAdvantage(std::uint16_t basePassiveSkill);
    int GetDPowerAdvantage(std::uint16_t basePassiveSkill);
    int GetHitRateAdvantage(std::uint16_t basePassiveSkill);
    int GetAutoRecoverHPAdvantage(std::uint16_t basePassiveSkill);
    int GetAutoRecoverManaAdvantage(std::uint16_t basePassiveSkill);
    int CanPickupItem(std::uint16_t basePassiveSkill);
    int GetDropRateAdvantage(std::uint16_t basePassiveSkill);
    int GetSTRAdvantage(std::uint16_t basePassiveSkill);
    int GetVITAdvantage(std::uint16_t basePassiveSkill);
    int GetDEXAdvantage(std::uint16_t basePassiveSkill);
    int GetINTAdvantage(std::uint16_t basePassiveSkill);
    int GetAttackSpeedAdvantage(std::uint16_t basePassiveSkill);

    int CanSetPetUsingSkill(std::uint8_t slot, std::uint16_t skillKind);
    void SetPetUsingSkill(std::uint8_t slot, std::uint16_t skillKind);
    int CanResetPetSkill(std::uint8_t slot);
    void ResetPetSkill(std::uint8_t slot);

    int CanAddPetSkill(std::uint16_t skillKind);
    void AddPetSkill(std::uint16_t skillKind);

    int GetPetSkillNum();
    std::uint16_t* GetPetSkillKind();
    std::uint16_t* GetPetUsingSkillKind();

private:
    static constexpr int kMaxSkills = 100;
    static constexpr int kMaxUsingSkills = 4;

    std::array<std::uint16_t, kMaxSkills> skillKinds_{};
    int skillCount_ = 0;
    std::array<std::uint16_t, kMaxUsingSkills> usingSkills_{};

    static cltPetSkillKindInfo* m_pclPetSkillKindInfo;
};
