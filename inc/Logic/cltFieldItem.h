#pragma once

#include <cstdint>

class Map;
class GameImage;
class cltItemKindInfo;

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
