// Network/CMoFNetwork.h（含 winsock2）必須排在 windows.h 任何傳遞引入之前。
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "Network/CMoFNetwork.h"

#include "Logic/Map.h"
#include "Info/cltMapInfo.h"
#include "Info/cltClimateKindInfo.h"
#include "Info/cltItemKindInfo.h"
#include "Info/cltMapUseItemInfoKindInfo.h"
#include "Info/cltClientPortalInfo.h"
#include "Info/cltPortalInfo.h"
#include "Character/cltMyCharData.h"
#include "Logic/CMap_Item_Climate.h"
#include "Character/ClientCharacter.h"
#include "Character/ClientCharacterManager.h"
#include "Object/CObjectManager.h"
#include "Object/CBaseObject.h"
#include "Object/CPortalObject.h"
#include "Object/CNPCObject.h"
#include "Object/cltAniObject.h"
#include "Object/cltAnimationObjectManager.h"
#include "Effect/CEffectManager.h"
#include "Image/cltImageManager.h"
#include "Image/ImageResource.h"
#include "Image/ImageResourceListDataMgr.h"
#include "Image/ResourceMgr.h"
#include "FileSystem/CMOFPacking.h"
#include "Logic/cltNPCManager.h"
#include "Logic/cltMatchManager.h"
#include "Logic/cltPKFlagManager.h"
#include "Logic/CSpiritSpeechMgr.h"
#include "System/cltHelpSystem.h"
#include "UI/CUIBasic.h"
#include "UI/CUIManager.h"
#include "UI/CControlAlphaBox.h"
#include "UI/CControlBase.h"
#include "System/cltNPCRecallSystem.h"
#include "global.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <new>

// ----------------------------------------------------------------------------
// 外部全域：原 binary 是分散的弱符號，本還原以 C++ 預設 0 初始化暫管，
// 待相關子系統還原後再串接。
// ----------------------------------------------------------------------------
extern int dword_829254;          // 是否使用 packing 載圖（0 = fopen，非 0 = pack）

// ----------------------------------------------------------------------------
// Constructor / Destructor — mofclient.c 0x4E1A60 / 0x4E1C00
// ----------------------------------------------------------------------------
Map::Map() {
    m_dwType            = 0;
    m_dwHeader_4        = 0;
    m_wHeader_8         = 0;
    m_wHeader_10        = 0;
    m_wTileWidth        = 0;
    m_wTileHeight       = 0;
    m_pTileInfo         = nullptr;
    m_wPortalCount      = 0;
    m_pPortalInfo       = nullptr;

    m_ppTileImageArray  = nullptr;
    m_pBgImage1         = nullptr;
    m_pBgImage2         = nullptr;
    m_pBgImageMiddle    = nullptr;
    m_dwBgResource1     = 0;
    m_dwBgResource2     = 0;

    m_iTileDrawCount    = 5;
    m_iWideOffset       = 0;
    m_iScrollX          = 0;
    m_iScrollY          = 0;
    m_iBgScrollX        = 0;
    m_iBgScrollY        = 0;
    m_pViewChar         = nullptr;
    m_wTileWidth2       = 0;
    m_wTileHeight2      = 0;
    m_wPortalCount2     = 0;
    m_iMapPixelWidth    = 0;
    m_iMapWidthClipped  = 0;
    m_iMapPixelHeight   = 0;
    m_iMapHeightClipped = 0;
    m_iTotalTileCount   = 0;
    m_iHasDrawBG        = 0;
    m_iHasBg1           = 0;
    m_iHasBg2           = 0;
    m_iHasBg3           = 0;
    m_wTopOffsetForBg   = 0;
    m_iClipRange0       = 0;
    m_iClipRange1       = 0;
    m_iClipRange2       = 0;

    m_wCollisionWidth   = 0;
    m_wCollisionHeight  = 0;

    m_dwMapItemEffect_1 = 0;
    m_dwMapItemEffect_2 = 0;
    m_dwMapItemEffect_3 = 0;
    m_fMonsterShake     = 0.0f;

    m_szMapFilePath[0]  = '\0';
    m_szBGMFilePath[0]  = '\0';
    m_szAmbientPath[0]  = '\0';
}

Map::~Map() {
    UnInitMap();
    DeleteAllMapItemClimate();
    // m_image1 / m_image2 / m_clMap_Climate / m_clMap_WeddingFlower /
    // m_clMapInfo / m_clMapCollisonInfo 由各自 dtor 自動釋放。
}

// ----------------------------------------------------------------------------
// InitMap / UnInitMap / CheckMap
// ----------------------------------------------------------------------------
int Map::InitMap() {
    if (!m_clMapInfo.Initialize(const_cast<char*>("mapinfo.txt"))) return 0;
    // cltMapInfo 繼承 cltClimateKindInfo；原 binary 直接呼叫
    // cltClimateKindInfo::Initialize on the same pointer。
    if (!m_clMapInfo.cltClimateKindInfo::Initialize(const_cast<char*>("ClimateKindInfo.txt"))) return 0;
    // mofclient.c 0x4E315A：把 mapInfo 註冊給 NPCRecallSystem 作為地圖名稱來源。
    cltNPCRecallSystem::InitializeStaticVariable(&m_clMapInfo);
    return 1;
}

void Map::UnInitMap() {
    // 釋放每個 tile 的 RES_MAPTILE 引用（mofclient.c 0x4E3190）。
    if (m_pTileInfo && m_wTileHeight2 > 0 && m_wTileWidth2 > 0) {
        unsigned char* tileBase = static_cast<unsigned char*>(m_pTileInfo);
        ResourceMgr* rm = ResourceMgr::GetInstance();
        if (rm) {
            for (int row = 0; row < static_cast<int>(m_wTileHeight2); ++row) {
                for (int col = 0; col < static_cast<int>(m_wTileWidth2); ++col) {
                    const int tIdx = col + row * static_cast<int>(m_wTileWidth2);
                    const unsigned int resID = *reinterpret_cast<unsigned int*>(
                        tileBase + 24 * tIdx + 16);
                    if (resID) rm->ReleaseImageResource(ResourceMgr::RES_MAPTILE, resID);
                }
            }
        }
    }

    if (m_pTileInfo) {
        ::operator delete(m_pTileInfo);
        m_pTileInfo = nullptr;
    }

    m_dwType        = 0;
    m_dwHeader_4    = 0;
    m_wHeader_8     = 0;
    m_wHeader_10    = 0;
    m_wTileWidth    = 0;
    m_wTileHeight   = 0;
    m_wPortalCount  = 0;
    if (m_pPortalInfo) {
        ::operator delete(m_pPortalInfo);
        m_pPortalInfo = nullptr;
    }
    if (m_ppTileImageArray) {
        ::operator delete(m_ppTileImageArray);
        m_ppTileImageArray = nullptr;
    }
    m_iHasBg1 = 0;
    m_iHasBg2 = 0;
    m_iHasBg3 = 0;

    m_image1.Free();
    m_image2.Free();
    g_clClientPortalInfo.Free();
    m_clMapCollisonInfo.Free();
    m_clMap_Climate.Free();
    m_clMap_WeddingFlower.Free();
    cltMyCharData::SetMapID(&g_clMyCharData, 0);
}

void Map::CheckMap() {
    // mofclient.c 0x4E1D00：debug 工具，逐張地圖開檔讀檔頭並輸出至
    // MapFileInfo.txt。
    FILE* out = std::fopen("MapFileInfo.txt", "wb");
    if (!out) return;

    unsigned short maxID = 0;
    const int total = GetTotalMapNum();
    for (int i = 0; i < total; ++i) {
        stMapInfo* info = GetMapInfoByIndex(i);
        if (!info) continue;
        const char* path = GetMapFileName(info->m_wID);
        FILE* fp = path ? std::fopen(path, "rb") : nullptr;
        char buf[512];
        if (fp) {
            unsigned int hdr0 = 0, hdr1 = 0;
            unsigned short hdr2 = 0, hdr3 = 0;
            unsigned short w = 0, h = 0;
            std::fread(&hdr0, 4, 1, fp);
            std::fread(&hdr1, 4, 1, fp);
            std::fread(&hdr2, 2, 1, fp);
            std::fread(&hdr3, 2, 1, fp);
            std::fread(&w, 2, 1, fp);
            std::fread(&h, 2, 1, fp);
            if (info->m_wID > maxID) maxID = info->m_wID;
            std::snprintf(buf, sizeof(buf),
                          "%5u:%s : Width:%3u, Height:%3u\n",
                          info->m_wID, path, w, h);
            std::fwrite(buf, std::strlen(buf), 1, out);
            std::fclose(fp);
        } else {
            std::snprintf(buf, sizeof(buf), "Not Open MapFile : %s\n", path ? path : "(null)");
            std::fwrite(buf, std::strlen(buf), 1, out);
        }
    }
    char tail[64];
    std::snprintf(tail, sizeof(tail), "MaxMapID:%u", maxID);
    std::fwrite(tail, std::strlen(tail), 1, out);
    std::fclose(out);
}

// ----------------------------------------------------------------------------
// CreateMap — mofclient.c 0x4E25C0：解析 .map 檔頭、tile / portal 陣列、碰撞
// 點陣，建立背景圖、portal 物件、NPC、AniObject、tile 影像、letterbox 旗標、
// 副本傳送門狀態，並依地圖類型啟動天氣 / 婚禮花瓣。
// ----------------------------------------------------------------------------
void Map::CreateMap(unsigned short mapID, ClientCharacter* pChar) {
    char* path = GetMapFileName(mapID);
    if (!path) return;

    // .map 結構：[header 16B] [tile 24B × N] [portalCount 2B] [portal 12B × M]
    //            [collision (height × ceil(width/8)) bytes — 僅 type==11]
    // 兩種來源：dword_829254 != 0 → 從 CMofPacking 背景緩衝區直接複製；
    //          否則 fopen 直接讀檔。
    if (dword_829254) {
        // ---- Packed 路徑（mofclient.c 184606-184722）---------------------
        CMofPacking* pk = CMofPacking::GetInstance();
        if (!pk) return;
        char* packName = pk->ChangeString(path);
        pk->FileReadBackGroundLoading(packName);
        unsigned char* buf = reinterpret_cast<unsigned char*>(pk->m_backgroundLoadBufferField);
        if (!buf) return;

        // header
        m_dwType      = *reinterpret_cast<unsigned int*>(buf + 0);
        m_dwHeader_4  = *reinterpret_cast<unsigned int*>(buf + 4);
        m_wHeader_8   = *reinterpret_cast<unsigned short*>(buf + 8);
        m_wHeader_10  = *reinterpret_cast<unsigned short*>(buf + 10);
        m_wTileWidth  = *reinterpret_cast<unsigned short*>(buf + 12);
        m_wTileHeight = *reinterpret_cast<unsigned short*>(buf + 14);

        m_wTileWidth2     = m_wTileWidth;
        m_wTileHeight2    = m_wTileHeight;
        m_iTotalTileCount = static_cast<int>(m_wTileWidth) * m_wTileHeight;

        // tile array：直接從 buf+16 複製
        unsigned char* tileSrc = buf + 16;
        if (m_iTotalTileCount > 0) {
            const std::size_t tileBytes =
                static_cast<std::size_t>(24) * m_iTotalTileCount;
            m_pTileInfo = ::operator new(tileBytes);
            std::memset(m_pTileInfo, 0, tileBytes);
            std::memcpy(m_pTileInfo, tileSrc, tileBytes);
        }

        // portal count + portal array
        unsigned char* afterTiles = tileSrc + 24 * m_iTotalTileCount;
        m_wPortalCount  = *reinterpret_cast<unsigned short*>(afterTiles);
        m_wPortalCount2 = m_wPortalCount;
        unsigned char* portalSrc = afterTiles + 2;
        if (m_wPortalCount > 0) {
            const std::size_t portalBytes =
                static_cast<std::size_t>(12) * m_wPortalCount;
            m_pPortalInfo = ::operator new(portalBytes);
            std::memset(m_pPortalInfo, 0, portalBytes);
            std::memcpy(m_pPortalInfo, portalSrc, portalBytes);
        }

        // collision bitmap：紧跟 portal data 之後
        unsigned char* collisionSrc = portalSrc + 12 * m_wPortalCount;
        m_wCollisionWidth  = static_cast<unsigned short>(
            static_cast<int>(m_wTileWidth) << 8);
        m_wCollisionHeight = static_cast<unsigned short>(
            static_cast<int>(m_wTileHeight) << 8);
        if (m_dwType == 11) {
            m_clMapCollisonInfo.InitializeInPack(
                m_wCollisionWidth, m_wCollisionHeight,
                reinterpret_cast<char*>(collisionSrc));
        } else {
            m_clMapCollisonInfo.Initialize(m_wCollisionWidth, m_wCollisionHeight);
        }
    } else {
        // ---- fopen 路徑（mofclient.c 184724-184838）---------------------
        FILE* fp = std::fopen(path, "rb");
        if (!fp) return;

        std::fread(&m_dwType,        4, 1, fp);
        std::fread(&m_dwHeader_4,    4, 1, fp);
        std::fread(&m_wHeader_8,     2, 1, fp);
        std::fread(&m_wHeader_10,    2, 1, fp);
        std::fread(&m_wTileWidth,    2, 1, fp);
        std::fread(&m_wTileHeight,   2, 1, fp);

        m_wTileWidth2     = m_wTileWidth;
        m_wTileHeight2    = m_wTileHeight;
        m_iTotalTileCount = static_cast<int>(m_wTileWidth) * m_wTileHeight;

        if (m_iTotalTileCount > 0) {
            const std::size_t tileBytes =
                static_cast<std::size_t>(24) * m_iTotalTileCount;
            m_pTileInfo = ::operator new(tileBytes);
            std::memset(m_pTileInfo, 0, tileBytes);
            std::fread(m_pTileInfo, tileBytes, 1, fp);
        }

        std::fread(&m_wPortalCount, 2, 1, fp);
        m_wPortalCount2 = m_wPortalCount;
        if (m_wPortalCount > 0) {
            const std::size_t portalBytes =
                static_cast<std::size_t>(12) * m_wPortalCount;
            m_pPortalInfo = ::operator new(portalBytes);
            std::memset(m_pPortalInfo, 0, portalBytes);
            std::fread(m_pPortalInfo, portalBytes, 1, fp);
        }

        m_wCollisionWidth  = static_cast<unsigned short>(
            static_cast<int>(m_wTileWidth) << 8);
        m_wCollisionHeight = static_cast<unsigned short>(
            static_cast<int>(m_wTileHeight) << 8);
        if (m_dwType == 11) {
            m_clMapCollisonInfo.Initialize(m_wCollisionWidth, m_wCollisionHeight, fp);
        } else {
            m_clMapCollisonInfo.Initialize(m_wCollisionWidth, m_wCollisionHeight);
        }
        std::fclose(fp);
    }

    // ---- 載入完成後的共用 metadata / letterbox 設定 ---------------------
    const int sysW = g_Game_System_Info.ScreenWidth;
    const int sysH = g_Game_System_Info.ScreenHeight;
    m_iMapPixelWidth    = static_cast<int>(m_wTileWidth) << 8;
    m_iMapPixelHeight   = static_cast<int>(m_wTileHeight) << 8;
    m_iMapWidthClipped  = m_iMapPixelWidth - sysW;
    m_iMapHeightClipped = m_iMapPixelHeight - sysH;

    m_iTileDrawCount = (sysW > 800) ? 7 : 5;
    m_iWideOffset    = 0;
    if (sysW > 800 && m_iMapPixelWidth < sysW) {
        m_iWideOffset = (sysW - m_iMapPixelWidth) / 2;
        // mofclient.c：letterbox 兩側遮罩位置寫入全域，並把右側 box 移至地圖右緣外。
        g_wLetterboxAW = static_cast<unsigned short>(m_iWideOffset);
        g_wLetterboxAH = static_cast<unsigned short>(sysH);
        g_wLetterboxBW = static_cast<unsigned short>(m_iWideOffset);
        g_wLetterboxBH = static_cast<unsigned short>(sysH);
        g_clAlphaClipBox_R.SetPos(m_iMapPixelWidth + m_iWideOffset, 0);
    }

    // 怪物頭頂抖動倍率 = 3.0 / tileH * 0.048（mofclient.c）。
    m_fMonsterShake = (m_wTileHeight > 0)
        ? static_cast<float>(3.0 / static_cast<double>(m_wTileHeight) * 0.048)
        : 0.0f;

    // ---- 背景圖旗標（從 cltMapInfo 讀取）-------------------------------
    m_iHasDrawBG     = static_cast<int>(GetDrawBG(mapID));
    m_wTopOffsetForBg = 0;
    m_iHasBg1 = 0;
    m_iHasBg2 = 0;
    m_iHasBg3 = 0;
    if (m_iHasDrawBG) {
        if (auto bgID = GetMapBgResourceID(mapID)) {
            m_iHasBg1       = 1;
            m_dwBgResource1 = bgID;
        }
        if (auto nrID = GetMapNearResourceID(mapID)) {
            m_iHasBg2       = 1;
            m_dwBgResource2 = nrID;
        }
        // top offset：從上往下找到第一個非碰撞列，計算與底邊距離。
        if (m_iMapPixelHeight > 0 && m_iMapPixelWidth > 0) {
            int y = 0;
            while (y < m_iMapPixelHeight) {
                int x = 0;
                while (x < m_iMapPixelWidth
                       && m_clMapCollisonInfo.IsCollison(
                              static_cast<unsigned short>(x),
                              static_cast<unsigned short>(y))) {
                    ++x;
                }
                if (x < m_iMapPixelWidth) {
                    int gap = std::abs(768 - m_iMapPixelHeight) - y;
                    if (gap < 0) gap = 0;
                    m_wTopOffsetForBg = static_cast<unsigned short>(gap);
                    y = m_iMapPixelHeight;  // break outer
                }
                ++y;
            }
        }
    }

    // ---- tile image 指標陣列（每 tile 1 個 GameImage*）----------------
    if (m_iTotalTileCount > 0) {
        const std::size_t arrBytes =
            static_cast<std::size_t>(sizeof(GameImage*)) * m_iTotalTileCount;
        m_ppTileImageArray = static_cast<GameImage**>(::operator new(arrBytes));
        std::memset(m_ppTileImageArray, 0, arrBytes);
    }

    // ---- ClientPortalInfo 初始化 --------------------------------------
    g_clClientPortalInfo.Init(mapID);

    // ---- portal info → CObjectManager（mofclient.c：每 portal 12 bytes，
    //   word+0 = portal kind，float+1 = x，float+2 = y）-----------------
    if (m_pPortalInfo && m_wPortalCount > 0) {
        unsigned char* base = static_cast<unsigned char*>(m_pPortalInfo);
        for (unsigned short i = 0; i < m_wPortalCount; ++i) {
            unsigned char* rec = base + 12 * i;
            const unsigned short pid = *reinterpret_cast<unsigned short*>(rec);
            const float fx = *reinterpret_cast<float*>(rec + 4);
            const float fy = *reinterpret_cast<float*>(rec + 8);
            CBaseObject* obj = g_ObjectManager.AddObject(pid);
            if (obj) {
                obj->m_nPosX = static_cast<int>(fx);
                obj->m_nPosY = static_cast<int>(fy);
            }
        }
    }

    // ---- 清空射擊特效 list（換圖前的清盤）-----------------------------
    g_EffectManager.BulletListAllDel();

    // ---- portal-spawn：每筆 portal 對應一個 CPortalObject -------------
    const int portalCnt = g_clClientPortalInfo.GetPortalCount();
    for (int p = 0; p < portalCnt; ++p) {
        CPortalObject* po = new CPortalObject();
        const int px = g_clClientPortalInfo.GetPosX(mapID, static_cast<uint16_t>(p));
        const int py = g_clClientPortalInfo.GetPosY(mapID, static_cast<uint16_t>(p));
        const uint16_t pt = g_clClientPortalInfo.GetPortalType(mapID, static_cast<uint16_t>(p));
        po->CreatePortal(pt, px, py);
        g_ObjectManager.AddObject(po);
    }

    // ---- NPC：mofclient.c 直接 inline 整段 cltNPCManager getter 取資料 +
    //   CNPCObject::AddNPC；本還原使用 cltNPCManager::AddAllNPC 一次處理。
    g_clNPCManager.AddAllNPC(mapID);
    cltMyCharData::AddQuestMark(&g_clMyCharData);

    // ---- 動畫物件：cltAnimationObjectManager 推送至 g_ObjectManager ---
    {
        const int aniCnt = g_clAniObjectMgr.GetAniObjCntInMap(mapID);
        for (int n = 0; n < aniCnt; ++n) {
            const uint16_t idx = g_clAniObjectMgr.GetIndexInMap(static_cast<uint16_t>(n));
            stAniObjectInfo* info = g_clAniObjectMgr.GetAniObejctInfoByIndex(idx);
            if (!info) continue;
            cltAniObject* ao = new cltAniObject();
            // mofclient.c 的 InitInfo arg 順序：
            //   (m_dwHexParam, m_wScale, m_dwResourceID, m_dwMaxFrames,
            //    m_byVisible, m_byTransform, m_byExtra)
            // — 與 cltAniObject::InitInfo 形參名稱不一致，但語意一致。
            ao->InitInfo(
                info->m_dwHexParam,
                static_cast<unsigned short>(info->m_wScale),
                static_cast<int>(info->m_dwResourceID),
                static_cast<int>(info->m_dwMaxFrames),
                static_cast<char>(info->m_byVisible),
                static_cast<unsigned char>(info->m_byTransform),
                static_cast<unsigned char>(info->m_byExtra));
            g_ObjectManager.AddObject(ao);
        }
    }

    // ---- 系統廣播：HelpSystem / SpiritSpeechMgr -----------------------
    g_clHelpSystem.OnMapEntered(mapID);
    g_clSpiritSpeechMgr.OnMapEntered(mapID, GetMapVorF(mapID));

    // ---- tile 影像綁定（mofclient.c：把 tile 陣列裡每筆 +16 的 resource ID
    //   送進 cltImageManager::GetGameImage(2, resID)，存於 m_ppTileImageArray）
    if (m_ppTileImageArray && m_pTileInfo
        && m_wTileWidth2 > 0 && m_wTileHeight2 > 0) {
        unsigned char* tileBase = static_cast<unsigned char*>(m_pTileInfo);
        cltImageManager* im = cltImageManager::GetInstance();
        if (im) {
            for (int row = 0; row < static_cast<int>(m_wTileHeight2); ++row) {
                for (int col = 0; col < static_cast<int>(m_wTileWidth2); ++col) {
                    const int tIdx = col + row * static_cast<int>(m_wTileWidth2);
                    const unsigned int resID = *reinterpret_cast<unsigned int*>(
                        tileBase + 24 * tIdx + 16);
                    m_ppTileImageArray[tIdx] = im->GetGameImage(2u, resID, 0, 1);
                }
            }
        }
    }

    m_dwMapItemEffect_1 = 0;
    m_dwMapItemEffect_2 = 0;
    m_dwMapItemEffect_3 = 0;

    // mofclient.c 0x4E29A0 之前：清空副本傳送門狀態。原 binary 是
    //   ClientCharacterManager 內嵌的成員（&unk_18C4D0C），這裡走 getter。
    g_ClientCharMgr.GetInstansDungeonPortal().Init();

    // ---- 視角 / 戰場狀態 -------------------------------------------------
    if (!IsPvPMap(mapID)) {
        g_iNotInFightRoomSession = 1;
    }
    SetViewChar(pChar);

    if (mapID == cltItemKindInfo::TranslateKindCode("A0004")) {
        m_clMap_WeddingFlower.InitWeddingFlower(10.0f, 256);
    } else {
        const int range = m_iHasDrawBG ? static_cast<int>(m_wTopOffsetForBg) : 256;
        const unsigned short ck = GetClimateKind(mapID);
        m_clMap_Climate.InitClimate(m_iHasDrawBG, ck, range);
    }
    // mofclient.c 185005-185006：婚禮 / 一般地圖都呼叫一次 CUIBasic 重置婚禮 UI。
    //   原 binary 是 SetWeddingMap(flag)；C++ 版以 InitWeddingMap() 統一處理。
    if (g_UIMgr) {
        if (auto* base = static_cast<CUIBasic*>(g_UIMgr->GetUIWindow(0))) {
            base->InitWeddingMap();
        }
    }

    // ---- 場景特效物件附掛（mofclient.c：CObjectManager::AddEffectObject）
    g_ObjectManager.AddEffectObject(mapID, pChar);

    // ---- 部分地圖類型額外讀入特效圖（mofclient.c）：V/F type ∉ {2,4,5,6} 時呼叫。
    {
        const unsigned short vf = GetMapVorF(mapID);
        if (vf != 2 && vf != 4 && vf != 5 && vf != 6) {
            cltMyCharData::LoadEffectImage(&g_clMyCharData);
        }
    }
}

// ----------------------------------------------------------------------------
// SetViewChar — mofclient.c 0x4E29A0
// ----------------------------------------------------------------------------
void Map::SetViewChar(ClientCharacter* a2) {
    m_pViewChar = a2;
    if (!a2) return;

    // 1) 若角色目的格碰撞 → 找首個 portal 並重設位置。
    if (m_clMapCollisonInfo.IsCollison(
            static_cast<unsigned short>(a2->m_iDestX),
            static_cast<unsigned short>(a2->m_iDestY))) {
        if (auto* p = g_clClientPortalInfo.GetPortalInfoInMap(0)) {
            const auto wx = static_cast<unsigned short>(*((unsigned short*)p + 2) + 30);
            const auto wy = static_cast<unsigned short>(*((unsigned short*)p + 4) + 10);
            g_Network.Move(wx, wy,
                            static_cast<unsigned char>(a2->m_fMoveSpeed), 0);
            a2->m_iDestX = wx;
            a2->m_iDestY = wy;
            ClientCharacter::SetCurPosition(a2, wx, wy);
        }
    }
    // 2) 若 dest 越界 → 同樣 fallback 至首個 portal。
    if (a2->m_iDestX > m_iMapPixelWidth || a2->m_iDestY > m_iMapPixelHeight) {
        if (auto* p = g_clClientPortalInfo.GetPortalInfoInMap(0)) {
            const auto wx = static_cast<unsigned short>(*((unsigned short*)p + 2) + 30);
            const auto wy = static_cast<unsigned short>(*((unsigned short*)p + 4) + 10);
            g_Network.Move(wx, wy,
                            static_cast<unsigned char>(a2->m_fMoveSpeed), 0);
            a2->m_iDestX = wx;
            a2->m_iDestY = wy;
            ClientCharacter::SetCurPosition(a2, wx, wy);
        }
    }
    // 3) 鎖定相機初始位置在角色為中心。
    const int sysW = g_Game_System_Info.ScreenWidth;
    const int sysH = g_Game_System_Info.ScreenHeight;
    int sx = a2->m_iPosX - sysW / 2;
    if (sx < 0) sx = 0;
    if (sx > m_iMapWidthClipped) sx = m_iMapWidthClipped;
    m_iScrollX = sx;

    int sy = a2->m_iPosY - sysH / 2;
    if (sy < 0) sy = 0;
    if (sy > m_iMapHeightClipped) sy = m_iMapHeightClipped;
    m_iScrollY = sy;

    m_iClipRange0 = 0;
    m_iClipRange1 = 511;
    m_iClipRange2 = 1023;

    // mofclient.c 185092-185093：把視角角色鏡射至全域 + 通知 ObjectManager。
    g_pLastSetViewChar = m_pViewChar;
    g_ObjectManager.SetCharacter(m_pViewChar);
}

// ----------------------------------------------------------------------------
// Poll — mofclient.c 0x4E2B90：每 tick 推進相機與子系統。
// ----------------------------------------------------------------------------
void Map::Poll() {
    if (!m_pViewChar) return;

    const int sysW = g_Game_System_Info.ScreenWidth;
    int newSX;
    if (m_iWideOffset) {
        newSX = -m_iWideOffset;
    } else {
        const int half = sysW / 2;
        const int leftBound  = m_iMapPixelWidth - sysW;
        const int charX = m_pViewChar->m_iPosX;
        if (charX >= leftBound) {
            if (charX > leftBound) {
                int candidate = charX - half;
                int cap = m_iMapPixelWidth - sysW;
                newSX = (candidate < cap) ? candidate : cap;
            } else {
                newSX = m_iScrollX;  // 不變動 X
            }
        } else {
            int candidate = charX - half;
            newSX = (candidate <= 0) ? 0 : candidate;
        }
    }
    m_iScrollX = newSX;

    // Y 軸：寬螢幕不滾動，否則依角色位置與 340 為 deadzone。
    if (sysW > 800) {
        m_iScrollY = m_iMapHeightClipped;
    } else {
        const int charY = m_pViewChar->m_iPosY;
        const int diff = charY - m_iScrollY;
        int newSY = m_iScrollY;
        if (diff < 340) {
            int candidate = charY - 340;
            int floor_ = -static_cast<int>(m_wTopOffsetForBg);
            newSY = (candidate <= floor_) ? floor_ : candidate;
        } else if (diff > 340) {
            int candidate = charY - 340;
            if (candidate >= m_iMapHeightClipped) candidate = m_iMapHeightClipped;
            newSY = candidate;
        }
        m_iScrollY = newSY;
    }

    m_clMap_Climate.Poll();
    m_clMap_WeddingFlower.Poll();
    // mofclient.c：對每筆 list 元素先把「地圖靜態氣候是否啟動」傳進來，
    //   再呼叫 Poll。原 binary 的 *((_DWORD *)this + 3608) 即 cltMap_Climate::m_iActive。
    const int staticActive = m_clMap_Climate.IsActive();
    for (auto* unit : m_lstMapItemClimate) {
        if (!unit) continue;
        unit->SetActiveStaticMapClimate(staticActive);
        unit->Poll();
    }
    // mofclient.c 185189-185191
    g_clMatchManager.Poll();
    g_clPKFlagManager.Poll();
    Poll_VibrationMode(1, 0.0f, 0);
}

// ----------------------------------------------------------------------------
// Poll_VibrationMode — mofclient.c 0x4E37A0
//   螢幕震動 state machine：依 g_Vibration_* 全域與時間決定對相機 Y 軸的補償量。
// ----------------------------------------------------------------------------
namespace {
    // mofclient.c 0x6E16A4 / 0x6E16A8 / 0x6E16AC / 0x6E16B0 / 0x6E16B4：
    //   震動 state（方向／時間戳／開始時間／是否啟動／剩餘振幅）。
    int   g_VibDir       = 0;   // dword_6E16A4
    float g_VibStepTime  = 0.0f; // flt_6E16A8
    float g_VibStartTime = 0.0f; // flt_6E16AC
    int   g_VibActive    = 0;   // dword_6E16B0
    int   g_VibAmplitude = 0;   // dword_6E16B4
}

void Map::Poll_VibrationMode(int mode, float arg, int /*x*/) {
    if (mode) {
        // mode == 1：每 tick 推進震動。
        if (mode == 1 && g_VibActive == 1) {
            const double sign = g_VibDir == 1
                                  ? -static_cast<double>(g_VibAmplitude)
                                  :  static_cast<double>(g_VibAmplitude);
            const float stepDeadline = g_VibStepTime + 50.0f;
            if (static_cast<double>(timeGetTime()) > static_cast<double>(stepDeadline)) {
                g_VibDir       = (g_VibDir == 0) ? 1 : 0;
                g_VibStepTime  = static_cast<float>(timeGetTime());
                if (--g_VibAmplitude < 0) g_VibAmplitude = 0;
            }
            const float endDeadline = g_VibStartTime + 800.0f;
            if (static_cast<double>(timeGetTime()) > static_cast<double>(endDeadline)) {
                g_VibActive = 0;
            } else {
                m_iScrollY = static_cast<int>(static_cast<double>(m_iScrollY) + sign);
            }
        }
    } else {
        // mode == 0：觸發震動（玩家在 abs(charX - arg) <= 400 範圍內時）。
        if (m_pViewChar) {
            const long long dx =
                static_cast<long long>(m_pViewChar->m_iPosX) - static_cast<long long>(arg);
            const long long absDx = (dx < 0) ? -dx : dx;
            if (absDx <= 400) {
                g_VibDir       = 0;
                g_VibActive    = 1;
                g_VibStepTime  = static_cast<float>(timeGetTime());
                g_VibStartTime = static_cast<float>(timeGetTime());
                g_VibAmplitude = 4;
            }
        }
    }
}

// ----------------------------------------------------------------------------
// Drawing pipeline — mofclient.c 0x4E2CC0 (PrepareDrawing) / 0x4E3060 (DrawMap)
// / 0x4E3D00 (DrawClip)。依賴 cltImageManager / GameImage / 邊框 letterbox。
// 註：bg image 的 magic positions 來自原始 32-bit binary 的 IEEE-754 字面量。
// ----------------------------------------------------------------------------
void Map::PrepareDrawing() {
    if (!m_wTileWidth) return;

    cltImageManager* im = cltImageManager::GetInstance();
    const int sysW = g_Game_System_Info.ScreenWidth;

    // ---- 背景圖（fixed + parallax-near + middle）-------------------------
    if (m_iHasDrawBG && im) {
        if (m_iHasBg1) {
            m_pBgImage1 = im->GetGameImage(0xDu, m_dwBgResource1, 0, 1);
            m_pBgImage2 = im->GetGameImage(0xDu, m_dwBgResource1, 0, 1);
            if (m_pBgImage1) {
                m_pBgImage1->m_fPosX  = 0.0f;
                m_pBgImage1->m_fPosY  = 0.0f;
                m_pBgImage1->m_wBlockID = 0;
                m_pBgImage1->m_bFlag_447 = true;
                m_pBgImage1->m_bFlag_446 = true;
                m_pBgImage1->m_bVertexAnimation = false;
            }
            if (sysW <= 800) {
                if (m_pBgImage2) {
                    // 0x44000000 = 512.0f
                    *reinterpret_cast<unsigned int*>(&m_pBgImage2->m_fPosX) = 0x44000000u;
                    m_pBgImage2->m_fPosY  = 0.0f;
                    m_pBgImage2->m_wBlockID = 1;
                    m_pBgImage2->m_bFlag_447 = true;
                    m_pBgImage2->m_bFlag_446 = true;
                }
            } else {
                if (m_pBgImage1) {
                    // 0x3FAA0000 = 1.328125f — 縮放倍率
                    *reinterpret_cast<unsigned int*>(&m_pBgImage1->m_dwColorOp) = 0x3FAA0000u;
                    m_pBgImage1->m_bFlag_449 = true;
                    m_pBgImage1->m_bVertexAnimation = false;
                }
                if (m_pBgImage2) {
                    // 0x44400000 = 768.0f（廣螢幕第二張水平偏移）
                    *reinterpret_cast<unsigned int*>(&m_pBgImage2->m_fPosX) = 0x44400000u;
                    m_pBgImage2->m_fPosY  = 0.0f;
                    m_pBgImage2->m_wBlockID = 1;
                    m_pBgImage2->m_bFlag_447 = true;
                    m_pBgImage2->m_bFlag_446 = true;
                    m_pBgImage2->m_bVertexAnimation = false;
                    *reinterpret_cast<unsigned int*>(&m_pBgImage2->m_dwColorOp) = 0x3FAA0000u;
                    m_pBgImage2->m_bFlag_449 = true;
                }
            }
            if (m_pBgImage2) m_pBgImage2->m_bVertexAnimation = false;
        }
        if (m_iHasBg2) {
            m_pBgImageMiddle = im->GetGameImage(0xDu, m_dwBgResource2, 0, 1);
            if (m_pBgImageMiddle) {
                // mofclient.c：透過 m_pGIData->m_Resource.m_pAnimationFrames[0].width
                //   取得紋理寬，作為視差捲動分母。
                int texWidth = 0;
                if (auto* gi = m_pBgImageMiddle->GetGIDataPtr()) {
                    if (auto* frames = gi->m_Resource.m_pAnimationFrames) {
                        texWidth = frames[0].width;
                    }
                }
                if (sysW <= 800) {
                    m_iBgScrollY = static_cast<int>(static_cast<double>(
                        -(m_iScrollY + static_cast<int>(m_wTopOffsetForBg))) * 0.7692307692307692);
                } else {
                    m_iBgScrollY = 0;
                    *reinterpret_cast<unsigned int*>(&m_pBgImageMiddle->m_dwColorOp) = 0x40000000u;  // 2.0f
                    m_pBgImageMiddle->m_bFlag_449 = true;
                    m_pBgImageMiddle->m_bVertexAnimation = false;
                    texWidth *= 2;
                }
                int parallax = (m_iMapPixelWidth - 200);
                if (parallax <= 0) parallax = 1;
                m_iBgScrollX = -(m_iScrollX * (texWidth - sysW) / parallax);
                m_pBgImageMiddle->m_fPosX = static_cast<float>(m_iBgScrollX);
                m_pBgImageMiddle->m_fPosY = static_cast<float>(m_iBgScrollY);
                m_pBgImageMiddle->m_wBlockID = 0;
                m_pBgImageMiddle->m_bFlag_447 = true;
                m_pBgImageMiddle->m_bFlag_446 = true;
                m_pBgImageMiddle->m_bVertexAnimation = false;
            }
        }
    }

    // ---- tile：可見列範圍內逐格綁影像並設座標 -------------------------
    if (m_ppTileImageArray && m_pTileInfo && im
        && m_wTileWidth2 > 0 && m_wTileHeight2 > 0) {
        unsigned char* tileBase = static_cast<unsigned char*>(m_pTileInfo);
        int firstCol = m_iScrollX / 256;
        if (firstCol < 0) firstCol = 0;
        int lastCol = m_iTileDrawCount + firstCol;
        if (lastCol >= static_cast<int>(m_wTileWidth2)) lastCol = m_wTileWidth2;
        for (int row = 0; row < static_cast<int>(m_wTileHeight2); ++row) {
            const int rowPx = row << 8;
            for (int col = firstCol; col < lastCol; ++col) {
                const int tIdx = col + row * static_cast<int>(m_wTileWidth2);
                const unsigned int resID = *reinterpret_cast<unsigned int*>(
                    tileBase + 24 * tIdx + 16);
                m_ppTileImageArray[tIdx] = im->GetGameImage(2u, resID, 0, 1);
                if (auto* gi = m_ppTileImageArray[tIdx]) {
                    gi->m_wBlockID = 0;
                    gi->m_fPosX = static_cast<float>((col << 8) - m_iScrollX);
                    gi->m_fPosY = static_cast<float>(rowPx        - m_iScrollY);
                    gi->m_bFlag_447 = true;
                    gi->m_bFlag_446 = true;
                    gi->m_bVertexAnimation = false;
                }
            }
        }
    }

    // ---- 寬度 < 螢幕時的左右黑邊（letterbox）---------------------------
    if (m_iMapPixelWidth < sysW) {
        g_clAlphaClipBox_L.PrepareDrawing();
        g_clAlphaClipBox_R.PrepareDrawing();
    }
}

void Map::DrawMap() {
    if (!m_wTileWidth) return;

    if (m_iHasDrawBG) {
        if (m_iHasBg1) {
            if (m_pBgImage1) m_pBgImage1->Draw();
            if (m_pBgImage2) m_pBgImage2->Draw();
        }
        if (m_iHasBg2 && m_pBgImageMiddle) {
            m_pBgImageMiddle->Draw();
        }
    }

    if (m_ppTileImageArray && m_wTileWidth2 > 0 && m_wTileHeight2 > 0) {
        int firstCol = m_iScrollX / 256;
        if (firstCol < 0) firstCol = 0;
        int lastCol = m_iTileDrawCount + firstCol;
        if (lastCol >= static_cast<int>(m_wTileWidth2)) lastCol = m_wTileWidth2;
        for (int row = 0; row < static_cast<int>(m_wTileHeight2); ++row) {
            for (int col = firstCol; col < lastCol; ++col) {
                const int tIdx = col + row * static_cast<int>(m_wTileWidth2);
                if (auto* gi = m_ppTileImageArray[tIdx]) gi->Draw();
            }
        }
    }
}

void Map::DrawClip() {
    if (m_iMapPixelWidth < g_Game_System_Info.ScreenWidth) {
        g_clAlphaClipBox_L.Draw();
        g_clAlphaClipBox_R.Draw();
    }
}

void Map::PrepareDrawingClimate() {
    m_clMap_Climate.PrepareDrawing();
    m_clMap_WeddingFlower.PrepareDrawing();
    for (auto* unit : m_lstMapItemClimate) {
        if (unit) unit->PrepareDrawing();
    }
}

void Map::DrawClimate() {
    m_clMap_Climate.Draw();
    m_clMap_WeddingFlower.Draw();
    for (auto* unit : m_lstMapItemClimate) {
        if (unit) unit->Draw();
    }
}

void Map::GetClimateIconResouce(unsigned short mapID,
                                 unsigned int* outResId,
                                 unsigned short* outBlockId) {
    if (auto* ci = GetClimateKindByMapId(mapID)) {
        if (outResId)   *outResId   = ci->iconResId;
        if (outBlockId) *outBlockId = ci->blockId;
    }
}

// ----------------------------------------------------------------------------
// Coord conversion / scroll
// ----------------------------------------------------------------------------
int  Map::MapXtoScreenX(int x) { return x - m_iScrollX; }
int  Map::MapYtoScreenY(int y) { return y - m_iScrollY; }
int  Map::GetScrollX() const   { return m_iScrollX; }
int  Map::GetScrollY() const   { return m_iScrollY; }

// ----------------------------------------------------------------------------
// 簡單委派：cltMapInfo / cltClimateKindInfo
// ----------------------------------------------------------------------------
stMapInfo* Map::GetMapInfoByID(unsigned short mapID) {
    return m_clMapInfo.GetMapInfoByID(mapID);
}
stMapInfo* Map::GetMapInfoByIndex(int index) {
    return m_clMapInfo.GetMapInfoByIndex(index);
}
unsigned short Map::GetTotalMapNum() {
    return m_clMapInfo.GetTotalMapNum();
}
char* Map::GetMapID(unsigned short mapID) {
    return cltMapInfo::TranslateMapID(mapID);
}
unsigned short Map::GetMapAreaName(unsigned short mapID) {
    auto* info = GetMapInfoByID(mapID);
    return info ? info->m_wRegionCode : 0;
}
unsigned short Map::GetExtensionMapAreaName(unsigned short mapID) {
    auto* info = GetMapInfoByID(mapID);
    return info ? info->m_wExtRegionCode : 0;
}
char* Map::GetMapFileName(unsigned short mapID) {
    auto* info = GetMapInfoByID(mapID);
    if (!info) return nullptr;
    const char* code = cltMapInfo::TranslateMapID(info->m_wFileName);
    if (dword_829254) {
        std::snprintf(m_szMapFilePath, sizeof(m_szMapFilePath),
                       "mofdata/map/%s.map", code ? code : "");
    } else {
        std::snprintf(m_szMapFilePath, sizeof(m_szMapFilePath),
                       "MoFData\\Map\\%s.map", code ? code : "");
    }
    return m_szMapFilePath;
}
unsigned int Map::GetMapBgResourceID(unsigned short mapID) {
    auto* info = GetMapInfoByID(mapID);
    return info ? info->m_dwFixedBG : 0;
}
unsigned int Map::GetMapNearResourceID(unsigned short mapID) {
    auto* info = GetMapInfoByID(mapID);
    return info ? info->m_dwMovingBG : 0;
}
unsigned int Map::GetMapMiddleBgResourceID(unsigned short mapID) {
    auto* info = GetMapInfoByID(mapID);
    return info ? info->m_dwMiddleBG : 0;
}
unsigned int Map::GetMapAreaTitleResourceID(unsigned short mapID) {
    auto* info = GetMapInfoByID(mapID);
    return info ? info->m_dwResourceID : 0;
}
unsigned short Map::GetMapAreaTitleBlockID(unsigned short mapID) {
    auto* info = GetMapInfoByID(mapID);
    return info ? info->m_wBlockID : 0;
}
unsigned short Map::GetMapCaps(unsigned short mapID) {
    auto* info = GetMapInfoByID(mapID);
    return info ? info->m_wRegionType : 0;
}
unsigned short Map::GetMapVorF(unsigned short mapID) {
    // mofclient.c：GetMapVorF 與 GetMapCaps 讀取相同欄位 (WORD+10)。
    return GetMapCaps(mapID);
}
unsigned short Map::GetRestrictionLevel(unsigned short mapID) {
    auto* info = GetMapInfoByID(mapID);
    return info ? info->m_wLevelRequired : 0;
}
unsigned int Map::GetDrawBG(unsigned short mapID) {
    auto* info = GetMapInfoByID(mapID);
    return info ? info->m_BG : 0;
}
char* Map::GetBGMFileName(unsigned short mapID) {
    auto* info = GetMapInfoByID(mapID);
    if (!info) return nullptr;
    const char* base = (timeGetTime() & 1u) ? info->m_szBGM1 : info->m_szBGM2;
    if (dword_829254) {
        std::snprintf(m_szBGMFilePath, sizeof(m_szBGMFilePath),
                       "mofdata/music/%s", base);
    } else {
        std::snprintf(m_szBGMFilePath, sizeof(m_szBGMFilePath),
                       "MoFData\\music\\%s", base);
    }
    return m_szBGMFilePath;
}
char* Map::GetAmbientFileName(unsigned short mapID) {
    auto* info = GetMapInfoByID(mapID);
    if (!info) return nullptr;
    if (dword_829254) {
        std::snprintf(m_szAmbientPath, sizeof(m_szAmbientPath),
                       "mofdata/music/%s", info->m_szAmbientSoundFile);
    } else {
        std::snprintf(m_szAmbientPath, sizeof(m_szAmbientPath),
                       "MoFData\\music\\%s", info->m_szAmbientSoundFile);
    }
    return m_szAmbientPath;
}
unsigned short Map::GetClimateKind(unsigned short mapID) {
    auto* info = GetMapInfoByID(mapID);
    return info ? info->m_wClimate : 0;
}
bool Map::IsMatchLobbyMap(unsigned short mapID) {
    // mofclient.c: *((_DWORD *)result + 76) = byte 304 = m_dwCanOpenPK。
    auto* info = GetMapInfoByID(mapID);
    return info && info->m_dwCanOpenPK != 0;
}
bool Map::IsPvPMap(unsigned short mapID) {
    auto* info = GetMapInfoByID(mapID);
    return info && info->m_dwPKField != 0;
}
bool Map::IsPvPLobbyMap(unsigned short mapID) {
    auto* info = GetMapInfoByID(mapID);
    return info && info->m_dwPKLobby != 0;
}
bool Map::IsContinent(unsigned short mapID) {
    auto* info = GetMapInfoByID(mapID);
    return info && info->m_byteContinentType != 0;
}
strClimateInfo* Map::GetClimateKindByMapId(unsigned short mapID) {
    return GetClimateKindByClimateKind(GetClimateKind(mapID));
}
strClimateInfo* Map::GetClimateKindByClimateKind(unsigned short kind) {
    return m_clMapInfo.cltClimateKindInfo::GetClimateKindInfo(kind);
}

// ----------------------------------------------------------------------------
// Map item climate — mofclient.c 0x4E3930 / 4E3BB0 / 4E3C50 / 4E3CA0 / 4E3CF0
// ----------------------------------------------------------------------------
// mofclient.c 透過 stItemKindInfo +210 (uint16) 找出 mapuse 編號；
// stItemKindInfo 沒有顯式對應命名，這裡以 raw offset 存取。
static unsigned short ItemKindInfoMapUseID(const void* it) {
    return *((const unsigned short*)it + 105);
}

void Map::StartMapItemClimate(unsigned short itemID, unsigned int duration, char* userName) {
    m_dwMapItemEffect_1 = 0;
    m_dwMapItemEffect_2 = 0;
    m_dwMapItemEffect_3 = 0;
    auto* it = g_clItemKindInfo.GetItemKindInfo(itemID);
    if (!it) return;
    const unsigned short muKind = ItemKindInfoMapUseID(it);
    auto* mu = g_clMapUseItemInfoKindInfo.GetMapUseItemInfoKindInfo(muKind);
    if (!mu) return;

    // mofclient.c：將三組旗標寫入 m_dwMapItemEffect_*（Poll 讀取以判斷氣候是否靜態）。
    //   *((_DWORD *)v7 + 19/20/21) → byte 76/80/84，#pragma pack(1) 下對應：
    m_dwMapItemEffect_1 = mu->EvasionPermille;    // byte 76
    m_dwMapItemEffect_2 = mu->AccuracyPermille;   // byte 80
    m_dwMapItemEffect_3 = mu->CriticalPermille;   // byte 84

    // 檢查 list 是否已有同 mapUseKind 的項目；若有就不重複建立。
    for (auto* node : m_lstMapItemClimate) {
        if (node && node->GetUseMapItemClimateKind() == muKind) {
            return;
        }
    }

    // 主氣候單元
    auto* main = new CMap_Item_Climate();
    if (main->InitMapItemClimate(muKind) == 1) {
        main->SetSubMapItemClimate(0);
        main->SetMapItemClimateKind(itemID);
        main->SetMapItemClimateRemainTime(duration);
        main->SetMapItemClimateUserName(userName);
        main->SetTimerID();
        m_lstMapItemClimate.push_back(main);
    } else {
        delete main;
    }

    // 子氣候單元（依 SubItemCount 與 SubItemID 各建立一筆）
    for (uint32_t s = 0; s < mu->SubItemCount && s < 5; ++s) {
        const uint16_t subKind = mu->SubItemID[s];
        if (!subKind) continue;
        auto* sub = new CMap_Item_Climate();
        if (sub->InitMapItemClimate(subKind) == 1) {
            sub->SetSubMapItemClimate(1);
            sub->SetMapItemClimateKind(0);
            sub->SetMapItemClimateRemainTime(0);
            sub->SetMapItemClimateUserName(nullptr);
            m_lstMapItemClimate.push_back(sub);
        } else {
            delete sub;
        }
    }
}

void Map::EndMapItemClimate(unsigned short itemID) {
    auto* it = g_clItemKindInfo.GetItemKindInfo(itemID);
    if (!it) return;
    auto* mu = g_clMapUseItemInfoKindInfo.GetMapUseItemInfoKindInfo(
                   ItemKindInfoMapUseID(it));
    if (!mu) return;
    if (mu->EvasionPermille)  m_dwMapItemEffect_1 = 0;
    if (mu->CriticalPermille) m_dwMapItemEffect_3 = 0;

    // mofclient.c：清空整條 list（不只是命中的項目）。
    DeleteAllMapItemClimate();
}

void Map::DeleteAllMapItemClimate() {
    for (auto* node : m_lstMapItemClimate) {
        if (node) {
            node->Free();
            delete node;
        }
    }
    m_lstMapItemClimate.clear();
}

// mofclient.c 0x4E3CA0：依 index 回傳第 N 個「非 sub」的氣候單元。
CMap_Item_Climate* Map::GetMapItemClimate(int index) {
    int counter = 0;
    for (auto* node : m_lstMapItemClimate) {
        if (!node || node->IsSubMapItemClimate()) continue;
        if (counter == index) return node;
        ++counter;
    }
    return nullptr;
}

int Map::GetMapItemClimateNumber() {
    return static_cast<int>(m_lstMapItemClimate.size());
}
