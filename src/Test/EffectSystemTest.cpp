#include "Test/EffectSystemTest.h"
#include "Effect/CEffectManager.h"
#include "Effect/CEAManager.h"
#include "Effect/cltMoFC_EffectKindInfo.h"
#include "Effect/CEffect_Battle_DownCut.h"
#include "Effect/CEffect_Battle_Hit_Normal.h"
#include "Effect/CEffect_Battle_UpperCut.h"
#include "Effect/CEffect_Battle_HorizonCut.h"
#include "Effect/CEffect_Battle_MagicBook.h"

#include "Character/ClientCharacter.h"
#include "global.h"
#include <stdio.h>

EffectSystemTest::EffectSystemTest() :
    m_pCaster(nullptr),
    m_fTimeSinceLastEffect(0.0f)
{
}

EffectSystemTest::~EffectSystemTest()
{
    Cleanup();
}

HRESULT EffectSystemTest::Initialize()
{
    printf("--- [EffectSystemTest] 初始化開始 ---\n");

    // 步驟 1: 確保所有相關的管理器都已建立。
    // GetInstance() 會在需要時自動建立單例物件。
    CEffectManager::GetInstance();
    CEAManager::GetInstance();

    // 注意：cltMoFC_EffectKindInfo 管理器雖然存在，但 CEffect_Battle_DownCut
    // 是直接實例化的，不透過 CEffectManager 的 AddEffect 工廠函式。
    // 因此，目前我們不需要初始化 g_clEffectKindInfo->Initialize(filename)。

    // 步驟 2: 建立一個模擬的施法者角色。
    printf("  正在建立模擬施法者 (ClientCharacter)...\n");
    m_pCaster = new ClientCharacter();
    if (!m_pCaster) {
        printf("錯誤：建立 ClientCharacter 失敗。\n");
        return E_FAIL;
    }
	m_pTarget = new ClientCharacter();
    m_pTarget->SetPosX(900);
	m_pTarget->SetPosY(200);
    if (!m_pTarget) {
        printf("錯誤：建立 ClientCharacter 失敗。\n");
        return E_FAIL;
	}
    printf("--- [EffectSystemTest] 初始化成功 ---\n");
    return S_OK;
}

void EffectSystemTest::Cleanup()
{
    printf("--- [EffectSystemTest] 清理資源 ---\n");

    // 透過 CEffectManager 刪除所有仍在活動中的特效。
    CEffectManager::GetInstance()->BulletListAllDel();

    // 刪除模擬角色。
    if (m_pCaster) {
        delete m_pCaster;
        m_pCaster = nullptr;
        printf("  已刪除模擬施法者。\n");
    }
}

void EffectSystemTest::Update(float fElapsedTime)
{
    // 累加時間
    m_fTimeSinceLastEffect += fElapsedTime;

    // 每隔 0.5 秒產生一個新的下劈斬特效
    if (m_fTimeSinceLastEffect > 3.0f) {
        //SpawnDownCutEffect();
        //SpawnUpperCutEffect();
		//SpawnHitNormalEffect();
        //SpawnHorizenCutEffect();
        SpawnMagicBookEffect();
        m_fTimeSinceLastEffect = 0.0f; // 重置計時器
    }

    // 更新所有在 CEffectManager 中註冊的特效。
    // 這會呼叫每個特效的 FrameProcess 函式，處理其生命週期。
    CEffectManager::GetInstance()->FrameProcess(fElapsedTime, false);
}

void EffectSystemTest::Render()
{
    // 步驟 1: 呼叫所有特效的 Process 函式。
    // 這會更新特效的位置、可見性等繪製前所需的狀態。
    CEffectManager::GetInstance()->Process();

    // 步驟 2: 呼叫所有特效的 Draw 函式。
    // 這會將特效的頂點數據提交給渲染管線。
    CEffectManager::GetInstance()->Draw();
}

void EffectSystemTest::SpawnDownCutEffect()
{
    if (!m_pCaster) return;

    printf("  正在產生 CEffect_Battle_DownCut 特效...\n");

    // 步驟 1: 直接 new 一個 CEffect_Battle_DownCut 物件。
    // 其建構函式會自動向 CEAManager 請求 "efn_downcut.ea" 特效數據。
    CEffect_Battle_DownCut* pEffect = new CEffect_Battle_DownCut();

    // 步驟 2: 從模擬角色獲取位置。
    float x = static_cast<float>(m_pCaster->GetPosX());
    float y = static_cast<float>(m_pCaster->GetPosY());

    // 步驟 3: 設定特效的位置與方向 (隨機翻轉以測試兩種情況)。
    bool bFlip = (rand() % 2 == 0);
    pEffect->SetEffect(x, y, bFlip, 0);

    // 步驟 4: 將建立好的特效實例加入到 CEffectManager 的管理鏈結串列中。
    // 這是關鍵步驟，只有被加入的特效才會被更新和繪製。
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnHitNormalEffect()
{
    if (!m_pCaster) return;

    printf("  正在產生 CEffect_Battle_Hit_Normal 特效...\n");

    // 步驟 1: 直接 new 一個 CEffect_Battle_DownCut 物件。
    // 其建構函式會自動向 CEAManager 請求 "efn_downcut.ea" 特效數據。
    CEffect_Battle_Hit_Normal* pEffect = new CEffect_Battle_Hit_Normal();

    // 步驟 2: 從模擬角色獲取位置。
    float x = static_cast<float>(m_pCaster->GetPosX());
    float y = static_cast<float>(m_pCaster->GetPosY());

    // 步驟 3: 設定特效的位置與方向 (隨機翻轉以測試兩種情況)。
    bool bFlip = (rand() % 2 == 0);
    pEffect->SetEffect(x, y);

    // 步驟 4: 將建立好的特效實例加入到 CEffectManager 的管理鏈結串列中。
    // 這是關鍵步驟，只有被加入的特效才會被更新和繪製。
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnUpperCutEffect()
{
    if (!m_pCaster) return;

    printf("  正在產生 CEffect_Battle_UpperCut 特效...\n");

    // 步驟 1: 直接 new 一個 CEffect_Battle_DownCut 物件。
    // 其建構函式會自動向 CEAManager 請求 "efn_downcut.ea" 特效數據。
    CEffect_Battle_UpperCut* pEffect = new CEffect_Battle_UpperCut();

    // 步驟 2: 從模擬角色獲取位置。
    float x = static_cast<float>(m_pCaster->GetPosX());
    float y = static_cast<float>(m_pCaster->GetPosY());

    // 步驟 3: 設定特效的位置與方向 (隨機翻轉以測試兩種情況)。
    bool bFlip = (rand() % 2 == 0);
    pEffect->SetEffect(x, y, bFlip, 0);

    // 步驟 4: 將建立好的特效實例加入到 CEffectManager 的管理鏈結串列中。
    // 這是關鍵步驟，只有被加入的特效才會被更新和繪製。
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnHorizenCutEffect()
{
    if (!m_pCaster) return;

    printf("  正在產生 CEffect_Battle_UpperCut 特效...\n");

    // 步驟 1: 直接 new 一個 CEffect_Battle_DownCut 物件。
    // 其建構函式會自動向 CEAManager 請求 "efn_downcut.ea" 特效數據。
    CEffect_Battle_HorizonCut* pEffect = new CEffect_Battle_HorizonCut();

    // 步驟 2: 從模擬角色獲取位置。
    float x = static_cast<float>(m_pCaster->GetPosX());
    float y = static_cast<float>(m_pCaster->GetPosY());

    // 步驟 3: 設定特效的位置與方向 (隨機翻轉以測試兩種情況)。
    bool bFlip = (rand() % 2 == 0);
    pEffect->SetEffect(x, y, bFlip, 0);

    // 步驟 4: 將建立好的特效實例加入到 CEffectManager 的管理鏈結串列中。
    // 這是關鍵步驟，只有被加入的特效才會被更新和繪製。
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnMagicBookEffect()
{
    if (!m_pCaster) return;
    if (!m_pTarget) return;
    printf("  正在產生 CEffect_Battle_UpperCut 特效...\n");

    // 步驟 1: 直接 new 一個 CEffect_Battle_DownCut 物件。
    // 其建構函式會自動向 CEAManager 請求 "efn_downcut.ea" 特效數據。
    CEffect_Battle_MagicBook* pEffect = new CEffect_Battle_MagicBook();

    // 步驟 2: 從模擬角色獲取位置。
    float x = static_cast<float>(m_pCaster->GetPosX());
    float y = static_cast<float>(m_pCaster->GetPosY());

    // 步驟 3: 設定特效的位置與方向 (隨機翻轉以測試兩種情況)。
    bool bFlip = (rand() % 2 == 0);
    pEffect->SetEffect(m_pCaster, m_pTarget, bFlip, 0);

    // 步驟 4: 將建立好的特效實例加入到 CEffectManager 的管理鏈結串列中。
    // 這是關鍵步驟，只有被加入的特效才會被更新和繪製。
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}