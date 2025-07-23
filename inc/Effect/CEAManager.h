#pragma once
#include "Effect/CCAEffect.h" // 包含 CCAEffect 的定義

/**
 * @class CEAManager
 * @brief 特效動畫 (.ea) 數據的管理器。
 * * 使用單例模式，作為一個全域的快取系統，負責懶漢式載入、
 * 儲存並提供所有特效的動畫數據。
 */
class CEAManager {
public:
    /// @brief 取得唯一的管理器實例。
    static CEAManager* GetInstance();

    /// @brief 解構函式，會釋放所有已載入的特效數據。
    ~CEAManager();

    /// @brief 獲取指定的特效動畫數據。
    /// 如果數據尚未載入，此函式會觸發檔案讀取和解析。
    /// @param effectID 特效的唯一 ID。
    /// @param szFileName 特效的檔案名稱。
    /// @param pEffect 要接收數據的 CCAEffect 物件指標。
    void GetEAData(int effectID, const char* szFileName, CCAEffect* pEffect);

    /// @brief 重設管理器，清空所有已載入的特效數據。
    void Reset();

private:
    // --- 私有函式 ---

    CEAManager();
    CEAManager(const CEAManager&) = delete;
    CEAManager& operator=(const CEAManager&) = delete;

    /// @brief 從獨立檔案載入 .ea 數據。
    void LoadEA(int effectID, const char* szFileName);

    /// @brief 從封裝檔 (mof.pak) 載入 .ea 數據。
    void LoadEAInPack(int effectID, char* szFileName);

    // --- 私有成員 ---
    static CEAManager* s_pInstance;

    // 虛擬函式表指標 (由編譯器管理)
    // void* m_pVftable; // 位移 +0

    // 指標陣列，作為特效數據的快取。
    EADATALISTINFO* m_pEaData[65535]; // 位移 +4
};