#pragma once
#include <d3dx9math.h>

// �e�V�ŧi (Forward Declarations)
class CCA;
class CCANormal;
class GameImage;
class CEffectBase;
class CControlChatBallon;
class CControlAlphaBox;
class cltMoF_CharacterActionStateSystem;
class cltPetObject;
class clTransportObject;
struct stCharOrder { int a; };

/**
 * @class ClientCharacter
 * @brief �N��C���@�ɤ��Ҧ����⪺��¦���O (���a�B�Ǫ��BNPC)�C
 * �t�d�޲z���⪺���A�B�ʵe�B��m�B���O�M�����S�ġC
 */
class ClientCharacter {
public:
    // --- ��k�쫬 (Method Prototypes) ---
    ClientCharacter();
    ~ClientCharacter();

    // �֤ߧ�s�Pø�s
    int Poll(int a2);
    void PrepareDrawingPlayer();
    void PrepareDrawingMonster();
    void DrawChar(int a2);
    void DrawPlayer(int a2);
    void DrawMonster(int a2);

    // ���A�P���O�B�z
    void ProcessOrder();
    void PushOrder(stCharOrder* pOrder);
    void PopOrder(stCharOrder* pOrder);
    void ClearOrder();
    unsigned int GetLastOrder();
    void SetActionState(unsigned int state);
    void SetSpecialState(unsigned char state);

    // �S�ĳЫػP�޲z
    void CreateUseSkillEffect();
    void CreateNormalAttackEffect(unsigned char motionType, unsigned int targetAccountID, unsigned char hitInfo);
    // ... ��L�Ҧ��b .cpp �ɤ��٭쪺�禡�쫬 ...

    // Getters & Setters
    int GetPosX() const;
    int GetPosY() const;
    void SetPosX(float x);
    void SetPosY(float y);
    int GetHP() const;
    void SetHP();
    unsigned short GetCharHeight() const;
    int GetActionSide() const;
    bool GetSustainSkillState(unsigned short skillID) const;
    int GetHitedInfoNum(unsigned int casterAccountID);
    void SetHited(int hitInfoID, int value);
    // ... ��L Getters & Setters ...

    /// @brief ���o���⪺ X �y��
    int GetPosX();
    /// @brief ���o���⪺ Y �y��
    int GetPosY();
    /// @brief ���o���⪺����
    int GetCharHeight();
    bool IsHide();

    bool IsTransparent();

    int GetActionSide();

    int GetHitedInfoNum(int a);

    int GetAccountID();

    bool GetSustainSkillState(unsigned short a);

    int GetPetPosX();

    int GetPetPosY();
	unsigned short GetMapID() const { return m_wMapID; }
	unsigned int GetActionState() const { return m_dwActionState; }
    int IsTransformed() const { return m_dwTransformation != 0; }
	int GetSomeOtherState() const { return m_someOtherState; }
	bool IsFashionItemActive() const { return m_fashionItemActive; }
	char* GetName() { return m_szName; }
	bool IsPCRoomUser() const { return m_isPCRoomUser; }
    bool IsPCRoomPremium() const { return m_isPCRoomPremium; }
	bool CanMove() const { return m_canMove; }
	bool IsFreezing() const { return m_isFreezing; }
	bool IsIntegrityActive() const { return m_isIntegrityActive; }
	void SetCanSpeedUp(bool value) { m_canSpeedUp = value; }
	void SetPreparingSpeedUp(bool value) { m_preparingSpeedUp = value; }

private:
    // �������U�禡
    void DecideDrawFrame(unsigned int* pOutResourceID, unsigned short* pOutBlockID);
    void DecideDrawFrame();
    void PrepareDrawingHPBox();
    void DrawHPBox();
    void InitScreenName(int a2);

    // --- �ݩ� (���O����첾�ƦC) ---

    // --- Offset 0x0000 - 0x00FF ---
    void* m_pVftable;                                 // �첾 +0
    GameImage* m_pShadowImage;                        // �첾 +4
    GameImage* m_pShadowImage_Sub;                    // �첾 +8
    void* m_pMonsterAniInfo;                          // �첾 +16
    unsigned int m_dwUnknown_20;                      // �첾 +20
    unsigned int m_dwUnknown_24;                      // �첾 +24
    unsigned int m_dwUnknown_28;                      // �첾 +28
    unsigned int m_dwUnknown_32;                      // �첾 +32
    GameImage* m_pGogglesImage;                       // �첾 +64
    unsigned short m_wUnknown_68;                     // �첾 +68
    unsigned int m_dwAttackSpeed;                     // �첾 +68 (*((_DWORD *)this + 17))
    unsigned char m_ucAttackAniRate;                  // �첾 +72 (*((_BYTE *)this + 72))
    // ... (��L�q 0x0 �� 0xFF ���p�����A�X�ЩM����)

    // --- Offset 0x0100 - 0x02FF ---
    unsigned char m_ucDigitFrames_Unk[10];            // �첾 +280
    unsigned char m_ucDoubleAttackMotion[2];          // �첾 +286 (*((_BYTE *)this + 286))
    unsigned char m_ucAttackMotionType;               // �첾 +288 (*((_BYTE *)this + 288))
    unsigned char m_ucAlpha;                          // �첾 +312 (*((_BYTE *)this + 312))

    // --- Offset 0x0300 - 0x1FFF ---
    char m_szName[128];                               // �첾 +460
    unsigned short m_wKind;                           // �첾 +568
    int m_dwLR_Flag;                                  // �첾 +572 (*((_DWORD *)this + 143))
	unsigned short m_wMapID;						  // �첾 +592
    unsigned char m_ucSex;                            // �첾 +594
    unsigned char m_ucHair;                           // �첾 +595
    unsigned char m_ucFace;                           // �첾 +596
    char m_cCharState;                                // �첾 +604 (Stun, etc.)
    int m_dwTransformation;                           // �첾 +676
    stCharOrder* m_pCurrentOrder;                     // �첾 +708
    unsigned char m_ucHittedSkillMotion;              // �첾 +732 (*((_BYTE *)this + 732))
    unsigned char m_ucAttackType;                     // �첾 +738 (*((_BYTE *)this + 738))
    stCharOrder m_OrderQueue[100];                    // �첾 +744

    // --- Offset 0x2000 - 0x2FFF ---
    //CControlChatBallon m_ChatBallon;                  // �첾 +4448
    unsigned int m_dwAccountID;                       // �첾 +456 (*((_DWORD *)this + 114))

    // ... (�j�q UI ����M�t�Ϊ���) ...
    //CControlAlphaBox m_HPBox_Back;                    // �첾 +7452
    //CControlAlphaBox m_HPBox_Front;                   // �첾 +7660
    //CControlAlphaBox m_NameBox;                       // �첾 +9732
    char m_szScreenName[128];                         // �첾 +9984

    // --- Offset 0x3000+ ---
    //cltMoF_CharacterActionStateSystem m_ActionStateSystem; // �첾 +11544
    //cltPetObject m_PetObject;                         // �첾 +11564
    //clTransportObject m_TransportObject;              // �첾 +14616

    // --- �ʵe��� ---
    CCA* m_pCCA;                                      // �첾 +4348 (*((_DWORD *)this + 1087))
    CCANormal* m_pCCANormal;                          // �첾 +4352 (*((_DWORD *)this + 1088))

    // --- �֤ߦ�m�P���A ---
    int m_iPosX;                                      // �첾 +4384 (*((_DWORD *)this + 1096))
    int m_iPosY;                                      // �첾 +4388 (*((_DWORD *)this + 1097))
    int m_iPrevPosX;                                  // �첾 +4392 (*((_DWORD *)this + 1098))
    int m_iPrevPosY;                                  // �첾 +4396 (*((_DWORD *)this + 1099))

    int m_iDestX;                                     // �첾 +556 (*((_DWORD *)this + 139))
    int m_iDestY;                                     // �첾 +560 (*((_DWORD *)this + 140))
    float m_fMoveSpeed;                               // �첾 +564 (*((float *)this + 141))

    unsigned int m_dwActionState;                     // �첾 +9684 (*((_DWORD *)this + 2421))
	bool m_fashionItemActive;                         // �첾 +9688 (*((_BYTE *)this + 4376))
    unsigned short m_wCurrentFrame;                   // �첾 +9694 (*((_WORD *)this + 4847))
    unsigned short m_wTotalFrame;                     // �첾 +9688 (*((_WORD *)this + 4844))
	bool m_isFreezing;                                // �첾 +9700 bit4
    bool m_canMove;                                   // �첾 +9700 bit1
    bool m_isIntegrityActive;                         // �첾 +9700 bit0
    int m_someOtherState;                             // �첾 +11528
    bool m_isPCRoomUser;                              // �첾 +11556
    bool m_isPCRoomPremium;                           // �첾 +11560
	bool m_preparingSpeedUp;                          // �첾 *((_DWORD *)a2 + 174)
	bool m_canSpeedUp;                                // �첾 *(_DWORD *)(v3 + 696)
};