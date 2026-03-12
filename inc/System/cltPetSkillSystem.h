#pragma once

#include <cstdint>

class CMofMsg;

class cltPetSkillSystem {
public:
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
    unsigned int CanAddPetSkill(std::uint16_t skillKind);
    void AddPetSkill(std::uint16_t skillKind);
    std::uint16_t GetPetSkillNum();
};

