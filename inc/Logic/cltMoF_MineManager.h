#pragma once
#include <cstdint>
#include "Info/cltMineKindInfo.h"
#include "Logic/cltMoF_ClientMine.h"

class cltMoF_MineManager {
public:
    cltMoF_MineManager();
    virtual ~cltMoF_MineManager();

    int  Initialize(const char* filename);
    void AddMine(unsigned int handle, uint16_t mineKind);
    void DeleteAllMine();
    void DeleteMineByHandle(unsigned int handle, uint16_t effectKind);
    void Poll();
    void PrepareDrawing();
    void Draw();

    cltMoF_ClientMine* GetMineByHandel(unsigned int handle);
    strMineKindInfo*   GetMineKindInfoByKind(uint16_t kind);
    void               ResetSearched(unsigned int handle);

    static constexpr int MAX_MINES = 50;

private:
    cltMineKindInfo    m_mineKindInfo;             // offset 4
    cltMoF_ClientMine  m_mines[MAX_MINES];         // offset 12
};
