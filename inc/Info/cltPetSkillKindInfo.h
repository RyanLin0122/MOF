#ifndef CLT_PET_SKILL_KIND_INFO_H
#define CLT_PET_SKILL_KIND_INFO_H

#include <cstddef>
#include <cstdint>
#include <cstdio>

#pragma pack(push, 1)
struct strPetSkillKindInfo
{
    std::uint16_t wSkillId;                                // Offset 0
    std::uint16_t wSkillNameTextId;                        // Offset 2
    std::uint16_t wSkillDescriptionTextId;                 // Offset 4
    std::uint8_t  padding_0[2];                            // Offset 6, 原始資料未使用
    std::uint32_t IconFileName;                            // Offset 8, 以 "%x" 讀入的 4-byte 值
    std::uint16_t wBlockName;                              // Offset 12
    std::uint16_t wRequiredLevel;                          // Offset 14
    std::uint32_t dwSkillAttackPowerIncrease;              // Offset 16
    std::uint32_t dwAttackPowerIncreasePerThousand;        // Offset 20
    std::uint32_t dwDefensePowerIncreasePerThousand;       // Offset 24
    std::uint32_t dwHitRateIncreasePerThousand;            // Offset 28
    std::uint32_t dwHpAutoRecoveryRateChangePerThousand;   // Offset 32
    std::uint32_t dwMpAutoRecoveryRateChangePerThousand;   // Offset 36
    std::uint32_t dwAttackSpeed;                           // Offset 40
    std::uint16_t wPickup;                                 // Offset 44
    std::uint8_t  padding_2[2];                            // Offset 46, 原始資料未使用
    std::uint32_t dwItemDropRateIncreasePerThousand;       // Offset 48
    std::uint32_t dwStr;                                   // Offset 52
    std::uint32_t dwDex;                                   // Offset 56
    std::uint32_t dwVit;                                   // Offset 60
    std::uint32_t dwInt;                                   // Offset 64
    std::uint16_t wCasterGround;                           // Offset 68
    std::uint16_t wCasterAbove;                            // Offset 70
};
#pragma pack(pop)

static_assert(offsetof(strPetSkillKindInfo, wSkillId) == 0, "wSkillId offset mismatch");
static_assert(offsetof(strPetSkillKindInfo, wSkillNameTextId) == 2, "wSkillNameTextId offset mismatch");
static_assert(offsetof(strPetSkillKindInfo, wSkillDescriptionTextId) == 4, "wSkillDescriptionTextId offset mismatch");
static_assert(offsetof(strPetSkillKindInfo, IconFileName) == 8, "IconFileName offset mismatch");
static_assert(offsetof(strPetSkillKindInfo, wBlockName) == 12, "wBlockName offset mismatch");
static_assert(offsetof(strPetSkillKindInfo, wCasterGround) == 68, "wCasterGround offset mismatch");
static_assert(offsetof(strPetSkillKindInfo, wCasterAbove) == 70, "wCasterAbove offset mismatch");
static_assert(sizeof(strPetSkillKindInfo) == 72, "strPetSkillKindInfo size mismatch");

class cltPetSkillKindInfo
{
public:
    cltPetSkillKindInfo();

    void Free();
    int Initialize(char* fileName);
    strPetSkillKindInfo* GetPetSkillKindInfo(std::uint16_t skillId);

    static std::uint16_t TranslateKindCode(char* text);
    std::uint16_t TranslateEffectKindCode(char* text);

    strPetSkillKindInfo* GetBuffer() const { return m_pInfo; }
    int GetCount() const { return m_nCount; }

private:
    strPetSkillKindInfo* m_pInfo;
    int m_nCount;
};

#endif // CLT_PET_SKILL_KIND_INFO_H
