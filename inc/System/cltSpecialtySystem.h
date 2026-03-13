#pragma once

#include <array>
#include <cstdint>
#include <algorithm>
#include <cstring>

#include "Info/cltSpecialtyKindInfo.h"
#include "Network/CMofMsg.h"
#include "System/cltLevelSystem.h"
#include "System/cltQuestSystem.h"
#include "System/cltSkillSystem.h"

class cltGradeSystem;

class cltSpecialtySystem {
public:
    static void InitializeStaticVariable(cltSpecialtyKindInfo* specialtyKindInfo);

    cltSpecialtySystem();
    ~cltSpecialtySystem();

    void Initialize(cltGradeSystem* gradeSystem, cltSkillSystem* skillSystem, cltLevelSystem* levelSystem,
                    cltQuestSystem* questSystem, std::int16_t specialtyPt,
                    std::uint16_t acquiredSpecialtyNum, const std::uint16_t* acquiredSpecialtyKind);
    void Free();

    std::int16_t GetSpecialtyPt();
    void IncreaseSpecialtyPt(std::int16_t value);
    void DecreaseSpecialtyPt(std::int16_t value);

    std::uint16_t GetAcquiredSpecialtyNum();
    std::uint16_t* GetAcquiredSpecialtyKind();

    int CanAcquireSpecialty(std::uint16_t specialtyKind);
    int IsAcquiredSpecialty(std::uint16_t specialtyKind);
    void AcquireSpecialty(std::uint16_t specialtyKind, std::uint16_t* outSkillNum,
                          std::uint16_t* outSkillKinds, std::uint16_t* outQuestKinds,
                          unsigned int* outQuestValues);
    int GetAcquireAbleSpecialtyList(char category, std::uint16_t* outSpecialtyKindList);

    void FillOutSpecialtyInfo(CMofMsg* msg);

    std::uint16_t GetAcquiredGenericSpecialty(std::uint16_t* outSpecialtyKinds);
    std::uint16_t GetAcquiredMakingItemSpecialty(std::uint16_t* outSpecialtyKinds);
    std::uint16_t GetAcquiredTransformSpecialty(std::uint16_t* outSpecialtyKinds);

    int CanResetSpecialty(int includeCircle);
    int CanResetCircleSpecialty(int allow);
    void ResetSpecialty(int includeCircle, int* outDeletedSpecialtyNum, std::uint16_t* outDeletedSpecialty,
                        int* outDeletedSkillNum, std::uint16_t* outDeletedSkillKinds);
    void ResetCircleSpecialty(int* outDeletedSpecialtyNum, std::uint16_t* outDeletedSpecialty,
                              int* outDeletedSkillNum, std::uint16_t* outDeletedSkillKinds);
    void DeleteSpecialtySkill(std::uint16_t specialtyKind, int* outDeletedSkillNum,
                              std::uint16_t* outDeletedSkillKinds);

    static cltSpecialtyKindInfo* m_pclSpecialtyKindInfo;

private:
    cltGradeSystem* m_pGradeSystem = nullptr;
    cltSkillSystem* m_pSkillSystem = nullptr;
    cltLevelSystem* m_pLevelSystem = nullptr;
    cltQuestSystem* m_pQuestSystem = nullptr;

    std::int16_t m_specialtyPt = 0;
    std::array<std::uint16_t, 30> m_acquiredSpecialtyKinds{};
    std::uint16_t m_acquiredSpecialtyNum = 0;
};
