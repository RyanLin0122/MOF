#include "Image/ResourceMgr.h"
#include <windows.h> // For FindFirstFileA, MessageBoxA
#include "CMOFPacking.h"


ResourceMgr* ResourceMgr::s_pInstance = nullptr;

// 靜態 GetInstance 方法的實現
ResourceMgr* ResourceMgr::GetInstance() {
    if (s_pInstance == nullptr) {
        s_pInstance = new (std::nothrow) ResourceMgr();
    }
    return s_pInstance;
}

ResourceMgr::ResourceMgr() {
    // 檢查封裝檔是否存在，並設定旗標
    WIN32_FIND_DATAA findFileData;
    if (FindFirstFileA("MoFData/mofdata.pak", &findFileData) == INVALID_HANDLE_VALUE) {
        m_bUsePackFile = false;
    } else {
        m_bUsePackFile = true;
    }

    // 初始化各個 cltGIResource 子管理器
    m_GIResources[RES_CHARACTER].Initialize("MoFData/Character", 10000);
    m_GIResources[RES_MONSTER].Initialize("MoFData/Monster", 10000);
    m_GIResources[RES_MAPTILE].Initialize("MoFData/MapTile", 0);
    m_GIResources[RES_ITEM].Initialize("MoFData/Item", 10000);
    m_GIResources[RES_NPC].Initialize("MoFData/NPC", 10000);
    m_GIResources[RES_UI].Initialize("MoFData/UI", 10000);
    m_GIResources[RES_IMAGE].Initialize("MoFData/Image", 10000);
    m_GIResources[RES_EFFECT].Initialize("MoFData/Effect", 60000);
    m_GIResources[RES_OBJECT].Initialize("MoFData/Object", 10000);
    m_GIResources[RES_MINIGAME].Initialize("MoFData/MiniGame", 10000);
    m_GIResources[RES_MOVEMAPBG].Initialize("MoFData/MoveMapBg", 10000);
    m_GIResources[RES_LOBBY].Initialize("MoFData/Lobby", 10000);
    m_GIResources[RES_SPIRIT].Initialize("MoFData/Spirit", 10000);
    m_GIResources[RES_MAPBG].Initialize("MoFData/MapBG", 10000);
    
    // 如果使用封裝檔，則開啟它們
    if (m_bUsePackFile) {
        if (!CMofPacking::GetInstance()->PackFileOpen("./mofdata/mofdata"))
            MessageBoxA(NULL, "PackFileOpenError: mofdata", "Error", 0);
        if (!CMofPacking::GetInstance()->PackFileOpen("./mofdata/character"))
            MessageBoxA(NULL, "PackFileOpenError: character", "Error", 0);
    }
}

ResourceMgr::~ResourceMgr() {
    // m_GIResources 和 m_Loader 的解構函式會被自動呼叫
}

void ResourceMgr::Poll() {
    // 輪詢背景載入執行緒
    m_Loader.Poll();

    // 輪詢所有子管理器，使其可以回收超時的資源
    for (int i = 0; i < RES_COUNT; ++i) {
        m_GIResources[i].Poll();
    }
}

ImageResourceListData* ResourceMgr::GetImageResource(eResourceType type, unsigned int resourceID, bool bAsync, int priority) {
    if (type >= RES_COUNT) return nullptr;

    // 對於特定類型(如CHARACTER/UI，原始碼中為 group 0)，啟用非同步載入邏輯
    // 原始碼中只有 group 0 有此邏輯，這裡為了範例假設是 RES_CHARACTER
    if (type == RES_CHARACTER && bAsync) {
        // 如果資源尚未被使用，且不在載入佇列中，則新增一個載入請求
        if (m_GIResources[type].GetRefCount(resourceID) < 1 && !m_Loader.FindInResLoadingList(resourceID)) {
            tResInfo resInfo;
            resInfo.dwResourceID = resourceID;
            resInfo.dwGroupID = type;
            resInfo.cPriority = (char)priority;
            m_Loader.AddBackGroundLoadingRes(resInfo);
        }
        return nullptr; // 非同步模式下立即返回
    }

    // 對於其他類型或同步請求，直接轉發給對應的子管理器
    //return m_GIResources[type].Get(resourceID, 1, priority);
    void* pGenericResource = m_GIResources[type].Get(resourceID, 1, priority);
    return static_cast<ImageResourceListData*>(pGenericResource);
}

void ResourceMgr::ReleaseImageResource(eResourceType type, unsigned int resourceID) {
    if (type >= RES_COUNT) return;
    m_GIResources[type].Release(resourceID);
}

void ResourceMgr::LoadingImageResource(unsigned int groupID, unsigned int resourceID, int a4, int a5) {
    if (groupID >= RES_COUNT) return;
    // 這是由背景執行緒回呼的，進行實際的同步載入
    m_GIResources[groupID].Get(resourceID, a4, a5);
}

bool ResourceMgr::FindInResLoadingList(unsigned int resourceID) {
    return m_Loader.FindInResLoadingList(resourceID);
}

void ResourceMgr::DelResInLoadingList(unsigned int resourceID) {
    m_Loader.DelResInLoadingList(resourceID);
}

void ResourceMgr::ClearLoadingList() {
    m_Loader.ClearLodingList();
}