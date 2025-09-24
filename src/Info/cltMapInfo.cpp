#include "Info/cltMapInfo.h"
#include <cstring>
#include <cstdlib>
#include <cctype>
#include <cstdio>
#include <list>

// 靜態和全局變數初始化
DCTTextManager* cltMapInfo::m_pclTextManager = nullptr;
unsigned int* cltMapInfo::m_dwMapTypeAtb = nullptr; // 應由其他部分初始化
cltMapInfo* g_pcltMapInfo = nullptr;


void cltMapInfo::InitializeStaticVariable(DCTTextManager* a1) {
	cltMapInfo::m_pclTextManager = a1;
}

cltMapInfo::cltMapInfo()
	: m_pMapInfoArray(nullptr), m_wTotalMapNum(0), m_wPKMapCount(0), m_pPKMapKindArray(nullptr) {
	g_pcltMapInfo = this;
	memset(m_DungeonMapKinds, 0, sizeof(m_DungeonMapKinds));
}

cltMapInfo::~cltMapInfo() {
	Free();
}

void cltMapInfo::Free() {
	if (m_pMapInfoArray) {
		delete[] m_pMapInfoArray;
		m_pMapInfoArray = nullptr;
	}
	if (m_pPKMapKindArray) {
		delete[] m_pPKMapKindArray;
		m_pPKMapKindArray = nullptr;
	}
	m_wTotalMapNum = 0;
	m_wPKMapCount = 0;

	if (g_pcltMapInfo == this) {
		g_pcltMapInfo = nullptr;
	}
}

// 輔助宏，用於簡化 strtok 的連續呼叫與錯誤檢查
#define NEXT_TOKEN(token_var) \
    token_var = strtok(nullptr, delimiters); \
    if (!token_var) { \
        success = false; \
        break; \
    }

/**
 * @brief 初始化地圖資訊管理器，讀取並解析指定的 MapInfo.txt 檔案。
 * @param filename 要讀取的檔案名稱 (例如 "MapInfo.txt")。
 * @return bool 如果初始化成功則為 true，否則為 false。
 */
bool cltMapInfo::Initialize(char* filename) {
	// 釋放舊資料
	Free();

	FILE* file = g_clTextFileManager.fopen(filename);
	if (!file) {
		return false;
	}

	char buffer[2048]; // 增加緩衝區大小以應對長行

	// 跳過前 3 行標頭
	if (!fgets(buffer, sizeof(buffer), file) ||
		!fgets(buffer, sizeof(buffer), file) ||
		!fgets(buffer, sizeof(buffer), file)) {
		g_clTextFileManager.fclose(file);
		return false;
	}

	fpos_t start_pos;
	fgetpos(file, &start_pos);

	// 第一次遍歷：計算地圖總數
	m_wTotalMapNum = 0;
	while (fgets(buffer, sizeof(buffer), file)) {
		// 確保不是空行
		if (strlen(buffer) > 1 && buffer[0] != '\n') {
			m_wTotalMapNum++;
		}
	}

	if (m_wTotalMapNum == 0) {
		g_clTextFileManager.fclose(file);
		return true; // 檔案為空但處理成功
	}

	// 分配儲存所有地圖資訊的記憶體
	m_pMapInfoArray = new stMapInfo[m_wTotalMapNum];
	memset(m_pMapInfoArray, 0, sizeof(stMapInfo) * m_wTotalMapNum);

	// 回到資料起始位置進行第二次遍歷以解析資料
	fsetpos(file, &start_pos);

	bool success = true;
	const char* delimiters = "\t\n";

	for (int i = 0; i < m_wTotalMapNum; ++i) {
		if (!fgets(buffer, sizeof(buffer), file)) {
			success = false;
			break;
		}

		// 指向當前要填充的地圖結構
		stMapInfo* current_info = &m_pMapInfoArray[i];

		char* token = strtok(buffer, delimiters);
		if (!token) {
			success = false;
			break;
		}

		// --- 開始逐欄解析 (共 73 欄) ---
		do {
			// 地圖 ID 
			current_info->m_wID = cltItemKindInfo::TranslateKindCode(token);

			// 檔案名稱 
			NEXT_TOKEN(token);
			current_info->m_wFileName = cltItemKindInfo::TranslateKindCode(token);

			// 地圖區域名稱 - 忽略
			NEXT_TOKEN(token);

			// 區域名稱代碼 
			NEXT_TOKEN(token); if (!IsDigit(token)) { success = false; break; }
			current_info->m_wRegionCode = atoi(token);

			// 擴展區域代碼 
			NEXT_TOKEN(token); if (!IsDigit(token)) { success = false; break; }
			current_info->m_wExtRegionCode = atoi(token);

			// 背景 
			NEXT_TOKEN(token);
			if (toupper(*token) == 'Y') current_info->m_BG = 1; else current_info->m_BG = 0;

			// 固定背景 
			NEXT_TOKEN(token); if (!IsAlphaNumeric(token)) { success = false; break; }
			sscanf(token, "%x", &current_info->m_dwFixedBG);

			// 中層背景 
			NEXT_TOKEN(token); if (!IsAlphaNumeric(token)) { success = false; break; }
			sscanf(token, "%x", &current_info->m_dwMiddleBG);

			// 移動背景 
			NEXT_TOKEN(token); if (!IsAlphaNumeric(token)) { success = false; break; }
			sscanf(token, "%x", &current_info->m_dwMovingBG);

			// 區域分類 
			NEXT_TOKEN(token);
			if (strcmp(token, "FIELD") == 0) current_info->m_wRegionType = MAP_CAPS_FIELD;
			else if (strcmp(token, "VILLAGE") == 0) current_info->m_wRegionType = MAP_CAPS_VILLAGE;
			else if (strcmp(token, "DUNGEON") == 0) current_info->m_wRegionType = MAP_CAPS_DUNGEON;
			else if (strcmp(token, "FFTV") == 0) current_info->m_wRegionType = MAP_CAPS_FFTV;
			else if (strcmp(token, "FFTV1") == 0) current_info->m_wRegionType = MAP_CAPS_FFTV1;
			else if (strcmp(token, "FFTV2") == 0) current_info->m_wRegionType = MAP_CAPS_FFTV2;
			else if (strcmp(token, "BUNKER") == 0) current_info->m_wRegionType = MAP_CAPS_BUNKER;
			else if (strcmp(token, "PRISON") == 0) current_info->m_wRegionType = MAP_CAPS_PRISON;
			else if (strcmp(token, "POSEIDOS") == 0) current_info->m_wRegionType = MAP_CAPS_POSEIDOS;
			else if (strcmp(token, "POSEIDOSSTART") == 0) current_info->m_wRegionType = MAP_CAPS_POSEIDOSSTART;
			else if (strcmp(token, "FIELDSTART") == 0) current_info->m_wRegionType = MAP_CAPS_FIELDSTART;
			else if (strcmp(token, "INDUN") == 0) current_info->m_wRegionType = MAP_CAPS_FIELDSTART;
			else if (strcmp(token, "CIRCLEROOM") == 0) current_info->m_wRegionType = MAP_CAPS_FIELDSTART;
			else if (strcmp(token, "LUNDWEL") == 0) current_info->m_wRegionType = MAP_CAPS_LUNDWEL;
			else if (strcmp(token, "LUNDWELSTART") == 0) current_info->m_wRegionType = MAP_CAPS_LUNDWELSTART;
			else if (strcmp(token, "WEDDINGHALL") == 0) current_info->m_wRegionType = MAP_CAPS_WEDDINGHALL;
			else if (strcmp(token, "SKYCASTLE") == 0) current_info->m_wRegionType = MAP_CAPS_SKYCASTLE;
			else if (strcmp(token, "SKYCASTLESTART") == 0) current_info->m_wRegionType = MAP_CAPS_SKYCASTLESTART;
			else current_info->m_wRegionType = MAP_CAPS_FIELD; // 預設值

			// 地城名稱 
			NEXT_TOKEN(token); if (!IsDigit(token)) { success = false; break; }
			current_info->m_wDungeonNameCode = atoi(token);

			// 大陸分類 
			NEXT_TOKEN(token); if (!IsDigit(token)) { success = false; break; }
			current_info->m_byteContinentType = (unsigned char)atoi(token);

			// 13: 背景音樂1 
			NEXT_TOKEN(token);
			strncpy(current_info->m_szBGM1, token, sizeof(current_info->m_szBGM1) - 1);

			// 14: 背景音樂2 
			NEXT_TOKEN(token);
			strncpy(current_info->m_szBGM2, token, sizeof(current_info->m_szBGM2) - 1);

			// 15: 環境音效 
			NEXT_TOKEN(token);
			strncpy(current_info->m_szAmbientSoundFile, token, sizeof(current_info->m_szAmbientSoundFile) - 1);

			// 世界地圖 X 座標 
			NEXT_TOKEN(token); if (!IsDigit(token)) { success = false; break; }
			current_info->m_dwWorldMapPosX = atoi(token);

			// 世界地圖 Y 座標 
			NEXT_TOKEN(token); if (!IsDigit(token)) { success = false; break; }
			current_info->m_dwWorldMapPosY = atoi(token);

			// 瞬間移動 X 座標 
			NEXT_TOKEN(token); if (!IsDigit(token)) { success = false; break; }
			current_info->m_dwTeleportX = atoi(token);

			// 瞬間移動 Y 座標 
			NEXT_TOKEN(token); if (!IsDigit(token)) { success = false; break; }
			current_info->m_dwTeleportY = atoi(token);

			// 飛龍地圖 X 座標 
			NEXT_TOKEN(token); if (!IsDigit(token)) { success = false; break; }
			current_info->m_dwDragonMapPosX = atoi(token);

			// 飛龍地圖 Y 座標 
			NEXT_TOKEN(token); if (!IsDigit(token)) { success = false; break; }
			current_info->m_dwDragonMapPosY = atoi(token);

			// 船隻移動 X 座標 
			NEXT_TOKEN(token); if (!IsDigit(token)) { success = false; break; }
			current_info->m_dwShipMovePosX = atoi(token);

			// 船隻移動 Y 座標 
			NEXT_TOKEN(token); if (!IsDigit(token)) { success = false; break; }
			current_info->m_dwShipMovePosY = atoi(token);

			// 24: 區域類型 
			NEXT_TOKEN(token);
			if (strcmp(token, "LIBITOWN") == 0) current_info->m_byAreaType = MAP_TILE_LIBITOWN;
			if (strcmp(token, "GOLDYLAN") == 0) current_info->m_byAreaType = MAP_TILE_GOLDYLAN;
			if (strcmp(token, "LAWN") == 0) current_info->m_byAreaType = MAP_TILE_LAWN;
			if (strcmp(token, "GLENN_WOOD") == 0) current_info->m_byAreaType = MAP_TILE_GLENN_WOOD;
			if (strcmp(token, "CREMBILL") == 0) current_info->m_byAreaType = MAP_TILE_CREMBILL;
			if (strcmp(token, "DARK_PILLAR") == 0) current_info->m_byAreaType = MAP_TILE_DARK_PILLAR;
			if (strcmp(token, "GREENSTAR") == 0) current_info->m_byAreaType = MAP_TILE_GREENSTAR;
			if (strcmp(token, "PAPIRION") == 0) current_info->m_byAreaType = MAP_TILE_PAPIRION;
			if (strcmp(token, "WESTENPA") == 0) current_info->m_byAreaType = MAP_TILE_WESTENPA;
			if (strcmp(token, "NORTHENPA") == 0) current_info->m_byAreaType = MAP_TILE_NORTHENPA;
			if (strcmp(token, "NABAHORN") == 0) current_info->m_byAreaType = MAP_TILE_NABAHORN;
			if (strcmp(token, "CORULMATIN") == 0) current_info->m_byAreaType = MAP_TILE_CORULMATIN;
			if (strcmp(token, "EASTNPA") == 0) current_info->m_byAreaType = MAP_TILE_EASTNPA;
			if (strcmp(token, "LESTIN") == 0) current_info->m_byAreaType = MAP_TILE_LESTIN;
			if (strcmp(token, "PAPERONPLAIN") == 0) current_info->m_byAreaType = MAP_TILE_PAPERONPLAIN;
			if (strcmp(token, "INDUN") == 0) current_info->m_byAreaType = MAP_TILE_INDUN;
			if (strcmp(token, "CIRCLEROOM") == 0) current_info->m_byAreaType = MAP_TILE_CIRCLEROOM;
			else current_info->m_byAreaType = MAP_TILE_NONE; // 預設值

			// 25: 資源 ID 
			NEXT_TOKEN(token); if (!IsAlphaNumeric(token)) { success = false; break; }
			sscanf(token, "%x", &current_info->m_dwResourceID);

			// 區塊 ID 
			NEXT_TOKEN(token); if (!IsDigit(token)) { success = false; break; }
			current_info->m_wBlockID = atoi(token);

			// 區域名稱代碼 
			NEXT_TOKEN(token); if (!IsDigit(token)) { success = false; break; }
			current_info->m_wRegionNameCode = atoi(token);

			// 重生位置 
			NEXT_TOKEN(token);
			current_info->m_wResurrectPos = cltResurrectInfo::TranslateKindCode(token);

			// 瞬移到達 X 座標 
			NEXT_TOKEN(token); if (!IsDigit(token)) { success = false; break; }
			current_info->m_wTeleportDestX = atoi(token);

			// 瞬移到達 Y 座標 
			NEXT_TOKEN(token); if (!IsDigit(token)) { success = false; break; }
			current_info->m_wTeleportDestY = atoi(token);

			// 船／飛龍到達 X 座標 
			NEXT_TOKEN(token); if (!IsDigit(token)) { success = false; break; }
			current_info->m_wShipDragonDestX = atoi(token);

			// 船／飛龍到達 Y 座標 
			NEXT_TOKEN(token); if (!IsDigit(token)) { success = false; break; }
			current_info->m_wShipDragonDestY = atoi(token);

			// 小地圖資源檔案 
			NEXT_TOKEN(token); if (!IsAlphaNumeric(token)) { success = false; break; }
			sscanf(token, "%x", &current_info->m_dwMiniMapResourceFile);

			// 小地圖區塊 ID 
			NEXT_TOKEN(token); if (!IsDigit(token)) { success = false; break; }
			current_info->m_wMiniMapBlockID = atoi(token);

			// 秘密空間 
			NEXT_TOKEN(token); if (!IsDigit(token)) { success = false; break; }
			current_info->m_dwSecretSpace = atoi(token);

			// 等級限制 
			NEXT_TOKEN(token); if (!IsDigit(token)) { success = false; break; }
			current_info->m_wLevelRequired = atoi(token);

			// 是否為市場 
			NEXT_TOKEN(token);
			if (toupper(*token) == 'Y') current_info->m_dwIsMarket = 1; else current_info->m_dwIsMarket = 0;

			// 氣候 
			NEXT_TOKEN(token);
			current_info->m_wClimate = cltItemKindInfo::TranslateKindCode(token);

			// 是否為對抗戰地區 
			NEXT_TOKEN(token); if (!IsDigit(token)) { success = false; break; }
			current_info->m_byIsPKArea = (unsigned char)atoi(token);

			// 是否可開啟對抗戰 
			NEXT_TOKEN(token);
			if (toupper(*token) == 'Y') current_info->m_dwCanOpenPK = 1; else current_info->m_dwCanOpenPK = 0;

			// 區域制霸 
			NEXT_TOKEN(token); if (!IsDigit(token)) { success = false; break; }
			current_info->m_dwAreaDomination = atoi(token);

			// 是否可在 PK 地圖（等待室） 
			NEXT_TOKEN(token); if (!IsDigit(token)) { success = false; break; }
			current_info->m_dwPKLobby = atoi(token);

			// 是否可在 PK 地圖（狩獵場） 
			NEXT_TOKEN(token); if (!IsDigit(token)) { success = false; break; }
			current_info->m_dwPKField = atoi(token);
			if (current_info->m_dwPKField) {
				m_wPKMapCount++;
			}

			// 是否為可戰鬥區域 
			NEXT_TOKEN(token); if (!IsDigit(token)) { success = false; break; }
			current_info->m_dwIsBattleArea = atoi(token);

			// 怪物強化（攻擊力 %） 
			NEXT_TOKEN(token); if (!IsDigit(token)) { success = false; break; }
			current_info->m_dwMonsterAttBoost = atoi(token);

			// 怪物強化（防禦力） 
			NEXT_TOKEN(token); if (!IsDigit(token)) { success = false; break; }
			current_info->m_dwMonsterDefBoost = atoi(token);

			// 怪物強化（經驗值 %） 
			NEXT_TOKEN(token); if (!IsDigit(token)) { success = false; break; }
			current_info->m_dwMonsterExpBoost = atoi(token);

			// 48: 드랍 확률(퍼밀) (掉落機率1)
			NEXT_TOKEN(token); if (!IsDigit(token)) { success = false; break; }
			current_info->m_dwDropRate1 = atoi(token);

			// 49-58: 掉落物品ID 1-5 (及其計數，計數欄位被程式忽略)
			for (int j = 0; j < 5; ++j) {
				NEXT_TOKEN(token); // 物品ID
				if (strcmp(token, "0") != 0) {
					current_info->m_wDropItemID1[j] = cltItemKindInfo::TranslateKindCode(token);
					current_info->m_byteDropItemCount1++;
				}
				NEXT_TOKEN(token); // 忽略原始檔案中的物品名稱欄位
			}

			// 59: (最小數量1)
			NEXT_TOKEN(token); if (!IsDigit(token)) { success = false; break; }
			current_info->m_wDropItemMin1 = atoi(token);

			// 60: (最大數量1)
			NEXT_TOKEN(token); if (!IsDigit(token)) { success = false; break; }
			current_info->m_wDropItemMax1 = atoi(token);

			// 61: (掉落機率2)
			NEXT_TOKEN(token); if (!IsDigit(token)) { success = false; break; }
			current_info->m_dwDropRate2 = atoi(token);

			// 62-71: 掉落物品ID 6-10 (及其計數)
			for (int j = 0; j < 5; ++j) {
				NEXT_TOKEN(token); // 物品ID
				if (strcmp(token, "0") != 0) {
					current_info->m_wDropItemID2[j] = cltItemKindInfo::TranslateKindCode(token);
					current_info->m_byteDropItemCount2++;
				}
				NEXT_TOKEN(token); // 忽略原始檔案中的物品名稱欄位
			}

			// 72: (最小數量2)
			NEXT_TOKEN(token); if (!IsDigit(token)) { success = false; break; }
			current_info->m_wDropItemMin2 = atoi(token);

			// 73: (最大數量2)
			NEXT_TOKEN(token); if (!IsDigit(token)) { success = false; break; }
			current_info->m_wDropItemMax2 = atoi(token);

		} while (false);

		if (!success) break;
	}

	if (success) {
		// 解析成功後，進行最後的資料完整性檢查
		for (int i = 0; i < m_wTotalMapNum; ++i) {
			stMapInfo* current_info = &m_pMapInfoArray[i];

			// 檢查1: 掉落物品數量與掉落機率的邏輯一致性
			if (current_info->m_wDropItemMin1 > current_info->m_wDropItemMax1) {
				success = false;
			}

			// 檢查2: 掉落物品數量與掉落機率的邏輯一致性 (第二組)
			if (current_info->m_wDropItemMin2 > current_info->m_wDropItemMax2) {
				success = false;
			}

			// 檢查3: 如果有設定掉落機率，但沒有對應的掉落物品，則為錯誤
			if (current_info->m_dwDropRate1 && !current_info->m_byteDropItemCount1) {
				success = false;
			}
			if (current_info->m_dwDropRate2 && !current_info->m_byteDropItemCount2) {
				success = false;
			}

			// 檢查4: 如果掉落物品數量大於0，對應的第一個物品ID不能為0
			// 反組譯碼中的判斷邏輯是：如果 byteDropItemCount1 不為0，但 m_wDropItemID1[0] 為0，則為錯誤。
			if (current_info->m_byteDropItemCount1 && !current_info->m_wDropItemID1[0]) {
				success = false;
			}
			if (current_info->m_byteDropItemCount2 && !current_info->m_wDropItemID2[0]) {
				success = false;
			}

			// 檢查5: 確保每個掉落物品ID都有效
			// 反組譯碼中有一系列基於 byteDropItemCount 的 if 判斷，來檢查每個可能存在的物品ID是否為0
			if (current_info->m_byteDropItemCount1 >= 2 && !current_info->m_wDropItemID1[1]) {
				success = false;
			}
			if (current_info->m_byteDropItemCount1 >= 3 && !current_info->m_wDropItemID1[2]) {
				success = false;
			}
			if (current_info->m_byteDropItemCount1 >= 4 && !current_info->m_wDropItemID1[3]) {
				success = false;
			}
			if (current_info->m_byteDropItemCount1 >= 5 && !current_info->m_wDropItemID1[4]) {
				success = false;
			}

			if (current_info->m_byteDropItemCount2 >= 2 && !current_info->m_wDropItemID2[1]) {
				success = false;
			}
			if (current_info->m_byteDropItemCount2 >= 3 && !current_info->m_wDropItemID2[2]) {
				success = false;
			}
			if (current_info->m_byteDropItemCount2 >= 4 && !current_info->m_wDropItemID2[3]) {
				success = false;
			}
			if (current_info->m_byteDropItemCount2 >= 5 && !current_info->m_wDropItemID2[4]) {
				success = false;
			}

			if (!success) break;
		}
		if (success) {
			// 如果所有地圖都解析成功，則建立 PK 地圖的索引
			SetPKMapKindAll();
		}
		else {
			// 如果解析失敗，釋放資源
			Free();
		}
	}

	g_clTextFileManager.fclose(file);
	return success;
}

unsigned short cltMapInfo::GetTotalMapNum() const {
	return m_wTotalMapNum;
}

stMapInfo* cltMapInfo::GetMapInfoByIndex(int index) {
	if (index < 0 || index >= m_wTotalMapNum) {
		return nullptr;
	}
	return &m_pMapInfoArray[index];
}

stMapInfo* cltMapInfo::GetMapInfoByID(unsigned short map_id) {
	for (int i = 0; i < m_wTotalMapNum; ++i) {
		if (m_pMapInfoArray[i].m_wID == map_id) {
			return &m_pMapInfoArray[i];
		}
	}
	return nullptr;
}

stMapInfo* cltMapInfo::GetMapInfoByMapName(char* map_name) {
	for (int i = 0; i < m_wTotalMapNum; ++i) {
		const char* current_map_name = m_pclTextManager->GetText(m_pMapInfoArray[i].m_wRegionCode);
		if (strcmp(map_name, current_map_name) == 0) {
			return &m_pMapInfoArray[i];
		}
	}
	return nullptr;
}

unsigned short cltMapInfo::GetMapCaps(unsigned short map_id) {
	stMapInfo* info = GetMapInfoByID(map_id);
	return info ? info->m_wRegionType : 0;
}

bool cltMapInfo::GetDrawBG(unsigned short map_id) {
	stMapInfo* info = GetMapInfoByID(map_id);
	return info ? (info->m_BG != 0) : false;
}

char* cltMapInfo::TranslateMapID(unsigned short map_id) {
	static char buffer[16];
	sprintf(buffer, "%c%04d", (char)((map_id >> 11) + 'A'), map_id & 0x7FF);
	return buffer;
}

uint16_t cltMapInfo::TranslateKindCode(char* a1)
{
	if (!a1) return 0;

	// 必須剛好 5 字元，例如 "D0123"、"I0007" 等
	if (std::strlen(a1) != 5)
		return 0;

	// 高位：以 (toupper(first) + 31) << 11 編碼
	int hi = (std::toupper(static_cast<unsigned char>(a1[0])) + 31) << 11;

	// 低位：後四碼數字（需 < 0x800）
	int lo = std::atoi(a1 + 1);
	if (lo < 0x800)
		return static_cast<uint16_t>(hi | lo);

	return 0;
}

/**
 * @brief 檢查在地圖上是否能使用回城道具。
 * @param map_id 要檢查的地圖ID。
 * @return bool 如果可以使用則為 true，否則為 false。
 */
bool cltMapInfo::CanUseReturnItem(unsigned short map_id) {
	// 檢查 m_dwMapTypeAtb 是否已被初始化
	if (!m_dwMapTypeAtb) {
		return false;
	}
	return (m_dwMapTypeAtb[map_id] >> 3) & 1;
}

void cltMapInfo::SetPKMapKindAll() {
	if (m_wPKMapCount == 0) return;

	m_pPKMapKindArray = new unsigned short[m_wPKMapCount];
	if (m_pPKMapKindArray) {
		int pk_index = 0;
		for (int i = 0; i < m_wTotalMapNum; ++i) {
			if (m_pMapInfoArray[i].m_dwPKField) {
				m_pPKMapKindArray[pk_index++] = m_pMapInfoArray[i].m_wID;
			}
		}
	}
}

int cltMapInfo::GetMapKindCountByDungeonNameCode(int dungeon_name_code) {
	std::list<unsigned short> map_ids;
	for (int i = 0; i < m_wTotalMapNum; ++i) {
		if (m_pMapInfoArray[i].m_wDungeonNameCode == dungeon_name_code) {
			map_ids.push_back(m_pMapInfoArray[i].m_wID);
		}
	}

	int count = 0;
	for (unsigned short id : map_ids) {
		if (count < 50) {
			m_DungeonMapKinds[count++] = id;
		}
		else {
			break;
		}
	}
	return map_ids.size();
}

unsigned int cltMapInfo::GetMiniMapPosX(unsigned short map_id) {
	stMapInfo* info = GetMapInfoByID(map_id);
	// 返回小地圖 UI 元素的 X 座標
	return info ? info->m_dwWorldMapPosX : 0;
}

unsigned int cltMapInfo::GetMiniMapPosY(unsigned short map_id) {
	stMapInfo* info = GetMapInfoByID(map_id);
	// 返回小地圖 UI 元素的 Y 座標
	return info ? info->m_dwWorldMapPosY : 0;
}

unsigned short cltMapInfo::GetWarpX(unsigned short map_id) {
	stMapInfo* info = GetMapInfoByID(map_id);
	return info ? info->m_wTeleportDestX : 0;
}

unsigned short cltMapInfo::GetWarpY(unsigned short map_id) {
	stMapInfo* info = GetMapInfoByID(map_id);
	return info ? info->m_wTeleportDestY : 0;
}

unsigned short cltMapInfo::GetTeleportX(unsigned short map_id) {
	stMapInfo* info = GetMapInfoByID(map_id);
	return info ? info->m_wShipDragonDestX : 0;
}

unsigned short cltMapInfo::GetTeleportY(unsigned short map_id) {
	stMapInfo* info = GetMapInfoByID(map_id);
	return info ? info->m_wShipDragonDestY : 0;
}

unsigned short cltMapInfo::GetAreaVorF(unsigned short map_id) {
	stMapInfo* info = GetMapInfoByID(map_id);
	// 返回地圖類型 (如村莊、野外等)
	return info ? info->m_wRegionType : 0;
}

unsigned int cltMapInfo::GetRestrictionLevel(unsigned short map_id) {
	stMapInfo* info = GetMapInfoByID(map_id);
	// 返回進入該地圖的等級限制
	return info ? info->m_wLevelRequired : 0;
}

unsigned short cltMapInfo::GetMapID(unsigned char match_map_id) {
	// 遍歷所有地圖，尋找指定競賽ID的地圖
	for (int i = 0; i < m_wTotalMapNum; ++i) {
		if (m_pMapInfoArray[i].m_byIsPKArea == match_map_id) {
			return m_pMapInfoArray[i].m_wID;
		}
	}
	return 0; // 未找到
}

unsigned short cltMapInfo::GetProvideMapID() {
	// 遍歷所有地圖，尋找第一個提供競賽的地圖
	for (int i = 0; i < m_wTotalMapNum; ++i) {
		if (m_pMapInfoArray[i].m_dwCanOpenPK) {
			return m_pMapInfoArray[i].m_wID;
		}
	}
	return 0; // 未找到
}

bool cltMapInfo::IsProvideMatch(unsigned short map_id) {
	stMapInfo* info = GetMapInfoByID(map_id);
	// 檢查該地圖是否提供競賽
	return info ? (info->m_byIsPKArea != 0) : false;
}

bool cltMapInfo::IsMatchMap(unsigned short map_id) {
	stMapInfo* info = GetMapInfoByID(map_id);
	// 檢查該地圖是否為競賽地圖
	return info ? (info->m_byIsPKArea != 0) : false;
}

// --- 補上的靜態函數實作 ---
// 這些函數依賴於一個外部設置的靜態屬性表 m_dwMapTypeAtb

bool cltMapInfo::CanUseTeleportItem(unsigned short map_id) {
	if (!m_dwMapTypeAtb) return false;
	// 檢查屬性表中代表「可否使用傳送道具」的位元 (bit 1)
	return (m_dwMapTypeAtb[map_id] >> 1) & 1;
}

bool cltMapInfo::CanMoveTeleportItem(unsigned short map_id) {
	if (!m_dwMapTypeAtb) return false;
	// 檢查屬性表中代表「可否移動到傳送點」的位元 (bit 2)
	return (m_dwMapTypeAtb[map_id] >> 2) & 1;
}

bool cltMapInfo::CanMoveTeleportDragon(unsigned short map_id) {
	if (!m_dwMapTypeAtb) return false;
	// 檢查屬性表中代表「龍可否傳送」的位元 (bit 0)
	return m_dwMapTypeAtb[map_id] & 1;
}

bool cltMapInfo::CanUserTakeShip(unsigned short map_id) {
	if (!m_dwMapTypeAtb) return false;
	// 檢查屬性表中代表「可否搭船」的位元 (bit 4)
	return (m_dwMapTypeAtb[map_id] >> 4) & 1;
}

/**
 * @brief 獲取可PK地圖的總數量。
 * @return unsigned short 可PK地圖的數量。
 */
unsigned short cltMapInfo::GetTotalPKMapNum() const {
	return m_wPKMapCount;
}