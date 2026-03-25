#pragma once
#include <cstdint>

class ClientCharacter;
class GameImage;

// cltMoF_ClientMine — 從 mofclient.c 反推的單顆地雷物件。
// cltMoF_MineManager 內含 50 個此物件 (each 104 bytes / 0x68)。
class cltMoF_ClientMine {
public:
    cltMoF_ClientMine();
    ~cltMoF_ClientMine();

    void CreateMine(unsigned int handle,
                    int hp, int maxHp,
                    int range, int maxRange,
                    unsigned int disappearMs,
                    unsigned int resourceIdHex,
                    uint16_t totalFlame,
                    char airGround,
                    char attackCountType,
                    char moveType);

    void DeleteMine();
    int  GetActive();
    void ResetSearched();
    void Poll();
    void AttackOneMine();
    void AttackMultiMine();
    void SearchAndMove();
    void PrepareDrawing();
    void Draw();
    int  IsValidSearch(ClientCharacter* pChar);
    int  IsValidAttack(ClientCharacter* pChar);

    // 用於外部存取
    unsigned int GetHandle() const { return m_handle; }
    float GetScreenX() const { return m_fScreenX; }
    float GetScreenY() const { return m_fScreenY; }

private:
    int          m_nActive = 0;
    unsigned int m_handle = 0;
    int          m_hp = 0;
    int          m_maxHp = 0;
    int          m_range = 0;
    int          m_maxRange = 0;
    unsigned int m_disappearMs = 0;
    unsigned int m_resourceIdHex = 0;
    uint16_t     m_totalFlame = 0;
    char         m_airGround = 0;
    char         m_attackCountType = 0;
    char         m_moveType = 0;
    int          m_bSearched = 0;
    unsigned int m_hTimer = 0;
    float        m_fScreenX = 0.0f;
    float        m_fScreenY = 0.0f;
    GameImage*   m_pImage = nullptr;
};
