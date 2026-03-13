#pragma once

#include <cstdint>
#include "System/cltSpecialtySystem.h"
#include "System/cltClassSystem.h"
#include "Network/CMofMsg.h"

class cltGradeSystem {
public:
    cltGradeSystem();

    void Initialize(cltSpecialtySystem* specialtySystem, cltClassSystem* classSystem, std::uint8_t grade);
    void Free();

    std::uint8_t GetGrade();
    int CanUpGrade();
    void UpGrade();
    int IsLastGrade();

    void FillOutGradeInfo(CMofMsg* msg);

private:
    std::uint8_t m_grade = 1;
    cltSpecialtySystem* m_pSpecialtySystem = nullptr;
    cltClassSystem* m_pClassSystem = nullptr;
};
