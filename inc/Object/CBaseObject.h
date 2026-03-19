#pragma once

#include "UI/CControlAlphaBox.h"
#include "UI/CControlChatBallon.h"

class CBaseObject {
public:
    CBaseObject();
    virtual ~CBaseObject();

    // Virtual methods for derived class override (vtable order matters)
    virtual void FrameProcess(float fElapsedTime) {}
    virtual void Process() {}
    virtual void Draw() {}
    virtual void Poll() {}

    void InitNameBackBox(char* szName, char* szTitle);
    void SetPosNameBox(float fWorldX, float fWorldY);
    void PrepareDrawingNameBox();
    void DrawNameBox();

    // --- Member variables (matching decompiled layout) ---

    unsigned short m_siID;           // WORD +2 (offset 4)
    unsigned short m_siKind;         // WORD +3 (offset 6)
    unsigned int   m_dwField1;       // DWORD +2 (offset 8)
    unsigned short m_siField2;       // WORD +6 (offset 12)
    unsigned short _pad1;            // padding (offset 14)
    unsigned int   m_dwValid;        // DWORD +4 (offset 16) - indicates if object is active
    unsigned short m_siType;         // WORD +10 (offset 20) - 0=static, 1=portal, 2=effect, 3=ani
    unsigned short _pad2;            // padding (offset 22)
    unsigned int   m_dwField3;       // DWORD +6 (offset 24)
    unsigned short m_siField4;       // WORD +14 (offset 28)
    unsigned short _pad3;            // padding (offset 30)
    int            m_nPosX;          // DWORD +8 (offset 32) - world X
    int            m_nPosY;          // DWORD +9 (offset 36) - world Y
    unsigned short m_siField5;       // WORD +20 (offset 40)
    unsigned short _pad4;            // padding (offset 42)
    int            m_nWidth;         // DWORD +11 (offset 44) - image/collision width
    int            m_nHeight;        // DWORD +12 (offset 48) - image/collision height
    unsigned char  m_byAlpha;        // BYTE +52 (offset 52) - 0xFF=normal, 0xAA=highlighted
    unsigned char  m_byField6;       // BYTE +53 (offset 53)
    unsigned char  m_byField7;       // BYTE +54 (offset 54)
    unsigned char  _pad5;            // padding (offset 55)
    int            m_bInitialized;   // DWORD +14 (offset 56) - whether the object is initialized

    CControlAlphaBox   m_NameBox;    // offset 60
    CControlAlphaBox   m_TitleBox;   // offset 272
    CControlChatBallon m_ChatBallon; // offset 484

    // These fields are accessed as text widths for the name/title boxes
    int m_nNameWidth;    // text width for name box
    int m_nTitleWidth;   // text width for title box
};
