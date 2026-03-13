#pragma once

#include <cstdint>

class cltPKRankKindInfo;
class cltSpecialtySystem;
class CMeritoriousSystem;
class cltQuestSystem;
class CMofMsg;

class cltPKRankSystem {
public:
    static void InitializeStaticVariable(cltPKRankKindInfo* pkRankKindInfo);

    cltPKRankSystem();
    ~cltPKRankSystem() = default;

    void Initailize(cltSpecialtySystem* specialtySystem, CMeritoriousSystem* meritoriousSystem, cltQuestSystem* questSystem, CMofMsg* msg);
    void Free();

    std::uint16_t GetPKRankKind();
    int GetWinNum();
    int GetLoseNum();
    int GetPoint();
    int GetAccPoint();

    int Win(int deltaPoint, int* gainedSpecialtyPoint, std::uint16_t* questKinds, unsigned int* questValues);
    int Lose(int deltaPoint, int* gainedSpecialtyPoint, std::uint16_t* questKinds, unsigned int* questValues);
    int IncreasePoint(int deltaPoint, int* gainedSpecialtyPoint);
    int CanDecreasePoint(int value);
    void DecreasePoint(int value);
    int CanPlayPVP();
    void PlayPVP();
    int GetLastestPlayPVPTime();
    int GetTodayPlayPVPCount();

private:
    cltSpecialtySystem* specialtySystem_ = nullptr;
    CMeritoriousSystem* meritoriousSystem_ = nullptr;
    cltQuestSystem* questSystem_ = nullptr;
    std::uint16_t pkRankKind_ = 0;
    std::uint16_t pad_ = 0;
    int accPoint_ = 0;
    int point_ = 0;
    int winNum_ = 0;
    int loseNum_ = 0;
    int lastestPlayPVPTime_ = 0;
    int todayPlayPVPCount_ = 0;

    static cltPKRankKindInfo* m_pclPKRankKindInfo;
};
