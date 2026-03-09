#include "cltFieldItem.h"

#include <cstring>

struct cltMapCollisonInfo;
struct cltImageManager;
struct ClientCharacterManager;
struct CEffectManager;
struct GameSound;
struct DCTTextManager;
struct cltSystemMessage;
struct cltPetSystem;
struct cltBaseInventory;
struct ClientCharacter;
struct CEffect_Field_ItemPick;
struct CEffect_Field_Pet_ItemPick;
struct D3DXVECTOR2;

extern "C" int _rand();
extern "C" int _wsprintfA(char* out, const char* fmt, ...);
extern "C" int MessageBoxA(void*, const char*, const char*, unsigned int);
extern "C" unsigned int timeGetTime();

extern int SETTING_FRAME;
extern int* g_nVariableY;
extern cltImageManager g_clImageManager;
extern ClientCharacterManager g_ClientCharMgr;
extern CEffectManager g_EffectManager_After_Chr;
extern GameSound g_GameSoundManager;
extern DCTTextManager g_DCTTextManager;
extern cltSystemMessage g_clSysemMessage;
extern cltPetSystem unk_21C2C9C;
extern cltBaseInventory unk_21B8F04;

namespace {
inline uint16_t& W(cltFieldItem* p, int off) { return *reinterpret_cast<uint16_t*>(reinterpret_cast<uint8_t*>(p) + off); }
inline int& D(cltFieldItem* p, int off) { return *reinterpret_cast<int*>(reinterpret_cast<uint8_t*>(p) + off); }
inline uint8_t& B(cltFieldItem* p, int off) { return *reinterpret_cast<uint8_t*>(reinterpret_cast<uint8_t*>(p) + off); }
inline float& F(cltFieldItem* p, int off) { return *reinterpret_cast<float*>(reinterpret_cast<uint8_t*>(p) + off); }
}

cltItemKindInfo* cltFieldItem::m_pclItemKindInfo = nullptr;
Map* cltFieldItem::m_pMap = nullptr;

void cltFieldItem::InitializeStaticVariable(cltItemKindInfo* itemKindInfo, Map* map) {
  m_pclItemKindInfo = itemKindInfo;
  m_pMap = map;
}

cltFieldItem::cltFieldItem() {
  D(this, 4) = 0;
  D(this, 8) = 0;
  W(this, 48) = 0;
  D(this, 72) = 0;
  D(this, 76) = 0;
  D(this, 92) = 0;
}

cltFieldItem::~cltFieldItem() = default;

int cltFieldItem::Init(uint16_t fieldItemID, int x, int y, int itemCount, uint16_t itemID, uint16_t itemInfo, uint8_t moveType) {
  if (itemCount * itemID) return 0;
  int v10 = 0;
  if (itemID) {
    auto* info = cltItemKindInfo::GetItemKindInfo(m_pclItemKindInfo, itemID);
    if (!info) return 0;
    D(this, 52) = *(reinterpret_cast<int*>(info) + 7);
    W(this, 56) = *(reinterpret_cast<uint16_t*>(info) + 16);
  } else {
    D(this, 52) = 402653686;
    W(this, 56) = 57;
  }
  W(this, 0) = fieldItemID;
  D(this, 12) = x;
  D(this, 4) = x;
  D(this, 36) = itemCount;
  D(this, 16) = y;
  D(this, 8) = y;
  W(this, 48) = itemID;
  W(this, 50) = itemInfo;
  D(this, 64) = 1;
  int r = _rand();
  int v12 = D(this, 36);
  D(this, 80) = 2;
  D(this, 84) = 4;
  int v13 = 2 * (r % 10);
  D(this, 40) = v13 + 90;
  B(this, 68) = moveType;
  B(this, 88) = static_cast<uint8_t>(v13 - 86);
  if (v12) v10 = 30;

  switch (moveType) {
    case 0:
      D(this, 44) = 0;
      W(this, 28) = 0;
      D(this, 20) = D(this, 12) - v10 + 80;
      F(this, 32) = static_cast<float>((80 - v10) / 30);
      return 1;
    case 1:
      D(this, 20) = D(this, 12) + v10 - 80;
      F(this, 32) = static_cast<float>((v10 - 80) / 30);
      D(this, 44) = 360;
      W(this, 28) = 0;
      return 1;
    case 2:
      D(this, 32) = 0;
      D(this, 44) = 0;
      D(this, 20) = D(this, 12) - v10;
      W(this, 28) = 0;
      return 1;
    case 3:
      D(this, 44) = 0;
      W(this, 28) = 0;
      D(this, 20) = D(this, 12) - v10 + 120;
      F(this, 32) = static_cast<float>((120 - v10) / 30);
      return 1;
    case 4:
      D(this, 20) = D(this, 12) + v10 - 120;
      F(this, 32) = static_cast<float>((v10 - 120) / 30);
      D(this, 44) = 360;
      W(this, 28) = 0;
      return 1;
    default:
      W(this, 28) = 0;
      return 1;
  }
}

void cltFieldItem::Free() {
  W(this, 0) = 0;
  D(this, 4) = 0;
  D(this, 8) = 0;
  W(this, 48) = 0;
  W(this, 50) = 0;
  D(this, 64) = 0;
  D(this, 72) = 0;
  D(this, 76) = 0;
}

void cltFieldItem::Poll() {
  int y = D(this, 8);
  int frame = D(this, 92) + 1;
  D(this, 92) = frame;
  if (frame < SETTING_FRAME) return;
  int moving = D(this, 64);
  D(this, 92) = 0;

  if (moving) {
    float vx = F(this, 32);
    int x = D(this, 4) + static_cast<int>(vx);
    if (cltMapCollisonInfo::IsCollison(reinterpret_cast<Map*>(reinterpret_cast<char*>(m_pMap) + 19776), W(this, 4) + static_cast<int>(vx), y)) {
      vx = F(this, 32) * -1.0f;
      F(this, 32) = vx;
      x += static_cast<int>(vx);
    }
    int y2 = D(this, 16);
    uint16_t top = static_cast<uint16_t>(W(this, 16) + static_cast<uint16_t>(g_nVariableY[W(this, 28)]));
    if (y2 >= top) y2 = top;
    if (!cltMapCollisonInfo::IsCollison(reinterpret_cast<Map*>(reinterpret_cast<char*>(m_pMap) + 19776), x, y2)) {
      D(this, 4) = x;
      D(this, 8) = y2;
    }
    uint16_t tick = ++W(this, 28);
    if (D(this, 64)) {
      if (B(this, 68)) {
        if (B(this, 68) == 1) {
          D(this, 44) = D(this, 44) - 24;
        }
      } else {
        D(this, 44) = D(this, 44) + 24;
      }
    }
    if (tick > 5) {
      if (tick <= 10) D(this, 40) += 20;
      else if (tick <= 15) D(this, 40) -= 20;
      else if (tick >= 20) D(this, 40) = 100;
    } else {
      D(this, 40) = 100;
    }
    if (tick >= 30) D(this, 64) = 0;
  }

  if (D(this, 72) == 1) {
    int a = D(this, 40) - 10;
    D(this, 40) = a;
    if (a <= 0) {
      D(this, 40) = 0;
      D(this, 76) = 1;
    }
  } else {
    int delta = D(this, 80);
    int alpha = delta + D(this, 40);
    uint8_t c = static_cast<uint8_t>(B(this, 84) + B(this, 88));
    D(this, 40) = alpha;
    B(this, 88) = c;
    if (alpha >= 110 || alpha <= 90) D(this, 80) = -delta;
    if (c >= 0xFB || c <= 0xAA) D(this, 84) = -D(this, 84);
  }
}

void cltFieldItem::PrepareDrawing() {
  char text[256];
  auto* img = cltImageManager::GetGameImage(&g_clImageManager, 3u, D(this, 52), 0, 1);
  D(this, 60) = reinterpret_cast<int>(img);
  if (!img) return;

  int frameInfo = *(reinterpret_cast<int*>(img) + 2);
  int w = 0, h = 0;
  uint16_t idx = W(this, 56);
  if (frameInfo) {
    if (*reinterpret_cast<uint16_t*>(frameInfo + 28) < idx) {
      _wsprintfA(text, "%s:%i", reinterpret_cast<const char*>(frameInfo + 52), idx);
      MessageBoxA(nullptr, text, "Block Error", 0);
    }
    int table = *reinterpret_cast<int*>(*(reinterpret_cast<int*>(img) + 2) + 32);
    w = *reinterpret_cast<int*>(table + 52 * idx + 20);
    h = *reinterpret_cast<int*>(table + 52 * idx + 24);
  }

  int y = D(this, 8) - h;
  int image = D(this, 60);
  float fy = static_cast<float>(y - *(reinterpret_cast<int*>(m_pMap) + 4899));
  float fx = static_cast<float>((D(this, 4) - w / 2) - *(reinterpret_cast<int*>(m_pMap) + 4898));
  *reinterpret_cast<uint16_t*>(image + 372) = W(this, 56);
  *reinterpret_cast<uint8_t*>(image + 447) = 1;
  *reinterpret_cast<uint8_t*>(image + 446) = 1;
  *reinterpret_cast<uint8_t*>(image + 444) = 0;
  *reinterpret_cast<float*>(image + 332) = fx;
  *reinterpret_cast<float*>(image + 336) = fy;

  *reinterpret_cast<int*>(image + 376) = D(this, 40);
  *reinterpret_cast<uint8_t*>(image + 449) = 1;
  *reinterpret_cast<uint8_t*>(image + 444) = 0;

  *reinterpret_cast<int*>(image + 388) = D(this, 44);
  *reinterpret_cast<uint8_t*>(image + 448) = 1;
  *reinterpret_cast<uint8_t*>(image + 444) = 0;

  *reinterpret_cast<int*>(image + 380) = B(this, 88);
  *reinterpret_cast<uint8_t*>(image + 450) = 1;
  *reinterpret_cast<uint8_t*>(image + 444) = 0;
}

void cltFieldItem::Draw() {
  auto* img = reinterpret_cast<GameImage*>(D(this, 60));
  if (*(reinterpret_cast<int*>(img) + 2)) GameImage::Draw(img);
}

bool cltFieldItem::IsUsing() { return W(this, 0) != 0; }

void cltFieldItem::GetItemInfo(uint16_t* fieldItemID, int* x, int* y, int* itemCount, uint16_t* itemID, uint16_t* itemInfo) {
  *fieldItemID = W(this, 0);
  *x = D(this, 4);
  *y = D(this, 8);
  *itemCount = D(this, 36);
  *itemID = W(this, 48);
  *itemInfo = W(this, 50);
}

void cltFieldItem::GetItemInfo(int* itemCount, uint16_t* itemID, uint16_t* itemInfo) {
  *itemCount = D(this, 36);
  *itemID = W(this, 48);
  *itemInfo = W(this, 50);
}

uint16_t cltFieldItem::GetFieldItemID() { return W(this, 0); }

cltFieldItemManager::cltFieldItemManager() {
  for (auto& i : m_items) new (&i) cltFieldItem();
  for (auto& e : m_tempBuffer) e.moveType = 0;
  m_tempBufferCount = 0;
  std::memset(m_tempBuffer, 0, sizeof(m_tempBuffer));
  m_activeItemCount = 0;
}

cltFieldItemManager::~cltFieldItemManager() {
  Free();
  for (auto& i : m_items) i.~cltFieldItem();
}

void cltFieldItemManager::Free() {
  for (auto& i : m_items) i.Free();
  m_activeItemCount = 0;
  std::memset(m_tempBuffer, 0, sizeof(m_tempBuffer));
  m_tempBufferCount = 0;
}

void cltFieldItemManager::AddItem(uint16_t fieldItemID, int x, int y, int itemCount, uint16_t itemID, uint16_t itemInfo, uint8_t moveType) {
  int idx = 0;
  for (; m_items[idx].IsUsing(); ++idx) if (idx >= 999) return;
  if (m_items[idx].Init(fieldItemID, x, y, itemCount, itemID, itemInfo, moveType)) ++m_activeItemCount;
}

void cltFieldItemManager::DelItem(uint32_t accountID, uint16_t fieldItemID, uint16_t, int isPetPickup) {
  if (DelTempBufferItem(fieldItemID)) return;
  auto* item = reinterpret_cast<int*>(GetFieldItem(fieldItemID));
  if (!item) return;
  auto* chr = ClientCharacterManager::GetCharByAccount(&g_ClientCharMgr, accountID);
  if (!chr) return;

  float pos[2] = {static_cast<float>(item[5]), static_cast<float>(item[4])};
  CEffect_Field_Pet_ItemPick* fx = nullptr;
  if (isPetPickup) {
    auto* p = static_cast<CEffect_Field_Pet_ItemPick*>(operator new(0xC4u));
    if (p) fx = CEffect_Field_Pet_ItemPick::CEffect_Field_Pet_ItemPick(p);
    CEffect_Field_Pet_ItemPick::SetEffect(fx, reinterpret_cast<D3DXVECTOR2*>(pos), accountID);
  } else {
    auto* p = static_cast<CEffect_Field_ItemPick*>(operator new(0xC4u));
    if (p) fx = CEffect_Field_ItemPick::CEffect_Field_ItemPick(reinterpret_cast<CEffect_Field_Pet_ItemPick*>(p));
    CEffect_Field_ItemPick::SetEffect(reinterpret_cast<CEffect_Field_ItemPick*>(fx), reinterpret_cast<D3DXVECTOR2*>(pos), accountID);
  }
  CEffectManager::BulletAdd(&g_EffectManager_After_Chr, fx);
  GameSound::PlaySoundA(&g_GameSoundManager, "J0013", *(reinterpret_cast<int*>(chr) + 1096), *(reinterpret_cast<int*>(chr) + 1097));
  reinterpret_cast<cltFieldItem*>(item)->Free();
  --m_activeItemCount;
}

void cltFieldItemManager::DelItem(uint16_t fieldItemID, uint16_t) {
  if (DelTempBufferItem(fieldItemID)) return;
  auto* item = GetFieldItem(fieldItemID);
  if (item) {
    D(item, 72) = 1;
    D(item, 76) = 0;
  }
}

void cltFieldItemManager::Poll() {
  m_cachedVisibleCount = 0;
  for (int i = 0; i < 1000; ++i) {
    auto* it = &m_items[i];
    if (it->IsUsing()) {
      m_cachedVisibleItems[m_cachedVisibleCount++] = it;
      it->Poll();
      if (D(it, 76) == 1) {
        it->Free();
        --m_activeItemCount;
      }
    }
  }
}

void cltFieldItemManager::PrepareDrawing() { for (auto& i : m_items) if (i.IsUsing()) i.PrepareDrawing(); }
void cltFieldItemManager::Draw() { for (auto& i : m_items) if (i.IsUsing()) i.Draw(); }

int cltFieldItemManager::GetNearItemInfo(float x, float y, uint16_t* outFieldItemID, float* outX, float* outY, int checkPet, int rangeX, int rangeY) {
  int best = 2400;
  int found = 0;
  int rx = rangeX ? rangeX : 200;
  int ry = rangeY ? rangeY : 150;

  for (int i = 0; i < 1000; ++i) {
    auto* it = &m_items[i];
    if (D(it, 72) == 1 || !it->IsUsing()) continue;

    uint16_t fid, iid, iinfo;
    int ix, iy, cnt;
    it->GetItemInfo(&fid, &ix, &iy, &cnt, &iid, &iinfo);
    int dx = abs32(static_cast<__int64>(x) - ix);
    int dy = abs32(static_cast<__int64>(y) - iy);
    if (dx > rx || dy > ry) continue;

    if (iid) {
      if (checkPet) {
        auto* petInv = cltPetSystem::GetPetInventorySystem(&unk_21C2C9C);
        int can = cltPetInventorySystem::CanAddItem(petInv, iid, iinfo);
        if (can == 1904) { SetSysMsg(8106); continue; }
        if (can) { SetSysMsg(8105); continue; }
      } else if (cltBaseInventory::CanAddInventoryItem(&unk_21B8F04, iid, iinfo)) {
        SetSysMsg(8104);
        continue;
      }
    }

    if (best > dx + dy) {
      best = dx + dy;
      *outFieldItemID = fid;
      *outX = static_cast<float>(ix);
      *outY = static_cast<float>(iy);
      found = 1;
    }
  }
  return found;
}

void cltFieldItemManager::SetSysMsg(int textID) {
  if (timeGetTime() - m_lastSysMsgTick > 0x1388) {
    m_lastSysMsgTick = timeGetTime();
    char* txt = DCTTextManager::GetText(&g_DCTTextManager, textID);
    cltSystemMessage::SetSystemMessage(&g_clSysemMessage, txt, 0, 0xFFFEEA00, 0);
  }
}

cltFieldItem* cltFieldItemManager::GetFieldItem(uint16_t fieldItemID) {
  for (int i = 0; i < 1000; ++i) {
    if (m_items[i].IsUsing() && m_items[i].GetFieldItemID() == fieldItemID && D(&m_items[i], 72) != 1) return &m_items[i];
  }
  return nullptr;
}

int cltFieldItemManager::DelTempBufferItem(uint16_t fieldItemID) {
  int i = 0;
  for (; m_tempBuffer[i].fieldItemID != fieldItemID; ++i) if (i >= 200) return 0;
  std::memset(&m_tempBuffer[i], 0, sizeof(m_tempBuffer[i]));
  --m_tempBufferCount;
  return 1;
}

void cltFieldItemManager::PushBuffer(uint32_t accountID, uint16_t fieldItemID, int itemCount, uint16_t itemID, uint16_t itemInfo, uint8_t moveType) {
  if (m_tempBufferCount <= 0xC8 && accountID) {
    int i = 0;
    for (; m_tempBuffer[i].accountID; ++i) if (i >= 200) return;
    m_tempBuffer[i].accountID = accountID;
    m_tempBuffer[i].fieldItemID = fieldItemID;
    m_tempBuffer[i].itemCount = itemCount;
    m_tempBuffer[i].itemID = itemID;
    m_tempBuffer[i].itemInfo = itemInfo;
    m_tempBuffer[i].moveType = moveType;
    ++m_tempBufferCount;
  }
}

void cltFieldItemManager::PopBuffer(uint32_t accountID) {
  if (!accountID) return;
  auto* chr = ClientCharacterManager::GetCharByAccount(&g_ClientCharMgr, accountID);
  if (!chr) return;

  for (int i = 0; i < 200; ++i) {
    auto& e = m_tempBuffer[i];
    if (e.accountID == accountID) {
      AddItem(
        e.fieldItemID,
        static_cast<uint16_t>(*(reinterpret_cast<int*>(chr) + 1096)),
        static_cast<uint16_t>(*(reinterpret_cast<int*>(chr) + 1097)),
        e.itemCount,
        e.itemID,
        e.itemInfo,
        e.moveType);
      --m_tempBufferCount;
      std::memset(&e, 0, sizeof(e));
    }
  }
}
