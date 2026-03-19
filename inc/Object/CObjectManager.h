#pragma once

#include <windows.h>

class CBaseObject;
class ClientCharacter;

struct ObjectNode {
    ObjectNode*  pPrev;
    ObjectNode*  pNext;
    CBaseObject* pObject;
};

struct EffectObjectInfo {
    unsigned short m_wObjectKind;     // offset 0
    unsigned short m_wEffectKind;     // offset 2
    unsigned short m_wPlacementType;  // offset 4 (2=NORMAL, 3=TOP)
    unsigned short _pad;              // offset 6
    int            m_nOffsetX;        // offset 8
    int            m_nOffsetY;        // offset 12
}; // 16 bytes

class CObjectManager {
public:
    CObjectManager();
    virtual ~CObjectManager();

    // Add a pre-created object
    void AddObject(CBaseObject* pObject);

    // Create and add a static object by ID
    CBaseObject* AddObject(unsigned short wObjectID);

    // Add effect objects matching the given object kind
    void AddEffectObject(unsigned short wObjectKind, ClientCharacter* pCharacter);

    // Delete the object at the given index
    void DelObject(int nIndex);

    // Delete all objects
    void DelAllObject();

    // Swap objects at two indices
    void SwapObject(int nIndex1, int nIndex2);

    // Get object at index
    CBaseObject* GetObjectA(unsigned short wIndex);

    // Get total object count
    unsigned short GetObjectCount();

    // Get the type of object at index
    unsigned short GetType(unsigned short wIndex);

    // Collision detection with a point
    int ColloisionObject(CBaseObject* pObject, POINT pt);

    // Collision detection by object kind (stub, returns nullptr)
    CBaseObject* ColloisionObjectKind(unsigned char byKind);

    // Collision detection by object ID (stub, returns nullptr)
    CBaseObject* ColloisionObject(unsigned short wObjectID);

    // Process all objects
    void Process();

    // Frame process all objects
    void FrameProcess(float fElapsedTime);

    // Draw all objects
    void Draw();

    // Set the character associated with this manager
    void SetCharacter(ClientCharacter* pCharacter);

    // Poll: process and check collisions
    void Poll();

    // Initialize map effect objects from file
    int InitMapEffectObject(char* szFileName);

private:
    ObjectNode*      m_pHead;                  // offset 4
    ObjectNode*      m_pTail;                  // offset 8
    unsigned short   m_wObjectCount;           // offset 12
    unsigned short   _pad1;                    // offset 14
    ClientCharacter* m_pCharacter;             // offset 16
    EffectObjectInfo* m_pEffectObjectInfos;    // offset 20
    unsigned short   m_wEffectObjectCount;     // offset 24
};

extern CObjectManager g_ObjectManager;
