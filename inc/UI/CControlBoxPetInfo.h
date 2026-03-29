#pragma once

#include "UI/CControlBase.h"
#include "UI/CControlImage.h"
#include "UI/CControlText.h"

// 寵物資訊盒（用於寵物市場、寵物寄放等介面）
// 反編譯對照：
//   this+0:     CControlBase (120 bytes)
//   this+120:   CControlImage m_BgImage    (背景/選取底圖)
//   this+312:   CControlText  m_Name       (寵物名稱)
//   this+744:   CControlImage m_IconImage  (寵物圖示)
//   this+936:   CControlImage m_SelectMark (選取標記)
// 總大小：1128 bytes
class CControlBoxPetInfo : public CControlBase
{
public:
    CControlBoxPetInfo();
    virtual ~CControlBoxPetInfo();

    // 設定為選取/未選取狀態（切換背景圖 block）
    void SetSelectImage();
    void SetNoneSelectImage();

    CControlImage m_BgImage;     // +120
    CControlText  m_Name;        // +312
    CControlImage m_IconImage;   // +744
    CControlImage m_SelectMark;  // +936
};
