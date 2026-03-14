#pragma once

#include <array>
#include <cstdint>

class CMofMsg;
class cltLevelSystem;
class cltMoneySystem;
class cltQuestSystem;

struct strTASMemberInfo {
    std::array<char, 32> name{};
    std::uint8_t level{};
    std::uint8_t classKind{};
    std::uint8_t grade{};
};

class cltTASSystem {
public:
    cltTASSystem();
    ~cltTASSystem() = default;

    void Initialize(cltLevelSystem* levelSystem, cltMoneySystem* moneySystem, cltQuestSystem* questSystem,
                    strTASMemberInfo* teacher, char teacherLevel, std::int64_t gatherExp,
                    int classMateCount, const strTASMemberInfo* classMates,
                    int studentCount, const strTASMemberInfo* students);
    void Initialize(cltLevelSystem* levelSystem, cltMoneySystem* moneySystem, cltQuestSystem* questSystem, CMofMsg* msg);

    void Free();
    int CanSetTeacher(std::uint8_t teacherLevel);
    void SetTeacher(char* teacherName, char classKind, char level, CMofMsg* msg, std::uint16_t* questKinds, unsigned int* questValues);
    void DelTeacher();
    void AddClassMate(char* name, std::uint8_t level);
    void DelClassMate(char* name);

    int CanAddStudent(char* name);
    void AddStudent(char* name, char level, char classKind, std::uint16_t* questKinds, unsigned int* questValues);
    int CanDelStudent(char* name, int needCost, int* outCost);
    int DelStudent(char* name, int needCost);

    strTASMemberInfo* GetTeacher();
    void GetClassMate(int* outCount, strTASMemberInfo* outMembers);
    void GetStudent(int* outCount, strTASMemberInfo* outMembers);

    std::int64_t GetGatherExpByStudents();
    std::int64_t GetMaxGatherableExpByStudents();
    static int GetMaxGatherExpPercent(strTASMemberInfo* member);
    static int GetMaxGatherExpPercent(std::uint8_t teacherLevel, std::uint8_t myLevel);

    void OnLevelUp(std::uint8_t oldLevel, std::int64_t* outGatherExp, int* outMoney);
    int IsThereTeacher();
    int AmITeacher();
    int AmIStudent();
    int GetMyGatherExpForTeacher();
    int GetDelStudentCost(std::uint8_t level);

    void FillOutClassMateInfo(CMofMsg* msg);
    void Refresh(CMofMsg* msg);
    int CanSendPostit(char* target);

private:
    static constexpr int kMaxClassMate = 10;
    static constexpr int kMaxStudent = 10;

    cltLevelSystem* levelSystem_{};
    cltMoneySystem* moneySystem_{};
    cltQuestSystem* questSystem_{};

    strTASMemberInfo teacher_{};
    std::array<strTASMemberInfo, kMaxClassMate> classMates_{};
    std::array<strTASMemberInfo, kMaxStudent> students_{};

    std::uint8_t myTeacherLevel_{};
    std::int64_t gatherExp_{};
    int classMateCount_{};
    int studentCount_{};
};
