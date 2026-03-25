#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

// Forward declaration — external joystick wrapper from the original binary.
namespace Gamemare {
    class XJoyStick;
}

//
// DirectInputManager
//
// Reconstructed from mofclient.c decompilation.
// Memory layout verified against raw byte/DWORD offset access patterns.
//
// Key state values: 0 = idle, 1 = pressed/down, 2 = released/up
//
class DirectInputManager {
public:
    // --- Memory layout (byte offsets verified from decompiled pointer arithmetic) ---
    LPDIRECTINPUT8A       m_pDirectInput;         // offset 0   (DWORD[0])
    LPDIRECTINPUTDEVICE8A m_pKeyboardDevice;      // offset 4   (DWORD[1])
    LPDIRECTINPUTDEVICE8A m_pMouseDevice;          // offset 8   (DWORD[2])
    int                   m_mouseMoveDeltaX;       // offset 12  (DWORD[3])  — DIMOUSESTATE.lX
    int                   m_mouseMoveDeltaY;       // offset 16  (DWORD[4])  — DIMOUSESTATE.lY
    int                   m_mouseMoveDeltaZ;       // offset 20             — DIMOUSESTATE.lZ
    unsigned char         m_mouseRawButtons[4];    // offset 24             — DIMOUSESTATE.rgbButtons
    unsigned char         m_keyState[256];         // offset 28             — per-scancode state
    unsigned char         m_asciiKeyState[256];    // offset 284            — per-VK state (via Scan2Ascii)
    HKL                   m_hkl;                   // offset 540 (DWORD[135])
    int                   m_isAnyKeyPressed;       // offset 544 (DWORD[136])
    int                   m_isMouseMoved;          // offset 548 (DWORD[137])
    unsigned char         m_lmButtonState;         // offset 552
    unsigned char         m_rmButtonState;         // offset 553
    // 2 bytes compiler padding for alignment        offset 554-555
    int                   m_mouseX;                // offset 556 (DWORD[139])
    int                   m_mouseY;                // offset 560 (DWORD[140])
    int                   m_updateFlag;            // offset 564 (DWORD[141])
    unsigned char         m_lmButtonDBClick;       // offset 568
    unsigned char         m_rmButtonDBClick;       // offset 569
    unsigned char         m_inputLocked;           // offset 570
    // 1 byte compiler padding for alignment         offset 571
    Gamemare::XJoyStick*  m_pJoyStick;             // offset 572 (DWORD[143])
    unsigned char         m_joyButtonRaw[8];       // offset 576
    unsigned char         m_joyButtonState[8];     // offset 584
    unsigned char         m_joyStickRaw[12];       // offset 592
    unsigned char         m_joyStickState[12];     // offset 604
    int                   m_joyStickDir[12];       // offset 616 (DWORD[154]..DWORD[165])
    int                   m_isAnyJoyInput;         // offset 664 (DWORD[166])

    // --- Keyboard ---
    char IsKeyDown(int scancode);
    char IsKeyUp(int scancode);
    bool IsKeyPressed(int scancode);
    bool IsAnyKeyPressed();
    void KeySetting(int scancode);

    // --- Left mouse button ---
    char IsLMButtonDBClick();
    bool IsLMButtonDown();
    char IsLMButtonUp();
    bool IsLMButtonPressed();
    void SetLMButtonStatue(int state);

    // --- Right mouse button ---
    char IsRMButtonDBClick();
    bool IsRMButtonDown();
    char IsRMButtonUp();
    bool IsRMButtonPressed();
    void SetRMButtonStatue(int state);

    // --- Mouse ---
    bool IsAnyMouseInput();
    int  GetMouseMove_X();
    int  GetMouseMove_Y();
    int  GetMouse_X();
    int  GetMouse_Y();
    void GetMouse_Pos(int* outX, int* outY);
    void SetMousePos(int x, int y);
    void SetIsMouseMoveFalse();

    // --- General ---
    void Update();
    bool IsAnyInput();
    UINT Scan2Ascii(unsigned int scancode);

    // --- DirectInput lifecycle ---
    int  hrReadInput();
    void Clean();
    HRESULT hrInitDirectInput(LPVOID* ppvOut);
    int  hrInitKeyboard(HWND hWnd);
    char hrInitMouse(HWND hWnd);
    void FreeAllKey();

    // --- Joystick ---
    int  InitJoystick(HWND hWnd, bool useJoystick);
    int  IsJoyStickPush(int axis, int dir);
    int  IsJoyStickUp(int axis, int dir);
    BOOL IsJoyStickDown(int axis, int dir);
    int  IsJoyButtonPush(int button);
    int  IsJoyButtonUp(int button);
    BOOL IsJoyButtonDown(int button);
    void ResetJoyStick();
    BOOL IsAnyJoyStickInput();
};
