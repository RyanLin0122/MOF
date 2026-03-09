#pragma once

#include <cstdint>

struct Map;
struct GameImage;
struct cltItemKindInfo;

class cltFieldItem {
public:
  static cltItemKindInfo* m_pclItemKindInfo;
  static Map* m_pMap;

  static void InitializeStaticVariable(cltItemKindInfo* itemKindInfo, Map* map);

  cltFieldItem();
  ~cltFieldItem();

  int Init(uint16_t fieldItemID, int x, int y, int itemCount, uint16_t itemID, uint16_t itemInfo, uint8_t moveType);
  void Free();
  void Poll();
  void PrepareDrawing();
  void Draw();
  bool IsUsing();
  void GetItemInfo(uint16_t* fieldItemID, int* x, int* y, int* itemCount, uint16_t* itemID, uint16_t* itemInfo);
  void GetItemInfo(int* itemCount, uint16_t* itemID, uint16_t* itemInfo);
  uint16_t GetFieldItemID();

private:
  uint8_t m_raw[0x60];
};

struct cltFieldItemBufferEntry {
  uint32_t accountID;
  uint16_t fieldItemID;
  uint16_t _pad;
  int itemCount;
  uint16_t itemID;
  uint16_t itemInfo;
  uint8_t moveType;
  uint8_t _pad2[3];
};

class cltFieldItemManager {
public:
  cltFieldItemManager();
  ~cltFieldItemManager();

  void Free();
  void AddItem(uint16_t fieldItemID, int x, int y, int itemCount, uint16_t itemID, uint16_t itemInfo, uint8_t moveType);
  void DelItem(uint32_t accountID, uint16_t fieldItemID, uint16_t itemInfo, int isPetPickup);
  void DelItem(uint16_t fieldItemID, uint16_t itemInfo);
  void Poll();
  void PrepareDrawing();
  void Draw();
  int GetNearItemInfo(float x, float y, uint16_t* outFieldItemID, float* outX, float* outY, int checkPet, int rangeX, int rangeY);
  void SetSysMsg(int textID);
  cltFieldItem* GetFieldItem(uint16_t fieldItemID);
  int DelTempBufferItem(uint16_t fieldItemID);
  void PushBuffer(uint32_t accountID, uint16_t fieldItemID, int itemCount, uint16_t itemID, uint16_t itemInfo, uint8_t moveType);
  void PopBuffer(uint32_t accountID);

private:
  cltFieldItem m_items[1000];
  cltFieldItemBufferEntry m_tempBuffer[200];
  uint8_t m_tempBufferCount;
  uint8_t m_pad100001;
  uint16_t m_activeItemCount;
  uint16_t m_cachedVisibleCount;
  uint16_t m_pad100006;
  uint32_t m_lastSysMsgTick;
  cltFieldItem* m_cachedVisibleItems[1000];
};
