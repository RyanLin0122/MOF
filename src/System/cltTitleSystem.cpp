#include "System/cltTitleSystem.h"

#include <cstdint>

cltTitleKindInfo* cltTitleSystem::m_pclTitleKindInfo = nullptr;
cltQuestKindInfo* cltTitleSystem::m_pclQuestKindInfo = nullptr;
cltCharKindInfo* cltTitleSystem::m_pclCharKindInfo = nullptr;
void (*cltTitleSystem::m_pOnNewTitleFuncPtr)(void*, std::uint16_t) = nullptr;

void cltTitleSystem::InitializeStaticVariable(cltTitleKindInfo* a1, cltQuestKindInfo* a2, cltCharKindInfo* a3, void (*a4)(void*, std::uint16_t)) {
    m_pclTitleKindInfo = a1;
    m_pclQuestKindInfo = a2;
    m_pclCharKindInfo = a3;
    m_pOnNewTitleFuncPtr = a4;
}

cltTitleSystem::cltTitleSystem() = default;

void cltTitleSystem::Initialize(void* a2, std::uint16_t a3, cltLevelSystem* a4, cltBaseInventory* a5) {
    owner_ = a2;
    titleKind_ = a3;
    levelSystem_ = a4;
    inventory_ = a5;
}

void cltTitleSystem::Free() {
    titleKind_ = 0;
    newTitleKind_ = 0;
    owner_ = nullptr;
    levelSystem_ = nullptr;
    inventory_ = nullptr;
}

int cltTitleSystem::CanSetTitleKind(std::uint16_t a2) { return newTitleKind_ == a2 ? titleKind_ != a2 : 0; }
void cltTitleSystem::SetTitleKind(std::uint16_t a2) { titleKind_ = a2; }
std::uint16_t cltTitleSystem::GetTitleKind() { return titleKind_; }
std::uint16_t cltTitleSystem::GetNewTitleKind() { return newTitleKind_; }

void cltTitleSystem::OnNewTitle(std::uint16_t a2) {
    if (titleKind_ != a2) {
        if (m_pOnNewTitleFuncPtr) m_pOnNewTitleFuncPtr(owner_, a2);
        newTitleKind_ = a2;
    }
}

void cltTitleSystem::OnEvent_killmonster(std::uint16_t a2, int a3) {
    void* charInfo = m_pclCharKindInfo ? m_pclCharKindInfo->GetCharKindInfo(a2) : nullptr;
    if (!charInfo) return;
    auto* list = m_pclTitleKindInfo ? m_pclTitleKindInfo->GetTitleKindInfo() : nullptr;
    if (!list) return;

    const int count = m_pclTitleKindInfo->GetTitleKindNum();
    for (int i = 0; i < count; ++i) {
        const auto& info = list[i];
        const int level = levelSystem_ ? levelSystem_->GetLevel() : 0;
        if (GetTitleKind() == info.title_code || info.condition != TGC_KILLMONSTER
            || (info.lv_min && level < info.lv_min) || (info.lv_max && level > info.lv_max)) {
            continue;
        }

        if (info.param1_code) {
            if (a2 == info.param1_code && a3 == info.param2) {
                OnNewTitle(info.title_code);
                return;
            }
        } else if (info.param3 && info.param3 + level <= static_cast<int>(reinterpret_cast<unsigned char*>(charInfo)[146])) {
            OnNewTitle(info.title_code);
            return;
        }
    }
}

void cltTitleSystem::OnEvent_clearquest(std::uint16_t a2) {
    if (!m_pclQuestKindInfo || !m_pclQuestKindInfo->GetQuestKindInfo(a2)) return;

    auto* list = m_pclTitleKindInfo ? m_pclTitleKindInfo->GetTitleKindInfo() : nullptr;
    if (!list) return;

    const int count = m_pclTitleKindInfo->GetTitleKindNum();
    for (int i = 0; i < count; ++i) {
        const auto& info = list[i];
        const int level = levelSystem_ ? levelSystem_->GetLevel() : 0;
        if (GetTitleKind() == info.title_code || info.condition != TGC_COMPLETEQUEST
            || (info.lv_min && level < info.lv_min) || (info.lv_max && level > info.lv_max)
            || info.param1_code != a2) {
            continue;
        }
        OnNewTitle(info.title_code);
        return;
    }
}

void cltTitleSystem::OnEvent_setemblem(std::uint16_t a2) {
    auto* list = m_pclTitleKindInfo ? m_pclTitleKindInfo->GetTitleKindInfo() : nullptr;
    if (!list) return;

    const int count = m_pclTitleKindInfo->GetTitleKindNum();
    for (int i = 0; i < count; ++i) {
        const auto& info = list[i];
        const int level = levelSystem_ ? levelSystem_->GetLevel() : 0;
        if (GetTitleKind() == info.title_code || info.condition != TGC_SETEMBLEM
            || (info.lv_min && level < info.lv_min) || (info.lv_max && level > info.lv_max)
            || info.param1_code != a2) {
            continue;
        }
        OnNewTitle(info.title_code);
        return;
    }
}

void cltTitleSystem::OnEvent_acquireskill(std::uint16_t a2) {
    auto* list = m_pclTitleKindInfo ? m_pclTitleKindInfo->GetTitleKindInfo() : nullptr;
    if (!list) return;

    const int count = m_pclTitleKindInfo->GetTitleKindNum();
    for (int i = 0; i < count; ++i) {
        const auto& info = list[i];
        const int level = levelSystem_ ? levelSystem_->GetLevel() : 0;
        if (GetTitleKind() == info.title_code || info.condition != TGC_GETSKILL
            || (info.lv_min && level < info.lv_min) || (info.lv_max && level > info.lv_max)
            || info.param1_code != a2) {
            continue;
        }
        OnNewTitle(info.title_code);
        return;
    }
}

void cltTitleSystem::OnEvent_getitem(int a2, int a3) {
    auto* list = m_pclTitleKindInfo ? m_pclTitleKindInfo->GetTitleKindInfo() : nullptr;
    if (!list) return;

    const int count = m_pclTitleKindInfo->GetTitleKindNum();
    for (int i = 0; i < count; ++i) {
        const auto& info = list[i];
        const int level = levelSystem_ ? levelSystem_->GetLevel() : 0;
        if (info.condition != TGC_GETITEM || (info.lv_min && level < info.lv_min)
            || (info.lv_max && level > info.lv_max) || info.param1_code != static_cast<std::uint16_t>(a2)) {
            continue;
        }

        const int qty = inventory_ ? inventory_->GetInventoryItemQuantity(a2) : 0;
        if (qty >= info.param2 && qty - a3 < info.param2) {
            OnNewTitle(info.title_code);
            return;
        }
    }
}
