#pragma once

#include "cltBaseResource.h" // 包含基底類別的定義

// 前向宣告，避免不必要的 include
struct ImageResourceListData;
class CDeviceResetManager;

/// @class cltGIResource
/// @brief GI 圖片資源管理器。
///
/// 繼承自 cltBaseResource，專門負責載入、管理和釋放 GI 格式的圖片資源。
class cltGIResource : public cltBaseResource {
public:
    cltGIResource();
    virtual ~cltGIResource();

    /// @brief 初始化 GI 資源管理器。
    /// @param pPath 存放 GI 資源的基礎路徑。
    /// @param timeout 資源自動回收的逾時時間。
    void Initialize(const char* pPath, unsigned int timeout);

protected:
    /// @brief 從獨立檔案載入 GI 資源。
    /// @override
    virtual ImageResourceListData* LoadResource(unsigned int id, int a3, unsigned char a4) override;

    /// @brief 從封裝檔 (pack file) 載入 GI 資源。
    /// @override
    virtual ImageResourceListData* LoadResourceInPack(unsigned int id, int a3, unsigned char a4) override;

    /// @brief 釋放一個已載入的 GI 資源。
    /// @override
    virtual void FreeResource(void* pResourceData) override;

private:
    char m_szBasePath[260];          // 資源的基礎搜尋路徑 (位移 +24)
    char m_szNationPath[260];        // 區域化資源的後備路徑 (位移 +284)
    bool m_bNationPathSearched;      // 是否已經搜尋過區域化路徑的旗標 (位移 +544, dword at +136)
};