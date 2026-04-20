#include "UI/CUICircle.h"

CUICircle::CUICircle() = default;
CUICircle::~CUICircle() = default;

const char* CUICircle::GetCircleName() {
    // Null-this-safe: the chat manager calls this via a pointer that may be
    // nullptr (CUIManager::GetUIWindow returns nullptr when the window is not
    // open).  Returning "" lets the caller's strlen() check drop through to
    // the "circle not joined" system message branch.
    if (!this) return "";
    return "";
}
