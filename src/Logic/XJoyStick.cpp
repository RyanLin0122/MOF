#include "Logic/XJoyStick.h"

namespace Gamemare {

XJoyStick* XJoyStick::GetInstance() {
    return nullptr;
}

HRESULT XJoyStick::Init(HWND /*hWnd*/, bool /*useJoystick*/) {
    return E_FAIL;
}

void XJoyStick::Acquire() {
}

void XJoyStick::Update() {
}

void XJoyStick::Free() {
}

} // namespace Gamemare
