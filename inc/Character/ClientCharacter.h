#pragma once
#include <d3dx9math.h>

// 前向宣告 (Forward Declarations)
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
 * @brief 代表遊戲世界中所有角色的基礎類別 (玩家、怪物、NPC)。
 * 負責管理角色的狀態、動畫、位置、指令和相關特效。
 */
class ClientCharacter {
public:
    // --- 方法原型 (Method Prototypes) ---
    ClientCharacter();
    ~ClientCharacter();

    // 核心更新與繪製
    int Poll(int a2);
    void PrepareDrawingPlayer();
    void PrepareDrawingMonster();
    void DrawChar(int a2);
    void DrawPlayer(int a2);
    void DrawMonster(int a2);

    // 狀態與指令處理
    void ProcessOrder();
    void PushOrder(stCharOrder* pOrder);
    void PopOrder(stCharOrder* pOrder);
    void ClearOrder();
    unsigned int GetLastOrder();
    void SetActionState(unsigned int state);
    void SetSpecialState(unsigned char state);

    // 特效創建與管理
    void CreateUseSkillEffect();
    void CreateNormalAttackEffect(unsigned char motionType, unsigned int targetAccountID, unsigned char hitInfo);
    // ... 其他所有在 .cpp 檔中還原的函式原型 ...

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
    // ... 其他 Getters & Setters ...

    /// @brief 取得角色的 X 座標
    int GetPosX();
    /// @brief 取得角色的 Y 座標
    int GetPosY();
    /// @brief 取得角色的高度
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
    // 內部輔助函式
    void DecideDrawFrame(unsigned int* pOutResourceID, unsigned short* pOutBlockID);
    void DecideDrawFrame();
    void PrepareDrawingHPBox();
    void DrawHPBox();
    void InitScreenName(int a2);

    // --- 屬性 (按記憶體位移排列) ---

    // --- Offset 0x0000 - 0x00FF ---
    void* m_pVftable;                                 // 位移 +0
    GameImage* m_pShadowImage;                        // 位移 +4
    GameImage* m_pShadowImage_Sub;                    // 位移 +8
    void* m_pMonsterAniInfo;                          // 位移 +16
    unsigned int m_dwUnknown_20;                      // 位移 +20
    unsigned int m_dwUnknown_24;                      // 位移 +24
    unsigned int m_dwUnknown_28;                      // 位移 +28
    unsigned int m_dwUnknown_32;                      // 位移 +32
    GameImage* m_pGogglesImage;                       // 位移 +64
    unsigned short m_wUnknown_68;                     // 位移 +68
    unsigned int m_dwAttackSpeed;                     // 位移 +68 (*((_DWORD *)this + 17))
    unsigned char m_ucAttackAniRate;                  // 位移 +72 (*((_BYTE *)this + 72))
    // ... (其他從 0x0 到 0xFF 的小型狀態旗標和指標)

    // --- Offset 0x0100 - 0x02FF ---
    unsigned char m_ucDigitFrames_Unk[10];            // 位移 +280
    unsigned char m_ucDoubleAttackMotion[2];          // 位移 +286 (*((_BYTE *)this + 286))
    unsigned char m_ucAttackMotionType;               // 位移 +288 (*((_BYTE *)this + 288))
    unsigned char m_ucAlpha;                          // 位移 +312 (*((_BYTE *)this + 312))

    // --- Offset 0x0300 - 0x1FFF ---
    char m_szName[128];                               // 位移 +460
    unsigned short m_wKind;                           // 位移 +568
    int m_dwLR_Flag;                                  // 位移 +572 (*((_DWORD *)this + 143))
	unsigned short m_wMapID;						  // 位移 +592
    unsigned char m_ucSex;                            // 位移 +594
    unsigned char m_ucHair;                           // 位移 +595
    unsigned char m_ucFace;                           // 位移 +596
    char m_cCharState;                                // 位移 +604 (Stun, etc.)
    int m_dwTransformation;                           // 位移 +676
    stCharOrder* m_pCurrentOrder;                     // 位移 +708
    unsigned char m_ucHittedSkillMotion;              // 位移 +732 (*((_BYTE *)this + 732))
    unsigned char m_ucAttackType;                     // 位移 +738 (*((_BYTE *)this + 738))
    stCharOrder m_OrderQueue[100];                    // 位移 +744

    // --- Offset 0x2000 - 0x2FFF ---
    //CControlChatBallon m_ChatBallon;                  // 位移 +4448
    unsigned int m_dwAccountID;                       // 位移 +456 (*((_DWORD *)this + 114))

    // ... (大量 UI 控制項和系統物件) ...
    //CControlAlphaBox m_HPBox_Back;                    // 位移 +7452
    //CControlAlphaBox m_HPBox_Front;                   // 位移 +7660
    //CControlAlphaBox m_NameBox;                       // 位移 +9732
    char m_szScreenName[128];                         // 位移 +9984

    // --- Offset 0x3000+ ---
    //cltMoF_CharacterActionStateSystem m_ActionStateSystem; // 位移 +11544
    //cltPetObject m_PetObject;                         // 位移 +11564
    //clTransportObject m_TransportObject;              // 位移 +14616

    // --- 動畫控制器 ---
    CCA* m_pCCA;                                      // 位移 +4348 (*((_DWORD *)this + 1087))
    CCANormal* m_pCCANormal;                          // 位移 +4352 (*((_DWORD *)this + 1088))

    // --- 核心位置與狀態 ---
    int m_iPosX;                                      // 位移 +4384 (*((_DWORD *)this + 1096))
    int m_iPosY;                                      // 位移 +4388 (*((_DWORD *)this + 1097))
    int m_iPrevPosX;                                  // 位移 +4392 (*((_DWORD *)this + 1098))
    int m_iPrevPosY;                                  // 位移 +4396 (*((_DWORD *)this + 1099))

    int m_iDestX;                                     // 位移 +556 (*((_DWORD *)this + 139))
    int m_iDestY;                                     // 位移 +560 (*((_DWORD *)this + 140))
    float m_fMoveSpeed;                               // 位移 +564 (*((float *)this + 141))

    unsigned int m_dwActionState;                     // 位移 +9684 (*((_DWORD *)this + 2421))
	bool m_fashionItemActive;                         // 位移 +9688 (*((_BYTE *)this + 4376))
    unsigned short m_wCurrentFrame;                   // 位移 +9694 (*((_WORD *)this + 4847))
    unsigned short m_wTotalFrame;                     // 位移 +9688 (*((_WORD *)this + 4844))
	bool m_isFreezing;                                // 位移 +9700 bit4
    bool m_canMove;                                   // 位移 +9700 bit1
    bool m_isIntegrityActive;                         // 位移 +9700 bit0
    int m_someOtherState;                             // 位移 +11528
    bool m_isPCRoomUser;                              // 位移 +11556
    bool m_isPCRoomPremium;                           // 位移 +11560
	bool m_preparingSpeedUp;                          // 位移 *((_DWORD *)a2 + 174)
	bool m_canSpeedUp;                                // 位移 *(_DWORD *)(v3 + 696)
};