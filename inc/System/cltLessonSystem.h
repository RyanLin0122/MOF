#pragma once

#include <array>
#include <cstdint>

class CMofMsg;
class cltBaseInventory;
class cltEmblemSystem;
class cltItemKindInfo;
class cltLessonKindInfo;
class cltItemList;
struct strLessonKindInfo;

struct strLessonHistory {
    std::uint32_t successCount = 0;
    std::uint32_t failCount = 0;
};

class cltLessonSystem {
public:
    static void InitializeStaticVariable(cltLessonKindInfo* lessonKindInfo, cltItemKindInfo* itemKindInfo);

    cltLessonSystem() = default;
    ~cltLessonSystem();

    void Initialize(cltBaseInventory* baseInventory, cltEmblemSystem* emblemSystem,
                    std::uint16_t traningItemKind, const std::uint8_t* lessonSchedule,
                    const std::uint8_t* lessonState, unsigned int swordPt, unsigned int bowPt,
                    unsigned int theologyPt, unsigned int magicPt, unsigned int totalSwordPt,
                    unsigned int totalBowPt, unsigned int totalTheologyPt, unsigned int totalMagicPt,
                    const strLessonHistory (*lessonHistory)[3]);
    void Initialize(cltBaseInventory* baseInventory, CMofMsg* msg);
    void Free();

    std::uint8_t* GetLessonSchedule();
    std::uint8_t GetLessonSchedule(int index);
    std::uint8_t* GetLessonState();
    std::uint8_t GetLessonState(int index);

    int CanCancelSchedule();
    void CancelSchedule();

    strLessonHistory* GetLessonHistory(std::uint8_t lessonType, std::uint8_t rankOrMode);
    std::uint16_t GetTraningItemKind();

    int CanSetupSchedule(int traningItemKind, const std::uint8_t* lessonSchedule);
    void SetupSchedule(std::uint16_t traningItemKind, const std::uint8_t* lessonSchedule, cltItemList* itemList, std::uint16_t* outUsingCardKind);

    int IsEmptySchedule();

    std::uint32_t GetSwordLessonPt();
    std::uint32_t GetBowLessonPt();
    std::uint32_t GetTheologyLessonPt();
    std::uint32_t GetMagicLessonPt();

    std::uint32_t GetTotalSwordLessonPt();
    std::uint32_t GetTotalBowLessonPt();
    std::uint32_t GetTotalTheologyLessonPt();
    std::uint32_t GetTotalMagicLessonPt();
    int GetTotalAllLessonPt();

    void IncLessonPt_Sword(unsigned int value);
    void IncLessonPt_Bow(unsigned int value);
    void IncLessonPt_Theology(unsigned int value);
    void IncLessonPt_Magic(unsigned int value);

    void DecLessonPt_Sword(unsigned int value);
    void DecLessonPt_Bow(unsigned int value);
    void DecLessonPt_Theology(unsigned int value);
    void DecLessonPt_Magic(unsigned int value);

    int CanTraningLesson(unsigned int lessonKind);
    int CanTraningLessonByType(std::uint8_t lessonType);
    int TraningLessonFinished(unsigned int seed, std::uint8_t hitType, int success, unsigned int* inoutPt);

    void ScheduleFinished();
    int GetThisLessonInfo(std::uint8_t* outIndex, std::uint8_t* outType, std::uint8_t* outKind);

    void FillOutLessonInfo(CMofMsg* msg);

    static cltLessonKindInfo* m_pclLessonKindInfo;
    static cltItemKindInfo* m_pclItemKindInfo;

private:
    cltBaseInventory* m_pBaseInventory = nullptr;
    cltEmblemSystem* m_pEmblemSystem = nullptr;

    std::uint16_t m_wTraningItemKind = 0;
    std::uint16_t m_pad = 0;

    std::uint32_t m_totalSwordLessonPt = 0;
    std::uint32_t m_totalBowLessonPt = 0;
    std::uint32_t m_totalTheologyLessonPt = 0;
    std::uint32_t m_totalMagicLessonPt = 0;

    std::uint32_t m_currentSwordLessonPt = 0;
    std::uint32_t m_currentBowLessonPt = 0;
    std::uint32_t m_currentTheologyLessonPt = 0;
    std::uint32_t m_currentMagicLessonPt = 0;

    std::array<std::uint8_t, 4> m_lessonSchedule{};
    std::array<std::uint8_t, 4> m_lessonState{};

    std::array<std::array<strLessonHistory, 3>, 4> m_lessonHistory{};
};
