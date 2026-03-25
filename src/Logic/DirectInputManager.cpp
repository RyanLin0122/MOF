#include "Logic/DirectInputManager.h"
#include "Logic/XJoyStick.h"
#include "global.h"
#include <cstring>

// ======== Keyboard ========

char DirectInputManager::IsKeyDown(int scancode) {
    if (m_keyState[scancode] != 1)
        return 0;
    m_keyState[scancode] = 0;
    return 1;
}

char DirectInputManager::IsKeyUp(int scancode) {
    if (m_keyState[scancode] != 2)
        return 0;
    m_keyState[scancode] = 0;
    return 1;
}

bool DirectInputManager::IsKeyPressed(int scancode) {
    return m_keyState[scancode] == 1;
}

bool DirectInputManager::IsAnyKeyPressed() {
    return m_isAnyKeyPressed != 0;
}

void DirectInputManager::KeySetting(int scancode) {
    m_keyState[scancode] = 1;
}

// ======== Left Mouse Button ========

char DirectInputManager::IsLMButtonDBClick() {
    if (!m_lmButtonDBClick)
        return 0;
    m_lmButtonDBClick = 0;
    return 1;
}

bool DirectInputManager::IsLMButtonDown() {
    if (m_lmButtonState != 1)
        return false;
    m_lmButtonState = 0;
    return true;
}

char DirectInputManager::IsLMButtonUp() {
    if (m_lmButtonState != 2)
        return 0;
    m_lmButtonState = 0;
    return 1;
}

bool DirectInputManager::IsLMButtonPressed() {
    return m_lmButtonState == 1;
}

void DirectInputManager::SetLMButtonStatue(int state) {
    m_lmButtonState = static_cast<unsigned char>(state);
}

// ======== Right Mouse Button ========

char DirectInputManager::IsRMButtonDBClick() {
    if (!m_rmButtonDBClick)
        return 0;
    m_rmButtonDBClick = 0;
    return 1;
}

bool DirectInputManager::IsRMButtonDown() {
    if (m_rmButtonState != 1)
        return false;
    m_rmButtonState = 0;
    return true;
}

char DirectInputManager::IsRMButtonUp() {
    if (m_rmButtonState != 2)
        return 0;
    m_rmButtonState = 0;
    return 1;
}

bool DirectInputManager::IsRMButtonPressed() {
    return m_rmButtonState == 1;
}

void DirectInputManager::SetRMButtonStatue(int state) {
    m_rmButtonState = static_cast<unsigned char>(state);
}

// ======== Mouse ========

bool DirectInputManager::IsAnyMouseInput() {
    if (m_isMouseMoved)
        return true;
    if (m_lmButtonState)
        return true;
    if (m_lmButtonDBClick)
        return true;
    if (m_rmButtonState)
        return true;
    return m_rmButtonDBClick != 0;
}

int DirectInputManager::GetMouseMove_X() {
    return m_mouseMoveDeltaX;
}

int DirectInputManager::GetMouseMove_Y() {
    return m_mouseMoveDeltaY;
}

int DirectInputManager::GetMouse_X() {
    return m_mouseX;
}

int DirectInputManager::GetMouse_Y() {
    return m_mouseY;
}

void DirectInputManager::GetMouse_Pos(int* outX, int* outY) {
    *outX = m_mouseX;
    *outY = m_mouseY;
}

void DirectInputManager::SetMousePos(int x, int y) {
    m_isMouseMoved = 1;
    m_mouseX = x;
    m_mouseY = y;
}

void DirectInputManager::SetIsMouseMoveFalse() {
    m_isMouseMoved = 0;
}

// ======== General ========

void DirectInputManager::Update() {
    // Original sets m_updateFlag to 1065353216 (= 1.0f as int bits)
    // then adds mouse delta to mouse position.
    m_updateFlag = 1065353216; // 1.0f reinterpreted as int
    m_mouseX += static_cast<int>(static_cast<double>(m_mouseMoveDeltaX));
    m_mouseY += static_cast<int>(static_cast<double>(m_mouseMoveDeltaY));
}

bool DirectInputManager::IsAnyInput() {
    if (IsAnyKeyPressed())
        return true;
    if (IsAnyMouseInput())
        return true;
    return m_isAnyJoyInput == 1;
}

UINT DirectInputManager::Scan2Ascii(unsigned int scancode) {
    return MapVirtualKeyExA(scancode, 1, m_hkl);
}

// ======== DirectInput Lifecycle ========

int DirectInputManager::hrReadInput() {
    if (m_inputLocked)
        return 0;

    DWORD dwElements = 10;
    if (!m_pKeyboardDevice || !m_pDirectInput)
        return -2130706430; // DIERR_NOTINITIALIZED equivalent

    DIDEVICEOBJECTDATA didod[10];
    HRESULT hrKeyboard = m_pKeyboardDevice->GetDeviceData(
        sizeof(DIDEVICEOBJECTDATA), didod, &dwElements, 0);

    // Process joystick if available and nation code matches (2, 3, 4, or 5)
    if (m_pJoyStick) {
        unsigned char nationCode = static_cast<unsigned char>(g_MoFFont.GetNationCode());
        if (nationCode == 2 || nationCode == 3 || nationCode == 4 || nationCode == 5) {
            m_pJoyStick->Acquire();
            m_pJoyStick->Update();
            m_isAnyJoyInput = 0;

            // Process joystick buttons (up to 8)
            unsigned char* pJoyStickBase = reinterpret_cast<unsigned char*>(m_pJoyStick);
            for (int i = 0; i < 8; ++i) {
                unsigned char buttonVal = pJoyStickBase[72 + i];
                if (i < 8 && buttonVal == 1) {
                    m_isAnyJoyInput = 1;
                    m_joyButtonRaw[i] = static_cast<unsigned char>((~m_joyButtonRaw[i] & 2) | 4) >> 1;
                } else if (m_joyButtonRaw[i] & 2) {
                    m_joyButtonRaw[i] = 4;
                }

                unsigned char rawVal = m_joyButtonRaw[i];
                if (rawVal & 1) {
                    m_joyButtonState[i] = 1;
                } else if (!(rawVal & 2) && (rawVal & 4)) {
                    m_joyButtonState[i] = 4;
                    m_joyButtonRaw[i] = 0;
                }
            }

            // Process joystick axes (up to 12 — 6 axes × 2 directions)
            int* pJoyAxes = reinterpret_cast<int*>(pJoyStickBase + 24);
            for (int i = 0; i < 12; ++i) {
                int axisVal = pJoyAxes[i];
                if (i < 12 && (axisVal >= 1000 || axisVal <= -1000)) {
                    m_isAnyJoyInput = 1;
                    m_joyStickRaw[i] = static_cast<unsigned char>((~m_joyStickRaw[i] & 2) | 4) >> 1;
                    if (axisVal >= 1000)
                        m_joyStickDir[i] = 2;
                    else if (axisVal <= -1000)
                        m_joyStickDir[i] = 1;
                } else if (m_joyStickRaw[i] & 2) {
                    m_joyStickRaw[i] = 4;
                }

                unsigned char rawVal = m_joyStickRaw[i];
                if (rawVal & 1) {
                    m_joyStickState[i] = 1;
                } else if (!(rawVal & 2) && (rawVal & 4)) {
                    m_joyStickRaw[i] = 0;
                    m_joyStickState[i] = 4;
                    m_joyStickDir[i] = 0;
                }
            }
        }
    }

    if (hrKeyboard >= 0) {
        // Process keyboard input
        if (dwElements > 0) {
            for (DWORD i = 0; i < dwElements; ++i) {
                DWORD scancode = didod[i].dwOfs;
                unsigned char vk = static_cast<unsigned char>(Scan2Ascii(scancode));

                if (!(didod[i].dwData & 0x80)) {
                    // Key released (high bit not set)
                    m_isAnyKeyPressed = 0;
                    if (m_asciiKeyState[vk] == 1 || m_keyState[scancode] == 1) {
                        m_asciiKeyState[vk] = 2;
                        m_keyState[scancode] = 2;
                    } else {
                        m_asciiKeyState[vk] = 0;
                        m_keyState[scancode] = 0;
                    }
                } else {
                    // Key pressed (high bit set)
                    m_asciiKeyState[vk] = 1;
                    m_keyState[scancode] = 1;
                    m_isAnyKeyPressed = 1;
                }
            }
        }

        // Read mouse state (DIMOUSESTATE = 16 bytes) into m_mouseMoveDeltaX area
        return m_pMouseDevice->GetDeviceState(
            sizeof(DIMOUSESTATE), &m_mouseMoveDeltaX) < 0 ? 1 : 0;
    } else {
        // Keyboard device lost — reacquire
        m_pKeyboardDevice->Acquire();
        return 0;
    }
}

void DirectInputManager::Clean() {
    if (m_pJoyStick) {
        m_pJoyStick->Free();
        m_pJoyStick = nullptr;
    }
    if (m_pKeyboardDevice) {
        m_pKeyboardDevice->Unacquire();
        m_pKeyboardDevice->Release();
        m_pKeyboardDevice = nullptr;
    }
    if (m_pMouseDevice) {
        m_pMouseDevice->Unacquire();
        m_pMouseDevice->Release();
        m_pMouseDevice = nullptr;
    }
    if (m_pDirectInput) {
        m_pDirectInput->Release();
        m_pDirectInput = nullptr;
    }
}

HRESULT DirectInputManager::hrInitDirectInput(LPVOID* ppvOut) {
    // In the original binary, ppvOut points to this->m_pDirectInput
    // (the caller passes (LPVOID*)&m_pDirectInput).
    HRESULT hr = 0;
    if (!*ppvOut) {
        HMODULE hModule = GetModuleHandleA(nullptr);
        hr = DirectInput8Create(
            hModule, DIRECTINPUT_VERSION, IID_IDirectInput8A,
            ppvOut, nullptr);
        if (hr < 0)
            return -2130706431; // custom error code from original
    }
    // The original stores HKL at ppvOut[135] — since ppvOut == (LPVOID*)this,
    // ppvOut[135] == byte offset 540 == m_hkl.
    m_hkl = GetKeyboardLayout(0);
    return hr;
}

int DirectInputManager::hrInitKeyboard(HWND hWnd) {
    if (m_pKeyboardDevice)
        return -2130706429; // already initialized

    if (!m_pDirectInput)
        return -2130706431; // no DirectInput

    if (m_pDirectInput->CreateDevice(GUID_SysKeyboard, &m_pKeyboardDevice, nullptr) < 0)
        return -2130706430;

    // Set buffer size property
    DIPROPDWORD dipdw;
    dipdw.diph.dwSize = sizeof(DIPROPDWORD);
    dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    dipdw.diph.dwObj = 0;
    dipdw.diph.dwHow = DIPH_DEVICE;
    dipdw.dwData = 10;

    if (m_pKeyboardDevice->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph) < 0)
        return -2130706430;

    if (m_pKeyboardDevice->SetDataFormat(&c_dfDIKeyboard) < 0)
        return -2130706430;

    // DISCL_NONEXCLUSIVE | DISCL_FOREGROUND = 2 | 4 = 6
    if (m_pKeyboardDevice->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND) < 0)
        return -2130706430;

    m_pKeyboardDevice->Acquire();
    return 0;
}

char DirectInputManager::hrInitMouse(HWND hWnd) {
    if (m_pDirectInput->CreateDevice(GUID_SysMouse, &m_pMouseDevice, nullptr) < 0)
        return 0;

    // DISCL_NONEXCLUSIVE | DISCL_FOREGROUND = 6
    if (m_pMouseDevice->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND) < 0)
        return 0;

    if (m_pMouseDevice->SetDataFormat(&c_dfDIMouse) < 0)
        return 0;

    m_pMouseDevice->Acquire();
    return 1;
}

void DirectInputManager::FreeAllKey() {
    memset(m_keyState, 0, 256);
}

// ======== Joystick ========

int DirectInputManager::InitJoystick(HWND hWnd, bool useJoystick) {
    m_pJoyStick = nullptr;
    m_isAnyJoyInput = 0;

    memset(m_joyButtonRaw, 0, sizeof(m_joyButtonRaw));
    memset(m_joyButtonState, 0, sizeof(m_joyButtonState));
    memset(m_joyStickRaw, 0, sizeof(m_joyStickRaw));
    memset(m_joyStickState, 0, sizeof(m_joyStickState));
    memset(m_joyStickDir, 0, sizeof(m_joyStickDir));

    Gamemare::XJoyStick* pJoy = Gamemare::XJoyStick::GetInstance();
    m_pJoyStick = pJoy;
    if (pJoy) {
        if (pJoy->Init(hWnd, useJoystick) == static_cast<HRESULT>(0x80004005)) {
            // E_FAIL = 0x80004005 = -2147467259
            return 0;
        } else {
            m_pJoyStick->Acquire();
            return 1;
        }
    }
    return 0;
}

int DirectInputManager::IsJoyStickPush(int axis, int dir) {
    if (m_joyStickState[axis] != 1 || m_joyStickDir[axis] != dir)
        return 0;
    m_joyStickState[axis] = 2;
    m_joyStickDir[axis] = 0;
    return 1;
}

int DirectInputManager::IsJoyStickUp(int axis, int dir) {
    if (m_joyStickState[axis] != 4 || m_joyStickDir[axis] != dir)
        return 0;
    m_joyStickState[axis] = 0;
    m_joyStickDir[axis] = 0;
    return 1;
}

BOOL DirectInputManager::IsJoyStickDown(int axis, int dir) {
    return m_joyStickState[axis] == 2 && m_joyStickDir[axis] == dir;
}

int DirectInputManager::IsJoyButtonPush(int button) {
    if (m_joyButtonState[button] != 1)
        return 0;
    m_joyButtonState[button] = 2;
    return 1;
}

int DirectInputManager::IsJoyButtonUp(int button) {
    if (m_joyButtonState[button] != 4)
        return 0;
    m_joyButtonState[button] = 0;
    return 1;
}

BOOL DirectInputManager::IsJoyButtonDown(int button) {
    return m_joyButtonState[button] == 2;
}

void DirectInputManager::ResetJoyStick() {
    memset(m_joyButtonRaw, 0, sizeof(m_joyButtonRaw));
    memset(m_joyButtonState, 0, sizeof(m_joyButtonState));
    memset(m_joyStickRaw, 0, sizeof(m_joyStickRaw));
    memset(m_joyStickState, 0, sizeof(m_joyStickState));
    memset(m_joyStickDir, 0, sizeof(m_joyStickDir));
    m_isAnyJoyInput = 0;
}

BOOL DirectInputManager::IsAnyJoyStickInput() {
    return m_isAnyJoyInput == 1;
}
