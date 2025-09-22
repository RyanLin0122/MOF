#include "Info/cltItemKindInfo.h"
#include <cstring>
#include <cstdlib>
#include <cctype>
#include <cstdio>

// =================================================================
// Parsing Helper Macros
// =================================================================

// 解析一個 token 為整數並賦值給成員
#define PARSE_INT(member) \
    token = strtok(nullptr, delimiter); \
    if (!token || !IsDigit(token)) goto next_line; \
    info->member = atoi(token)

// 解析一個 token 為十六進制整數並賦值給成員
#define PARSE_HEX(member) \
    token = strtok(nullptr, delimiter); \
    if (!token || !IsAlphaNumeric(token)) goto next_line; \
    sscanf(token, "%x", &info->member)

// 解析一個 token，如果為 'Y' 或 '1'，則將成員設為 true
#define PARSE_BOOL_Y(member) \
    token = strtok(nullptr, delimiter); \
    if (!token) goto next_line; \
    info->member = (toupper(*token) == 'Y' || toupper(*token) == '1')

// 解析一個 token 為字串並複製到成員
#define PARSE_STR(member) \
    token = strtok(nullptr, delimiter); \
    if (!token) goto next_line; \
    strncpy(info->member, token, sizeof(info->member) - 1); \
    info->member[sizeof(info->member) - 1] = '\0'

// 跳過一個 token
#define SKIP_TOKEN() \
    token = strtok(nullptr, delimiter); \
    if (!token) goto next_line

// 與 IDA 相符的格式：1 字母 + 4 數字 + '\0'（額外留白以防萬一）
static char s_szKindCode[1024] = { 0 };

// 物品類別屬性陣列的初始化
// 每個索引對應 EItemClass 的一個值
unsigned int cltItemKindInfo::m_dwItemTypeAtb[25] = {
	/*INSTANT*/           ITEM_ATTR_USE | ITEM_ATTR_SELL | ITEM_ATTR_DROP | ITEM_ATTR_TRADE | ITEM_ATTR_QUICKSLOT | ITEM_ATTR_STORAGE | ITEM_ATTR_SELL_AGENCY | ITEM_ATTR_TIMER | ITEM_ATTR_EX_STORAGE | ITEM_ATTR_PET_INVEN,
	/*HUNT*/              ITEM_ATTR_SELL | ITEM_ATTR_DROP | ITEM_ATTR_EQUIP | ITEM_ATTR_TRADE | ITEM_ATTR_STORAGE | ITEM_ATTR_SELL_AGENCY | ITEM_ATTR_TIMER | ITEM_ATTR_EX_STORAGE | ITEM_ATTR_PET_INVEN,
	/*FASHION*/           ITEM_ATTR_SELL | ITEM_ATTR_DROP | ITEM_ATTR_EQUIP | ITEM_ATTR_TRADE | ITEM_ATTR_STORAGE | ITEM_ATTR_SELL_AGENCY | ITEM_ATTR_TIMER | ITEM_ATTR_EX_STORAGE | ITEM_ATTR_PET_INVEN,
	/*TRAINING*/          ITEM_ATTR_USE | ITEM_ATTR_SELL | ITEM_ATTR_DROP | ITEM_ATTR_TRADE | ITEM_ATTR_QUICKSLOT | ITEM_ATTR_STORAGE | ITEM_ATTR_SELL_AGENCY | ITEM_ATTR_TIMER | ITEM_ATTR_TRAINING | ITEM_ATTR_EX_STORAGE | ITEM_ATTR_PET_INVEN,
	/*ETC*/               ITEM_ATTR_SELL | ITEM_ATTR_DROP | ITEM_ATTR_TRADE | ITEM_ATTR_STORAGE | ITEM_ATTR_SELL_AGENCY | ITEM_ATTR_EX_STORAGE | ITEM_ATTR_PET_INVEN,
	/*NPCITEM*/           0,
	/*GENERICBAG*/        ITEM_ATTR_SELL | ITEM_ATTR_DROP | ITEM_ATTR_TRADE | ITEM_ATTR_STORAGE | ITEM_ATTR_SELL_AGENCY | ITEM_ATTR_EX_STORAGE | ITEM_ATTR_PET_INVEN,
	/*UPGRADE*/           ITEM_ATTR_SELL | ITEM_ATTR_DROP | ITEM_ATTR_TRADE | ITEM_ATTR_STORAGE | ITEM_ATTR_SELL_AGENCY | ITEM_ATTR_EX_STORAGE | ITEM_ATTR_PET_INVEN,
	/*CASH_INSTANCE*/     ITEM_ATTR_USE | ITEM_ATTR_QUICKSLOT | ITEM_ATTR_STORAGE | ITEM_ATTR_CASH | ITEM_ATTR_TIMER | ITEM_ATTR_EX_STORAGE | ITEM_ATTR_PET_INVEN,
	/*CASH_HUNT*/         ITEM_ATTR_EQUIP | ITEM_ATTR_STORAGE | ITEM_ATTR_CASH | ITEM_ATTR_TIMER | ITEM_ATTR_EX_STORAGE | ITEM_ATTR_PET_INVEN,
	/*CASH_FASHION*/      ITEM_ATTR_EQUIP | ITEM_ATTR_STORAGE | ITEM_ATTR_CASH | ITEM_ATTR_TIMER | ITEM_ATTR_EX_STORAGE | ITEM_ATTR_PET_INVEN,
	/*QUIZ*/              ITEM_ATTR_USE | ITEM_ATTR_DROP | ITEM_ATTR_QUICKSLOT | ITEM_ATTR_STORAGE | ITEM_ATTR_QUIZ | ITEM_ATTR_EX_STORAGE | ITEM_ATTR_PET_INVEN,
	/*CASH_TRAINING*/     ITEM_ATTR_USE | ITEM_ATTR_QUICKSLOT | ITEM_ATTR_STORAGE | ITEM_ATTR_CASH | ITEM_ATTR_TIMER | ITEM_ATTR_TRAINING | ITEM_ATTR_EX_STORAGE | ITEM_ATTR_PET_INVEN,
	/*STOREHOUSE*/        ITEM_ATTR_SELL | ITEM_ATTR_DROP | ITEM_ATTR_TRADE | ITEM_ATTR_STORAGE | ITEM_ATTR_SELL_AGENCY | ITEM_ATTR_EX_STORAGE | ITEM_ATTR_PET_INVEN,
	/*EVENT_INSTANT*/     ITEM_ATTR_USE | ITEM_ATTR_QUICKSLOT | ITEM_ATTR_STORAGE | ITEM_ATTR_TIMER | ITEM_ATTR_EX_STORAGE | ITEM_ATTR_PET_INVEN,
	/*EVENT_FASHION*/     ITEM_ATTR_EQUIP | ITEM_ATTR_STORAGE | ITEM_ATTR_TIMER | ITEM_ATTR_EX_STORAGE | ITEM_ATTR_PET_INVEN,
	/*ETC_NOTTRADE*/      ITEM_ATTR_SELL | ITEM_ATTR_STORAGE | ITEM_ATTR_EX_STORAGE | ITEM_ATTR_PET_INVEN,
	/*EXPAND_CIRCLEMEMBER*/ ITEM_ATTR_USE | ITEM_ATTR_SELL | ITEM_ATTR_DROP | ITEM_ATTR_TRADE | ITEM_ATTR_QUICKSLOT | ITEM_ATTR_STORAGE | ITEM_ATTR_SELL_AGENCY | ITEM_ATTR_EX_STORAGE | ITEM_ATTR_PET_INVEN,
	/*PCBANG_INSTANT*/    ITEM_ATTR_USE | ITEM_ATTR_QUICKSLOT | ITEM_ATTR_STORAGE | ITEM_ATTR_TIMER | ITEM_ATTR_EX_STORAGE | ITEM_ATTR_PET_INVEN,
	/*EVENT_HUNT*/        ITEM_ATTR_EQUIP | ITEM_ATTR_STORAGE | ITEM_ATTR_TIMER | ITEM_ATTR_EX_STORAGE | ITEM_ATTR_PET_INVEN,
	/*EVENT_TRAINING*/    ITEM_ATTR_USE | ITEM_ATTR_QUICKSLOT | ITEM_ATTR_STORAGE | ITEM_ATTR_TIMER | ITEM_ATTR_TRAINING | ITEM_ATTR_EX_STORAGE | ITEM_ATTR_PET_INVEN,
	/*EVENT_STOREHOUSE*/  ITEM_ATTR_STORAGE | ITEM_ATTR_EX_STORAGE | ITEM_ATTR_PET_INVEN,
	/*EVENT_UPGRADE*/     ITEM_ATTR_STORAGE | ITEM_ATTR_EX_STORAGE | ITEM_ATTR_PET_INVEN,
	/*COIN*/              ITEM_ATTR_STORAGE | ITEM_ATTR_EX_STORAGE | ITEM_ATTR_PET_INVEN | ITEM_ATTR_COIN,
	/*EVENT_NO_EX_STORAGE_BAG*/ ITEM_ATTR_USE | ITEM_ATTR_QUICKSLOT | ITEM_ATTR_STORAGE | ITEM_ATTR_TIMER | ITEM_ATTR_PET_INVEN,
};

// Definition of static members
cltClassKindInfo* cltItemKindInfo::m_pclClassKindInfo = nullptr;
cltPetKindInfo* cltItemKindInfo::m_pclPetKindInfo = nullptr;

cltItemKindInfo::cltItemKindInfo() {
	// Zero out all member variables.
	memset(m_pItemKindInfo, 0, sizeof(m_pItemKindInfo));
	m_pItemKindForMakingItemList = nullptr;
	m_wMakingItemKindListNum = 0;
	memset(m_pHangleID2ItemKind, 0, sizeof(m_pHangleID2ItemKind));
	memset(m_pEnglishID2ItemKind, 0, sizeof(m_pEnglishID2ItemKind));
}

cltItemKindInfo::~cltItemKindInfo() {
	Free();
}

void cltItemKindInfo::InitializeStaticVariable(cltClassKindInfo* a1, cltPetKindInfo* a2) {
	m_pclClassKindInfo = a1;
	m_pclPetKindInfo = a2;
}

void cltItemKindInfo::Free() {
	// Delete all allocated stItemKindInfo objects.
	for (int i = 0; i < 65536; ++i) {
		if (m_pItemKindInfo[i]) {
			delete m_pItemKindInfo[i];
			m_pItemKindInfo[i] = nullptr;
		}
	}

	// Delete the list of making items.
	if (m_pItemKindForMakingItemList) {
		delete[] m_pItemKindForMakingItemList;
		m_pItemKindForMakingItemList = nullptr;
	}
}

int cltItemKindInfo::Initialize(const char* itemkindinfo, const char* item_instant, const char* item_hunt,
	const char* item_fashion, const char* koreatext, const char* trainningcardinfo) {

	if (!m_pclClassKindInfo) return 0;

	if (!LoadItemList(itemkindinfo)) return 0;
	if (!LoadInstantItem(item_instant)) return 0;
	if (!LoadHuntItem(item_hunt)) return 0;
	if (!LoadFashionItem(item_fashion)) return 0;
	if (!LoadQuizItem(koreatext)) return 0; // Note: Uses koreatext.txt
	if (!LoadTrainningCardItem(trainningcardinfo)) return 0;
	if (!LoadHangleInfo(koreatext)) return 0;

	CreateItemKindForMakingItemList();
	return 1;
}

bool cltItemKindInfo::LoadItemList(const char* filename) {
	FILE* file = g_clTextFileManager.fopen((char*)filename);
	if (!file) {
		MessageBoxA(NULL, "[LOAD ERROR] Cannot open ItemKindInfo.txt", "Error", MB_OK);
		return false;
	}

	char buffer[1024];
	unsigned short last_kind_code = 0;

	// Skip header lines
	fgets(buffer, sizeof(buffer), file);
	fgets(buffer, sizeof(buffer), file);
	fgets(buffer, sizeof(buffer), file);

	while (fgets(buffer, sizeof(buffer), file)) {
		const char* delimiter = "\t\n";
		char* token = strtok(buffer, delimiter); //物品 ID
		if (!token) continue;

		unsigned short kind = TranslateKindCode(token);
		last_kind_code = kind;
		if (!kind || m_pItemKindInfo[kind]) goto load_error;

		stItemKindInfo* info = new stItemKindInfo();
		memset(info, 0, sizeof(stItemKindInfo));
		m_pItemKindInfo[kind] = info;
		info->m_wKind = kind;

		token = strtok(nullptr, delimiter); // 아이템 이름 (사용안함) 物品名稱
		if (!token) goto load_error;

		token = strtok(nullptr, delimiter); // 홈페이지 아이템 여부
		if (!token) goto load_error;
		info->m_bHomepageView = (toupper(*token) == 'Y');

		token = strtok(nullptr, delimiter); // 文字代碼
		if (!token || !IsDigit(token)) goto load_error;
		info->m_wTextCode = atoi(token);

		token = strtok(nullptr, delimiter); // 描述代碼
		if (!token || !IsDigit(token)) goto load_error;
		info->m_wDescCode = atoi(token);

		token = strtok(nullptr, delimiter); // 物品價格
		if (!token || !IsDigit(token)) goto load_error;
		info->m_dwPrice = atoi(token);
		if (info->m_dwPrice > 999999999) goto load_error;
		if (info->m_dwPrice == 0) goto load_error; // Price cannot be 0 check

		token = strtok(nullptr, delimiter); // PVP 購買點數
		if (!token || !IsDigit(token)) goto load_error;
		info->m_dwPVPPoint = atoi(token);

		token = strtok(nullptr, delimiter); // 金幣價格
		if (!token || !IsDigit(token)) goto load_error;
		info->m_dwPrice1 = atoi(token);

		token = strtok(nullptr, delimiter); // 銀幣價格
		if (!token || !IsDigit(token)) goto load_error;
		info->m_dwPrice2 = atoi(token);

		token = strtok(nullptr, delimiter); // 銅幣價格
		if (!token || !IsDigit(token)) goto load_error;
		info->m_dwPrice3 = atoi(token);

		token = strtok(nullptr, delimiter); // 物品圖像檔案 ID
		if (!token || !IsAlphaNumeric(token)) goto load_error;
		sscanf(token, "%x", &info->m_dwIconResID);

		token = strtok(nullptr, delimiter); // 物品圖像方塊 ID
		if (!token || !IsDigit(token)) goto load_error;
		info->m_wIconBlockID = atoi(token);

		token = strtok(nullptr, delimiter); // 最大堆疊數量
		if (!token || !IsDigit(token)) goto load_error;
		info->m_byMaxPileUpNum = atoi(token);

		token = strtok(nullptr, delimiter); // 物品類別
		if (!token) goto load_error;
		if (!_stricmp(token, "INSTANT")) info->m_byItemClass = ITEM_CLASS_INSTANT;
		else if (!_stricmp(token, "HUNT")) info->m_byItemClass = ITEM_CLASS_HUNT;
		else if (!_stricmp(token, "FASHION")) info->m_byItemClass = ITEM_CLASS_FASHION;
		else if (!_stricmp(token, "TRAINING")) info->m_byItemClass = ITEM_CLASS_TRAINING;
		else if (!_stricmp(token, "ETC")) info->m_byItemClass = ITEM_CLASS_ETC;
		else if (!_stricmp(token, "ETC_NOTTRADE")) info->m_byItemClass = ITEM_CLASS_ETC_NOTTRADE;
		else if (!_stricmp(token, "NPCITEM")) info->m_byItemClass = ITEM_CLASS_NPCITEM;
		else if (!_stricmp(token, "GENERICBAG")) info->m_byItemClass = ITEM_CLASS_GENERICBAG;
		else if (!_stricmp(token, "UPGRADE")) info->m_byItemClass = ITEM_CLASS_UPGRADE;
		else if (!_stricmp(token, "CASH_INSTANCE")) info->m_byItemClass = ITEM_CLASS_CASH_INSTANCE;
		else if (!_stricmp(token, "CASH_HUNT")) info->m_byItemClass = ITEM_CLASS_CASH_HUNT;
		else if (!_stricmp(token, "CASH_FASHION")) info->m_byItemClass = ITEM_CLASS_CASH_FASHION;
		else if (!_stricmp(token, "QUIZ")) info->m_byItemClass = ITEM_CLASS_QUIZ;
		else if (!_stricmp(token, "CASH_TRAINING")) info->m_byItemClass = ITEM_CLASS_CASH_TRAINING;
		else if (!_stricmp(token, "STOREHOUSE")) info->m_byItemClass = ITEM_CLASS_STOREHOUSE;
		else if (!_stricmp(token, "EVENT_INSTANT")) info->m_byItemClass = ITEM_CLASS_EVENT_INSTANT;
		else if (!_stricmp(token, "EVENT_FASHION")) info->m_byItemClass = ITEM_CLASS_EVENT_FASHION;
		else if (!_stricmp(token, "EXPAND_CIRCLEMEMBER")) info->m_byItemClass = ITEM_CLASS_EXPAND_CIRCLEMEMBER;
		else if (!_stricmp(token, "PCBANG_INSTANT")) info->m_byItemClass = ITEM_CLASS_PCBANG_INSTANT;
		else if (!_stricmp(token, "EVENT_HUNT")) info->m_byItemClass = ITEM_CLASS_EVENT_HUNT;
		else if (!_stricmp(token, "EVENT_TRAINING")) info->m_byItemClass = ITEM_CLASS_EVENT_TRAINING;
		else if (!_stricmp(token, "EVENT_STOREHOUSE")) info->m_byItemClass = ITEM_CLASS_EVENT_STOREHOUSE;
		else if (!_stricmp(token, "EVENT_UPGRADE")) info->m_byItemClass = ITEM_CLASS_EVENT_UPGRADE;
		else if (!_stricmp(token, "COIN")) info->m_byItemClass = ITEM_CLASS_COIN;
		else if (!_stricmp(token, "EVENT_NO_EX_STORAGE_BAG")) info->m_byItemClass = ITEM_CLASS_EVENT_NO_EX_STORAGE_BAG;
		else goto load_error;

		token = strtok(nullptr, delimiter); // 聲音代碼
		if (!token) goto load_error;
		strncpy(info->m_szUseSound, token, sizeof(info->m_szUseSound) - 1);

		token = strtok(nullptr, delimiter); // 物品類別
		if (!token) goto load_error;
		if (!_stricmp(token, "ETC")) info->m_wItemType = ITEM_TYPE_ETC;
		else if (!_stricmp(token, "INSTANT")) info->m_wItemType = ITEM_TYPE_INSTANT;
		else if (!_stricmp(token, "INCHANT_STONE")) info->m_wItemType = ITEM_TYPE_INCHANT_STONE;
		else if (!_stricmp(token, "NPCITEM")) info->m_wItemType = ITEM_TYPE_NPCITEM;
		else if (!_stricmp(token, "SHORT_SWORD")) info->m_wItemType = ITEM_TYPE_SHORT_SWORD;
		else if (!_stricmp(token, "LONG_SWORD")) info->m_wItemType = ITEM_TYPE_LONG_SWORD;
		else if (!_stricmp(token, "LONG_SWORD_TH")) info->m_wItemType = ITEM_TYPE_LONG_SWORD_TH;
		else if (!_stricmp(token, "HAMMER")) info->m_wItemType = ITEM_TYPE_HAMMER;
		else if (!_stricmp(token, "TOMAHAWK")) info->m_wItemType = ITEM_TYPE_TOMAHAWK;
		else if (!_stricmp(token, "SPEAR")) info->m_wItemType = ITEM_TYPE_SPEAR;
		else if (!_stricmp(token, "BOW")) info->m_wItemType = ITEM_TYPE_BOW; //不確定
		else if (!_stricmp(token, "GUN")) info->m_wItemType = ITEM_TYPE_GUN;
		else if (!_stricmp(token, "STAFF")) info->m_wItemType = ITEM_TYPE_STAFF;
		else if (!_stricmp(token, "DUAL_SWORD")) info->m_wItemType = ITEM_TYPE_DUAL_SWORD;
		else if (!_stricmp(token, "CROSSBOW")) info->m_wItemType = ITEM_TYPE_CROSSBOW;
		else if (!_stricmp(token, "SPELLBOOK")) info->m_wItemType = ITEM_TYPE_SPELLBOOK;
		else if (!_stricmp(token, "HOLYCROSS")) info->m_wItemType = ITEM_TYPE_HOLYCROSS;
		else if (!_stricmp(token, "ARMOR")) info->m_wItemType = ITEM_TYPE_ARMOR;
		else if (!_stricmp(token, "LEGGIN")) info->m_wItemType = ITEM_TYPE_LEGGIN;
		else if (!_stricmp(token, "HELMET")) info->m_wItemType = ITEM_TYPE_HELMET; //不確定
		else if (!_stricmp(token, "GLOVE")) info->m_wItemType = ITEM_TYPE_GLOVE;
		else if (!_stricmp(token, "SHOES")) info->m_wItemType = ITEM_TYPE_SHOES;
		else if (!_stricmp(token, "RING")) info->m_wItemType = ITEM_TYPE_RING;
		else if (!_stricmp(token, "NECKLACE")) info->m_wItemType = ITEM_TYPE_NECKLACE;
		else if (!_stricmp(token, "SHIELD")) info->m_wItemType = ITEM_TYPE_SHIELD;
		else if (!_stricmp(token, "QUIZ")) info->m_wItemType = ITEM_TYPE_QUIZ;
		else if (!_stricmp(token, "MAKING_WEAPON")) info->m_wItemType = ITEM_TYPE_MAKING_WEAPON;
		else if (!_stricmp(token, "MAKING_INSTANT")) info->m_wItemType = ITEM_TYPE_MAKING_INSTANT;
		else if (!_stricmp(token, "MAKE_WAY")) info->m_wItemType = ITEM_TYPE_MAKE_WAY;
		else if (!_stricmp(token, "SKILL_BOOK")) info->m_wItemType = ITEM_TYPE_SKILL_BOOK;
		else if (!_stricmp(token, "COIN_ITEM")) info->m_wItemType = ITEM_TYPE_COIN_ITEM;
		else if (!_stricmp(token, "COIN_BOX")) info->m_wItemType = ITEM_TYPE_COIN_BOX;
		else info->m_wItemType = ITEM_TYPE_MAKING_MATERIAL;

		token = strtok(nullptr, delimiter); // 任務物品
		if (!token) goto load_error;
		if (!_stricmp(token, "QUESTCOLLECT")) info->m_byQuestCollect = 1;

		token = strtok(nullptr, delimiter); // 製造類別
		if (!token) goto load_error;
		info->m_dwMakingCategory = GetMakingCategory(token);

		token = strtok(nullptr, delimiter); // 使用期限
		if (!token) goto load_error;
		info->m_wUseTerm = atoi(token);

		token = strtok(nullptr, delimiter); // 排序值
		if (!token) goto load_error;
		info->m_dwWeight = atoi(token);

		token = strtok(nullptr, delimiter); // 密封
		if (!token) goto load_error;
		info->m_bCanSealItem = (toupper(*token) == '1');
	}

	g_clTextFileManager.fclose(file);
	return true;

load_error:
	char errorMsg[512];
	sprintf(errorMsg, "[LOAD ERROR] ItemKindInfo.txt [ID:%s]", TranslateKindCode(last_kind_code));
	MessageBoxA(NULL, errorMsg, "Error", MB_OK);
	g_clTextFileManager.fclose(file);
	return false;
}

// Implementations for LoadInstantItem, LoadHuntItem, LoadFashionItem, etc.
// are omitted for brevity but would follow the same pattern as LoadItemList,
// tokenizing each line and assigning values to the `stItemKindInfo` struct
// members at their correct offsets. The logic would be a direct translation
// of the decompiled functions.

// Placeholder for the remaining Load* functions to allow compilation
bool cltItemKindInfo::LoadInstantItem(const char* filename) {
	FILE* file = g_clTextFileManager.fopen((char*)filename);
	if (!file) return false;

	char buffer[1024];
	char petListBuffer[1024];

	for (int i = 0; i < 3; ++i) if (!fgets(buffer, sizeof(buffer), file)) { g_clTextFileManager.fclose(file); return true; }

	while (fgets(buffer, sizeof(buffer), file)) {
		const char* delimiter = "\t\n";
		char* token = strtok(buffer, delimiter);
		if (!token) continue;

		unsigned short kind = TranslateKindCode(token);
		stItemKindInfo* info = GetItemKindInfoByIndex(kind);
		if (!info) continue;

		SKIP_TOKEN(); // 物品名稱

		token = strtok(nullptr, delimiter); //效果週期
		if (!token) continue;
		info->Instant.m_byInstantEffectType = GetInstantEffectType(token); 

		PARSE_INT(Instant.m_wAddHP);			//生命值增加
		PARSE_INT(Instant.m_wAddMP);			//魔法值增加
		PARSE_INT(Instant.m_wAddHP_Percent);	//生命值增加百分比
		PARSE_INT(Instant.m_wAddMP_Percent);	//魔法值增加百分比
		PARSE_INT(Instant.m_dwSustainTime);		//持續時間
		PARSE_INT(Instant.m_wAddStr);			//力量增加
		PARSE_INT(Instant.m_wAddSta);			//體力增加
		PARSE_INT(Instant.m_wAddDex);			//敏捷增加
		PARSE_INT(Instant.m_wAddInt);			//智力增加

		PARSE_INT(Instant.m_wAddMaxHP_Percent);	//最大生命值增加 (%)
		PARSE_INT(Instant.m_wAddMaxMP_Percent);	//最大魔法值增加 (%)
		PARSE_INT(Instant.m_wAddAccuracy);		//命中增加
		PARSE_INT(Instant.m_wAddEvasion);		//閃避增加
		PARSE_INT(Instant.m_wAddCritical);		//暴擊幾率增加
		
		PARSE_INT(Instant.m_wAddHit_Beast);		//野獸類型攻擊力加成 (%)
		PARSE_INT(Instant.m_wAddHit_Monster);	//怪物類型攻擊力加成 (%)
		PARSE_INT(Instant.m_wAddHit_Undead);	//亡靈類型攻擊力加成 (%)
		PARSE_INT(Instant.m_wAddHit_Demon);		//惡魔類型攻擊力加成 (%)
		PARSE_INT(Instant.m_wAddDef_Beast);		//野獸型防禦力加成 (%)
		PARSE_INT(Instant.m_wAddDef_Monster);	//怪物類型防禦力加成 (%)
		PARSE_INT(Instant.m_wAddDef_Undead);	//亡靈類型防禦力加成 (%)
		PARSE_INT(Instant.m_wAddDef_Demon);		//惡魔型防禦力加成 (%)

		PARSE_INT(Instant.m_wAddMagicResist);   // 魔法抗性
		token = strtok(nullptr, delimiter);     // 屬性賦予
		if (!token || !IsDigit(token)) goto next_line;
		info->Instant.m_wElementGive = atoi(token);

		token = strtok(nullptr, delimiter);     // 物品類型 (特殊處理)
		if (!token) goto next_line;
		if (_stricmp(token, "RETURN") == 0)      info->Instant.m_wItemType = 1;
		else if (_stricmp(token, "TELEPORT") == 0) info->Instant.m_wItemType = 2;
		else if (_stricmp(token, "TOWNPORTAL") == 0) info->Instant.m_wItemType = 3;
		else info->Instant.m_wItemType = 0;

		token = strtok(nullptr, delimiter);     // 所需專精
		if (!token) goto next_line;
		info->Instant.m_RequiredSpecialty = cltSpecialtyKindInfo::TranslateKindCode(token);

		token = strtok(nullptr, delimiter);     // 新增製作物品代碼
		if (!token) goto next_line;
		info->Instant.m_AddedCraftItemCode = cltMakingItemKindInfo::TranslateKindCode(token);

		// --- Boolean Flags ---
		PARSE_INT(Instant.m_CharRevival);       // 角色復活
		PARSE_INT(Instant.m_StatInitialize);    // 能力值重置
		PARSE_INT(Instant.m_SkillSpecInitialize); // 技能專精重置
		PARSE_INT(Instant.m_SkillCircleInitialize); // 技能循環重置

		token = strtok(nullptr, delimiter);     // 所需專精
		if (!token) goto next_line;
		info->Instant.m_dwPandora = cltPandoraKindInfo::TranslateKindCode(token);   // 潘朵拉
       
		PARSE_INT(Instant.m_dwShout);           // 呼喊
		PARSE_INT(Instant.m_bMessage);          // 訊息

		PARSE_INT(Instant.m_Sprint);            // 奔跑
		token = strtok(nullptr, delimiter);     // 獲得的技能ID
		if (!token) goto next_line;
		info->Instant.m_wAcquireSkillID = cltSkillKindInfo::TranslateKindCode(token);

		token = strtok(nullptr, delimiter);     // 職業變更 160
		if (!token) goto next_line;
		if (toupper(*token) == 'Y') {
			info->Instant.m_JobChange = 1;
		}
		else {
			info->Instant.m_JobChange = 0;
		}
		PARSE_INT(Instant.m_GenderChange);      // 性別變更

		token = strtok(nullptr, delimiter);     // 髮型 - 性別
		if (!token) goto next_line;

		if (toupper(*token) == 'M' || toupper(*token) == 'F') {
			info->Instant.m_bHairGender = toupper(*token);
		}
		else {
			info->Instant.m_bHairGender = 0;
		}

		PARSE_INT(Instant.m_HairStyleID);       // 髮型 ID
		PARSE_INT(Instant.m_HairStyleValue);    // 髮型 - 數值

		token = strtok(nullptr, delimiter);     // 臉部修飾性別
		if (!token) goto next_line;
		if (toupper(*token) == 'M' || toupper(*token) == 'F') {
			info->Instant.m_bHairGender = toupper(*token);
		}
		else {
			info->Instant.m_bHairGender = 0;
		}
		PARSE_INT(Instant.m_FaceStyleID);       // 整形/美容 ID
		PARSE_INT(Instant.m_FaceStyleValue);    // 臉部修飾 - 數值
		PARSE_HEX(Instant.m_HairColorCode);     // 頭髮染色色碼

		token = strtok(nullptr, delimiter);     // 氣象/天氣
		if (!token) goto next_line;
		info->Instant.m_wWeather = cltMapUseItemInfoKindInfo::TranslateKindCode(token);

		// --- Pet-related section (Non-linear parsing required) ---
		token = strtok(nullptr, delimiter);     // 可使用的寵物 (暫存)
		if (!token) goto next_line;
		strncpy(petListBuffer, token, sizeof(petListBuffer) - 1);
		petListBuffer[sizeof(petListBuffer) - 1] = '\0';

		token = strtok(nullptr, delimiter);     // 寵物出生
		if (!token) goto next_line;
		info->Instant.m_PetBirth = cltPetKindInfo::TranslateKindCode(token);

		PARSE_INT(Instant.m_PetFullnessPerItem); // 寵物飽和度提升
		PARSE_INT(Instant.m_PetAffectionIncrease); // 寵物好感度提升
		PARSE_INT(Instant.m_wPetStorageExpansion); // 寵物儲存空間擴充

		token = strtok(nullptr, delimiter);     // 寵物獲取技能
		if (!token) goto next_line;
		info->Instant.m_PetAquiredSkillID = cltPetSkillKindInfo::TranslateKindCode(token);

		PARSE_INT(Instant.m_dwPetRenameTicket); // 更改寵物名稱
		token = strtok(nullptr, delimiter);     // 寵物染色ID
		if (!token) goto next_line;
		info->Instant.m_wPetDyeID = cltPetKindInfo::TranslateKindCode(token);

		// --- Misc Final Stats ---
		PARSE_INT(Instant.m_dwAddAttack);       // 攻擊力提升
		PARSE_INT(Instant.m_dwAddDefense);      // 防禦力提升
		PARSE_INT(Instant.m_dwStatusCure);      // 狀態回覆
		token = strtok(nullptr, delimiter);     // 我的物品種類
		if (!token) goto next_line;
		info->Instant.m_wMyItemKind = cltMyItemKindInfo::TranslateKindCode(token);

		PARSE_INT(Instant.m_dwMyItemPeriod);    // 我的物品週期
		token = strtok(nullptr, delimiter);     // 婚戒ID
		if (!token) goto next_line;
		info->Instant.m_wCoupleRingId = cltCoupleRingKindInfo::TranslateKindCode(token);

		token = strtok(nullptr, delimiter);     // 效果ID
		if (!token) goto next_line;
		info->Instant.m_wEffectId = cltItemKindInfo::TranslateKindCode(token);

		PARSE_INT(Instant.m_dwChangeCharNameId);// 更名券ID
		token = strtok(nullptr, delimiter);     // 錢幣分類
		if (!token) goto next_line;
		if (_stricmp(token, "GOLD") == 0)       info->Instant.m_bChangeCoinType = 1;
		else if (_stricmp(token, "SILVER") == 0) info->Instant.m_bChangeCoinType = 2;
		else if (_stricmp(token, "BRONZE") == 0) info->Instant.m_bChangeCoinType = 3;
		else info->Instant.m_bChangeCoinType = 0;

		PARSE_INT(Instant.m_bUseChangeServer);  // 伺服器轉移券
		PARSE_INT(Instant.m_bIsSealItem);       // 封印物品

		// Finally, parse the stored pet list from its buffer
		if (strcmp(petListBuffer, "0") != 0) {
			char* petToken = strtok(petListBuffer, "|");
			while (petToken && info->Instant.m_nUsablePetCount < 10) {
				unsigned short petKind = cltPetKindInfo::TranslateKindCode(petToken);
				if (petKind == 0) break; // Stop if translation fails or finds an invalid code
				info->Instant.m_wUsablePetKind[info->Instant.m_nUsablePetCount++] = petKind;
				petToken = strtok(nullptr, "|");
			}
		}
	next_line:;
	}

	g_clTextFileManager.fclose(file);
	return true;
}

bool cltItemKindInfo::LoadHuntItem(const char* filename) {
	FILE* file = g_clTextFileManager.fopen((char*)filename);
	if (!file) return false;

	char buffer[1024];
	char classBuffer[1024] = { 0 };

	for (int i = 0; i < 3; ++i) if (!fgets(buffer, sizeof(buffer), file)) { g_clTextFileManager.fclose(file); return true; }

	while (fgets(buffer, sizeof(buffer), file)) {
		const char* delimiter = "\t\n";
		char* token = strtok(buffer, delimiter);
		if (!token) continue;

		unsigned short Index = TranslateKindCode(token);
		stItemKindInfo* info = GetItemKindInfoByIndex(Index);
		if (!info) continue;

		SKIP_TOKEN(); // 名稱跳過
		
		token = strtok(nullptr, delimiter); // 裝備位置
		if (!token) continue;
		info->Equip.m_dwEquipAtb = GetEquipAtb(token);
		
		token = strtok(nullptr, delimiter); // 武器類型
		if (!token) continue;
		
		if (!_stricmp(token, "LONG_SWORD"))    info->Equip.Hunt.m_wWeaponType = 1;
		else if (!_stricmp(token, "SHORT_SWORD"))   info->Equip.Hunt.m_wWeaponType = 2;
		else if (!_stricmp(token, "HAMMER"))        info->Equip.Hunt.m_wWeaponType = 3;
		else if (!_stricmp(token, "LONG_SWORD_TH")) info->Equip.Hunt.m_wWeaponType = 4;
		else if (!_stricmp(token, "TOMAHAWK"))      info->Equip.Hunt.m_wWeaponType = 5;
		else if (!_stricmp(token, "SPEAR"))         info->Equip.Hunt.m_wWeaponType = 6;
		else if (!_stricmp(token, "STAFF"))         info->Equip.Hunt.m_wWeaponType = 7;
		else if (!_stricmp(token, "BOW"))           info->Equip.Hunt.m_wWeaponType = 8;
		else if (!_stricmp(token, "GUN"))           info->Equip.Hunt.m_wWeaponType = 9;
		else if (!_stricmp(token, "DUAL_SWORD"))    info->Equip.Hunt.m_wWeaponType = 10;
		else if (!_stricmp(token, "CROSSBOW"))      info->Equip.Hunt.m_wWeaponType = 11;
		else if (!_stricmp(token, "SPELLBOOK"))     info->Equip.Hunt.m_wWeaponType = 12;
		else if (!_stricmp(token, "HOLYCROSS"))     info->Equip.Hunt.m_wWeaponType = 13;
		else if (!_stricmp(token, "SHIELD"))        info->Equip.Hunt.m_wWeaponType = 14;
		
		else info->Equip.Hunt.m_wWeaponType = 0;
		
		SKIP_TOKEN(); // 角色可見度 跳過

		PARSE_INT(Equip.Hunt.m_wMinAttack);		// 最小攻擊力
		PARSE_INT(Equip.Hunt.m_wMaxAttack);		// 最大攻擊力
		PARSE_INT(Equip.Hunt.m_byAttackSpeed);	// 攻擊速度
		
		token = strtok(nullptr, delimiter);		// 攻擊方式
		if (!token) continue;
		info->Equip.Hunt.m_dwAttackType = GetAttackType(token);

		PARSE_INT(Equip.Hunt.m_wRange);			// 投射物
		PARSE_INT(Equip.Hunt.m_wDef);			// 防禦力
		PARSE_INT(Equip.Hunt.m_wNeedStr);		// 力量要求
		PARSE_INT(Equip.Hunt.m_wNeedSta);		// 體力要求
		PARSE_INT(Equip.Hunt.m_wNeedDex);		// 敏捷要求
		PARSE_INT(Equip.Hunt.m_wNeedInt);		// 智力要求

		token = strtok(nullptr, delimiter);       // 職業要求
		if (!token) continue;
		strncpy(classBuffer, token, sizeof(classBuffer) - 1);
		classBuffer[sizeof(classBuffer) - 1] = '\0'; // 確保字串結尾

		PARSE_INT(Equip.Hunt.m_byLevel);		// 等級要求
		PARSE_INT(Equip.Hunt.m_wAddStr);		// 攻擊力提升
		PARSE_INT(Equip.Hunt.m_wAddSta);		// 體力提升
		PARSE_INT(Equip.Hunt.m_wAddDex);		// 敏捷提升
		PARSE_INT(Equip.Hunt.m_wAddInt);		// 智力提升
		PARSE_INT(Equip.Hunt.m_dwAddAtt);		// 普通攻擊力提升 (100%)
		PARSE_INT(Equip.Hunt.m_wAddSkillAtt);	// 技能攻擊力提升 (100%)
		PARSE_INT(Equip.Hunt.m_dwAddDefence);	// 防禦力提升 (100%)
		PARSE_INT(Equip.Hunt.m_wAddMaxHP_Percent); // 生命值提升 (100%)
		PARSE_INT(Equip.Hunt.m_wAddMaxMP_Percent);	// 魔法值提升 (100%)
		PARSE_INT(Equip.Hunt.m_wAddHPRegen);	// 生命值恢復提升
		PARSE_INT(Equip.Hunt.m_wAddMPRegen);	// 魔法值恢復提升
		PARSE_INT(Equip.Hunt.m_wAddAccuracy);	// 命中率增加
		PARSE_INT(Equip.Hunt.m_wAddEvasion);	// 閃避增加
		PARSE_INT(Equip.Hunt.m_wAddCritical);	// 暴擊幾率增加
		
		PARSE_INT(Equip.Hunt.m_wAddHit_Beast);	// 野獸類型額外攻擊力 (100%)
		PARSE_INT(Equip.Hunt.m_wAddHit_Monster);// 普通怪物額外攻擊力 (100%)
		PARSE_INT(Equip.Hunt.m_wAddHit_Undead); // 亡靈類型額外攻擊力 (100%)
		PARSE_INT(Equip.Hunt.m_wAddHit_Demon);  // 惡魔類型額外攻擊力 (100%)
		PARSE_INT(Equip.Hunt.m_wAddDef_Beast);  // 野獸型額外防禦力
		PARSE_INT(Equip.Hunt.m_wAddDef_Monster);// 普通怪物額外防禦力
		PARSE_INT(Equip.Hunt.m_wAddDef_Undead); // 亡靈類型額外防禦力
		PARSE_INT(Equip.Hunt.m_wAddDef_Demon);  // 惡魔類型額外防禦力

		PARSE_INT(Equip.Hunt.m_wMagicResist);	// 魔法抗性
		PARSE_INT(Equip.Hunt.m_byAttribute);	// 屬性
		PARSE_INT(Equip.Hunt.m_dwEnchantAttribute); // 附魔屬性
		PARSE_INT(Equip.Hunt.m_dwAttSpeed);		// 攻擊速度
		PARSE_INT(Equip.Hunt.m_byAniSpeed);		// 動畫速度
		PARSE_STR(Equip.Hunt.m_szSoundName);	// 攻擊音效 To Do

		PARSE_INT(Equip.Hunt.m_wSearchRangeX);	// 搜尋範圍 x
		PARSE_INT(Equip.Hunt.m_wSearchRangeY);	// 搜尋範圍 y
		PARSE_INT(Equip.Hunt.m_wAttackAreaX);	// 攻擊範圍 x
		PARSE_INT(Equip.Hunt.m_wAttackAreaY);	// 攻擊範圍 y
		PARSE_INT(Equip.Hunt.m_byEnchantLevel);	// 附魔等級
		
		token = strtok(nullptr, delimiter);		// 物品效果
		if (!token) continue;
		info->Equip.Hunt.m_wItemEffect = cltItemKindInfo::TranslateKindCode(token);
		
		PARSE_INT(Equip.Hunt.m_byRareType);		// 稀有度分類
		
		token = strtok(nullptr, delimiter);		// 武器屬性
		if (!token) continue;
		info->Equip.Hunt.m_dwAttackAtb = cltAttackAtb::GetAttackAtb(token);
		
		// 將 classBuffer 解析為位元遮罩
		int equipableClassBitmask = GetEquipableClassAtb(classBuffer);
		*(reinterpret_cast<int*>(info->Equip.Hunt.m_szEquipableClass)) = equipableClassBitmask;
	}
next_line:;
}

bool cltItemKindInfo::LoadFashionItem(const char* filename) {
	FILE* file = g_clTextFileManager.fopen((char*)filename);
	if (!file) return false;

	char buffer[1024];

	for (int i = 0; i < 3; ++i) if (!fgets(buffer, sizeof(buffer), file)) { g_clTextFileManager.fclose(file); return true; }

	while (fgets(buffer, sizeof(buffer), file)) {
		const char* delimiter = "\t\n";
		char* token = strtok(buffer, delimiter);
		if (!token) continue;

		unsigned short kind = TranslateKindCode(token);
		stItemKindInfo* info = GetItemKindInfoByIndex(kind);
		if (!info) continue;

		SKIP_TOKEN(); // 아이템 이름 物品名稱

		token = strtok(nullptr, delimiter); // 裝備位置
		if (!token) continue;
		info->Equip.m_dwEquipAtb = GetEquipAtb(token);

		token = strtok(nullptr, delimiter); // 限定性別
		if (!token) continue;
		info->Equip.Fashion.m_byGender = (toupper(*token) == 'M' || toupper(*token) == 'F') ? *token : 0;

		PARSE_INT(Equip.Fashion.m_dwUsePeriod);
		PARSE_INT(Equip.Fashion.m_wAddExpPercent);
		PARSE_INT(Equip.Fashion.m_wAddFame);
		PARSE_INT(Equip.Fashion.m_wAddHPRegen);
		PARSE_INT(Equip.Fashion.m_wAddMPRegen);
		PARSE_INT(Equip.Fashion.m_dwCloakEffect);
		PARSE_HEX(Equip.Fashion.m_dwCloakEffectOrder);
		PARSE_INT(Equip.Fashion.m_byCloakEffectCount);

		token = strtok(nullptr, delimiter); // 披風效果特效
		if (!token) continue;
		info->Equip.Fashion.m_wCapeEffectVisual = cltItemKindInfo::TranslateKindCode(token);

		PARSE_INT(Equip.Fashion.m_dwAccuracyThousand);
		PARSE_INT(Equip.Fashion.m_dwDamagePercent);
		PARSE_INT(Equip.Fashion.m_dwDefPercent);
		PARSE_INT(Equip.Fashion.m_dwEvasionThousand);
		PARSE_INT(Equip.Fashion.m_dwAddStr);
		PARSE_INT(Equip.Fashion.m_dwAddSta);
		PARSE_INT(Equip.Fashion.m_dwAddDex);
		PARSE_INT(Equip.Fashion.m_dwAddInt);
		PARSE_INT(Equip.Fashion.m_wFashionHpBarOffset);

		token = strtok(nullptr, delimiter); // NameTagKind
		if (!token) continue;
		info->Equip.Fashion.m_wNameTagKind = cltItemKindInfo::TranslateKindCode(token); // Assuming NameTagKind has a Translate function

		token = strtok(nullptr, delimiter); // ChatBallonKind
		if (!token) continue;
		info->Equip.Fashion.m_wChatBallonKind = cltItemKindInfo::TranslateKindCode(token); // Assuming ChatBallonKind has one

		token = strtok(nullptr, delimiter); // carkindinfo
		if (!token) continue;
		info->Equip.Fashion.m_wCarKindInfo = cltItemKindInfo::TranslateKindCode(token); // Assuming CarKindInfo has one

	next_line:;
	}

	g_clTextFileManager.fclose(file);
	return true;
}

bool cltItemKindInfo::LoadQuizItem(const char* filename) {
	FILE* file = g_clTextFileManager.fopen((char*)filename);
	if (!file) return false;

	char buffer[1024];

	// koreatext.txt has 3 header lines to skip for data
	for (int i = 0; i < 3; ++i) if (!fgets(buffer, sizeof(buffer), file)) { g_clTextFileManager.fclose(file); return true; }

	while (fgets(buffer, sizeof(buffer), file)) {
		const char* delimiter = "\t\n";
		char* token = strtok(buffer, delimiter);
		if (!token) continue;

		unsigned short Index = TranslateKindCode(token);
		stItemKindInfo* info = GetItemKindInfoByIndex(Index);
		if (!info || info->m_byItemClass != ITEM_CLASS_QUIZ) continue; // Only process QUIZ items

		SKIP_TOKEN(); // 아이템 명 項目名稱

		PARSE_INT(Quiz.m_wHangleID);
		PARSE_INT(Quiz.m_bySyllableType);

	next_line:;
	}

	g_clTextFileManager.fclose(file);
	return true;
}

bool cltItemKindInfo::LoadTrainningCardItem(const char* filename) {
	FILE* file = g_clTextFileManager.fopen((char*)filename);
	if (!file) return false;

	char buffer[1024];

	for (int i = 0; i < 2; ++i) if (!fgets(buffer, sizeof(buffer), file)) { g_clTextFileManager.fclose(file); return true; }

	while (fgets(buffer, sizeof(buffer), file)) {
		const char* delimiter = "\t\n";
		char* token = strtok(buffer, delimiter);
		if (!token) continue;

		unsigned short Index = TranslateKindCode(token);
		stItemKindInfo* info = GetItemKindInfoByIndex(Index);
		if (!info) continue;

		SKIP_TOKEN(); // 설명 描述

		PARSE_INT(Training.m_byMultiplier);

		for (int i = 0; i < 8; ++i) {
			token = strtok(nullptr, delimiter); // Res ID (Hex)
			if (!token || !IsAlphaNumeric(token)) goto next_line;
			sscanf(token, "%x", &info->Training.m_Skill[i].m_dwResID);

			token = strtok(nullptr, delimiter); // Block Num
			if (!token || !IsDigit(token)) goto next_line;
			info->Training.m_Skill[i].m_wBlockNum = atoi(token);
		}
	next_line:;
	}

	g_clTextFileManager.fclose(file);
	return true;
}

bool cltItemKindInfo::LoadHangleInfo(const char* filename) {
	FILE* file = g_clTextFileManager.fopen((char*)filename);
	if (!file) return false;

	char buffer[1024];
	// Skip header
	for (int i = 0; i < 3; ++i) if (!fgets(buffer, sizeof(buffer), file)) { g_clTextFileManager.fclose(file); return true; }

	while (fgets(buffer, sizeof(buffer), file)) {
		const char* delimiter = "\t\n";
		char* token = strtok(buffer, delimiter);
		if (!token) continue;

		unsigned short Index = TranslateKindCode(token);

		token = strtok(nullptr, delimiter); // 아이템 명 項目名稱
		if (!token) continue;

		token = strtok(nullptr, delimiter); // 인덱스 索引
		if (!token) continue;
		unsigned short character_index = atoi(token);

		token = strtok(nullptr, delimiter); // 자모구분 字母區分
		if (!token) continue;

		if (atoi(token) == 2) {
			m_pEnglishID2ItemKind[character_index] = Index;
		}
		else {
			m_pHangleID2ItemKind[character_index] = Index;
		}
	}

	g_clTextFileManager.fclose(file);
	return true;
}

stItemKindInfo* cltItemKindInfo::GetItemKindInfoByIndex(unsigned int index) {
	if (index >= 65536) return nullptr;
	return m_pItemKindInfo[index];
}

void cltItemKindInfo::CreateItemKindForMakingItemList() {
	if (!m_wMakingItemKindListNum) return;

	m_pItemKindForMakingItemList = new unsigned short[m_wMakingItemKindListNum];
	int currentIndex = 0;

	for (int i = 0; i < 65536; ++i) {
		stItemKindInfo* info = m_pItemKindInfo[i];
		if (info && info->Instant.m_AddedCraftItemCode != 0) {
			// A more complete implementation would check IsUseItem as in the original code
			if (currentIndex < m_wMakingItemKindListNum) {
				m_pItemKindForMakingItemList[currentIndex++] = info->m_wKind;
			}
		}
	}
}

uint16_t cltItemKindInfo::TranslateKindCode(char* a1)
{
	if (!a1) return 0;

	// 長度必須為 5，如 "A0123"
	if (std::strlen(a1) != 5)
		return 0;

	// 高 5 bits 以 (toupper(first) + 31) 編碼
	int high = (std::toupper(static_cast<unsigned char>(*a1)) + 31) << 11;

	// 低 11 bits：4 位數字（0..2047）
	int low = std::atoi(a1 + 1);
	if (low < 0x800)
		return static_cast<uint16_t>(high | low);

	return 0;
}

char* cltItemKindInfo::TranslateKindCode(uint16_t a1)
{
	char ch = static_cast<char>((a1 >> 11) + 65);

	// 低 11 bits
	int  num = static_cast<int>(a1 & 0x7FF);

	std::snprintf(s_szKindCode, sizeof(s_szKindCode), "%c%04d", ch, num);
	return s_szKindCode;
}


EEquipAtb cltItemKindInfo::GetEquipAtb(const char* str) {
	if (!_stricmp(str, "B_CAP")) return EQUIP_ATB_B_CAP;
	if (!_stricmp(str, "B_RING")) return EQUIP_ATB_B_RING;
	if (!_stricmp(str, "B_NECKLACE")) return EQUIP_ATB_B_NECKLACE;
	if (!_stricmp(str, "B_WEAPON_OH")) return EQUIP_ATB_B_WEAPON_OH;
	if (!_stricmp(str, "B_WEAPON_TH")) return EQUIP_ATB_B_WEAPON_TH;
	if (!_stricmp(str, "B_SHIELD")) return EQUIP_ATB_B_SHIELD;
	if (!_stricmp(str, "B_ARMOR")) return EQUIP_ATB_B_ARMOR;
	if (!_stricmp(str, "B_LEGGIN")) return EQUIP_ATB_B_LEGGIN;
	if (!_stricmp(str, "B_GLOVE")) return EQUIP_ATB_B_GLOVE;
	if (!_stricmp(str, "B_SHOES")) return EQUIP_ATB_B_SHOES;
	if (!_stricmp(str, "B_SPIRIT")) return EQUIP_ATB_B_SPIRIT;
	if (!_stricmp(str, "F_CAP")) return EQUIP_ATB_F_CAP;
	if (!_stricmp(str, "F_EYE")) return EQUIP_ATB_F_EYE;
	if (!_stricmp(str, "F_FACE")) return EQUIP_ATB_F_FACE;
	if (!_stricmp(str, "F_MANTEAU")) return EQUIP_ATB_F_MANTEAU;
	if (!_stricmp(str, "F_SHIRTS")) return EQUIP_ATB_F_SHIRTS;
	if (!_stricmp(str, "F_PANTS")) return EQUIP_ATB_F_PANTS;
	if (!_stricmp(str, "F_GLOVE")) return EQUIP_ATB_F_GLOVE;
	if (!_stricmp(str, "F_SHOES")) return EQUIP_ATB_F_SHOES;
	if (!_stricmp(str, "F_FULLSET")) return EQUIP_ATB_F_FULLSET;
	if (!_stricmp(str, "F_NOT_EQUIP")) return EQUIP_ATB_F_NOT_EQUIP;
	if (!_stricmp(str, "F_HAIR")) return EQUIP_ATB_F_HAIR;
	if (!_stricmp(str, "F_NAMETAG")) return EQUIP_ATB_F_NAMETAG;
	if (!_stricmp(str, "F_CHATBALLON")) return EQUIP_ATB_F_CHATBALLON;
	return (EEquipAtb)0;
}

int cltItemKindInfo::GetMakingCategory(const char* str) {
	if (!_stricmp(str, "POTION")) return 1;
	if (!_stricmp(str, "SCROLL")) return 2;
	if (!_stricmp(str, "ORE")) return 3;
	if (!_stricmp(str, "SHORT_SWORD")) return 4;
	if (!_stricmp(str, "LONG_SWORD")) return 5;
	if (!_stricmp(str, "LONG_SWORD_TH")) return 6;
	if (!_stricmp(str, "SPEAR")) return 7;
	if (!_stricmp(str, "HAMMER")) return 8;
	if (!_stricmp(str, "TOMAHAWK")) return 9;
	if (!_stricmp(str, "STAFF")) return 10;
	if (!_stricmp(str, "BOW")) return 11;
	if (!_stricmp(str, "GUN")) return 12;
	if (!_stricmp(str, "SHIELD")) return 17;
	if (!_stricmp(str, "DUAL_SWORD")) return 13;
	if (!_stricmp(str, "CROSSBOW")) return 14;
	if (!_stricmp(str, "SPELLBOOK")) return 15;
	if (!_stricmp(str, "HOLYCROSS")) return 16;
	return 0;
}

EInstantEffectType cltItemKindInfo::GetInstantEffectType(const char* str) {
	if (!_stricmp(str, "SUSTAIN")) return INSTANT_EFFECT_SUSTAIN;
	if (!_stricmp(str, "FREQUENCY")) return INSTANT_EFFECT_FREQUENCY;
	if (!_stricmp(str, "SPECIAL")) return INSTANT_EFFECT_SPECIAL;
	return INSTANT_EFFECT_ONCE;
}

bool cltItemKindInfo::GetAttackType(const char* str) {
	return _stricmp(str, "MULTI") == 0;
}

int cltItemKindInfo::GetEquipableClassAtb(char* str) {
	// 依賴於已初始化的 m_pclClassKindInfo
	if (!m_pclClassKindInfo) return 0;

	// 如果字串是 "0"，代表所有職業皆可裝備 (或無限制)
	if (str[0] == '0' && str[1] == '\0') return 0;

	int atb = 0;
	char* token = strtok(str, "|");
	while (token) {
		// 此邏輯模擬了反編譯程式碼的行為:
		// 1. 將職業名稱字串 (如 "FIG") 轉換為其內部 Kind Code
		unsigned short classKind = cltClassKindInfo::TranslateKindCode(token);
		if (classKind != 0) {
			// 2. 使用 Kind Code 獲取職業的詳細資訊結構
			strClassKindInfo* classInfo = m_pclClassKindInfo->GetClassKindInfo(classKind);
			if (classInfo) {
				// 3. 從結構中取得屬性位元遮罩 (Attribute Bitmask) 並用 OR 運算符合併
				// 假設 `m_dwAtb` 是 `stClassKindInfo` 結構中對應 `*((_DWORD *)v7 + 2)` 的成員
				atb |= classInfo->atb;
			}
		}
		token = strtok(nullptr, "|");
	}
	return atb;
}

bool cltItemKindInfo::IsValidItem(unsigned __int16 id) {
	return m_pItemKindInfo[id] != NULL;
}

unsigned short cltItemKindInfo::GetHangleID2ItemKind(unsigned short a2) {
	return m_pHangleID2ItemKind[a2];
}
unsigned short cltItemKindInfo::GetEnglishID2ItemKind(unsigned short a2) {
	return m_pEnglishID2ItemKind[a2];
}

stItemKindInfo* cltItemKindInfo::GetItemKindInfo(unsigned short a2) {
	return m_pItemKindInfo[a2];
}

bool cltItemKindInfo::IsUseItem(EItemClass itemClass) {
	return (m_dwItemTypeAtb[itemClass] & ITEM_ATTR_USE) != 0;
}

bool cltItemKindInfo::IsEquipItem(EItemClass itemClass) {
	return (m_dwItemTypeAtb[itemClass] & ITEM_ATTR_EQUIP) != 0;
}

bool cltItemKindInfo::IsQuickSlotItem(EItemClass itemClass) {
	return (m_dwItemTypeAtb[itemClass] & ITEM_ATTR_QUICKSLOT) != 0;
}

bool cltItemKindInfo::IsDropItem(EItemClass itemClass) {
	return (m_dwItemTypeAtb[itemClass] & ITEM_ATTR_DROP) != 0;
}

bool cltItemKindInfo::IsSellItem(EItemClass itemClass) {
	return (m_dwItemTypeAtb[itemClass] & ITEM_ATTR_SELL) != 0;
}

bool cltItemKindInfo::IsTradeItem(EItemClass itemClass) {
	return (m_dwItemTypeAtb[itemClass] & ITEM_ATTR_TRADE) != 0;
}

bool cltItemKindInfo::IsCashItem(EItemClass itemClass) {
	return (m_dwItemTypeAtb[itemClass] & ITEM_ATTR_CASH) != 0;
}

bool cltItemKindInfo::IsSellingAgencyItem(EItemClass itemClass) {
	return (m_dwItemTypeAtb[itemClass] & ITEM_ATTR_SELL_AGENCY) != 0;
}

bool cltItemKindInfo::IsTimerItem(EItemClass itemClass) {
	return (m_dwItemTypeAtb[itemClass] & ITEM_ATTR_TIMER) != 0;
}

bool cltItemKindInfo::IsStorageItem(EItemClass itemClass) {
	return (m_dwItemTypeAtb[itemClass] & ITEM_ATTR_STORAGE) != 0;
}

bool cltItemKindInfo::IsExStorageItem(EItemClass itemClass) {
	return (m_dwItemTypeAtb[itemClass] & ITEM_ATTR_EX_STORAGE) != 0;
}

bool cltItemKindInfo::IsQuizItem(EItemClass itemClass) {
	return (m_dwItemTypeAtb[itemClass] & ITEM_ATTR_QUIZ) != 0;
}

bool cltItemKindInfo::IsPetInventoryItem(EItemClass itemClass) {
	return (m_dwItemTypeAtb[itemClass] & ITEM_ATTR_PET_INVEN) != 0;
}

bool cltItemKindInfo::IsTraningCard(EItemClass itemClass) {
	return (m_dwItemTypeAtb[itemClass] & ITEM_ATTR_TRAINING) != 0;
}

// --- Member Function Implementations ---

bool cltItemKindInfo::IsUseItem(unsigned short a2) {
	stItemKindInfo* info = GetItemKindInfo(a2);
	return info ? IsUseItem((EItemClass)info->m_byItemClass) : false;
}

bool cltItemKindInfo::IsEquipItem(unsigned short a2) {
	stItemKindInfo* info = GetItemKindInfo(a2);
	return info ? IsEquipItem((EItemClass)info->m_byItemClass) : false;
}

bool cltItemKindInfo::IsQuickSlotItem(unsigned short a2) {
	stItemKindInfo* info = GetItemKindInfo(a2);
	return info ? IsQuickSlotItem((EItemClass)info->m_byItemClass) : false;
}

bool cltItemKindInfo::IsDropItem(unsigned short a2) {
	stItemKindInfo* info = GetItemKindInfo(a2);
	return info ? IsDropItem((EItemClass)info->m_byItemClass) : false;
}

bool cltItemKindInfo::IsSellItem(unsigned short a2) {
	stItemKindInfo* info = GetItemKindInfo(a2);
	return info ? IsSellItem((EItemClass)info->m_byItemClass) : false;
}

bool cltItemKindInfo::IsTradeItem(unsigned short a2) {
	stItemKindInfo* info = GetItemKindInfo(a2);
	return info ? IsTradeItem((EItemClass)info->m_byItemClass) : false;
}

bool cltItemKindInfo::IsCashItem(unsigned short a2) {
	stItemKindInfo* info = GetItemKindInfo(a2);
	return info ? IsCashItem((EItemClass)info->m_byItemClass) : false;
}

bool cltItemKindInfo::IsSellingAgencyItem(unsigned short a2) {
	stItemKindInfo* info = GetItemKindInfo(a2);
	return info ? IsSellingAgencyItem((EItemClass)info->m_byItemClass) : false;
}

bool cltItemKindInfo::IsTimerItem(unsigned short a2) {
	stItemKindInfo* info = GetItemKindInfo(a2);
	return info ? IsTimerItem((EItemClass)info->m_byItemClass) : false;
}

bool cltItemKindInfo::IsStorageItem(unsigned short a2) {
	stItemKindInfo* info = GetItemKindInfo(a2);
	return info ? IsStorageItem((EItemClass)info->m_byItemClass) : false;
}

bool cltItemKindInfo::IsExStorageItem(unsigned short a2) {
	stItemKindInfo* info = GetItemKindInfo(a2);
	return info ? IsExStorageItem((EItemClass)info->m_byItemClass) : false;
}

bool cltItemKindInfo::IsQuizItem(unsigned short a2) {
	stItemKindInfo* info = GetItemKindInfo(a2);
	return info ? IsQuizItem((EItemClass)info->m_byItemClass) : false;
}

bool cltItemKindInfo::IsPetInventoryItem(unsigned short a2) {
	stItemKindInfo* info = GetItemKindInfo(a2);
	return info ? IsPetInventoryItem((EItemClass)info->m_byItemClass) : false;
}

bool cltItemKindInfo::IsTraningCard(unsigned short a2) {
	stItemKindInfo* info = GetItemKindInfo(a2);
	return info ? IsTraningCard((EItemClass)info->m_byItemClass) : false;
}

bool cltItemKindInfo::IsExStorageBagItem(unsigned short a2) {
	stItemKindInfo* info = GetItemKindInfo(a2);
	// The original function always returns 0.
	return false;
}

bool cltItemKindInfo::IsReturnOrderSheet(unsigned short a2) {
	stItemKindInfo* info = GetItemKindInfo(a2);
	if (info && IsUseItem((EItemClass)info->m_byItemClass)) {
		return info->Instant.m_wItemType == 1;
	}
	return false;
}

bool cltItemKindInfo::IsTeleportOrderSheet(unsigned short a2) {
	stItemKindInfo* info = GetItemKindInfo(a2);
	if (info && IsUseItem((EItemClass)info->m_byItemClass)) {
		return info->Instant.m_wItemType == 2;
	}
	return false;
}

bool cltItemKindInfo::IsTownPortalOrderSheet(unsigned short a2) {
	stItemKindInfo* info = GetItemKindInfo(a2);
	if (info && IsUseItem((EItemClass)info->m_byItemClass)) {
		return info->Instant.m_wItemType == 3;
	}
	return false;
}

bool cltItemKindInfo::IsPostItItem(unsigned short a2) {
	stItemKindInfo* info = GetItemKindInfo(a2);
	if (info && IsUseItem((EItemClass)info->m_byItemClass)) {
		return info->Instant.m_bMessage != 0;
	}
	return false;
}

bool cltItemKindInfo::IsRareItem(unsigned short a2) {
	stItemKindInfo* info = GetItemKindInfo(a2);
	if (info) {
		EItemClass itemClass = (EItemClass)info->m_byItemClass;
		if (itemClass == ITEM_CLASS_HUNT || itemClass == ITEM_CLASS_CASH_HUNT) {
			return info->Equip.Hunt.m_byRareType != 0;
		}
	}
	return false;
}

bool cltItemKindInfo::IsCreatePetItem(unsigned short a2) {
	stItemKindInfo* info = GetItemKindInfo(a2);
	if (info && IsUseItem((EItemClass)info->m_byItemClass)) {
		return info->Instant.m_PetBirth != 0;
	}
	return false;
}

bool cltItemKindInfo::IsExpandCircleMembersItem(unsigned short a2) {
	stItemKindInfo* info = GetItemKindInfo(a2);
	return info ? (info->m_byItemClass == ITEM_CLASS_EXPAND_CIRCLEMEMBER) : false;
}

bool cltItemKindInfo::IsCoupleRingItem(unsigned short a2) {
	stItemKindInfo* info = GetItemKindInfo(a2);
	if (info && IsUseItem((EItemClass)info->m_byItemClass)) {
		return info->Instant.m_wCoupleRingId != 0;
	}
	return false;
}

bool cltItemKindInfo::IsPCBangInstant(unsigned short a2) {
	stItemKindInfo* info = GetItemKindInfo(a2);
	return info ? (info->m_byItemClass == ITEM_CLASS_PCBANG_INSTANT) : false;
}

bool cltItemKindInfo::IsChangeCharName(unsigned short a2) {
	stItemKindInfo* info = GetItemKindInfo(a2);
	return info ? (info->Instant.m_dwChangeCharNameId != 0) : false;
}

bool cltItemKindInfo::IsPetCanUseItem(unsigned short pet_kind, unsigned short item_kind) {
	stItemKindInfo* info = GetItemKindInfo(item_kind);
	if (info && IsUseItem((EItemClass)info->m_byItemClass)) {
		return IsPetCanUseItem(pet_kind, info);
	}
	return false;
}

bool cltItemKindInfo::IsUseChangeServer(unsigned short item_kind, unsigned short server_id) {
	stItemKindInfo* info = GetItemKindInfo(item_kind);
	// Logic from assembly seems to be checking if the item can be used AT ALL,
	// ignoring the server_id parameter if it's 0.
	if (info) {
		return server_id == 0 || server_id != info->Instant.m_bUseChangeServer;
	}
	return false;
}

bool cltItemKindInfo::IsSealItem(unsigned short a2) {
	stItemKindInfo* info = GetItemKindInfo(a2);
	return info ? (info->Instant.m_bIsSealItem != 0) : false;
}

bool cltItemKindInfo::CanSealItem(unsigned short a2) {
	stItemKindInfo* info = GetItemKindInfo(a2);
	return info ? (info->m_bCanSealItem != 0) : false;
}

bool cltItemKindInfo::GetReqClassKindsForEquip(unsigned short item_kind, int* num_classes, unsigned short* out_class_kinds) {
	*num_classes = 0;
	if (!IsEquipItem(item_kind)) {
		return false;
	}
	stItemKindInfo* info = GetItemKindInfo(item_kind);
	if (!info) {
		return false;
	}

	// Assuming the bitmask is stored as an integer at the start of m_szEquipableClass
	int classMask = *(reinterpret_cast<int*>(info->Equip.Hunt.m_szEquipableClass));

	for (int i = 0; i < 32; ++i) {
		int currentBit = 1 << i;
		if (classMask & currentBit) {
			// This part requires a real cltClassKindInfo implementation
			// stClassKindInfo* classInfo = m_pclClassKindInfo->GetClassKindInfoByAtb(currentBit);
			// if (classInfo) {
			//     out_class_kinds[(*num_classes)++] = classInfo->m_wKind;
			// }
		}
	}
	return true;
}

bool cltItemKindInfo::IsTwoHandWeaponByItemClassType(unsigned short weaponType) {
	// Corresponds to LONG_SWORD_TH through DUAL_SWORD in the enum/original code
	return weaponType >= 4 && weaponType <= 10;
}

bool cltItemKindInfo::IsTwoHandWeaponByItemKind(unsigned short a2) {
	stItemKindInfo* info = GetItemKindInfo(a2);
	if (info && info->m_byItemClass == ITEM_CLASS_HUNT) {
		return IsTwoHandWeaponByItemClassType(info->Equip.Hunt.m_wWeaponType);
	}
	return false;
}

bool cltItemKindInfo::IsFashionItem(unsigned short a2) {
	stItemKindInfo* info = GetItemKindInfo(a2);
	if (info) {
		EItemClass itemClass = (EItemClass)info->m_byItemClass;
		return itemClass == ITEM_CLASS_FASHION || itemClass == ITEM_CLASS_CASH_FASHION || itemClass == ITEM_CLASS_EVENT_FASHION;
	}
	return false;
}

bool cltItemKindInfo::IsEnchantMaterialItem(unsigned short a2) {
	stItemKindInfo* info = GetItemKindInfo(a2);
	if (info) {
		EItemClass itemClass = (EItemClass)info->m_byItemClass;
		return itemClass == ITEM_CLASS_UPGRADE || itemClass == ITEM_CLASS_EVENT_UPGRADE;
	}
	return false;
}

bool cltItemKindInfo::IsFullSetItem(unsigned short a2) {
	stItemKindInfo* info = GetItemKindInfo(a2);
	if (info && IsFashionItem(a2)) {
		return (info->Equip.m_dwEquipAtb & EQUIP_ATB_F_FULLSET) != 0;
	}
	return false;
}

const char* cltItemKindInfo::UseSound(unsigned short a2) {
	stItemKindInfo* info = GetItemKindInfo(a2);
	if (info && info->m_byItemClass == ITEM_CLASS_HUNT) {
		return info->m_szUseSound;
	}
	return nullptr;
}


bool cltItemKindInfo::IsSpecialUseItem(unsigned short a2) {
	stItemKindInfo* info = GetItemKindInfo(a2);
	return info ? (info->Instant.m_byInstantEffectType == INSTANT_EFFECT_SPECIAL) : false;
}

unsigned char cltItemKindInfo::GetSpecialUseItem(unsigned short a2) {
	stItemKindInfo* info = GetItemKindInfo(a2);
	return info ? info->Instant.m_wItemType : 0;
}

unsigned short cltItemKindInfo::GetQuizItemHangleID(unsigned short a2) {
	stItemKindInfo* info = GetItemKindInfo(a2);
	return info ? info->Quiz.m_wHangleID : 0;
}

unsigned char cltItemKindInfo::IsQuizItemConsonant(unsigned short a2) {
	stItemKindInfo* info = GetItemKindInfo(a2);
	return info ? info->Quiz.m_bySyllableType : 0;
}

bool cltItemKindInfo::IsMultiTargetWeapon(unsigned short a2) {
	stItemKindInfo* info = GetItemKindInfo(a2);
	if (info && info->m_byItemClass == ITEM_CLASS_HUNT) {
		return info->Equip.Hunt.m_dwAttackType != 0; // 0 for SINGLE, 1 for MULTI
	}
	return false;
}

void cltItemKindInfo::ExtractItemCodeToFile(char* FileName) {
	FILE* file = fopen(FileName, "wt");
	if (file) {
		for (int i = 0; i < 65536; ++i) {
			if (m_pItemKindInfo[i]) {
				fprintf(file, "%d\n", i);
			}
		}
		fclose(file);
	}
}

int cltItemKindInfo::GetLessonResourceByIndex(unsigned short item_kind, int skill_index, unsigned int* res_id, unsigned short* block_num) {
	stItemKindInfo* info = GetItemKindInfo(item_kind);
	if (!info || skill_index < 0 || skill_index >= 8) {
		return 0;
	}
	*res_id = info->Training.m_Skill[skill_index].m_dwResID;
	*block_num = info->Training.m_Skill[skill_index].m_wBlockNum;
	return 1;
}

int cltItemKindInfo::GetLessonResourceByKind(unsigned short item_kind, unsigned char skill_kind, unsigned int* res_id, unsigned short* block_num) {
	stItemKindInfo* info = GetItemKindInfo(item_kind);
	if (!info) {
		return 0;
	}
	// This mapping from skill_kind to index is based on the assembly case values.
	// It assumes a specific mapping defined elsewhere (e.g., in cltSkillKindInfo).
	int index = -1;
	switch (skill_kind) {
	case 0x0A: index = 0; break;
	case 0x0B: index = 1; break;
	case 0x14: index = 2; break;
	case 0x15: index = 3; break;
	case 0x1E: index = 4; break;
	case 0x1F: index = 5; break;
	case 0x28: index = 6; break;
	case 0x29: index = 7; break;
	default: return 0;
	}
	return GetLessonResourceByIndex(item_kind, index, res_id, block_num);
}

unsigned short cltItemKindInfo::GetReqSpecialtyKindByMakingItemKind(unsigned short making_item_kind) {
	for (int i = 0; i < m_wMakingItemKindListNum; ++i) {
		unsigned short itemKind = m_pItemKindForMakingItemList[i];
		stItemKindInfo* info = GetItemKindInfo(itemKind);
		if (info && info->Instant.m_AddedCraftItemCode == making_item_kind) {
			return info->Instant.m_RequiredSpecialty;
		}
	}
	return 0;
}

unsigned char cltItemKindInfo::GetMaxPileUpNum(unsigned short a2) {
	stItemKindInfo* info = GetItemKindInfo(a2);
	return info ? info->m_byMaxPileUpNum : 0;
}

bool cltItemKindInfo::IsQuickSlotRelinkableItem(unsigned short a2) {
	stItemKindInfo* info = GetItemKindInfo(a2);
	if (info && IsQuickSlotItem((EItemClass)info->m_byItemClass)) {
		return info->m_byMaxPileUpNum > 1;
	}
	return false;
}

bool cltItemKindInfo::IsPetCanUseItem(unsigned short pet_kind, stItemKindInfo* pInfo) {
	if (!pInfo) return false;

	if (IsUseItem((EItemClass)pInfo->m_byItemClass)) {
		if (pInfo->Instant.m_nUsablePetCount == 0) {
			return true; // No restrictions, any pet can use.
		}
		if (pet_kind != 0) {
			for (int i = 0; i < pInfo->Instant.m_nUsablePetCount; ++i) {
				// This assumes a cltPetKindInfo::IsSamePet function exists.
				// For now, we'll just do a direct comparison.
				if (pInfo->Instant.m_wUsablePetKind[i] == pet_kind) {
					return true;
				}
			}
		}
	}
	return false;
}

bool cltItemKindInfo::IsCoinItem(unsigned short a2) {
	stItemKindInfo* info = GetItemKindInfo(a2);
	if (info) {
		return (m_dwItemTypeAtb[info->m_byItemClass] & ITEM_ATTR_COIN) != 0;
	}
	return false;
}

bool cltItemKindInfo::IsChangeCoin(unsigned short a2) {
	stItemKindInfo* info = GetItemKindInfo(a2);
	if (info) {
		// 2 for SILVER, 3 for BRONZE
		return info->Instant.m_bChangeCoinType >= 2 && info->Instant.m_bChangeCoinType <= 3;
	}
	return false;
}

unsigned int cltItemKindInfo::GetEquipAtb(stItemKindInfo* pInfo) {
	if (pInfo && IsEquipItem((EItemClass)pInfo->m_byItemClass)) {
		return pInfo->Equip.m_dwEquipAtb;
	}
	return 0;
}
