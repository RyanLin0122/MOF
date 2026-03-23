#pragma once
#include <cstdint>
#include "UI/CControlChatBallon.h"
#include "UI/CControlAlphaBox.h"

struct strPetKindInfo;
struct GameImage;
struct stEffectKindInfo;
class ClientCharacter;
class CCA;
class cltPetAniInfo;
class CEffect_Pet_Base;

class cltPetObject {
public:
    cltPetObject();
    virtual ~cltPetObject();

    void Release();
    void SetActive(int active);
    int  GetActive();

    void PetLevelUP(uint16_t petKind);
    void InitPet(ClientCharacter* owner, CCA* cca, uint16_t petKind, int dyeFlag, int hiddenFlag);

    void CreatePetEffect();
    void DeleteEffect();

    void DyePet(uint16_t petKind, int hiddenFlag);
    void SetPetName(char* name);

    void Poll();
    void PrepareDrawing(int speechFlag, int forceShow);
    void Draw(int forceShow);

    void MoveTrace();
    void MovePatrol();
    void DecideDrawFrame(unsigned int* outResId, uint16_t* outFrame);

    void SetNearItemInfo(int itemCount, uint16_t itemKind, uint16_t itemQty);
    void RequestPickUpItem();
    void PickUpItem();

    void StartHidingSelf();
    void FinishHidingSelf();

    uint8_t IsDrawingOrder();
    bool    IsBasePet();

    // Accessors used externally
    int  GetPosX() const { return m_nPosX; }
    int  GetPosY() const { return m_nPosY; }

private:
    GameImage* m_pGameImage;       // DWORD 1
    ClientCharacter* m_pOwnerChar;       // DWORD 2
    cltPetAniInfo* m_pAniInfo;         // DWORD 3
    CCA* m_pCCA;             // DWORD 4

    CControlChatBallon m_chatBallon;      // embedded at byte 20

    strPetKindInfo* m_pPetKindInfo;     // DWORD 672

    CControlAlphaBox   m_alphaBox;        // embedded at byte 2692

    GameImage* m_pNameBgLeft;      // DWORD 724
    GameImage* m_pNameBgCenter;    // DWORD 725
    GameImage* m_pNameBgRight;     // DWORD 726
    GameImage* m_pNameBgExtra;     // DWORD 727
    CEffect_Pet_Base* m_pEffect;          // DWORD 728
    int               m_nActive;          // DWORD 729

    char              m_szPetName[80];    // starts at byte 2920

    int               m_nDyeFlag;         // DWORD 733

    uint16_t          m_wNameBoxW;        // WORD 1468 (name box width indicator)
    uint16_t          m_wNameBoxW2;       // WORD 1469

    int               m_nNameWidth;       // DWORD 735 (text width + 8)
    int               m_nNameHalfWidth;   // DWORD 736

    int               m_nPosX;            // DWORD 737
    int               m_nPosY;            // DWORD 738

    uint16_t          m_wNameHeight;      // WORD 1480 (Y offset for name above pet)
    uint16_t          m_wNameHeightPad;

    int               m_nFacing;          // DWORD 741 (0=left, 1=right)
    int               m_nSpeechFlag;      // DWORD 742
    int               m_nPatrolDir;       // DWORD 743 (direction step: +1 or -1)
    int               m_nOffsetX;         // DWORD 744 (X offset from owner)
    int               m_nPatrolStep;      // DWORD 745
    float             m_fTraceRatio;      // DWORD 746 (starts at 1.0f)
    int               m_nTraceReady;      // DWORD 747 (1 = ready)
    float             m_fPatrolRatio;     // DWORD 748
    int               m_nPatrolActive;    // DWORD 749
    int               m_bIsMyPet;         // DWORD 750

    int               m_nReserved751;     // DWORD 751
    int               m_nNearItemCount;   // DWORD 752 (money amount or field item info)
    int               m_nReserved753;     // DWORD 753
    int               m_nCanPickup;       // DWORD 754
    DWORD             m_dwLastPickupTime; // DWORD 755
    DWORD             m_dwLastInvFullMsg; // DWORD 756
    int               m_nLastSatiety;     // DWORD 757
    int               m_nPatrolState;     // DWORD 758 (patrol state flag)
    int               m_nAlpha;           // DWORD 759 (opacity: 127 or 255)
    int               m_nDrawMode;        // DWORD 760 (action state)
    int               m_nPickupInterval;  // DWORD 761 (250 or 500 ms)
    int               m_nTransportInit;   // DWORD 762

    uint16_t          m_wTotalFrames;     // WORD 1502
    uint16_t          m_wCurrentFrame;    // WORD 1503

    uint16_t          m_wNearItemKind;    // WORD 1506
    uint16_t          m_wNearItemQty;     // WORD 1507
    uint16_t          m_wNearItemGrade;   // WORD 1508
};
