#include "System/cltTASSystem.h"

namespace {
constexpr int kFunctionQuestSetTeacher = 13;
constexpr int kFunctionQuestAddStudent = 26;
constexpr int kStudentMax = 10;
constexpr int kClassMateMax = 10;

static int FindByName(const std::array<strTASMemberInfo, kStudentMax>& arr, int n, const char* name) {
    for (int i = 0; i < n; ++i) {
        if (std::strcmp(arr[i].name.data(), name) == 0) return i;
    }
    return -1;
}

template <std::size_t N>
static void EraseAt(std::array<strTASMemberInfo, N>& arr, int& n, int idx) {
    if (idx < 0 || idx >= n) return;
    for (int i = idx; i + 1 < n; ++i) arr[i] = arr[i + 1];
    arr[n - 1] = {};
    --n;
}
} // namespace

cltTASSystem::cltTASSystem() { Free(); }

// Ground truth: direct memcpy of full arrays, no clamping
void cltTASSystem::Initialize(cltLevelSystem* levelSystem, cltMoneySystem* moneySystem, cltQuestSystem* questSystem,
                              strTASMemberInfo* teacher, char teacherLevel, std::int64_t gatherExp,
                              int classMateCount, const strTASMemberInfo* classMates,
                              int studentCount, const strTASMemberInfo* students) {
    levelSystem_ = levelSystem;
    moneySystem_ = moneySystem;
    questSystem_ = questSystem;

    std::memcpy(&teacher_, teacher, sizeof(teacher_));
    std::memcpy(classMates_.data(), classMates, sizeof(classMates_));
    std::memcpy(students_.data(), students, sizeof(students_));

    myTeacherLevel_ = static_cast<std::uint8_t>(teacherLevel);
    gatherExp_ = gatherExp;
    classMateCount_ = classMateCount;
    studentCount_ = studentCount;
}

void cltTASSystem::Initialize(cltLevelSystem* levelSystem, cltMoneySystem* moneySystem, cltQuestSystem* questSystem, CMofMsg* msg) {
    strTASMemberInfo teacher{};
    std::array<strTASMemberInfo, kClassMateMax> classMates{};
    std::array<strTASMemberInfo, kStudentMax> students{};

    std::uint8_t classMateCount = 0;
    std::uint8_t studentCount = 0;
    int gatherExp = 0;
    std::uint8_t teacherLevel = 0;

    if (msg) {
        msg->Get_Z1(teacher.name.data(), 0, 0, nullptr);

        msg->Get_BYTE(&classMateCount);
        for (std::uint8_t i = 0; i < classMateCount; ++i) {
            msg->Get_Z1(classMates[i].name.data(), 0, 0, nullptr);
            msg->Get_BYTE(&classMates[i].level);
        }

        msg->Get_BYTE(&studentCount);
        for (std::uint8_t i = 0; i < studentCount; ++i) {
            msg->Get_Z1(students[i].name.data(), 0, 0, nullptr);
            msg->Get_BYTE(&students[i].level);
            msg->Get_BYTE(&students[i].classKind);
            msg->Get_BYTE(&students[i].grade);
        }

        msg->Get_LONG(&gatherExp);
        msg->Get_BYTE(&teacherLevel);
    }

    Initialize(levelSystem, moneySystem, questSystem,
               &teacher, static_cast<char>(teacherLevel), static_cast<std::int64_t>(gatherExp),
               classMateCount, classMates.data(),
               studentCount, students.data());
}

// Ground truth: only clears levelSystem_; moneySystem_ and questSystem_ are NOT cleared
void cltTASSystem::Free() {
    levelSystem_ = nullptr;

    teacher_ = {};
    classMates_.fill({});
    students_.fill({});

    myTeacherLevel_ = 0;
    gatherExp_ = 0;
    classMateCount_ = 0;
    studentCount_ = 0;
}

// Ground truth: no null guard on levelSystem_
int cltTASSystem::CanSetTeacher(std::uint8_t teacherLevel) {
    if (IsThereTeacher()) return 1501;
    return (levelSystem_->GetLevel() + 10 <= teacherLevel) ? 0 : 0x5DC;
}

// Ground truth: stores classKind param into teacher_.level (struct offset 34), not teacher_.classKind
void cltTASSystem::SetTeacher(char* teacherName, char classKind, char level, CMofMsg* msg,
                              std::uint16_t* questKinds, unsigned int* questValues) {
    std::strcpy(teacher_.name.data(), teacherName);
    teacher_.level = static_cast<std::uint8_t>(classKind);
    myTeacherLevel_ = static_cast<std::uint8_t>(level);

    std::uint8_t count = 0;
    msg->Get_BYTE(&count);
    classMateCount_ = count;
    for (int i = 0; i < classMateCount_; ++i) {
        msg->Get_Z1(classMates_[i].name.data(), 0, 0, nullptr);
        msg->Get_BYTE(&classMates_[i].level);
    }

    questSystem_->CompleteFunctionQuest(kFunctionQuestSetTeacher, questKinds, questValues);
}

void cltTASSystem::DelTeacher() {
    teacher_ = {};
    classMates_.fill({});
    classMateCount_ = 0;
    myTeacherLevel_ = 0;
}

// Ground truth: only checks count < 10, no null/duplicate check
void cltTASSystem::AddClassMate(char* name, std::uint8_t level) {
    if (classMateCount_ >= kClassMateMax) return;
    std::strncpy(classMates_[classMateCount_].name.data(), name, 31);
    classMates_[classMateCount_].level = level;
    ++classMateCount_;
}

void cltTASSystem::DelClassMate(char* name) {
    const int idx = FindByName(classMates_, classMateCount_, name);
    EraseAt(classMates_, classMateCount_, idx);
}

// Ground truth: checks count >= 10 first, then teacher name (no empty check), then students
int cltTASSystem::CanAddStudent(char* name) {
    if (studentCount_ >= kStudentMax) return 0;
    if (std::strcmp(teacher_.name.data(), name) == 0) return 0;
    for (int i = 0; i < studentCount_; ++i) {
        if (std::strcmp(students_[i].name.data(), name) == 0) return 0;
    }
    return 1;
}

// Ground truth: direct count check, grade = classKind (not 0)
void cltTASSystem::AddStudent(char* name, char level, char classKind,
                              std::uint16_t* questKinds, unsigned int* questValues) {
    if (studentCount_ >= kStudentMax) return;
    int i = studentCount_;
    std::strncpy(students_[i].name.data(), name, 31);
    students_[i].level    = static_cast<std::uint8_t>(level);
    students_[i].classKind = static_cast<std::uint8_t>(classKind);
    students_[i].grade    = static_cast<std::uint8_t>(classKind);  // ground truth: grade = classKind
    ++studentCount_;
    questSystem_->CompleteFunctionQuest(kFunctionQuestAddStudent, questKinds, questValues);
}

// Ground truth: *outCost only written when needCost!=0 AND student found; never written on not-found
int cltTASSystem::CanDelStudent(char* name, int needCost, int* outCost) {
    const int idx = FindByName(students_, studentCount_, name);
    if (idx < 0) return 0;

    if (!needCost) return 1;

    // GT uses struct[33] = grade for cost, not level
    const int cost = GetDelStudentCost(students_[idx].grade);
    *outCost = cost;
    return moneySystem_->CanDecreaseMoney(cost);
}

// Ground truth: returns cost (not 1); zeroes gatherExp_ when last student removed
int cltTASSystem::DelStudent(char* name, int needCost) {
    const int idx = FindByName(students_, studentCount_, name);
    if (idx < 0) return 0;

    int cost = 0;
    if (needCost) {
        cost = GetDelStudentCost(students_[idx].grade);  // GT: struct[33] = grade
        moneySystem_->DecreaseMoney(cost);
    }

    EraseAt(students_, studentCount_, idx);

    if (studentCount_ == 0) {
        gatherExp_ = 0;
    }

    return cost;
}

strTASMemberInfo* cltTASSystem::GetTeacher() { return &teacher_; }

// Ground truth: always memcpy full 350 bytes, no null check on outMembers
void cltTASSystem::GetClassMate(int* outCount, strTASMemberInfo* outMembers) {
    *outCount = classMateCount_;
    std::memcpy(outMembers, classMates_.data(), sizeof(classMates_));
}

void cltTASSystem::GetStudent(int* outCount, strTASMemberInfo* outMembers) {
    *outCount = studentCount_;
    std::memcpy(outMembers, students_.data(), sizeof(students_));
}

// Ground truth: returns raw value, no clamping
std::int64_t cltTASSystem::GetGatherExpByStudents() {
    return gatherExp_;
}

std::int64_t cltTASSystem::GetMaxGatherableExpByStudents() {
    return 5 * levelSystem_->GetTotalExpOfLevel() / 100;
}

// Ground truth: passes struct[32] and struct[33] — after reorder: classKind and grade
int cltTASSystem::GetMaxGatherExpPercent(strTASMemberInfo* member) {
    return GetMaxGatherExpPercent(member->classKind, member->grade);
}

int cltTASSystem::GetMaxGatherExpPercent(std::uint8_t teacherLevel, std::uint8_t myLevel) {
    if (teacherLevel >= myLevel) return 5;
    int value = (static_cast<int>(myLevel) - static_cast<int>(teacherLevel)) / 2 + 5;
    if (value > 25) value = 25;
    return value;
}

// Ground truth: only writes to *outGatherExp and *outMoney; does NOT modify gatherExp_
void cltTASSystem::OnLevelUp(std::uint8_t oldLevel, std::int64_t* outGatherExp, int* outMoney) {
    *outGatherExp = 0;
    *outMoney = 0;

    if (!AmIStudent()) return;

    if (myTeacherLevel_ + 20 > oldLevel && myTeacherLevel_ + 20 <= levelSystem_->GetLevel())
        *outMoney += 100000;
    if (myTeacherLevel_ + 30 > oldLevel && myTeacherLevel_ + 30 <= levelSystem_->GetLevel())
        *outMoney += 500000;

    const std::uint8_t curLv = levelSystem_->GetLevel();
    if (oldLevel < curLv) {
        std::uint8_t v5 = oldLevel;
        std::uint8_t v9 = oldLevel;
        do {
            ++v5;
            const std::int64_t p = GetMaxGatherExpPercent(myTeacherLevel_, v9);
            const std::int64_t add = p * cltLevelSystem::GetExpByLevel(v5) / 100;
            v9 = v5;
            *outGatherExp += add;
        } while (v5 < curLv);
    }
}

int cltTASSystem::IsThereTeacher() {
    return teacher_.name[0] != 0;
}

int cltTASSystem::AmITeacher() {
    return studentCount_ > 0;
}

int cltTASSystem::AmIStudent() {
    return myTeacherLevel_ != 0;
}

int cltTASSystem::GetMyGatherExpForTeacher() {
    const std::uint8_t myLv = levelSystem_->GetLevel();
    return GetMaxGatherExpPercent(myTeacherLevel_, myLv);
}

int cltTASSystem::GetDelStudentCost(std::uint8_t level) {
    return 1000 * level;
}

void cltTASSystem::FillOutClassMateInfo(CMofMsg* msg) {
    msg->Put_BYTE(static_cast<std::uint8_t>(classMateCount_));
    for (int i = 0; i < classMateCount_; ++i) {
        msg->Put_Z1(classMates_[i].name.data());
        msg->Put_BYTE(classMates_[i].level);
    }
}

void cltTASSystem::Refresh(CMofMsg* msg) {
    std::uint8_t type = 0;
    std::uint8_t count = 0;
    msg->Get_BYTE(&type);
    msg->Get_BYTE(&count);

    if (type == 1) {
        classMateCount_ = 0;
        int v3 = 0;
        if (count != 0) {
            do {
                char name[32]{};
                std::uint8_t byteVal = 0;
                msg->Get_Z1(name, 0, 0, nullptr);
                msg->Get_BYTE(&byteVal);

                if (v3 == 0) {
                    // Ground truth: byte goes to teacher_.level (struct[34] = offset 46), NOT myTeacherLevel_
                    if (name[0]) {
                        teacher_.level = byteVal;
                    } else if (IsThereTeacher()) {
                        DelTeacher();
                    }
                } else {
                    std::strcpy(classMates_[classMateCount_].name.data(), name);
                    classMates_[classMateCount_].level = byteVal;
                    ++classMateCount_;
                }
                ++v3;
            } while (v3 < count);
        }
    } else if (type == 2) {
        studentCount_ = 0;
        if (count != 0) {
            int v11 = 0;
            do {
                char name[32]{};
                std::uint8_t v6 = 0, v9 = 0, v8 = 0;
                msg->Get_Z1(name, 0, 0, nullptr);
                msg->Get_BYTE(&v6);   // → student.level  (struct[34])
                msg->Get_BYTE(&v9);   // → student.classKind (struct[32])
                msg->Get_BYTE(&v8);   // → student.grade  (struct[33])
                std::strcpy(students_[studentCount_].name.data(), name);
                students_[studentCount_].level    = v6;
                students_[studentCount_].classKind = v9;
                students_[studentCount_].grade    = v8;
                ++studentCount_;
                ++v11;
            } while (v11 < count);
        }
        // Ground truth: Get_LONG (4 bytes), then assign to gatherExp_ directly (no clamp)
        int rawExp = 0;
        msg->Get_LONG(&rawExp);
        gatherExp_ = static_cast<std::int64_t>(rawExp);
    }
}

int cltTASSystem::CanSendPostit(char* target) {
    if (!target) return 0;
    int found = 0;
    if (std::strlen(target)) {
        if (std::strcmp(teacher_.name.data(), target) == 0) found = 1;
        if (studentCount_ > 0) {
            for (int i = 0; i < studentCount_; ++i) {
                if (std::strcmp(students_[i].name.data(), target) == 0) found = 1;
            }
        }
    }
    return found;
}
