#pragma once

#include <windows.h>
#include <cstdio>
#include <cstring>
#include <cctype> // for toupper

// 前向宣告
class cltTextFileManager; // 假設的檔案管理類別

/**
 * @enum E_EFFECT_TYPE
 * @brief 定義特效的行為類型，從 Initialize 函式的邏輯推斷。
 */
enum E_EFFECT_TYPE : unsigned char {
    EFFECT_TYPE_UNKNOWN = 0,
    EFFECT_TYPE_ONCE = 1, // 在施法者位置播放一次
    EFFECT_TYPE_DIRECTED = 2, // 在目標位置播放一次
    EFFECT_TYPE_SUSTAIN = 3, // 在施法者身上持續播放
    EFFECT_TYPE_SHOOT_UNIT = 4, // 作為投射物飛行
    EFFECT_TYPE_SHOOT_NO_EA = 5, // 投射物，但不使用 .ea 檔案 (例如子彈)
    EFFECT_TYPE_ITEM_ONCE = 6  // 物品使用時的一次性特效
};

/**
 * @struct stEffectKindInfo
 * @brief (大小: 132 bytes) 儲存單一特效種類的完整定義。
 */
struct stEffectKindInfo {
    /// @brief (位移 +0) 特效的唯一數字 ID。
    unsigned short usKindID;

    /// @brief (位移 +2) 特效資源檔 (.ea) 的名稱。
    char szFileName[128];

    /// @brief (位移 +130) 特效的行為類型。
    E_EFFECT_TYPE ucType;

    /// @brief (位移 +131) 為了記憶體對齊的填充位元組。
    char padding;
};

/**
 * @class cltMoFC_EffectKindInfo
 * @brief 管理所有特效種類定義的資料庫。
 * * 作為一個單例或全域物件，它載入並提供對 stEffectKindInfo 結構的快速查詢。
 */
class cltMoFC_EffectKindInfo {
public:
    cltMoFC_EffectKindInfo();
    ~cltMoFC_EffectKindInfo();

    /// @brief 從指定的文字檔案載入所有特效定義。
    /// @param szFileName 包含特效定義的檔案路徑。
    /// @return 成功返回 1，失敗返回 0。
    int Initialize(char* szFileName);

    /// @brief 根據數字 ID 獲取特效定義。
    /// @param kindID 特效的 short 類型 ID。
    /// @return 指向 stEffectKindInfo 的指標，若不存在則為 nullptr。
    stEffectKindInfo* GetEffectKindInfo(unsigned short kindID);

    /// @brief 根據字串 ID 獲取特效定義。
    /// @param szKindCode 格式為 "A0001" 的字串 ID。
    /// @return 指向 stEffectKindInfo 的指標，若不存在則為 nullptr。
    stEffectKindInfo* GetEffectKindInfo(char* szKindCode);

    /// @brief 將字串 ID 轉換為 short 類型的數字索引。
    /// @param szKindCode 格式為 "A0001" 的字串 ID。
    /// @return 對應的 short 索引。
    unsigned short TranslateKindCode(char* szKindCode);

private:
    // 虛擬函式表指標，由編譯器管理
    // void* m_pVftable; // 位移 +0

    // 指標陣列，用於快速查詢。陣列大小 65535 (0xFFF F)。
    stEffectKindInfo* m_pEffectInfo[65535]; // 位移 +4
};