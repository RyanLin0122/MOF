#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <string>
#include <vector>
#include "Info/cltMineKindInfo.h"
#include "Info/cltDebuffKindInfo.h"
#include "Other/cltAttackAtb.h"
#include "global.h"

class cltClassKindInfo;

// ──────────────────────────────────────────────────────────────────────────
// 技能記錄：嚴格 400 bytes（與反編譯一致）。
// 我們用原始 byte 區塊，並在 .cpp 以位移寫入/讀取。
// 下面提供常用位移對照（韓文欄位名｜中文說明）
// ──────────────────────────────────────────────────────────────────────────
#pragma pack(push,1)
struct stSkillKindInfo {
    // 生的 400 bytes，內部以 offset 操作（與反編譯一致）
    uint8_t raw[400];
};
/*
struct stSkillKindInfo {
    // 偏移(Offset) +0 | 大小(Size) 2
    // 來源: p_skillinfo[1], a_skillinfo[1]
    uint16_t id;                // ID (技能ID, 'A'/'P' + 數字)

    // 偏移 +2 | 大小 2
    // 來源: p_skillinfo[2], a_skillinfo[2]
    uint16_t init_id;           // 초기 ID (初始ID)

    // 偏移 +4 | 大小 2
    // 來源: p_skillinfo[4], a_skillinfo[4]
    uint16_t name_code;         // 이름 코드 (名稱代碼)

    // 偏移 +6 | 大小 2
    // 來源: p_skillinfo[10], a_skillinfo[11]
    uint16_t desc_code;         // 스킬설명코드 (技能說明碼)

    // 偏移 +8 | 大小 4
    // 來源: p_skillinfo[5], a_skillinfo[5]
    uint32_t icon_id_hex;       // 아이콘 그림 ID (圖示資源ID, 十六進制字串)

    // 偏移 +12 | 大小 2
    // 來源: p_skillinfo[6], a_skillinfo[6]
    uint16_t icon_block;        // 아이콘 그림 블록 (圖示圖塊)

    // 偏移 +14 | 大小 2 | (推測, C++程式碼中無此欄位)
    uint16_t pad14;             // 補齊位元組

    // 偏移 +16 | 大小 4
    // 來源: p_skillinfo[7], a_skillinfo[7]
    uint32_t sm_icon_id_hex;    // 작은 아이콘 그림 ID (小圖示ID, 十六進制)

    // 偏移 +20 | 大小 2
    // 來源: p_skillinfo[8], a_skillinfo[8]
    uint16_t sm_icon_block;     // 작은 아이콘 그림 블록 (小圖示圖塊)

    // 偏移 +22 | 大小 2 | (推測)
    uint16_t pad22;

    // 偏移 +24 | 大小 4
    // 來源: p_skillinfo[20] "공격력 상승[%]", a_skillinfo[14] "공격력 상승(P)"
    uint32_t value_p_atk;       // 數值欄 (攻擊力相關)

    // 偏移 +28 | 大小 4
    // 來源: p_skillinfo[21] "방어력 상승[%]", a_skillinfo[15] "방어력 상승(P)"
    uint32_t value_p_def;       // 數值欄 (防禦力相關)

    // 偏移 +32 | 大小 1
    // (由 Initialize() 函式在載入後計算並回填)
    uint8_t  skill_level;       // 技能等級 (由前置技能鏈計算, 1起算)

    // 偏移 +33 | 大小 3 | (推測)
    uint8_t  pad33[3];

    // 偏移 +36 | 大小 4
    // 來源: a_skillinfo[24] "재사용시간"
    uint32_t reuse_ms;          // 재사용시간 (冷卻時間, 毫秒)

    // 偏移 +40 | 大小 8 (64-bit)
    // 來源: p_skillinfo[23], a_skillinfo[25] "습득 클래스"
    uint64_t req_class_mask;    // 需求職業遮罩 (GetReqClasses 的結果)

    // 偏移 +48 | 大小 4
    // 來源: p_skillinfo[12], a_skillinfo[26] "스킬 구매 타입"
    uint32_t buy_class_type;    // 스킬 구매 타입 (SWORD/BOW/...)

    // 偏移 +52 | 大小 16
    // 來源: p_skillinfo[13-16], a_skillinfo[27-30]
    uint32_t need_point_fig;    // 필요 전사 포인트
    uint32_t need_point_arc;    // 필요 궁수 포인트
    uint32_t need_point_mag;    // 필요 법사 포인트
    uint32_t need_point_pri;    // 필요 사제 포인트

    // 偏移 +68 | 大小 2
    // 來源: p_skillinfo[22], a_skillinfo[32] "필요 레벨"
    uint16_t req_level;         // 필요 레벨 (需求等級)

    // 偏移 +70 | 大小 2
    // 來源: p_skillinfo[9], a_skillinfo[10] "상위 스킬 코드"
    uint16_t prev_skill_code;   // 이전/상위 스킬 코드 (前置技能碼)

    // 偏移 +72 | 大小 16
    // 來源: p_skillinfo[17], a_skillinfo[31] "사용시 필요 장착 무기 클래스"
    uint8_t  req_weapon_flags[16]; // 各武器位旗標 (GetReqWeaponClasses 寫入)

    // 偏移 +88 | 大小 4
    // (由 GetReqWeaponClasses 計算並回傳)
    uint32_t req_weapon_count;  // 上述旗標被設為 1 的數量

    // 偏移 +92 | 大小 20
    // 來源: p_skillinfo[80-89], a_skillinfo[80-90]
    uint16_t eff_codes[10];     // 效果代碼 (EffectKindCode)
                                // [0-3]: eff1-4, [4-5]: eff5-6, [6-9]: eff7-10 (順序依反編譯)

    // 偏移 +112 | 大小 2
    union {
        // Passive: p_skillinfo[11] "스킬 짧은 설명" (短說明碼)
        uint16_t short_desc_code;
        // Active: a_skillinfo[9] "스킬 타입"
        uint8_t  active_effect_type; // GetActiveEffectType() (1..11)
    };

    // 偏移 +114 | 大小 2
    // 來源: p_skillinfo[18], a_skillinfo[12]
    uint16_t value114;          // 數值欄 (HP/MP 상승)

    // 偏移 +116 | 大小 2
    // 來源: p_skillinfo[19], a_skillinfo[13]
    uint16_t value116;          // 數值欄 (HP/MP 상승)

    // 偏移 +118 | 大小 2 | (推測)
    uint16_t pad118;

    // 偏移 +120 | 大小 28
    // 來源: p_skillinfo, a_skillinfo 中的各類數值欄
    uint32_t d120, d124, d128;   // 攻擊/防禦/命中/爆擊等
    uint16_t w132, w134, w136, w138;
    uint32_t d140, d144;

    // 偏移 +148 | 大小 16
    // 來源: a_skillinfo[33] "사운드"
    char     use_sound[16];     // 사운드 (使用音效字串, 如 "S0001")

    // 偏移 +164 | 大小 1
    // 來源: a_skillinfo[34] "원거리 여부"
    uint8_t  is_long;           // 投射軌跡類型 ("LONG"=1, 其餘 0)

    // 偏移 +165 | 大小 1 | (推測)
    uint8_t  pad165;

    // 偏移 +166 | 大小 4
    // 來源: a_skillinfo[35-36] "타겟범위x", "타겟범위y"
    uint16_t w166, w168;        // 發射範圍等

    // 偏移 +170 | 大小 1
    // 來源: a_skillinfo[38] "시전 애니"
    uint8_t  cast_anim_type;    // NORMAL/SPELL/PRAY/TRIPLE/DOUBLE → 0..4

    // 偏移 +171 | 大小 1
    // 來源: a_skillinfo[39] "맞는 속성"
    uint8_t  element_tag;       // NONE/NORMAL/FIRE/ELEC/ICE → 0..4

    // 偏移 +172 | 大小 4
    // 來源: a_skillinfo[40] "공격 속성"
    uint32_t attack_atb;        // 공격 속성 (FIRE/ICE/…)

    // 偏移 +176 | 大小 24
    // 來源: a_skillinfo[42-47]
    uint32_t stun_chance;       // 기절 확률
    uint32_t stun_duration_ms;  // 기절 시간
    uint32_t block_chance;      // 봉쇄 확률
    uint32_t block_duration_ms; // 봉쇄 시간
    uint32_t freeze_chance;     // 결빙 확률
    uint32_t freeze_duration_ms;// 결빙 시간

    // 偏移 +200 | 大小 4
    // 來源: a_skillinfo[48] "mine 아이디"
    uint16_t mine_id;           // mine 아이디 (地雷技能ID)
    uint16_t mine_pad;

    // 偏移 +204 | 大小 40
    // 來源: p_skillinfo[43] (以 '&' 分隔的字串)
    uint32_t link_skill_codes[10]; // 連動技能碼

    // 偏移 +244 | 大小 4
    // (由解析 p_skillinfo[43] 時計算)
    uint32_t link_skill_count;  // 連動技能數量

    // 偏移 +248 | 大小 4
    // 來源: p_skillinfo[41], a_skillinfo[41] "파티 공격속성"
    uint32_t working_type;      // Passive:GetWorkingType(), Active:派對攻擊屬性

    // 偏移 +252 | 大小 4
    // 來源: p_skillinfo[44]
    int32_t  attack_target_atb; // 攻擊目標屬性(-1 表示 "0")

    // 偏移 +256 | 大小 144
    // 來源: p_skillinfo, a_skillinfo 末端的大量純數字欄位
    uint32_t d256;              // 各類機率/時間/增益等
    uint32_t d260;
    uint32_t d264;
    uint32_t d268;
    uint32_t d272;
    int32_t  d276;
    uint32_t d280;
    uint32_t d284;
    uint32_t d288;
    uint8_t  b292;
    uint8_t  pad293[3];
    uint32_t d296;
    uint16_t w300;
    uint16_t w302;
    uint16_t debuff_id; // 來源: a_skillinfo[75]
    uint16_t pad306;
    uint32_t d308;
    uint32_t d312;
    uint32_t d316;
    uint32_t d320;
    uint32_t d324;
    uint16_t w328;
    uint16_t pad330;
    uint32_t d332;
    uint32_t d336;
    uint32_t d340;
    uint32_t d344;
    uint32_t d348;
    uint32_t d352;
    uint32_t d356;
    uint32_t d360;
    uint32_t d364;
    uint32_t d368;
    uint32_t d372;
    uint32_t d376;
    uint32_t d380;
    uint32_t d384;
    uint32_t d388;
    uint32_t d392;
    uint32_t d396;
};
*/
#pragma pack(pop)
static_assert(sizeof(stSkillKindInfo) == 400, "stSkillKindInfo must be 400 bytes");

// ──────────────────────────────────────────────────────────────────────────
// 主類別
// ──────────────────────────────────────────────────────────────────────────
class cltSkillKindInfo {
public:
    // 反編譯：設定外部 Class 資訊供查詢
    static void InitializeStaticVariable(cltClassKindInfo* p) { m_pclClassKindInfo = p; }

    cltSkillKindInfo();
    ~cltSkillKindInfo() { Free(); }

    int  Initialize(char* passiveFile, char* activeFile);  // 先 P 再 A；並建立索引與等級
    int  Initialize_P(char* filename);                     // 解析 p_skillinfo.txt
    int  Initialize_A(char* filename);                     // 解析 a_skillinfo.txt
    void Free();

    // 依碼取記錄（位移行為對齊：原本為 *((_DWORD*)this + code)）
    stSkillKindInfo* GetSkillKindInfo(uint16_t code);
    stSkillKindInfo* GetSkillKindInfo_P(uint16_t code);
    stSkillKindInfo* GetSkillKindInfo_A(uint16_t code);
    stSkillKindInfo* GetSkillKindInfo_P() { return m_passive.data(); }
    stSkillKindInfo* GetSkillKindInfo_A() { return m_active.data(); }

    // 數量查詢
    uint16_t GetPassiveSkillNum() const { return static_cast<uint16_t>(m_passive.size()); }
    uint16_t GetActiveSkillNum()  const { return static_cast<uint16_t>(m_active.size()); }
    int      GetTotalSkillNum()   const { return GetPassiveSkillNum() + GetActiveSkillNum(); }

    // 工具（與反編譯一致）
    static bool     IsPassiveSkill(uint16_t code);
    static bool     IsActiveSkill(uint16_t code);
    static uint16_t TranslateKindCode(const char* s);        // 'A'/'P' + 5 digits, 15-bit
    static uint16_t TranslateEffectKindCode(const char* s);  // 5 碼：字母 + 數字，(ch+31)<<11 | num<0x800

    static int      GetWorkingType(const char* s);           // ATTACK_ONCE/… → 1..6
    static uint8_t  GetActiveEffectType(const char* s);      // ATTACK_ONE/… → 1..11
    uint8_t         GetActiveEffectType(uint16_t code);      // 讀取記錄 +112 位移

    static int      GetSkillClass(const char* s);            // SWORD/BOW/THEOLOGY/MAGIC/SPECIALTY → 0..4

    // 來自表格欄位的需求解析
    static uint64_t GetReqClasses(cltSkillKindInfo* self, char* classes);        // 回傳 64-bit 遮罩
    static int      GetReqWeaponClasses(cltSkillKindInfo* self, char* weapons,
        uint8_t* const outFlags /* 指向 +72 位移 */);

    // 額外查詢
    char* UseSkillSound(uint16_t code);                    // 回傳 +148 的字串
    uint16_t GetSkillLevel(uint16_t code);                   // 依「上一級鏈」計算等級（至少 1）
    int      IsLastLevelSkill(uint16_t code);                // 反編譯語意

    // 職業鏈判定（等級層級）
    int IsUnderTwoLevelClassSkill(int64_t atbMask);
    int IsTwoLevelClassSkill(int64_t atbMask);
    int IsUnderThreeLevelClassSkill(int64_t atbMask);
    int IsThreeLevelClassSkill(int64_t atbMask);
    int IsFourLevelClassSkill(int64_t atbMask);

    // 依職業取可用技能（沿職業樹向上蒐集；回傳寫入個數）
    int GetClassActiveSkill(uint16_t startClass, uint16_t* outCodes);
    int GetClassPassiveSkill(uint16_t startClass, uint16_t* outCodes);

    // 靜態外掛：職業資料
    static cltClassKindInfo* m_pclClassKindInfo;

    // 工具
    static bool IsDigits(const char* s);           // 十進位數字
    static bool IsHexLike(const char* s);          // 0-9A-Fa-f
    static bool NextTok(char*& out, const char* delims); // strtok 包裝

private:
    // 內部資料
    std::vector<stSkillKindInfo> m_passive;   // P*
    std::vector<stSkillKindInfo> m_active;    // A*
    stSkillKindInfo* m_idx[65536];// 技能碼 → 記錄指標（含 P/A 全部）
};

// 可選：M 版（按反編譯行為，這裡給最小還原）
class cltSkillKindInfoM {
public:
    cltSkillKindInfoM();
    ~cltSkillKindInfoM() { Free(); }

    int  Initialize(char* pfile, char* afile);
    int  Initialize_P(char* filename);
    int  Initialize_A(char* filename) { (void)filename; return 1; }
    void Free();

    static uint16_t TranslateKindCode(const char* s) { return cltSkillKindInfo::TranslateKindCode(s); }
    static int      IsPassiveSkill(uint16_t c) { return cltSkillKindInfo::IsPassiveSkill(c); }
    static int      IsActiveSkill(uint16_t c) { return cltSkillKindInfo::IsActiveSkill(c); }

private:
    struct Rec28 { uint8_t raw[28]; };
    std::vector<Rec28>   m_pList; // 28 bytes
    std::vector<Rec28>   m_aList; // 未使用
    stSkillKindInfo* m_idx[65536]{};
};