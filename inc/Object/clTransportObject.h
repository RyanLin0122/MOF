#pragma once

#include <cstdint>

class GameImage;
class ClientCharacter;
class CCA;
struct stTransportKindInfo;
class clTransportAniInfo;

// clTransportObject — 交通工具(坐騎)物件
// 大小 60 bytes (15 DWORDs)
class clTransportObject {
public:
    clTransportObject();
    virtual ~clTransportObject();

    void Release();
    void InitTransport(ClientCharacter* pOwner, CCA* pCCA, std::uint16_t transportKind);
    void SetActive(int active);
    int  GetActive();

    // mofclient.c：clTransportObject 物件首兩個 byte 即為當前坐騎 kind code，
    // ClientCharacterManager::SetTransport 會比對「請求的 kind ≠ 當前 kind」
    // 來決定是 InitTransport 換騎還是 SetActive(0) 下馬。
    std::uint16_t GetTransportKind() const { return m_wKind; }
    void PrepareDrawing(int param);
    void DrawUp(int param);
    void DrawDown(int param);
    void TransportMove();
    void TransportStop();
    void TransportHitted();
    void UpdatePosition();
    void DecideDrawFrame(unsigned int* outResUp, unsigned int* outResDown,
                         std::uint16_t* outFrameUp, std::uint16_t* outFrameDown);
    void Poll();
    void SetActionState(unsigned int actionState);

private:
    // mofclient.c：clTransportObject 物件第 0、1 byte 是 kind code（IDA 將它
    // 直接當 *v7 比對使用）。本還原把它顯式儲存於 m_wKind，由 InitTransport
    // 寫入；上層 SetTransport 透過 GetTransportKind() 比對。
    std::uint16_t        m_wKind{ 0 };     // (邏輯上對應 decomp 物件 +0)
    std::uint16_t        _padKind{ 0 };
    GameImage*           m_pImageUp;       // DWORD offset 1 (byte 4)
    GameImage*           m_pImageDown;     // DWORD offset 2 (byte 8)
    ClientCharacter*     m_pOwner;         // DWORD offset 3 (byte 12)
    clTransportAniInfo*  m_pAniInfoUp;     // DWORD offset 4 (byte 16)
    clTransportAniInfo*  m_pAniInfoDown;   // DWORD offset 5 (byte 20)
    stTransportKindInfo* m_pKindInfo;      // DWORD offset 6 (byte 24)
    CCA*                 m_pCCA;           // DWORD offset 7 (byte 28)
    int                  _reserved8;       // DWORD offset 8 (byte 32)
    int                  m_nActive;        // DWORD offset 9 (byte 36)
    std::uint16_t        m_wTotalFrameUp;  // WORD offset 20 (byte 40)
    std::uint16_t        m_wTotalFrameDown;// WORD offset 21 (byte 42)
    std::uint16_t        m_wCurFrameUp;    // WORD offset 22 (byte 44)
    std::uint16_t        m_wCurFrameDown;  // WORD offset 23 (byte 46)
    int                  m_nPosX;          // DWORD offset 12 (byte 48)
    int                  m_nPosY;          // DWORD offset 13 (byte 52)
    unsigned int         m_dwActionState;  // DWORD offset 14 (byte 56)
};
