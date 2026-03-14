#pragma once

#include <array>
#include <cstdint>

struct stSpiritInfo {
    std::uint16_t wSpiritId = 0;            // 정령 ID
    std::uint16_t wNameTextCode = 0;        // 정령 이름 (텍스트 코드)
    std::uint16_t wDescTextCode = 0;        // 정령 내용 (텍스트 코드)
    std::uint16_t wNeedLevelFrom = 0;       // 필요 레벨(시작)
    std::uint16_t wNeedLevelTo = 0;         // 필요 레벨(끝)
    std::uint16_t wNeedItemId = 0;          // 필요 아이템 아이디
    std::uint16_t wSpiritAttribute = 0;     // 정령 속성
    std::uint16_t wIncStr = 0;              // 힘 증가량
    std::uint16_t wIncInt = 0;              // 지력 증가량
    std::uint16_t wIncDex = 0;              // 민첩 증가량
    std::uint16_t wIncVit = 0;              // 체력 증가량
    std::uint16_t wReserved = 0;            // 미사용(패딩/예약)
    std::uint32_t dwResourceId = 0;         // 리소스ID (hex)
    std::uint16_t wStartBlockId = 0;        // 시작 블록 ID
    std::uint16_t wBlockCount = 0;          // 블록 수
};
static_assert(sizeof(stSpiritInfo) == 32, "stSpiritInfo must be 32 bytes");

class CSpiritSystem {
public:
    CSpiritSystem();
    ~CSpiritSystem();
    int Initialize(char* filename);

    std::uint16_t GetSpiritKind(std::uint16_t level);
    stSpiritInfo* GetSpiritInfo(std::uint16_t spiritKind, std::uint16_t level);
    stSpiritInfo* GetSpiritInfo(std::uint16_t kind);

    std::uint16_t TranslateKindCode(char* s);

private:
    std::array<stSpiritInfo*, 200> m_infos{};
    std::uint16_t m_infoCount = 0;
};
