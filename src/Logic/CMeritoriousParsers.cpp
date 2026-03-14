#include "Logic/CMeritoriousParsers.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstring>

namespace {
std::uint16_t Translate5Code(const char* s) {
    if (!s || std::strlen(s) != 5) return 0;
    const int head = (std::toupper(static_cast<unsigned char>(s[0])) + 31) << 11;
    const unsigned int tail = static_cast<unsigned int>(std::atoi(s + 1));
    if (tail >= 0x800u) return 0;
    return static_cast<std::uint16_t>(head | tail);
}
}


CExpRewardParser::CExpRewardParser() = default;

CExpRewardParser::~CExpRewardParser() {
    Free();
}

int CExpRewardParser::Initialize(char* fileName) {
    FILE* fp = g_clTextFileManager.fopen(fileName);
    if (!fp) return 0;

    Free();

    char buffer[1024]{};
    char delim[] = "\t\n";
    int ok = 0;

    if (std::fgets(buffer, sizeof(buffer), fp) &&
        std::fgets(buffer, sizeof(buffer), fp) &&
        std::fgets(buffer, sizeof(buffer), fp)) {
        int line = 1;
        if (std::fgets(buffer, sizeof(buffer), fp)) {
            while (true) {
                char* tok = std::strtok(buffer, delim);
                if (!tok) break;
                if (std::atoi(tok) != line) break;
                tok = std::strtok(nullptr, delim); if (!tok) break;
                infos_[line].circleQuizRewardExp = static_cast<unsigned int>(std::atoi(tok));
                tok = std::strtok(nullptr, delim); if (!tok) break;
                infos_[line].meritoriousRewardExp = static_cast<unsigned int>(std::atoi(tok));
                ++line;
                if (line >= static_cast<int>(infos_.size())) break;
                if (!std::fgets(buffer, sizeof(buffer), fp)) {
                    ok = 1;
                    break;
                }
            }
            count_ = line - 1;
        }
    }

    g_clTextFileManager.fclose(fp);
    return ok;
}

void CExpRewardParser::Free() {
    infos_.fill({});
    count_ = 0;
}

unsigned int CExpRewardParser::GetCircleQuizRewardExp(int level) {
    if (level < 1 || level > count_) return 0;
    return infos_[level].circleQuizRewardExp;
}

unsigned int CExpRewardParser::GetMeritoriousRewardExp(int level) {
    if (level < 1 || level > count_) return 0;
    return infos_[level].meritoriousRewardExp;
}

CSupplyMeritoriousParser::CSupplyMeritoriousParser() = default;

CSupplyMeritoriousParser::~CSupplyMeritoriousParser() = default;

bool IsDigit(const char* t)
{
    if (!t || !*t) return false;
    for (const unsigned char* p = (const unsigned char*)t; *p; ++p)
        if (!std::isdigit(*p)) return false;
    return true;
}

int CSupplyMeritoriousParser::Initialize(char* fileName) {
    FILE* fp = g_clTextFileManager.fopen(fileName);
    if (!fp) return 0;

    infos_.clear();
    char buffer[1024]{};
    char delim[] = "\t\n";
    int ok = 0;

    if (std::fgets(buffer, sizeof(buffer), fp) &&
        std::fgets(buffer, sizeof(buffer), fp) &&
        std::fgets(buffer, sizeof(buffer), fp)) {
        if (std::fgets(buffer, sizeof(buffer), fp)) {
            while (true) {
                char* tok = std::strtok(buffer, delim);
                if (!tok) break;
                const auto id = TranslateKindCode(tok);
                (void)std::strtok(nullptr, delim);
                tok = std::strtok(nullptr, delim); if (!tok) break;
                const auto itemKind = TranslateKindCode(tok);
                tok = std::strtok(nullptr, delim); if (!tok || !IsDigit(tok)) break;
                const auto itemCount = static_cast<std::uint16_t>(std::atoi(tok));
                tok = std::strtok(nullptr, delim); if (!tok || !IsDigit(tok)) break;
                const auto rewardPoint = static_cast<std::uint16_t>(std::atoi(tok));
                tok = std::strtok(nullptr, delim); if (!tok || !IsDigit(tok)) break;
                const auto rewardLibi = static_cast<std::uint32_t>(std::atoi(tok));
                AddSupplyMeritorious(id, itemKind, itemCount, rewardPoint, rewardLibi);
                if (!std::fgets(buffer, sizeof(buffer), fp)) {
                    ok = 1;
                    break;
                }
            }
        }
    }

    g_clTextFileManager.fclose(fp);
    return ok;
}

std::uint16_t CSupplyMeritoriousParser::GetSupplyMeritoriousID() {
    if (infos_.empty()) return 0;
    std::srand(timeGetTime());
    const auto n = static_cast<std::size_t>(std::rand() % infos_.size());
    auto it = infos_.begin();
    std::advance(it, static_cast<long>(n));
    return it->first;
}

void CSupplyMeritoriousParser::AddSupplyMeritorious(std::uint16_t id, std::uint16_t itemKind,
                                                     std::uint16_t itemCount, std::uint16_t rewardPoint,
                                                     std::uint32_t rewardLibi) {
    if (infos_.find(id) == infos_.end()) {
        infos_.emplace(id, CSupplyMeritoriousInfo(itemKind, itemCount, rewardPoint, rewardLibi));
    }
}

std::uint16_t CSupplyMeritoriousParser::TranslateKindCode(char* kindCode) {
    return Translate5Code(kindCode);
}

CMeritoriousRewardParser::CMeritoriousRewardParser() {
    list_.fill(nullptr);
    count_ = 0;
}

CMeritoriousRewardParser::~CMeritoriousRewardParser() {
    for (auto*& p : list_) { delete p; p = nullptr; }
}

int CMeritoriousRewardParser::Initialize(char* fileName) {
    FILE* fp = g_clTextFileManager.fopen(fileName);
    if (!fp) return 0;

    char buffer[1024]{};
    char delim[] = "\t\n";
    int ok = 0;

    if (std::fgets(buffer, sizeof(buffer), fp) &&
        std::fgets(buffer, sizeof(buffer), fp) &&
        std::fgets(buffer, sizeof(buffer), fp)) {
        if (std::fgets(buffer, sizeof(buffer), fp)) {
            while (true) {
                char* tok = std::strtok(buffer, delim);
                if (!tok || !IsDigit(tok)) break;
                const std::uint16_t idx = static_cast<std::uint16_t>(std::atoi(tok));
                if (idx >= list_.size() || list_[idx]) break;

                auto* info = new stMeritoriousRewardItemInfo{};
                info->rewardID = idx;
                list_[idx] = info;

                if (!std::strtok(nullptr, delim)) break;
                tok = std::strtok(nullptr, delim); if (!tok) break;
                info->itemKind = TranslateKindCode(tok);
                tok = std::strtok(nullptr, delim); if (!tok || !IsDigit(tok)) break;
                info->requirePoint = static_cast<std::uint32_t>(std::atoi(tok));
                tok = std::strtok(nullptr, delim); if (!tok || !IsDigit(tok)) break;
                info->quantity = static_cast<std::uint16_t>(std::atoi(tok));
                ++count_;

                if (!std::fgets(buffer, sizeof(buffer), fp)) {
                    ok = 1;
                    break;
                }
            }
        }
    }

    g_clTextFileManager.fclose(fp);
    return ok;
}

stMeritoriousRewardItemInfo** CMeritoriousRewardParser::GetMeritoriousRewardItemList() { return list_.data(); }

stMeritoriousRewardItemInfo* CMeritoriousRewardParser::GetMeritoriousRewardItemList(std::uint16_t itemKind) {
    for (auto* p : list_) {
        if (p && p->itemKind == itemKind) return p;
    }
    return nullptr;
}

unsigned int CMeritoriousRewardParser::GetMeritoriousRewardItemRequirePoint(std::uint16_t itemKind) {
    auto* p = GetMeritoriousRewardItemList(itemKind);
    return p ? p->requirePoint : 0;
}

std::uint16_t CMeritoriousRewardParser::GetMeritoriousRewardItemKind(std::uint16_t index) {
    if (index >= list_.size() || !list_[index]) return 0;
    return list_[index]->itemKind;
}

std::uint16_t CMeritoriousRewardParser::TranslateKindCode(char* kindCode) {
    return Translate5Code(kindCode);
}

CMeritoriousGradeParser::CMeritoriousGradeParser() {
    list_.fill(nullptr);
    count_ = 0;
}

CMeritoriousGradeParser::~CMeritoriousGradeParser() {
    for (auto*& p : list_) { delete p; p = nullptr; }
}

int CMeritoriousGradeParser::Initialize(char* fileName) {
    FILE* fp = g_clTextFileManager.fopen(fileName);
    if (!fp) return 0;

    char buffer[1024]{};
    char delim[] = "\t\n";
    int ok = 0;

    if (std::fgets(buffer, sizeof(buffer), fp) &&
        std::fgets(buffer, sizeof(buffer), fp) &&
        std::fgets(buffer, sizeof(buffer), fp)) {
        if (std::fgets(buffer, sizeof(buffer), fp)) {
            while (true) {
                char* tok = std::strtok(buffer, delim);
                if (!tok || !IsDigit(tok)) break;
                const std::uint16_t idx = static_cast<std::uint16_t>(std::atoi(tok));
                if (idx >= list_.size() || list_[idx]) break;

                auto* info = new stMeritoriousGradeInfo{};
                info->grade = idx;
                list_[idx] = info;

                tok = std::strtok(nullptr, delim); if (!tok) break;
                info->needPoint = static_cast<std::uint32_t>(std::atoi(tok));
                if (!std::strtok(nullptr, delim)) break;
                tok = std::strtok(nullptr, delim); if (!tok || !IsDigit(tok)) break;
                info->rank = static_cast<std::uint16_t>(std::atoi(tok));
                tok = std::strtok(nullptr, delim); if (!tok || !IsDigit(tok)) break;
                info->nextGrade = static_cast<std::uint16_t>(std::atoi(tok));
                tok = std::strtok(nullptr, delim); if (!tok || !IsDigit(tok)) break;
                info->emblem = static_cast<std::uint16_t>(std::atoi(tok));
                ++count_;

                if (!std::fgets(buffer, sizeof(buffer), fp)) {
                    ok = 1;
                    break;
                }
            }
        }
    }

    g_clTextFileManager.fclose(fp);
    return ok;
}

stMeritoriousGradeInfo** CMeritoriousGradeParser::GetMeritoriousGradeInfoList() { return list_.data(); }

stMeritoriousGradeInfo* CMeritoriousGradeParser::GetMeritoriousGradeInfoList(std::uint16_t index) {
    if (index >= list_.size()) return nullptr;
    return list_[index];
}

int CMeritoriousGradeParser::CalcMeritoriousGrade(std::uint16_t point, std::uint16_t grade,
                                                  std::uint16_t* outGrade, std::uint16_t* outGradePoint) {
    if (!point || grade == 10) return 0;

    for (auto* p : list_) {
        if (!p) continue;
        if (p->nextGrade != static_cast<std::uint16_t>(grade + 1)) continue;
        if (p->needPoint <= point) {
            if (outGrade) *outGrade = p->nextGrade;
            if (outGradePoint) *outGradePoint = p->emblem;
            return 1;
        }
        if (outGrade) *outGrade = grade;
        if (outGradePoint) *outGradePoint = 0;
        return 0;
    }
    return 0;
}
