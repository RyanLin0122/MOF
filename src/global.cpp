#include "global.h"
#include "Info/cltClassKindInfo.h"
#include "System/cltUsingItemSystem.h"
#include "System/cltSexSystem.h"
#include "System/cltPlayerAbility.h"
#include "System/cltUsingSkillSystem.h"
#include "System/cltSpecialtySystem.h"
#include "Info/cltItemKindInfo.h"
#include "Info/cltNPCInfo.h"
#include "Info/cltCharKindInfo.h"
#include "Info/cltPetKindInfo.h"
#include "Info/cltClientPetKindInfo.h"
#include "Info/cltPortalInfo.h"
#include "Info/cltClientPortalInfo.h"
#include "Info/cltMoFC_EffectKindInfo.h"
#include "Info/cltSkillKindInfo.h"
#include "Info/cltLessonKindInfo.h"
#include "Info/cltEmblemKindInfo.h"
#include "Info/cltSpecialtyKindInfo.h"
#include "Info/cltMakingItemKindInfo.h"
#include "Info/cltPetSkillKindInfo.h"
#include "Info/cltCoupleRingKindInfo.h"
#include "Info/cltTransformKindInfo.h"
#include "Info/cltEmoticonKindInfo.h"
#include "Info/cltPKRankKindInfo.h"
#include "Info/cltRegenMonsterKindInfo.h"
#include "Info/cltMapUseItemInfoKindInfo.h"
#include "Info/cltMapInfo.h"
#include "System/cltSkillSystem.h"
#include "System/cltEquipmentSystem.h"
#include "Character/ClientCharacter.h"
#include "Character/ClientCharacterManager.h"
#include "Character/CAManager.h"
#include "Character/CCAClone.h"
#include "System/cltQuestSystem.h"
#include "System/CMeritoriousSystem.h"
#include "System/CSpiritSystem.h"
#include "System/cltLevelSystem.h"
#include "System/cltClassSystem.h"
#include "System/cltPetSystem.h"
#include "System/cltMoneySystem.h"
#include "Util/cltTimer.h"
#include "Logic/Map.h"
#include "Logic/cltFieldItem.h"
#include "Logic/cltFieldItemManager.h"
#include "Logic/cltHelpMessage.h"
#include "Logic/cltBaseInventory.h"
#include "Logic/cltConfiguration.h"
#include "Logic/CShortKey.h"
#include "Logic/cltNPCManager.h"
#include "Logic/CObjectManager.h"
#include "Logic/cltMapTitle.h"
#include "Logic/cltMoF_MineManager.h"
#include "Logic/cltMatchManager.h"
#include "Logic/cltPKManager.h"
#include "Logic/cltPKFlagManager.h"
#include "Logic/CSpiritSpeechMgr.h"
#include "Logic/SpiritSpeechParser.h"
#include "Logic/SpiritGossipParser.h"
#include "Info/cltQuestKindInfo.h"
#include "Logic/clTransportKindInfo.h"
#include "Logic/clClientTransportKindInfo.h"
#include "Logic/cltMyCharData.h"
#include "Logic/cltSystemMessage.h"
#include "UI/CUITutorial.h"
#include "UI/CUIManager.h"
#include "Object/CObjectResourceManager.h"
#include "Object/DrawObject_All.h"
#include "Network/CMoFNetwork.h"
#include "Effect/CEffectManager.h"
#include "Image/CDeviceManager.h"
#include "Logic/cltCashShopItem.h"
#include "Logic/cltChattingMgr.h"
#include "UI/CControlAlphaBox.h"
#include "UI/CInterfaceDataCommunity.h"
#include "Other/DCTAbuseWordManager.h"
#include "System/cltClientPartySystem.h"
#include "Info/cltBasicAppearKindInfo.h"
#include "Sound/GameSound.h"
#include "System/cltTradeSystem.h"
#include "System/cltStorageSystem.h"
#include "System/cltExStorageSystem.h"
#include "System/cltLessonSystem.h"
#include "MiniGame/cltMoF_BaseMiniGame.h"
#include "MiniGame/cltMoF_MiniGame_Mgr.h"


CDeviceManager& g_clDeviceManager = *CDeviceManager::GetInstance();

LPDIRECT3D9 g_pD3D = NULL;
LPDIRECT3DDEVICE9 g_pd3dDevice = NULL;
D3DPRESENT_PARAMETERS g_d3dpp = {};
LPDIRECT3DDEVICE9 Device = NULL;
HWND g_hWnd = NULL;

int SETTING_FRAME = 1;
int IsInMemory = 1;
bool IsDialogBoxMode = false;
int g_LButtonUp = 1;

GameSystemInfo g_Game_System_Info = { 1280, 720 , 0, 0};

bool DontDraw = false;
unsigned char NationCode = 4;
bool g_bRenderStateLocked = false;

short kGlyphIndexByStyleChar[] = { 51 };
int   kDotGlyphIndexByStyle[] = { -1 };
int   kAdvanceByStyle[] = { 0 };
int   kAtlasIdByStyle[] = { 570425345 };

int dword_A73088 = 0;
int dword_A7308C = 0;

unsigned int D3DRS_BLENDOP_TYPE[8] = { 1u, 1u, 1u, 5u, 4u, 1u, 2u, 1u };
unsigned int D3DRS_SRCBLEND_TYPE[8] = { 5u, 10u, 9u, 2u, 2u, 2u, 2u, 9u };
unsigned int D3DRS_DESTBLEND_TYPE[8] = { 6u, 2u, 1u, 2u, 2u, 2u, 2u, 5u };

DWORD g_dwHeartBeatTime = 0;
int g_nFieldItemPickupFlag = 0;
unsigned int g_dwMyAccountID = 0;
int g_nTutorialState = 0;

char           byte_23158C8[288]  = {};
short          word_23158E8[144]  = {};
char           byte_23158EA[288]  = {};
short          word_23158EC[144]  = {};
char           byte_23158EE[288]  = {};
char           byte_23158EF[288]  = {};
int            dword_23158F0[72]  = {};
char           byte_23158F4[288]  = {};
unsigned short word_23158F8[144]  = {};
unsigned short word_231590E[144]  = {};

cltTextFileManager g_clTextFileManager;
DCTTextManager g_DCTTextManager;
MoFFont g_MoFFont;
DCTIMMList g_IMMList;
Map g_Map;
unsigned short dword_21B8DF4 = 0; // current map ID (ground truth: 0x21B8DF4)
CMoFNetwork g_Network;
cltClassKindInfo g_clClassKindInfo;
cltItemKindInfo g_clItemKindInfo;
cltNPCInfo g_clNPCInfo;
cltQuestKindInfo g_clQuestKindInfo;
CSpiritSystem g_clSpiritSystem;
cltQuestSystem g_clQuestSystem;
CMeritoriousSystem g_clMeritoriousSystem;
cltLevelSystem g_clLevelSystem;
cltClassSystem g_clClassSystem;
cltBaseInventory g_clMyInventory;
cltTimerManager g_clTimerManager;
cltFieldItemManager g_clFieldItemMgr;
cltHelpMessage g_clHelpMessage;
cltNPCManager g_clNPCManager;
ClientCharacterManager g_ClientCharMgr;
CAManager g_CAManager;
CCAClone g_clCAClone;
cltConfiguration* g_clConfig = nullptr;
CShortKey* g_pShortKeyList = nullptr;
CObjectManager g_ObjectManager;
CObjectResourceManager g_ObjectResourceManager;
DrawObject_All g_DrawObject_ALL;
CEffectManager g_EffectManager;
CUITutorial* g_pUITutorial = nullptr;
CUIManager* g_UIMgr = nullptr;

cltMapTitle g_clMapTitle;
cltMoF_MineManager g_clMineMgr;
cltMatchManager g_clMatchManager;
cltPKManager g_clPKManager;
cltPKFlagManager g_clPKFlagManager;
CSpiritSpeechMgr g_clSpiritSpeechMgr;
CSpiritSpeechParser g_clSpiritSpeechParser;
CSpiritGossipParser g_clSpiritGossipParser;
clTransportKindInfo g_clTransportKindInfo;
clClientTransportKindInfo g_clClientTransportKindInfo;
CEffectManager* g_pEffectManager_Before_Chr = nullptr;
CEffectManager* g_pEffectManager_After_Chr = nullptr;

cltPetKindInfo g_clPetKindInfoBase;
cltClientPetKindInfo g_clClientPetKindInfo;
cltPortalInfo g_clPortalInfo;
cltClientPortalInfo g_clClientPortalInfo;
cltPetSystem g_clPetSystem;
cltMoneySystem g_clMoneySystem;
cltMoFC_EffectKindInfo g_clEffectKindInfo;
// cltMyCharData g_clMyCharData is defined in Logic/cltMyCharData.cpp
// cltSystemMessage g_clSysemMessage is defined in Logic/cltSystemMessage.cpp

CMessageBoxManager* g_pMsgBoxMgr = nullptr;
cltChattingMgr g_clChattingMgr;

// Chat-related globals (mofclient.c)
CControlAlphaBox g_Block;
int              dword_6E17B0 = 0;
HDC              hdc = nullptr;
HGDIOBJ          h = nullptr;
unsigned int     g_dwMainGameState = 0;
DCTAbuseWordManager      g_DCTAbuseWordManager;
CInterfaceDataCommunity* g_pInterfaceDataCommunity = nullptr;
cltClientPartySystem     g_clPartySystem;

cltCashShopItem g_clCashShopItem;
cltBasicAppearKindInfo g_clBasicAppearKindInfo;

GameSound g_GameSoundManager;

// MiniGame
cltMoF_BaseMiniGame g_clMiniGame;
cltMoF_MiniGame_Mgr g_clMiniGameMgr;
CEffectManager      g_EffectManager_MiniGame;
cltLessonSystem     g_clLessonSystem;

// cltMini_Sword 使用的全域狀態
unsigned char g_cGameSwordState = 0;
unsigned char g_cGameSword_2State = 0;
unsigned char g_cGameExorcist_2State = 0;
unsigned char g_cGameExorcistState = 0;
unsigned char g_cGameBowState = 0;
unsigned char g_cGameBow_2State = 0;
unsigned char g_cGameMagicState = 0;
unsigned char g_cGameMagic_2State = 0;
unsigned char g_cCardState[3] = { 0, 0, 0 };
int           g_stGsGameFieldArg = 0;
unsigned int  dwFrameCnt = 0;
int           MonPosY[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int           g_GAMESCORE = 0;
// mofclient.c .data @ 0x6CDB80 — cltBow2_Spear::Create 讀取的 17 組
// (xOff, yOff) 生成偏移，近似分布在半徑 ~210 的四分之一弧上，
// 搭配 direction 1..4 投影到四個象限。IDA 反編譯把這張 float[17][2]
// 拆成 g_fPoint + dword_6CDB84 兩個 "weak" 符號，只保留第一個值，
// 其餘由直接讀 .data hex dump 還原。
float         g_fBowSpearTable[34] = {
    0.0f,   210.0f,  //  0
    26.0f,  208.0f,  //  1
    51.0f,  204.0f,  //  2
    74.0f,  197.0f,  //  3
    94.0f,  188.0f,  //  4
    111.0f, 178.0f,  //  5
    126.0f, 168.0f,  //  6
    138.0f, 158.0f,  //  7
    148.0f, 148.0f,  //  8
    158.0f, 138.0f,  //  9
    168.0f, 126.0f,  // 10
    178.0f, 111.0f,  // 11
    188.0f, 94.0f,   // 12
    197.0f, 74.0f,   // 13
    204.0f, 51.0f,   // 14
    208.0f, 26.0f,   // 15
    210.0f, 0.0f,    // 16
};
int dword_AFD34C = 0;
int dword_AFD344 = 0;
int cHeight = 0;

int dword_73D154 = 0;
int dword_B4BAB4 = 0;
char byte_21CB35D = 0;
int dword_829254 = 1;   // default: prefer packed .ca loads

// CCA / CCAClone per-slot fallback case maps (mofclient.c 0x525EB4 / 0x5280FC).
// Both are zero in the shipped binary (weak single-zero arrays), meaning every
// slot that drops into the recovery path re-resolves through the current hair
// layer (case 0).  Exposed as named externs so future tooling can toggle any
// slot without touching CCA internals — see global.h for the case legend.
unsigned char byte_525EB4[19] = {};
unsigned char byte_5280FC[19] = {};
cltFieldItem* unk_73D15C[1024] = {};
void* unk_813AA8[1024] = {};
void* unk_B4B924[1024] = {};
ClientCharacter unk_1409D80[1000] = {};
ClientCharacter* dword_1843F78[1000] = {};
size_t NumOfElements = 0;

// Info (additional)
cltSkillKindInfo g_clSkillKindInfo;
cltLessonKindInfo g_clLessonKindInfo;
cltEmblemKindInfo g_clEmblemKindInfo;
cltSpecialtyKindInfo g_clSpecialtyKindInfo;
cltMakingItemKindInfo g_clMakingItemKindInfo;
cltPetSkillKindInfo g_clPetSkillKindInfo;
cltCoupleRingKindInfo g_clCoupleRingKindInfo;
cltTransformKindInfo g_clTransformKindInfo;
cltEmoticonKindInfo g_clEmoticonKindInfo;
cltPKRankKindInfo g_clPKRankKindInfo;
cltCharKindInfo* g_pcltCharKindInfo = nullptr;
cltRegenMonsterKindInfo g_cltRegenMonsterKindInfo;
cltMapUseItemInfoKindInfo g_clMapUseItemInfoKindInfo;
cltMapInfo* g_pcltMapInfo = nullptr;

// System (additional)
cltEquipmentSystem* dword_21BA32C = nullptr;
cltSkillSystem g_clSkillSystem;
cltSpecialtySystem g_clSpecialtySystem;
cltUsingItemSystem g_clUsingItemSystem;
cltSexSystem g_clSexSystem;
cltPlayerAbility g_clPlayerAbility;
cltUsingSkillSystem g_clUsingSkillSystem;
cltMarriageSystem* g_clMarriageSystem = nullptr;
cltStorageSystem* dword_21BB2AC = nullptr;

// OutputCashShopTime 用的外部符號（對齊反編譯）
unsigned short dword_21C9C54[8] = {};
int dword_B3D72C[40] = {};

unsigned int ExGetTimeOutItemRemindTime(unsigned int a1, unsigned short a2)
{
    // TODO: 從 mofclient.c 還原完整實作
    return a1;
}

unsigned int ExGetMyAccount()
{
    return g_dwMyAccountID;
}

// CControlChatBallon 九宮格樣式表（對齊反編譯 0x6C6AD0）
// 每種樣式 11 個 uint16 block ID，順序：TL, TM, TR, ML, MM, MC, BL, BM, BR, Arrow, ArrowTail
// 注意：實際值需從二進制 dump 還原，此處為佔位
const uint16_t g_ChatBallonStyleTable[] = {
    // style 0 (預設白色氣球)
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
    // style 1
    11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
    // style 2
    22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
    // style 3
    33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43,
};
