#pragma once
#include "Effect/CEffectBase.h"
#include "Effect/CEffect_OverMind_Screen.h"
#include "Effect/CEffect_Skill_Type_Once.h" // 雖然在提供的程式碼中未見其用途，但依據建構函式還原
#include "Effect/cltMoFC_EffectKindInfo.h"

/**
 * @struct EffectInfo
 * @brief 用於串接 CEffectBase 物件的雙向鏈結串列節點。
 * (大小: 12 bytes)
 */
struct EffectInfo {
    EffectInfo* pPrev;       // 指向上一個節點
    EffectInfo* pNext;       // 指向後一個節點
    CEffectBase* pEffect;    // 指向實際的特效物件
};

/**
 * @class CEffectManager
 * @brief 特效管理器，使用單例模式。
 * * 負責所有動態遊戲特效的生命週期管理。它維護一個 CEffectBase 物件的
 * 雙向鏈結串列，並在每一幀中處理它們的更新、繪製和銷毀。
 */
class CEffectManager {
public:
    /// @brief 取得唯一的管理器實例。
    static CEffectManager* GetInstance();

    /// @brief 解構函式，會清理所有殘餘的特效。
    ~CEffectManager();

    // --- 特效管理核心函式 ---

    /// @brief 將一個已經建立的特效物件加入到管理器的鏈結串列中。
    /// @param pEffect 指向 CEffectBase 衍生類別物件的指標。
    void BulletAdd(CEffectBase* pEffect);

    /// @brief 根據特效種類ID，建立並加入一個新的特效。
    /// 這是主要的特效工廠函式。
    /// @param effectKindID 特效種類的ID (對應 .ea 檔案)。
    /// @param pCaster 施法者或來源角色。
    /// @param pTarget 目標角色。
    /// @param ... 其他參數用於特定特效。
    /// @return 成功建立並加入的特效物件指標，失敗則返回 nullptr。
    CEffectBase* AddEffect(unsigned short effectKindID, ClientCharacter* pCaster, ClientCharacter* pTarget = nullptr, int a5 = 0, unsigned short a6 = 0, unsigned short a7 = 0, unsigned char a8 = 2);

    /// @brief 根據特效名稱，建立並加入一個新的特效。
    CEffectBase* AddEffect(char* szEffectName, ClientCharacter* pCaster);

    /// @brief 處理所有受管理特效的生命週期，並移除已結束的特效。
    /// @param fElapsedTime 經過的時間。
    /// @param bForceDeleteAll 是否強制刪除所有特效。
    void FrameProcess(float fElapsedTime, bool bForceDeleteAll = false);

    /// @brief 呼叫所有受管理特效的 Process 函式，準備繪製數據。
    void Process();

    /// @brief 呼叫所有受管理特效的 Draw 函式，將其繪製到螢幕。
    void Draw();

    /// @brief 從管理器中手動刪除一個指定的特效。
    /// @param pEffect 要刪除的特效物件指標。
    /// @return 成功返回 true，失敗返回 false。
    bool DeleteEffect(CEffectBase* pEffect);

    /// @brief 處理特殊的、非鏈結串列管理的特效（如全螢幕效果）。
    void AddEtcEffect(unsigned short type, unsigned int accountID);

    /// @brief 清空所有特效鏈結串列。
    void BulletListAllDel();
	
    cltMoFC_EffectKindInfo g_clEffectKindInfo;
    
private:
    // --- 私有成員 ---

    // 私有建構函式，確保單例模式
    CEffectManager();
    CEffectManager(const CEffectManager&) = delete;
    CEffectManager& operator=(const CEffectManager&) = delete;
    static CEffectManager* s_pInstance;

    // 特效鏈結串列的頭尾指標和計數器
    EffectInfo* m_pHead;           // 位移 +0
    EffectInfo* m_pTail;           // 位移 +4
    unsigned int     m_uEffectCount;    // 位移 +8

    // 特殊的、作為成員變數的特效物件
    CEffect_OverMind_Screen m_OverMindScreenEffect; // 位移 +12
    CEffect_Skill_Type_Once m_SkillTypeOnceEffect;  // 位移 +32, 雖然用途不明，但依據建構函式還原
};