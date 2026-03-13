#include "System/cltPKRankSystem.h"



cltPKRankKindInfo* cltPKRankSystem::m_pclPKRankKindInfo = nullptr;

void cltPKRankSystem::InitializeStaticVariable(cltPKRankKindInfo* pkRankKindInfo) { m_pclPKRankKindInfo = pkRankKindInfo; }

cltPKRankSystem::cltPKRankSystem() = default;

void cltPKRankSystem::Initailize(cltSpecialtySystem* specialtySystem, CMeritoriousSystem* meritoriousSystem, cltQuestSystem* questSystem, CMofMsg* msg) {
    specialtySystem_ = specialtySystem;
    meritoriousSystem_ = meritoriousSystem;
    questSystem_ = questSystem;
    if (msg) {
        msg->Get_WORD(&pkRankKind_);
        msg->Get_LONG(&accPoint_);
        msg->Get_LONG(&point_);
        msg->Get_LONG(&winNum_);
        msg->Get_LONG(&loseNum_);
        msg->Get_LONG(&lastestPlayPVPTime_);
        msg->Get_LONG(&todayPlayPVPCount_);
    }
}

void cltPKRankSystem::Free() {
    specialtySystem_ = nullptr;
    meritoriousSystem_ = nullptr;
    pkRankKind_ = 0;
    accPoint_ = 0;
    point_ = 0;
    winNum_ = 0;
    loseNum_ = 0;
    lastestPlayPVPTime_ = 0;
    todayPlayPVPCount_ = 0;
}

std::uint16_t cltPKRankSystem::GetPKRankKind() { return pkRankKind_; }
int cltPKRankSystem::GetWinNum() { return winNum_; }
int cltPKRankSystem::GetLoseNum() { return loseNum_; }
int cltPKRankSystem::GetPoint() { return point_; }
int cltPKRankSystem::GetAccPoint() { return accPoint_; }

int cltPKRankSystem::Win(int deltaPoint, int* gainedSpecialtyPoint, std::uint16_t* questKinds, unsigned int* questValues) {
    ++winNum_;
    questSystem_->CompleteFunctionQuest(29, questKinds, questValues);
    if (deltaPoint > 0) return IncreasePoint(deltaPoint, gainedSpecialtyPoint);
    if (deltaPoint < 0) DecreasePoint(-deltaPoint);
    return 0;
}

int cltPKRankSystem::Lose(int deltaPoint, int* gainedSpecialtyPoint, std::uint16_t* questKinds, unsigned int* questValues) {
    ++loseNum_;
    questSystem_->CompleteFunctionQuest(29, questKinds, questValues);
    if (deltaPoint > 0) return IncreasePoint(deltaPoint, gainedSpecialtyPoint);
    if (deltaPoint < 0) DecreasePoint(-deltaPoint);
    return 0;
}

int cltPKRankSystem::IncreasePoint(int deltaPoint, int* gainedSpecialtyPoint) {
    if (gainedSpecialtyPoint) *gainedSpecialtyPoint = 0;

    point_ += deltaPoint;
    accPoint_ += deltaPoint;
    if (point_ <= 0) point_ = 0;

    const std::uint16_t nextKind = m_pclPKRankKindInfo->GetPKRankKindByPoint(accPoint_);
    const std::uint16_t currentKind = pkRankKind_;
    if (currentKind != nextKind) {
        if (currentKind) {
            strPKRankKindInfo* cur = m_pclPKRankKindInfo->GetPKRankKindInfo(currentKind);
            strPKRankKindInfo* nxt = m_pclPKRankKindInfo->GetPKRankKindInfo(nextKind);
            if (cur && nxt && cur->needPoint < nxt->needPoint) {
                pkRankKind_ = nextKind;
                if (nxt->meritPoint) {
                    specialtySystem_->IncreaseSpecialtyPt(static_cast<unsigned short>(nxt->meritPoint));
                    if (gainedSpecialtyPoint) *gainedSpecialtyPoint = nxt->meritPoint;
                }
            }
        } else {
            strPKRankKindInfo* nxt = m_pclPKRankKindInfo->GetPKRankKindInfo(nextKind);
            pkRankKind_ = nextKind;
            if (nxt && nxt->meritPoint) {
                specialtySystem_->IncreaseSpecialtyPt(static_cast<unsigned short>(nxt->meritPoint));
                if (gainedSpecialtyPoint) *gainedSpecialtyPoint = nxt->meritPoint;
            }
        }
    }

    return 1;
}

int cltPKRankSystem::CanDecreasePoint(int value) { return point_ >= value; }

void cltPKRankSystem::DecreasePoint(int value) {
    point_ -= value;
    if (point_ <= 0) point_ = 0;
}

int cltPKRankSystem::CanPlayPVP() {
    std::time_t now = std::time(nullptr);
    std::tm nowTm = *std::localtime(&now);

    std::time_t last = static_cast<std::time_t>(lastestPlayPVPTime_);
    std::tm lastTm = *std::localtime(&last);

    return nowTm.tm_year != lastTm.tm_year || nowTm.tm_yday != lastTm.tm_yday || todayPlayPVPCount_ < 20;
}

void cltPKRankSystem::PlayPVP() {
    std::time_t now = std::time(nullptr);
    std::tm nowTm = *std::localtime(&now);

    std::time_t last = static_cast<std::time_t>(lastestPlayPVPTime_);
    std::tm lastTm = *std::localtime(&last);

    lastestPlayPVPTime_ = static_cast<int>(now);
    if (nowTm.tm_year == lastTm.tm_year && nowTm.tm_yday == lastTm.tm_yday) ++todayPlayPVPCount_;
    else todayPlayPVPCount_ = 1;
}

int cltPKRankSystem::GetLastestPlayPVPTime() { return lastestPlayPVPTime_; }
int cltPKRankSystem::GetTodayPlayPVPCount() { return todayPlayPVPCount_; }
