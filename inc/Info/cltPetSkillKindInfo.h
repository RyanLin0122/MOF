#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include "global.h"

#pragma pack(push, 1)

// 單筆寵物技能（大小 = 72 bytes，步距 72；與反編譯一致）
struct strPetSkillKindInfo
{
    // 0x00
    uint16_t SkillID;           // 스킬 ID                 // 技能ID（6碼：P00001/A00001）
    // 0x02
    uint16_t NameTextID;        // 스킬명 text ID          // 技能名文字ID
    // 0x04
    uint16_t DescTextID;        // 스킬설명 text ID        // 技能說明文字ID
    // 0x06
    uint16_t _pad0;             // 對齊至 0x08（反編譯為 +8 的 DWORD）

    // 0x08
    uint32_t IconResHex;        // 아이콘 파일명(HEX)      // 圖示資源ID(十六進位 %x)
    // 0x0C
    uint16_t BlockNameID;       // 블록명                  // 區塊名稱ID
    // 0x0E
    uint16_t RequiredLevel;     // 필요레벨                // 需求等級

    // 0x10 ~ 0x27：主要能力值／加成
    uint32_t SkillAtkAdd;       // 스킬공격력 상승         // 技能攻擊力增加
    uint32_t AtkPermille;       // 공격력 상승(천분율)     // 攻擊力↑(千分比)
    uint32_t DefPermille;       // 방어력 상승(천분율)     // 防禦力↑(千分比)
    uint32_t AccPermille;       // 명중률 상승(천분율)     // 命中率↑(千分比)
    uint32_t HpAutoPermille;    // HP 자동 회복 변화율     // HP自動回復變化率(千分比)
    uint32_t MpAutoPermille;    // MP 자동 회복 변화율     // MP自動回復變化率(千分比)

    // 0x28
    uint32_t AttackSpeed;       // 공격 속도               // 攻擊速度
    // 0x2C
    uint16_t Pickup;            // 줍기                     // 撿取（WORD，0/1等）

    // 0x2E
    uint16_t _pad1;             // 對齊至 0x30

    // 0x30 ~ 0x40：掉落率與四維
    uint32_t DropRatePermille;  // 아이템 드랍율 상승      // 掉落率↑(千分比)
    uint32_t STR;               // STR(공격)               // 力量(攻擊)
    uint32_t DEX;               // DEX(민첩)               // 敏捷
    uint32_t VIT;               // VIT(체력)               // 體力
    uint32_t INT;               // INT(지력)               // 智力

    // 0x44 ~ 0x47：施放特效（5碼類別碼轉WORD）
    uint16_t EffectCasterGround;// 시전자-바닥             // 施法者-地面（效果KindCode）
    uint16_t EffectCasterUp;    // 시전자-위               // 施法者-上方（效果KindCode）
};
static_assert(sizeof(strPetSkillKindInfo) == 72, "strPetSkillKindInfo size must be 72 bytes");

#pragma pack(pop)

class cltPetSkillKindInfo
{
public:
    cltPetSkillKindInfo() : table_(nullptr), count_(0) {}
    ~cltPetSkillKindInfo() { Free(); }

    // 讀取 Petskillkindinfo.txt；成功回傳 1，失敗回傳 0
    int Initialize(char* filename);

    void Free();

    // 依 SkillID（6碼 P/A**** 轉碼）取得資料
    strPetSkillKindInfo* GetPetSkillKindInfo(uint16_t id);

    // SkillID 轉碼：長度6，'A'→設高位0x8000，'P'→高位0；其餘無效
    static uint16_t TranslateKindCode(char* s);

    // 效果KindCode 轉碼（5碼通式：A/N/G/H...）
    uint16_t TranslateEffectKindCode(char* s);

    const strPetSkillKindInfo* data()  const { return table_; }
    int                        count() const { return count_; }

private:
    static bool IsDigitString(const char* s);
    static int  ToHexU32(const char* s, uint32_t& out);

private:
    strPetSkillKindInfo* table_;
    int                  count_;
};
