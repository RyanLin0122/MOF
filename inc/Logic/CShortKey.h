#pragma once

#include <cstdint>
#include <windows.h>

// ----------------------------------------------------------------------------
// strBoardKey — single physical-key descriptor used by the key-setting UI.
// Exact byte layout from mofclient.c (total size 148 bytes, stride 0x94):
//     +0x00  int      scanCode   (-1 for unused slots)
//     +0x04  int      x          screen x of the on-screen keyboard sprite
//     +0x08  int      y          screen y of the on-screen keyboard sprite
//     +0x0C  uint16_t w1         sprite width (varies by key)
//     +0x0E  uint16_t w2         sprite height (varies by key)
//     +0x10  char     name[128]  printable label ("Q", "Enter", ...)
//     +0x90  int      isSetable  0 = informational only (ESC / PrintScreen)
// ----------------------------------------------------------------------------
struct strBoardKey {
    int      scanCode;
    int      x;
    int      y;
    uint16_t w1;
    uint16_t w2;
    char     name[128];
    int      isSetable;

    strBoardKey();

    void Init();
    void Set(int a2, int a3, int a4, const char* a5, int a6, uint16_t a7, uint16_t a8);
    int  IsSetableKey() const;
};

// ----------------------------------------------------------------------------
// CShortKey — user-rebindable shortcut-key manager.
// Object layout (total size 0x1C0 = 448 bytes, from mofclient.c):
//     +0x000  int m_nSettedKey[56]   committed bindings (what the game uses)
//     +0x0E0  int m_nSettingKey[56]  editor working copy (key-setting UI)
//
// Bindings are scan codes (DirectInput DIK_*).  A "not bound" entry stores a
// sentinel of the form ((i << 24) | 0xFFFFFF), i.e. any value >= 0xFFFFFF.
// ----------------------------------------------------------------------------
class CShortKey {
public:
    enum { KEY_COUNT = 56 };
    enum { BOARD_CAPACITY = 84 };               // slot count of m_strBoardKey
    enum : unsigned int { INVALID_KEY = 0xFFFFFFu };

    int m_nSettedKey[KEY_COUNT];
    int m_nSettingKey[KEY_COUNT];

    // ---- Per-instance methods (thiscall in mofclient.c) ----
    void ReadKeySetting();
    void CopySettedKeyToSettingKey();
    void CopySettingKeyToSettedKey();
    void SaveKeySetting();
    void ProcessInvalidKey();
    BOOL IsRequiredKeyInKeyboard();
    void SetAllDefaultKey();
    void SendExtraKeySpace(int a2);
    void* GetSettingKey(int a2);
    void  SetSettingKey(int a2, int a3);
    int   GetShortKey(int a2);
    void  AdjustClientKey();

    // ---- Static helpers ----
    static int          FindKeyBoardIndex(int scanCode);
    static strBoardKey* GetBoardKey(int index);
    static void         AddKeyboard(int scanCode, int x, int y, const char* name,
                                    int isSetable, uint16_t w1, uint16_t w2);
    static char*        GetUserKeyName(int a1);
    static char*        GetUserKeySettingName(int a1, int a2);
    static void         InitStaticDefaultKey();

    // ---- Thiscall-compat wrappers ----
    // mofclient.c calls methods via the `ClassName::Method(this, ...)` form that
    // IDA reconstructs for __thiscall.  Keep the same surface so ported code
    // reads like the ground truth.
    static void  SetAllDefaultKey(CShortKey* self)                 { if (self) self->SetAllDefaultKey(); }
    static void  SaveKeySetting(CShortKey* self)                   { if (self) self->SaveKeySetting(); }
    static int   GetShortKey(CShortKey* self, int a2)              { return self ? self->GetShortKey(a2) : 0; }
    static void* GetSettingKey(CShortKey* self, int a2)            { return self ? self->GetSettingKey(a2) : reinterpret_cast<void*>(static_cast<uintptr_t>(INVALID_KEY) | (static_cast<uintptr_t>(a2) << 24)); }
    static void  SetSettingKey(CShortKey* self, int a2, int a3)    { if (self) self->SetSettingKey(a2, a3); }
    static void  SendExtraKeySpace(CShortKey* self, int a2)        { if (self) self->SendExtraKeySpace(a2); }
    static void  AdjustClientKey(CShortKey* self)                  { if (self) self->AdjustClientKey(); }
    static void  ReadKeySetting(CShortKey* self)                   { if (self) self->ReadKeySetting(); }
    static void  CopySettedKeyToSettingKey(CShortKey* self)        { if (self) self->CopySettedKeyToSettingKey(); }
    static void  CopySettingKeyToSettedKey(CShortKey* self)        { if (self) self->CopySettingKeyToSettedKey(); }
    static void  ProcessInvalidKey(CShortKey* self)                { if (self) self->ProcessInvalidKey(); }
    static BOOL  IsRequiredKeyInKeyboard(CShortKey* self)          { return self ? self->IsRequiredKeyInKeyboard() : FALSE; }

    // ---- Static data (storage lives in CShortKey.cpp) ----
    static int         m_nDefaultKey[KEY_COUNT];
    static strBoardKey m_strBoardKey[BOARD_CAPACITY];
    static uint16_t    m_wUserKeyName[KEY_COUNT];
    static int         s_nKeyCount;             // next free slot in m_strBoardKey
};
