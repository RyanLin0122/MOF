#include "System/cltGradeSystem.h"

cltGradeSystem::cltGradeSystem() = default;

void cltGradeSystem::Initialize(cltSpecialtySystem* specialtySystem, cltClassSystem* classSystem, std::uint8_t grade) {
    m_pSpecialtySystem = specialtySystem;
    m_pClassSystem = classSystem;
    m_grade = grade;
}

void cltGradeSystem::Free() { m_grade = 1; }

std::uint8_t cltGradeSystem::GetGrade() { return m_grade; }

int cltGradeSystem::CanUpGrade() { return m_grade < 0x14u; }

void cltGradeSystem::UpGrade() {
    int next = static_cast<int>(m_grade) + 1;
    if (next >= 20) {
        next = 20;
    }
    m_grade = static_cast<std::uint8_t>(next);
    if (m_pSpecialtySystem) {
        m_pSpecialtySystem->IncreaseSpecialtyPt(2);
    }
}

int cltGradeSystem::IsLastGrade() { return m_grade >= 0x14u; }

void cltGradeSystem::FillOutGradeInfo(CMofMsg* msg) {
    if (msg) {
        msg->Put_BYTE(m_grade);
    }
}
