#include "Test/ImageDrawTest.h"
#include "Image/cltImageManager.h"
#include "Image/ResourceMgr.h"
#include "Image/GameImage.h"
#include "CMOFPacking.h"
#include <stdio.h>

ImageDrawTest::ImageDrawTest()
{
    // 在建構函式中，將所有指標初始化為 nullptr，確保安全
    m_pImageManager = nullptr;
    m_pItemImage1 = nullptr;
    m_pItemImage2 = nullptr;
    m_pMapBgImage = nullptr;
}

ImageDrawTest::~ImageDrawTest()
{
    // 在解構時，自動呼叫 Cleanup 函式釋放所有資源
    Cleanup();
}

HRESULT ImageDrawTest::Initialize()
{
    printf("--- [ImageDrawTest] 初始化測試場景 ---\n");

    // 1. 開啟 VFS (虛擬檔案系統)
    // ResourceMgr 的建構函式會檢查 mof.pak 是否存在並設定 m_bUsePackFile 旗標。
    printf("  [ImageDrawTest] 正在開啟 VFS 封裝檔 mof.pak...\n");
    if (!CMofPacking::GetInstance()->PackFileOpen("mof"))
    {
        printf("  [ImageDrawTest] 錯誤：無法開啟 mof.pak/mof.paki。\n");
        return E_FAIL;
    }
    printf("  [ImageDrawTest] VFS 開啟成功。\n");

    // 2. 初始化圖片物件池管理器
    printf("  [ImageDrawTest] 正在初始化 cltImageManager...\n");
    m_pImageManager = cltImageManager::GetInstance();
    m_pImageManager->Initialize(); // 為物件池中的所有圖片預先建立頂點緩衝區
    printf("  [ImageDrawTest] cltImageManager 初始化完畢。\n");

    // 3. 同步載入您指定的資源
    printf("  [ImageDrawTest] 正在同步載入資源...\n");

    // 載入物品 1 (ID: 201326853)
    printf("    載入物品 1 (ID: 201326853, Type: RES_ITEM)...\n");
    m_pItemImage1 = m_pImageManager->GetGameImage(ResourceMgr::RES_ITEM, 201326853);
    if (!m_pItemImage1) { printf("    載入失敗!\n"); return E_FAIL; }

    // 載入物品 2 (ID: 201327617)
    printf("    載入物品 2 (ID: 201327617, Type: RES_ITEM)...\n");
    m_pItemImage2 = m_pImageManager->GetGameImage(ResourceMgr::RES_ITEM, 201327617);
    if (!m_pItemImage2) { printf("    載入失敗!\n"); return E_FAIL; }

    // 載入地圖背景 (ID: 536871078)
    printf("    載入地圖背景 (ID: 536871078, Type: RES_MAPBG)...\n");
    m_pMapBgImage = m_pImageManager->GetGameImage(ResourceMgr::RES_MAPBG, 536871078);
    if (!m_pMapBgImage) { printf("    載入失敗!\n"); return E_FAIL; }

    printf("  [ImageDrawTest] 所有資源載入成功。\n");

    return S_OK;
}

void ImageDrawTest::Render()
{
    // 檢查管理器是否存在，確保安全
    if (!m_pImageManager) return;

    // --- 設定物件狀態 ---
    // 在繪製前，為每個物件設定位置、動畫影格、縮放等屬性

    // 處理地圖背景
    if (m_pMapBgImage)
    {
        m_pMapBgImage->SetPosition(0, 0);     // 放在左上角
        m_pMapBgImage->SetBlockID(0);         // 顯示第一個畫格
        m_pMapBgImage->Process();             // 計算頂點
    }

    // 處理物品 1
    if (m_pItemImage1)
    {
        m_pItemImage1->SetPosition(100, 200);  // 指定位置
        m_pItemImage1->SetBlockID(0);
        m_pItemImage1->SetScale(150);         // 放大到 150%
        m_pItemImage1->Process();
    }

    // 處理物品 2
    if (m_pItemImage2)
    {
        m_pItemImage2->SetPosition(300, 200);  // 指定位置
        m_pItemImage2->SetBlockID(0);
        m_pItemImage2->SetFlipX(true);        // 水平翻轉
        m_pItemImage2->Process();
    }

    // --- 實際繪製 ---
    // 繪製順序很重要，後畫的會蓋在先畫的上面
    if (m_pMapBgImage) m_pMapBgImage->Draw(); //
    if (m_pItemImage1) m_pItemImage1->Draw();
    if (m_pItemImage2) m_pItemImage2->Draw();
}

void ImageDrawTest::Cleanup()
{
    if (m_pImageManager != nullptr)
    {
        printf("  [ImageDrawTest] 正在釋放所有 GameImage...\n");
        m_pImageManager->ReleaseAllGameImage(); //

        printf("  [ImageDrawTest] 正在刪除 cltImageManager...\n");
        delete m_pImageManager;
        m_pImageManager = nullptr;
    }

    // 由於 CMofPacking 和 ResourceMgr 是單例，它們的生命週期由 GetInstance/DestroyInstance 管理
    // 在這裡我們不手動刪除它們，交由主程式的 Cleanup 函式統一處理。
}