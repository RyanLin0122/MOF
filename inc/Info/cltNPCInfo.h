#pragma once
#include <cstdint>
#include <windows.h>

class DCTTextManager;

// ============================================================================
// stNPCInfo  ─  NPC 정보 (NPC 資訊)
// ----------------------------------------------------------------------------
//   GT: mofclient.c:317034 cltNPCInfo::Initialize 直接以 byte offset 寫入
//   sizeof(stNPCInfo) == 500 bytes (operator new(500 * count))
//   GetNPCInfoByIndex/ByID 以「+500 * idx」 步進、「i += 250 (WORD)」 走訪
//   GetNPCInfoByID  從 entry+0(=NPCKindCode) 比對
//   GetNPCInfoByBookID 從 entry+2(=BookID/分類) 比對
//   GetNPCInfoByShopID 從 entry+36(=ShopID) 比對
//   GetNPCInfoByNpcName 從 entry+4(=NameTextID) 透過 DCTTextManager 比對
//   GetMapID = entry+12 (WORD)；GetShopID = entry+36 (WORD)
//   GetQuestID(npcKind, idx) = entry+200+idx*2 (WORD)
// ============================================================================
#pragma pack(push, 1)
struct stNPCInfo
{
    // ─── 헤더 (Header, 0..43) ──────────────────────────────────────────
    std::uint16_t  m_wKindCode;         // offset   0  NPC ID         (NPC 종류 코드, TranslateKindCode 결과)
    std::uint16_t  m_wBookCode;         // offset   2  NPC 분류        (TranslateKindCode 결과; 도감 ID)
    std::uint16_t  m_wNameTextID;       // offset   4  NPC 이름        (text id; DCTTextManager 사용)
    std::uint16_t  m_wDescTextID;       // offset   6  NPC 분류 텍스트 (text id)
    std::uint16_t  m_wTitleTextID;      // offset   8  NPC 분류 (보조)  (text id)
    std::uint16_t  m_wLocationTextID;   // offset  10  NPC 위치 정보   (text id)
    std::uint16_t  m_wMapID;            // offset  12  맵 아이디        (TranslateKindCode 결과)
    std::uint16_t  _pad14;              // offset  14  (padding/alignment)
    std::uint16_t  _pad16;              // offset  16  (padding/alignment)
    std::uint16_t  _pad18;              // offset  18  (padding/alignment)
    std::uint32_t  m_dwX;               // offset  20  x 座標
    std::uint32_t  m_dwY;               // offset  24  y 座標
    std::uint32_t  m_dwAreaWidth;       // offset  28  Area_Width
    std::uint32_t  m_dwAreaHeight;      // offset  32  Area_Height
    std::uint16_t  m_wShopID;           // offset  36  상점 id          (TranslateKindCode 결과)
    std::uint16_t  m_wGreetingMsgID;    // offset  38  인사말 메시지 ID
    std::uint16_t  m_wChatBalloon[3];   // offset  40,42,44  말풍선1/2/3 (text id)
    char           m_szFace[3][16];     // offset  46,62,78  페이스_1/2/3 (16-byte 字串)
    std::uint8_t   m_byNPCType[54];     // offset  94  NPC 타입 플래그 배열 (0..53; cltNPCInfo::GetNPCType 設定)
                                        //                (參考 ENPCType；範圍 1..52 為已定義常數)
    std::uint32_t  m_dwResourceID;      // offset 148  ResourceID                    (hex)
    std::uint16_t  m_wPortalIcon;       // offset 152  포탈 아이콘                   (digit)
    std::uint16_t  _pad154;             // offset 154  (padding for DWORD alignment)
    std::uint32_t  m_dwIllustResID;     // offset 156  일러스트 ResourceID            (hex)
    std::uint32_t  m_dwMarryIllustResID;// offset 160  결혼 일러스트 리소스           (hex)
    std::uint16_t  m_wMarryIllustFileID;// offset 164  결혼 일러스트 파일아이디        (digit)
    std::uint16_t  _pad166;             // offset 166  (padding for DWORD alignment)
    std::uint32_t  m_dwIsLeftFacing;    // offset 168  좌우 방향 (L=1, R=0)
    std::uint32_t  m_dwIsCutIn;         // offset 172  npc 컷 (T=1, 그 외=0)
    std::uint16_t  m_wMsgFreeMove;      // offset 176  마음대로 메시지
    std::uint16_t  m_wMsgBlocked;       // offset 178  막혔어요 메시지_1
    std::uint16_t  m_wMsgCantGo;        // offset 180  가지 갈수 없어 메시지
    std::uint16_t  m_wMsgCanGo;         // offset 182  갈수 있어 메시지
    std::uint16_t  m_wMsgCantGo2;       // offset 184  갈수 없네 메시지
    std::uint16_t  _pad186;             // offset 186  (padding for DWORD alignment)
    std::uint32_t  m_dwSummonTime;      // offset 188  소환 시점
    std::uint32_t  m_dwIsTestField;     // offset 192  테스트 필드 NPC
    std::uint32_t  m_dwNoRecall;        // offset 196  노소환 (NPC 소환 불가 플래그)
    std::uint16_t  m_wQuestIDs[70];     // offset 200..339  퀘스트01..70 (TranslateKindCode 결과; 70 WORD)
    char           m_szExFace[10][16];  // offset 340..499  확장_페이스1..10 (16-byte 字串 × 10)
};
#pragma pack(pop)
static_assert(sizeof(stNPCInfo) == 500, "stNPCInfo must be 500 bytes (GT layout)");

// ─── NPC 타입 ENUM (cltNPCInfo::GetNPCType 設定; m_byNPCType 索引) ──────────
enum ENPCType : std::uint8_t {
    NPCTYPE_QUEST                  = 1,
    NPCTYPE_JOBCHANGE              = 2,
    NPCTYPE_SWORD                  = 3,
    NPCTYPE_BOW                    = 4,
    NPCTYPE_MAGIC                  = 5,
    NPCTYPE_THEOLOGY               = 6,
    NPCTYPE_HELP                   = 7,
    NPCTYPE_LESSONSET              = 8,
    NPCTYPE_SALESAGENCY            = 9,
    NPCTYPE_STORAGE                = 10,
    NPCTYPE_TELEDRAGON             = 11,
    NPCTYPE_STORY                  = 12,
    NPCTYPE_TEST                   = 13,
    NPCTYPE_SPECIALTY              = 14,
    NPCTYPE_CIRCLEQUEST            = 15,
    NPCTYPE_BOARD                  = 16,
    NPCTYPE_WISHBOARD              = 17,
    NPCTYPE_SHIP                   = 18,
    NPCTYPE_ENCHANT                = 19,
    NPCTYPE_QUIZ                   = 20,
    NPCTYPE_ETC                    = 21,
    NPCTYPE_WAR                    = 22,
    NPCTYPE_SUPPLY                 = 23,
    NPCTYPE_EMBLEM                 = 25,
    NPCTYPE_CIRCLEINDUN            = 26,
    NPCTYPE_PETSHOP                = 27,
    NPCTYPE_PETNURSERY             = 28,
    NPCTYPE_PETSELL                = 29,
    NPCTYPE_PETSELLINFO            = 30,
    NPCTYPE_PETSEARCH              = 31,
    NPCTYPE_EXPANDCIRCLEMEMBERS    = 32,
    NPCTYPE_DIVORCE                = 33,
    NPCTYPE_MARRY_PETITION         = 34,
    NPCTYPE_WEDDINGBOARD           = 35,
    NPCTYPE_TELEBLUESTAR           = 36,
    NPCTYPE_TELELIBI               = 37,
    NPCTYPE_TELESKYCASTLE          = 38,
    NPCTYPE_CHEER                  = 39,
    NPCTYPE_SHOPWEAPON             = 40,
    NPCTYPE_SHOPDEFENSE            = 41,
    NPCTYPE_SHOPJEWEL              = 42,
    NPCTYPE_SHOPINDEMNITY          = 43,
    NPCTYPE_SHOPONETIME            = 44,
    NPCTYPE_SHOPINCHANT_STONE      = 45,
    NPCTYPE_SHOPMAKING_ONETIME     = 46,
    NPCTYPE_SHOPETC                = 47,
    NPCTYPE_PVPITEM                = 48,
    NPCTYPE_MARRY_SHOP             = 49,
    NPCTYPE_TEST_INSTANT           = 50,
    NPCTYPE_INDUN                  = 51,
    NPCTYPE_SHOP_COIN              = 52,
};

// ============================================================================
// cltNPCInfo  ─  NPC 정보 매니저
//   GT: mofclient.c:317017 ... 317815
//   *(_DWORD *)this        = stNPCInfo* m_pInfo            (offset 0)
//   *((_WORD  *)this + 2)  = uint16_t   m_wTotalNPCNum     (offset 4)
// ============================================================================
class cltNPCInfo
{
public:
    cltNPCInfo();
    ~cltNPCInfo() = default;

    // mofclient.c:317017
    static void InitializeStaticVariable(DCTTextManager* a1);

    // mofclient.c:317034  parse npclist.txt
    int  Initialize(char* String2);

    // mofclient.c:317510
    void Free();

    // mofclient.c:317492  TranslateKindCode("X1234") -> ((toupper(X)+31)<<11) | atoi("1234")
    static std::uint16_t TranslateKindCode(const char* a1);

    // mofclient.c:317521
    std::uint16_t  GetTotalNPCNum() const;

    // mofclient.c:317527
    std::uint16_t  GetShopID(std::uint16_t npcKind);

    // mofclient.c:317533
    stNPCInfo*     GetNPCInfoByIndex(int index);

    // mofclient.c:317545
    stNPCInfo*     GetNPCInfoByID(std::uint16_t npcKind);

    // mofclient.c:317566
    stNPCInfo*     GetNPCInfoByBookID(std::uint16_t bookID);

    // mofclient.c:317587
    std::uint16_t  GetNPCBookID(std::uint16_t npcKind);

    // mofclient.c:317608  same map id as a3-th entry?
    BOOL           GetNPCInSameMapID(std::uint16_t mapID, int index);

    // mofclient.c:317620  count NPCs in map
    int            GetNPCInSameMapID(std::uint16_t mapID);

    // mofclient.c:317644
    std::uint16_t  GetQuestID(std::uint16_t npcKind, std::uint8_t idx);

    // mofclient.c:317650
    std::uint16_t  GetMapID(std::uint16_t npcKind);

    // mofclient.c:317656
    std::uint16_t  GetNPCIDByQuestID(std::uint16_t questID, int* outIndex);

    // mofclient.c:317695
    std::uint16_t  GetNPCNameByQuestID(std::uint16_t questID);

    // mofclient.c:317731
    stNPCInfo*     GetNPCInfoByShopID(std::uint16_t shopID);

    // mofclient.c:317752
    stNPCInfo*     GetNPCInfoByNpcName(char* npcName, std::uint16_t mapID);

    // mofclient.c:317815  parse "QUEST|SHOPETC|..." string -> sets info->m_byNPCType[*]
    void           GetNPCType(char* String1, std::uint8_t* a3);

    // GT 對外取直接資料指標 (cltClient_NPC 直接以 *(_DWORD *)this 取出)
    stNPCInfo*     GetInfoBuffer() const { return m_pInfo; }

    // GT 內部 / 全域使用
    static DCTTextManager* m_pclTextManager;

private:
    // GT 與 mofclient.c:342909 IsDigit / 342945 IsAlphaNumeric 等價 (空字串 = true; 允許前導 +/-)
    static bool IsDigit(const char* a1);
    static bool IsAlphaNumeric(const char* a1);

    // ─── 成員佈局 (與 GT 完全一致) ───────────────────────────────────
    stNPCInfo*     m_pInfo;             // offset 0  (_DWORD)
    std::uint16_t  m_wTotalNPCNum;      // offset 4  (_WORD index 2)
};
static_assert(sizeof(cltNPCInfo) >= 8, "cltNPCInfo basic layout check");
