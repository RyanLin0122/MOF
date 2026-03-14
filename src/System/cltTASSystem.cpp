#include "System/cltTASSystem.h"

namespace {
constexpr int kFunctionQuestSetTeacher = 13;
constexpr int kFunctionQuestAddStudent = 26;
constexpr int kStudentMax = 10;
constexpr int kClassMateMax = 10;
constexpr std::uint8_t kDefaultClass = 0;

static bool IsEmptyName(const std::array<char, 32>& n) {
    return n[0] == '\0';
}

static bool NameEqual(const std::array<char, 32>& a, const char* b) {
    return b && std::strcmp(a.data(), b) == 0;
}

static void AssignName(std::array<char, 32>& out, const char* in) {
    out.fill(0);
    if (!in) return;
    std::strncpy(out.data(), in, out.size() - 1);
}

static int FindByName(const std::array<strTASMemberInfo, kStudentMax>& arr, int n, const char* name) {
    if (!name || !name[0]) return -1;
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

static std::int64_t ClampAdd64(std::int64_t a, std::int64_t b) {
    if (b > 0 && a > (INT64_MAX - b)) return INT64_MAX;
    if (b < 0 && a < (INT64_MIN - b)) return INT64_MIN;
    return a + b;
}
} // namespace

cltTASSystem::cltTASSystem() { Free(); }

void cltTASSystem::Initialize(cltLevelSystem* levelSystem, cltMoneySystem* moneySystem, cltQuestSystem* questSystem,
                              strTASMemberInfo* teacher, char teacherLevel, std::int64_t gatherExp,
                              int classMateCount, const strTASMemberInfo* classMates,
                              int studentCount, const strTASMemberInfo* students) {
    levelSystem_ = levelSystem;
    moneySystem_ = moneySystem;
    questSystem_ = questSystem;

    teacher_ = teacher ? *teacher : strTASMemberInfo{};
    myTeacherLevel_ = static_cast<std::uint8_t>(teacherLevel);
    gatherExp_ = std::max<std::int64_t>(0, gatherExp);

    classMates_.fill({});
    classMateCount_ = std::clamp(classMateCount, 0, kClassMateMax);
    for (int i = 0; i < classMateCount_; ++i) {
        classMates_[i] = classMates ? classMates[i] : strTASMemberInfo{};
    }

    students_.fill({});
    studentCount_ = std::clamp(studentCount, 0, kStudentMax);
    for (int i = 0; i < studentCount_; ++i) {
        students_[i] = students ? students[i] : strTASMemberInfo{};
    }
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
        for (std::uint8_t i = 0; i < classMateCount && i < classMates.size(); ++i) {
            msg->Get_Z1(classMates[i].name.data(), 0, 0, nullptr);
            msg->Get_BYTE(&classMates[i].level);
        }

        msg->Get_BYTE(&studentCount);
        for (std::uint8_t i = 0; i < studentCount && i < students.size(); ++i) {
            msg->Get_Z1(students[i].name.data(), 0, 0, nullptr);
            msg->Get_BYTE(&students[i].level);
            msg->Get_BYTE(&students[i].classKind);
            msg->Get_BYTE(&students[i].grade);
        }

        msg->Get_LONG(&gatherExp);
        msg->Get_BYTE(&teacherLevel);
    }

    Initialize(levelSystem, moneySystem, questSystem,
               &teacher, static_cast<char>(teacherLevel), gatherExp,
               classMateCount, classMates.data(),
               studentCount, students.data());
}

void cltTASSystem::Free() {
    levelSystem_ = nullptr;
    moneySystem_ = nullptr;
    questSystem_ = nullptr;

    teacher_ = {};
    classMates_.fill({});
    students_.fill({});

    myTeacherLevel_ = 0;
    gatherExp_ = 0;
    classMateCount_ = 0;
    studentCount_ = 0;
}

int cltTASSystem::CanSetTeacher(std::uint8_t teacherLevel) {
    if (IsThereTeacher()) return 1501;
    if (!levelSystem_) return 1500;

    const auto myLv = levelSystem_->GetLevel();
    return (myLv + 10 <= teacherLevel) ? 0 : 1500;
}

void cltTASSystem::SetTeacher(char* teacherName, char classKind, char level, CMofMsg* msg,
                              std::uint16_t* questKinds, unsigned int* questValues) {
    AssignName(teacher_.name, teacherName);
    teacher_.classKind = static_cast<std::uint8_t>(classKind);
    myTeacherLevel_ = static_cast<std::uint8_t>(level);

    std::uint8_t count = 0;
    if (msg) {
        msg->Get_BYTE(&count);
        classMateCount_ = std::min<int>(count, kClassMateMax);
        for (int i = 0; i < classMateCount_; ++i) {
            msg->Get_Z1(classMates_[i].name.data(), 0, 0, nullptr);
            msg->Get_BYTE(&classMates_[i].level);
        }
    }

    if (questSystem_) {
        questSystem_->CompleteFunctionQuest(kFunctionQuestSetTeacher, questKinds, questValues);
    }
}

void cltTASSystem::DelTeacher() {
    teacher_ = {};
    classMates_.fill({});
    classMateCount_ = 0;
    myTeacherLevel_ = 0;
}

void cltTASSystem::AddClassMate(char* name, std::uint8_t level) {
    if (!name || !name[0]) return;
    if (classMateCount_ >= kClassMateMax) return;
    if (FindByName(classMates_, classMateCount_, name) >= 0) return;

    auto& m = classMates_[classMateCount_++];
    AssignName(m.name, name);
    m.level = level;
    m.classKind = kDefaultClass;
    m.grade = 0;
}

void cltTASSystem::DelClassMate(char* name) {
    const int idx = FindByName(classMates_, classMateCount_, name);
    EraseAt(classMates_, classMateCount_, idx);
}

int cltTASSystem::CanAddStudent(char* name) {
    if (!name) return 0;
    if (studentCount_ >= kStudentMax) return 0;

    if (FindByName(students_, studentCount_, name) >= 0) return 0;
    if (teacher_.name[0] && std::strcmp(teacher_.name.data(), name) == 0) return 0;

    return 1;
}

void cltTASSystem::AddStudent(char* name, char level, char classKind,
                              std::uint16_t* questKinds, unsigned int* questValues) {
    if (!CanAddStudent(name)) return;

    auto& s = students_[studentCount_++];
    AssignName(s.name, name);
    s.level = static_cast<std::uint8_t>(level);
    s.classKind = static_cast<std::uint8_t>(classKind);
    s.grade = 0;

    if (questSystem_) {
        questSystem_->CompleteFunctionQuest(kFunctionQuestAddStudent, questKinds, questValues);
    }
}

int cltTASSystem::CanDelStudent(char* name, int needCost, int* outCost) {
    if (outCost) *outCost = 0;

    const int idx = FindByName(students_, studentCount_, name);
    if (idx < 0) return 0;

    const int cost = GetDelStudentCost(students_[idx].level);
    if (outCost) *outCost = cost;

    if (!needCost) return 1;
    return moneySystem_ ? moneySystem_->CanDecreaseMoney(cost) : 0;
}

int cltTASSystem::DelStudent(char* name, int needCost) {
    int cost = 0;
    if (!CanDelStudent(name, needCost, &cost)) return 0;

    if (needCost && moneySystem_) moneySystem_->DecreaseMoney(cost);

    const int idx = FindByName(students_, studentCount_, name);
    if (idx < 0) return 0;

    EraseAt(students_, studentCount_, idx);
    return 1;
}

strTASMemberInfo* cltTASSystem::GetTeacher() { return &teacher_; }

void cltTASSystem::GetClassMate(int* outCount, strTASMemberInfo* outMembers) {
    if (outCount) *outCount = classMateCount_;
    if (!outMembers) return;

    for (int i = 0; i < classMateCount_; ++i) outMembers[i] = classMates_[i];
}

void cltTASSystem::GetStudent(int* outCount, strTASMemberInfo* outMembers) {
    if (outCount) *outCount = studentCount_;
    if (!outMembers) return;

    for (int i = 0; i < studentCount_; ++i) outMembers[i] = students_[i];
}

std::int64_t cltTASSystem::GetGatherExpByStudents() {
    return std::max<std::int64_t>(0, gatherExp_);
}

std::int64_t cltTASSystem::GetMaxGatherableExpByStudents() {
    if (!levelSystem_) return 0;
    // ground-truth: return 5% of total exp at current level
    return (levelSystem_->GetTotalExpOfLevel() * 5) / 100;
}

int cltTASSystem::GetMaxGatherExpPercent(strTASMemberInfo* member) {
    if (!member) return 0;
    return GetMaxGatherExpPercent(member->level, member->classKind);
}

int cltTASSystem::GetMaxGatherExpPercent(std::uint8_t teacherLevel, std::uint8_t myLevel) {
    // ground-truth:
    // if teacherLevel >= myLevel => 5
    // else ((myLevel - teacherLevel) / 2) + 5, capped at 25
    if (teacherLevel >= myLevel) return 5;
    int value = (static_cast<int>(myLevel) - static_cast<int>(teacherLevel)) / 2 + 5;
    if (value > 25) value = 25;
    return value;
}

void cltTASSystem::OnLevelUp(std::uint8_t oldLevel, std::int64_t* outGatherExp, int* outMoney) {
    if (outGatherExp) *outGatherExp = 0;
    if (outMoney) *outMoney = 0;

    if (!AmIStudent() || !levelSystem_) return;

    const std::uint8_t curLv = levelSystem_->GetLevel();
    if (oldLevel >= curLv) return;

    std::int64_t gathered = 0;
    int money = 0;

    for (std::uint8_t lv = oldLevel; lv < curLv; ++lv) {
        const std::int64_t p = GetMaxGatherExpPercent(myTeacherLevel_, lv);
        const std::int64_t add = (p * cltLevelSystem::GetExpByLevel(static_cast<std::uint8_t>(lv + 1))) / 100;
        gathered = ClampAdd64(gathered, add);
    }

    if (myTeacherLevel_ + 20 > oldLevel && myTeacherLevel_ + 20 <= curLv) money += 100000;
    if (myTeacherLevel_ + 30 > oldLevel && myTeacherLevel_ + 30 <= curLv) money += 500000;

    gatherExp_ = ClampAdd64(gatherExp_, gathered);

    if (outGatherExp) *outGatherExp = gathered;
    if (outMoney) *outMoney = money;
}

int cltTASSystem::IsThereTeacher() {
    return !IsEmptyName(teacher_.name);
}

int cltTASSystem::AmITeacher() {
    return studentCount_ > 0;
}

int cltTASSystem::AmIStudent() {
    return myTeacherLevel_ != 0;
}

int cltTASSystem::GetMyGatherExpForTeacher() {
    if (!levelSystem_) return 0;
    return GetMaxGatherExpPercent(myTeacherLevel_, levelSystem_->GetLevel());
}

int cltTASSystem::GetDelStudentCost(std::uint8_t level) {
    return 1000 * level;
}

void cltTASSystem::FillOutClassMateInfo(CMofMsg* msg) {
    if (!msg) return;

    msg->Put_BYTE(static_cast<std::uint8_t>(classMateCount_));
    for (int i = 0; i < classMateCount_; ++i) {
        msg->Put_Z1(classMates_[i].name.data());
        msg->Put_BYTE(classMates_[i].level);
    }
}

void cltTASSystem::Refresh(CMofMsg* msg) {
    if (!msg) return;

    std::uint8_t type = 0;
    std::uint8_t count = 0;
    msg->Get_BYTE(&type);
    msg->Get_BYTE(&count);

    if (type == 1) {
        classMateCount_ = 0;

        for (std::uint8_t i = 0; i < count; ++i) {
            strTASMemberInfo row{};
            msg->Get_Z1(row.name.data(), 0, 0, nullptr);
            msg->Get_BYTE(&row.level);

            if (i == 0) {
                if (row.name[0]) {
                    myTeacherLevel_ = row.level;
                } else if (IsThereTeacher()) {
                    DelTeacher();
                }
                continue;
            }

            if (classMateCount_ < kClassMateMax) {
                classMates_[classMateCount_++] = row;
            }
        }
    } else if (type == 2) {
        studentCount_ = 0;

        for (std::uint8_t i = 0; i < count && studentCount_ < kStudentMax; ++i) {
            auto& s = students_[studentCount_++];
            msg->Get_Z1(s.name.data(), 0, 0, nullptr);
            msg->Get_BYTE(&s.level);
            msg->Get_BYTE(&s.classKind);
            msg->Get_BYTE(&s.grade);
        }

        msg->Get_INT64(&gatherExp_);
        gatherExp_ = std::max<std::int64_t>(0, gatherExp_);
    }
}

int cltTASSystem::CanSendPostit(char* target) {
    if (!target || !target[0]) return 0;

    if (NameEqual(teacher_.name, target)) return 1;

    for (int i = 0; i < studentCount_; ++i) {
        if (NameEqual(students_[i].name, target)) return 1;
    }
    return 0;
}
