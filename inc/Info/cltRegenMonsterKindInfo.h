#pragma once
#include <cstdint>

/**
 * cltRegenMonsterKindInfo - 地圖怪物重生資訊管理
 *
 * 對齊反編譯：提供地圖怪物重生種類查詢
 */
class cltRegenMonsterKindInfo
{
public:
	cltRegenMonsterKindInfo() = default;

	// 依地圖 ID 取得該地圖的重生怪物種類陣列（最多 5 個）
	// 回傳 unsigned short* 指向 5 個 uint16_t 的陣列
	uint16_t* GetRegenMonsterKindByMapID(uint16_t mapID);

	// 依怪物種類取得重生地圖 ID 陣列
	// 回傳 uint16_t* 指向地圖 ID 陣列（至少 2 個）
	uint16_t* GetRegenMapIDByMonsterKind(uint16_t monsterKind);

private:
	// 內部資料（由初始化時載入）
	// 實際結構待還原
	char m_data[4096] = {};
};
