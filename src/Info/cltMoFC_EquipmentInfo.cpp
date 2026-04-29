// 還原自 mofclient.c 0x4FF800（cltMoFC_EquipmentInfo）。詳細語意見對應 .h。
// 目前所有方法以 no-op 實作，僅保留介面，讓 cltMyCharData::Initialize 等
// 流程能呼叫到對應位置。

#include "Info/cltMoFC_EquipmentInfo.h"

cltMoFC_EquipmentInfo::cltMoFC_EquipmentInfo() = default;
cltMoFC_EquipmentInfo::~cltMoFC_EquipmentInfo() = default;

void cltMoFC_EquipmentInfo::Free() {}

void cltMoFC_EquipmentInfo::Init(std::uint16_t /*classKind*/) {}

void cltMoFC_EquipmentInfo::WeaponItemCnt(stItemKindInfo* /*info*/) {}
void cltMoFC_EquipmentInfo::WeaponItemInit(stItemKindInfo* /*info*/) {}

std::uint16_t cltMoFC_EquipmentInfo::GetEquipItemListPtr(
    std::uint16_t /*a2*/, stItemKindInfo** /*a3*/, stItemKindInfo*** /*a4*/,
    std::uint16_t* /*a5*/, std::uint16_t* /*a6*/)
{
    return 0;
}
