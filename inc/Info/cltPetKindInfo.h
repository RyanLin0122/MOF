#pragma once
#include <cstdint>
#include <cstdio>
#include <cstddef>
#include "global.h"
#include "Info/cltPetSkillKindInfo.h"

#pragma pack(push, 1)

struct strPetKindInfo
{
    uint16_t wPetId;                         // 0: 펫 ID
    uint16_t wPetNameTextCode;               // 2: 펫 이름(text code)
    uint16_t wPetDescriptionTextCode;        // 4: 펫 설명(text code)
    uint16_t wPreviousPetIdOnJobChange;      // 6: 전직시 이전 펫 ID
    char     AnimationInfoFileGi[32];        // 8: 애니메이션 정보 파일(gi)
    uint16_t wDyeCount;                      // 40: 염색 수
    uint16_t awDyeCodes[20];                 // 42: 염색 kind code 목록 (최대 20개 슬롯)
    uint16_t wCharacterWidth;                // 82: 캐릭터 Width
    uint16_t wCharacterHeightPosY;           // 84: 캐릭터 Height(POS_Y)
    uint16_t wReserved86;                    // 86: 미사용/정렬용
    uint32_t dwDotResourceId;                // 88: 도트 리소스 ID
    uint16_t wBlockId;                       // 92: 블록 ID
    uint16_t wReserved94;                    // 94: 미사용/정렬용
    uint32_t dwPetUiAlertResource;           // 96: 펫 UI 알림 리소스
    uint16_t wBlockIdDuplicate;              // 100: 블록 ID (중복 저장)
    uint16_t wBasicPassiveSkill;             // 102: 기본 패시브 스킬
    uint16_t wPetLevel;                      // 104: GetPetLevel()로 계산된 단계
    uint16_t wReserved106;                   // 106: 미사용/정렬용
    uint32_t dwLoveExperience;               // 108: LOVE(경험치)
    uint16_t wSaturation;                    // 112: 포만감
    uint16_t wReserved114;                   // 114: 미사용/정렬용
    uint32_t dwSaturationDecreasePerMinute;  // 116: 포만감 하락치(1분당)
    uint16_t wPetStageBaseYCoordinate;       // 120: 펫 단계별 기준 y좌표
    uint16_t wPetTypeTextCode;               // 122: 펫 종류(text code)
    uint16_t wPetBasicSkillNameTextCode;     // 124: 펫 베이직 스킬 이름(text code)
    uint16_t wReserved126;                   // 126: 미사용/정렬용
    uint32_t dwSkillAcquisitionStatus;       // 128: 스킬획득여부
    uint8_t  bDefaultBagCount;               // 132: 기본지급가방수
    uint8_t  bMaxExtendedBagCount;           // 133: 최대확장가방수
    uint16_t wReserved134;                   // 134: 미사용/정렬용
    uint32_t dwPetNameChangePossible;        // 136: 펫이름 변경 가능 여부
    uint8_t  bCharacterFrontBack;            // 140: 캐릭터 앞(0)/뒤(1)
    uint8_t  bReserved141[3];                // 141: 미사용/정렬용
    uint32_t dwPetPositionAdjustmentY;       // 144: 펫 위치 조정(Y)
    uint32_t dwPetStopResource;              // 148: 펫 정지 리소스
    uint16_t wPetStopBlockId;                // 152: 펫 정지 블록 ID
    uint16_t wReserved154;                   // 154: 미사용/정렬용
    uint32_t dwPetRadiationPriceLib;         // 156: 펫 방사 가격(리비)
    uint32_t dwRadiationSkillUnitPriceLib;   // 160: 방사시 스킬 1개 단가(리비)
    uint8_t  bPetCategory;                   // 164: 펫 분류
    uint8_t  bReserved165;                   // 165: 미사용/정렬용
    uint16_t wPetEffect;                     // 166: 펫 이펙트
    uint16_t wPetRequiredLevel;              // 168: 펫 요구레벨
    uint16_t wReserved170;                   // 170: 구조체 총 크기 172 보장
};

struct strPetDyeKindInfo
{
    uint16_t wDyeCode;                       // 0: 染色代碼
    uint16_t wReserved2;                     // 2: 파일의 색상표시용欄位는 메모리에 저장하지 않음
    uint32_t dwDyeIndex;                     // 4: 染色索引
};

#pragma pack(pop)

static_assert(sizeof(strPetKindInfo) == 172, "strPetKindInfo size must be 172 bytes");
static_assert(offsetof(strPetKindInfo, AnimationInfoFileGi) == 8, "AnimationInfoFileGi offset mismatch");
static_assert(offsetof(strPetKindInfo, wDyeCount) == 40, "wDyeCount offset mismatch");
static_assert(offsetof(strPetKindInfo, awDyeCodes) == 42, "awDyeCodes offset mismatch");
static_assert(offsetof(strPetKindInfo, dwDotResourceId) == 88, "dwDotResourceId offset mismatch");
static_assert(offsetof(strPetKindInfo, wBlockId) == 92, "wBlockId offset mismatch");
static_assert(offsetof(strPetKindInfo, dwPetUiAlertResource) == 96, "dwPetUiAlertResource offset mismatch");
static_assert(offsetof(strPetKindInfo, wPetLevel) == 104, "wPetLevel offset mismatch");
static_assert(offsetof(strPetKindInfo, bPetCategory) == 164, "bPetCategory offset mismatch");
static_assert(offsetof(strPetKindInfo, wPetEffect) == 166, "wPetEffect offset mismatch");
static_assert(offsetof(strPetKindInfo, wPetRequiredLevel) == 168, "wPetRequiredLevel offset mismatch");
static_assert(sizeof(strPetDyeKindInfo) == 8, "strPetDyeKindInfo size must be 8 bytes");

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
    strPetKindInfo* m_pPetKindInfo;       // this + 0
    int                m_nPetKindInfoCount;  // this + 4
    strPetDyeKindInfo* m_pPetDyeKindInfo;    // this + 8
    int                m_nPetDyeKindInfoCount; // this + 12
};
