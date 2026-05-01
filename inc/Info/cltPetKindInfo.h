#pragma once
#include <cstdint>
#include <cstdio>
#include <cstddef>
#include "global.h"
#include "Info/cltPetSkillKindInfo.h"

// ---------------------------------------------------------------------------
// strPetKindInfo / strPetDyeKindInfo / cltPetKindInfo
//
// Ground truth: mofclient.c (sub_00584C90 .. sub_00585B40)
//   - cltPetKindInfo::Initialize         @ 00584C90
//   - cltPetKindInfo::LoadPetKindInfo    @ 00584CA0
//   - cltPetKindInfo::LoadPetDyeKindInfo @ 005856C0
//   - cltPetKindInfo::Free               @ 005858A0
//   - cltPetKindInfo::GetNextPetKind     @ 005858E0
//   - cltPetKindInfo::GetPetKindInfo     @ 00585930
//   - cltPetKindInfo::GetOriginalPetKind @ 00585980
//   - cltPetKindInfo::GetPetDyeKindInfo  @ 005859A0
//   - cltPetKindInfo::GetPetTypeTextCode @ 005859E0
//   - cltPetKindInfo::TranslateKindCode  @ 00585A30
//   - cltPetKindInfo::GetPetLevel        @ 00585A80
//   - cltPetKindInfo::GetPetReleaseCost  @ 00585AC0
//   - cltPetKindInfo::IsSamePet          @ 00585AF0
//
// 寵物資料：每筆 strPetKindInfo 為 172 bytes（從反編譯 sizeof 計算 = 172）。
// 染色資料：每筆 strPetDyeKindInfo 為 8 bytes。
// ---------------------------------------------------------------------------

#pragma pack(push, 1)

// 對應 .txt 標頭欄位順序（petkindinfo.txt）：
//   pet_ID / 펫이름(기획자용)/skip / 펫 이름(text code) / 펫 설명(text code) /
//   전직시 이전 펫 ID / 애니메이션 정보 파일(gi) / 염색 수 / [염색 코드 …] /
//   캐릭터 Width / 캐릭터 Height(POS_Y좌표) / 도트 리소스 ID / 블록 ID /
//   펫UI 알 리소스 / 블록ID / 기본 패시브 스킬 / 펫 분류 / 레벨(기획자용)/skip /
//   LOVE (경험치) / 포만감 / 포만감 하락치(1분당) / 펫 단계별 기준 y좌표 /
//   펫 종류(text code) / 펫 베이직 스킬 이름(text code) / 스킬획득여부 /
//   기본지급가방수 / 최대확장가방수 / 펫이름 변경 가능 여부 /
//   캐릭터 앞(0)과 뒤(1) / 팻 위치 조정(Y값) / 펫방사가격(리비) /
//   방사시스킬1개단가(리비) / 펫 정지 리소스 / 펫 정지 블록ID /
//   펫 이펙트 / 펫 요구레벨 (optional)
struct strPetKindInfo
{
    // pet_ID (펫 ID / 寵物 ID) — 由 cltPetKindInfo::TranslateKindCode 解析的 5 字 kind code
    uint16_t wPetId;                         // KR: 펫 ID                       CN: 寵物 ID                offset: 0    size: 2
    // 펫 이름 text code (펫 이름의 텍스트 코드) — 對應 MoFTexts.txt
    uint16_t wPetNameTextCode;               // KR: 펫 이름(text code)          CN: 寵物名稱文字代碼       offset: 2    size: 2
    // 펫 설명 text code — 對應 MoFTexts.txt
    uint16_t wPetDescriptionTextCode;        // KR: 펫 설명(text code)          CN: 寵物說明文字代碼       offset: 4    size: 2
    // 전직시 이전 펫 ID (轉職前的寵物 ID) — 用於演化鏈
    uint16_t wPreviousPetIdOnJobChange;      // KR: 전직시 이전 펫 ID           CN: 轉職前一階寵物 ID      offset: 6    size: 2
    // 애니메이션 정보 파일(gi) — 寵物動畫檔名 (cltPetAniInfo::Initialize 載入)
    char     AnimationInfoFileGi[32];        // KR: 애니메이션 정보 파일        CN: 動畫資料檔名 (gi)      offset: 8    size: 32
    // 염색 수 (染色 slot 數量, 0..20)
    uint16_t wDyeCount;                      // KR: 염색 수                     CN: 染色數量               offset: 40   size: 2
    // 염색 kind code 목록 (最大 20 個 dye code)
    uint16_t awDyeCodes[20];                 // KR: 염색 kind code 목록         CN: 染色代碼列表(最多20)   offset: 42   size: 40
    // 캐릭터 Width (寵物角色寬度 px)
    uint16_t wCharacterWidth;                // KR: 캐릭터 Width                CN: 角色寬度               offset: 82   size: 2
    // 캐릭터 Height (POS_Y 座標基準)
    uint16_t wCharacterHeightPosY;           // KR: 캐릭터 Height(POS_Y)        CN: 角色高度(POS_Y基準)    offset: 84   size: 2
    // padding 對齊用 (反編譯 dwDotResourceId 在 +88，+86~+87 為對齊空隙)
    uint16_t wReserved86;                    // KR: padding                     CN: 對齊填充               offset: 86   size: 2
    // 도트 리소스 ID — 以 sscanf "%x" 讀入的 32-bit hex
    uint32_t dwDotResourceId;                // KR: 도트 리소스 ID              CN: 點陣資源 ID (hex)      offset: 88   size: 4
    // 블록 ID
    uint16_t wBlockId;                       // KR: 블록 ID                     CN: 區塊 ID                offset: 92   size: 2
    // padding (+94~+95 對齊空隙)
    uint16_t wReserved94;                    // KR: padding                     CN: 對齊填充               offset: 94   size: 2
    // 펫 UI 알림 리소스 — 以 sscanf "%x" 讀入的 32-bit hex
    uint32_t dwPetUiAlertResource;           // KR: 펫UI 알 리소스              CN: 寵物 UI 警示資源(hex)  offset: 96   size: 4
    // 블록ID — 第二個 block id (txt 內為獨立欄位)
    uint16_t wBlockIdDuplicate;              // KR: 블록ID                      CN: 區塊 ID(重複欄位)      offset: 100  size: 2
    // 기본 패시브 스킬 — cltPetSkillKindInfo::TranslateKindCode 解析 (6 字)
    uint16_t wBasicPassiveSkill;             // KR: 기본 패시브 스킬            CN: 基本被動技能           offset: 102  size: 2
    // 펫 단계 (由 GetPetLevel 計算, 不在 .txt 中) — parser 末段透過 GetPetLevel 設值
    uint16_t wPetLevel;                      // KR: 펫 단계 (계산값)            CN: 寵物階段(計算)         offset: 104  size: 2
    // padding (+106~+107)
    uint16_t wReserved106;                   // KR: padding                     CN: 對齊填充               offset: 106  size: 2
    // LOVE (經驗值)
    uint32_t dwLoveExperience;               // KR: LOVE (경험치)               CN: 親密度經驗             offset: 108  size: 4
    // 포만감 (飽食度上限)
    uint16_t wSaturation;                    // KR: 포만감                      CN: 飽食度                 offset: 112  size: 2
    // padding (+114~+115)
    uint16_t wReserved114;                   // KR: padding                     CN: 對齊填充               offset: 114  size: 2
    // 포만감 하락치(1分當) — 每分鐘下降量
    uint32_t dwSaturationDecreasePerMinute;  // KR: 포만감 하락치(1분당)        CN: 飽食度每分鐘下降       offset: 116  size: 4
    // 펫 단계별 기준 y좌표 — 寵物階段的 Y 基準
    uint16_t wPetStageBaseYCoordinate;       // KR: 펫 단계별 기준 y좌표        CN: 寵物階段Y基準座標      offset: 120  size: 2
    // 펫 종류 text code (與 GetPetTypeTextCode 對應 bPetCategory)
    uint16_t wPetTypeTextCode;               // KR: 펫 종류(text code)          CN: 寵物種類文字代碼       offset: 122  size: 2
    // 펫 베이직 스킬 이름 text code
    uint16_t wPetBasicSkillNameTextCode;     // KR: 펫 베이직 스킬 이름(text)   CN: 寵物基本技能名稱代碼   offset: 124  size: 2
    // padding (+126~+127)
    uint16_t wReserved126;                   // KR: padding                     CN: 對齊填充               offset: 126  size: 2
    // 스킬획득여부
    uint32_t dwSkillAcquisitionStatus;       // KR: 스킬획득여부                CN: 技能獲取狀態           offset: 128  size: 4
    // 기본지급가방수 — 預設給予的背包數
    uint8_t  bDefaultBagCount;               // KR: 기본지급가방수              CN: 預設配發背包數         offset: 132  size: 1
    // 최대확장가방수 — 最大可擴充的背包數
    uint8_t  bMaxExtendedBagCount;           // KR: 최대확장가방수              CN: 最大可擴充背包數       offset: 133  size: 1
    // padding (+134~+135)
    uint16_t wReserved134;                   // KR: padding                     CN: 對齊填充               offset: 134  size: 2
    // 펫이름 變更 가능 여부
    uint32_t dwPetNameChangePossible;        // KR: 펫이름 변경 가능 여부       CN: 寵物名稱可否變更       offset: 136  size: 4
    // 캐릭터 앞(0)과 뒤(1) — 0=前 / 1=後
    uint8_t  bCharacterFrontBack;            // KR: 캐릭터 앞(0)/뒤(1)          CN: 角色前(0)/後(1)        offset: 140  size: 1
    // padding 3 bytes (+141~+143)
    uint8_t  bReserved141[3];                // KR: padding                     CN: 對齊填充(3 bytes)      offset: 141  size: 3
    // 팻 위치 조정(Y값)
    uint32_t dwPetPositionAdjustmentY;       // KR: 팻 위치 조정(Y값)           CN: 寵物位置 Y 調整        offset: 144  size: 4
    // 펫 정지 리소스 — 以 sscanf "%x" 讀入的 32-bit hex
    uint32_t dwPetStopResource;              // KR: 펫 정지 리소스              CN: 寵物停止資源(hex)      offset: 148  size: 4
    // 펫 정지 블록ID
    uint16_t wPetStopBlockId;                // KR: 펫 정지 블록ID              CN: 寵物停止區塊 ID        offset: 152  size: 2
    // padding (+154~+155)
    uint16_t wReserved154;                   // KR: padding                     CN: 對齊填充               offset: 154  size: 2
    // 펫방사가격(리비) — 釋放/解除契約所需 Libi (貨幣)
    uint32_t dwPetRadiationPriceLib;         // KR: 펫방사가격(리비)            CN: 寵物釋放價格(Libi)     offset: 156  size: 4
    // 방사시스킬1개단가(리비) — 釋放時每個技能單價
    uint32_t dwRadiationSkillUnitPriceLib;   // KR: 방사시 스킬 1개 단가(리비)  CN: 釋放時每技能單價       offset: 160  size: 4
    // 펫 분류 — BASICPET=0, DRAGON=1, TINKERBELL=2, PENGUIN=3, BEARDOLL=4, DEVIL=5, BISQUEDOLL=6, MECHANIC=7
    uint8_t  bPetCategory;                   // KR: 펫 분류                     CN: 寵物分類               offset: 164  size: 1
    // padding (+165)
    uint8_t  bReserved165;                   // KR: padding                     CN: 對齊填充               offset: 165  size: 1
    // 펫 이펙트 — TranslateKindCode 解析的 kind code
    uint16_t wPetEffect;                     // KR: 펫 이펙트                   CN: 寵物特效               offset: 166  size: 2
    // 펫 요구레벨 — optional 欄位 (若缺，會跳出 format 警告)
    uint16_t wPetRequiredLevel;              // KR: 펫 요구레벨                 CN: 寵物需求等級           offset: 168  size: 2
    // padding (+170~+171) — 確保總大小為 172
    uint16_t wReserved170;                   // KR: padding                     CN: 結尾對齊填充           offset: 170  size: 2
};

// 對應 PetDyeKindInfo.txt 欄位順序：
//   염색 코드 / 컬러(기획자용)/skip / 염색 인덱스
struct strPetDyeKindInfo
{
    // 염색 코드 — TranslateKindCode 解析的 5 字 kind code (D0001 等)
    uint16_t wDyeCode;                       // KR: 염색 코드                   CN: 染色代碼               offset: 0    size: 2
    // padding — 컬러(기획자용) 文字欄不存入記憶體；對齊空隙
    uint16_t wReserved2;                     // KR: padding                     CN: 對齊填充               offset: 2    size: 2
    // 염색 인덱스 — atoi(IsDigit 通過)
    uint32_t dwDyeIndex;                     // KR: 염색 인덱스                 CN: 染色索引               offset: 4    size: 4
};

#pragma pack(pop)

static_assert(sizeof(strPetKindInfo) == 172, "strPetKindInfo size must be 172 bytes");
static_assert(offsetof(strPetKindInfo, wPetId) == 0, "wPetId offset mismatch");
static_assert(offsetof(strPetKindInfo, wPetNameTextCode) == 2, "wPetNameTextCode offset mismatch");
static_assert(offsetof(strPetKindInfo, wPetDescriptionTextCode) == 4, "wPetDescriptionTextCode offset mismatch");
static_assert(offsetof(strPetKindInfo, wPreviousPetIdOnJobChange) == 6, "wPreviousPetIdOnJobChange offset mismatch");
static_assert(offsetof(strPetKindInfo, AnimationInfoFileGi) == 8, "AnimationInfoFileGi offset mismatch");
static_assert(offsetof(strPetKindInfo, wDyeCount) == 40, "wDyeCount offset mismatch");
static_assert(offsetof(strPetKindInfo, awDyeCodes) == 42, "awDyeCodes offset mismatch");
static_assert(offsetof(strPetKindInfo, wCharacterWidth) == 82, "wCharacterWidth offset mismatch");
static_assert(offsetof(strPetKindInfo, wCharacterHeightPosY) == 84, "wCharacterHeightPosY offset mismatch");
static_assert(offsetof(strPetKindInfo, dwDotResourceId) == 88, "dwDotResourceId offset mismatch");
static_assert(offsetof(strPetKindInfo, wBlockId) == 92, "wBlockId offset mismatch");
static_assert(offsetof(strPetKindInfo, dwPetUiAlertResource) == 96, "dwPetUiAlertResource offset mismatch");
static_assert(offsetof(strPetKindInfo, wBlockIdDuplicate) == 100, "wBlockIdDuplicate offset mismatch");
static_assert(offsetof(strPetKindInfo, wBasicPassiveSkill) == 102, "wBasicPassiveSkill offset mismatch");
static_assert(offsetof(strPetKindInfo, wPetLevel) == 104, "wPetLevel offset mismatch");
static_assert(offsetof(strPetKindInfo, dwLoveExperience) == 108, "dwLoveExperience offset mismatch");
static_assert(offsetof(strPetKindInfo, wSaturation) == 112, "wSaturation offset mismatch");
static_assert(offsetof(strPetKindInfo, dwSaturationDecreasePerMinute) == 116, "dwSaturationDecreasePerMinute offset mismatch");
static_assert(offsetof(strPetKindInfo, wPetStageBaseYCoordinate) == 120, "wPetStageBaseYCoordinate offset mismatch");
static_assert(offsetof(strPetKindInfo, wPetTypeTextCode) == 122, "wPetTypeTextCode offset mismatch");
static_assert(offsetof(strPetKindInfo, wPetBasicSkillNameTextCode) == 124, "wPetBasicSkillNameTextCode offset mismatch");
static_assert(offsetof(strPetKindInfo, dwSkillAcquisitionStatus) == 128, "dwSkillAcquisitionStatus offset mismatch");
static_assert(offsetof(strPetKindInfo, bDefaultBagCount) == 132, "bDefaultBagCount offset mismatch");
static_assert(offsetof(strPetKindInfo, bMaxExtendedBagCount) == 133, "bMaxExtendedBagCount offset mismatch");
static_assert(offsetof(strPetKindInfo, dwPetNameChangePossible) == 136, "dwPetNameChangePossible offset mismatch");
static_assert(offsetof(strPetKindInfo, bCharacterFrontBack) == 140, "bCharacterFrontBack offset mismatch");
static_assert(offsetof(strPetKindInfo, dwPetPositionAdjustmentY) == 144, "dwPetPositionAdjustmentY offset mismatch");
static_assert(offsetof(strPetKindInfo, dwPetStopResource) == 148, "dwPetStopResource offset mismatch");
static_assert(offsetof(strPetKindInfo, wPetStopBlockId) == 152, "wPetStopBlockId offset mismatch");
static_assert(offsetof(strPetKindInfo, dwPetRadiationPriceLib) == 156, "dwPetRadiationPriceLib offset mismatch");
static_assert(offsetof(strPetKindInfo, dwRadiationSkillUnitPriceLib) == 160, "dwRadiationSkillUnitPriceLib offset mismatch");
static_assert(offsetof(strPetKindInfo, bPetCategory) == 164, "bPetCategory offset mismatch");
static_assert(offsetof(strPetKindInfo, wPetEffect) == 166, "wPetEffect offset mismatch");
static_assert(offsetof(strPetKindInfo, wPetRequiredLevel) == 168, "wPetRequiredLevel offset mismatch");

static_assert(sizeof(strPetDyeKindInfo) == 8, "strPetDyeKindInfo size must be 8 bytes");
static_assert(offsetof(strPetDyeKindInfo, wDyeCode) == 0, "wDyeCode offset mismatch");
static_assert(offsetof(strPetDyeKindInfo, dwDyeIndex) == 4, "dwDyeIndex offset mismatch");

// ---------------------------------------------------------------------------
// cltPetKindInfo — 4 個 32-bit 成員，總計 16 bytes (32-bit build)。
// 64-bit build 因指標寬度 8 bytes，記憶體佈局會擴張，但成員意義保持一致：
//   m_pPetKindInfo            (this+0  / 32-bit)
//   m_nPetKindInfoCount       (this+4  / 32-bit)
//   m_pPetDyeKindInfo         (this+8  / 32-bit)
//   m_nPetDyeKindInfoCount    (this+12 / 32-bit)
// ---------------------------------------------------------------------------
class cltPetKindInfo
{
public:
    cltPetKindInfo();
    ~cltPetKindInfo();

    int Initialize(char* petKindPath, char* petDyeKindPath);
    int LoadPetKindInfo(char* filePath);
    int LoadPetDyeKindInfo(char* filePath);
    void Free();

    uint16_t GetNextPetKind(uint16_t previousPetId) const;
    strPetKindInfo* GetPetKindInfo(uint16_t petId) const;
    uint16_t GetOriginalPetKind(uint16_t petId) const;
    strPetDyeKindInfo* GetPetDyeKindInfo(uint16_t dyeCode) const;
    uint16_t GetPetTypeTextCode(uint8_t petCategory) const;
    static uint16_t TranslateKindCode(const char* text);
    int16_t GetPetLevel(uint16_t petId) const;
    uint32_t GetPetReleaseCost(uint16_t petId, uint16_t skillCount) const;
    int IsSamePet(uint16_t petIdA, uint16_t petIdB) const;

private:
    strPetKindInfo*    m_pPetKindInfo;        // KR: 펫 종류 정보 배열         CN: 寵物種類資料陣列       offset: 0  (32-bit)
    int                m_nPetKindInfoCount;   // KR: 펫 종류 정보 개수         CN: 寵物種類資料筆數       offset: 4  (32-bit)
    strPetDyeKindInfo* m_pPetDyeKindInfo;     // KR: 펫 염색 정보 배열         CN: 寵物染色資料陣列       offset: 8  (32-bit)
    int                m_nPetDyeKindInfoCount;// KR: 펫 염색 정보 개수         CN: 寵物染色資料筆數       offset: 12 (32-bit)
};
