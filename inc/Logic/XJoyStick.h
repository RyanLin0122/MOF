#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

namespace Gamemare {

class XJoyStick {
public:
    static XJoyStick* GetInstance();

    HRESULT Init(HWND hWnd, bool useJoystick);
    void Acquire();
    void Update();
    void Free();
};

} // namespace Gamemare
