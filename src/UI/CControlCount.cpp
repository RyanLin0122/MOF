#include "UI/CControlCount.h"

// ==========================
// 建構
// 反編譯：
//   CControlBase::CControlBase(this);
//   CControlImage::CControlImage((char*)this + 120);
//   CControlImage::CControlImage((char*)this + 312);
//   CControlImage::CControlImage((char*)this + 504);
//   QWORD fields = 0
//   this[13] = 1;  // CControlBase active flag?
// ==========================
CControlCount::CControlCount()
    : CControlBase()
    , m_Left()
    , m_Mid()
    , m_Right()
{
    m_Min = 0;
    m_Max = 0;
    m_Cur = 0;

    // 對齊反編譯：*((_DWORD *)this + 13) = 1;
    reinterpret_cast<int*>(this)[13] = 1;
}

CControlCount::~CControlCount()
{
    // 反編譯：逆序解構 m_Right, m_Mid, m_Left, CControlBase
}

// ==========================
// Create
// 反編譯：
//   CControlBase::Create(this, a2, a3, a9);
//   m_Left.Create(this);   // via vtbl+12
//   m_Mid.Create(this);
//   m_Right.Create(this);
//   SetImage(a5, a6, a7, a8);
//   WORD[17] = m_Left.GetHeight();  // via vtbl+60
//   WORD[16] = a4;  // totalWidth
// ==========================
void CControlCount::Create(int x, int y, int totalWidth,
    unsigned int group, int idLeft, int idMid, int idRight,
    CControlBase* pParent)
{
    CControlBase::Create(x, y, pParent);

    m_Left.Create(this);
    m_Mid.Create(this);
    m_Right.Create(this);

    SetImage(group, idLeft, idMid, idRight);

    SetSize(static_cast<uint16_t>(totalWidth),
        m_Left.GetHeight());
}

// ==========================
// SetImage
// 反編譯：
//   m_Left.SetImage(a2, a3);   // via vtbl+84
//   m_Mid.SetImage(a2, a4);
//   m_Right.SetImage(a2, a5);
//   v9 = m_Left.GetWidth();    // via vtbl+56
//   m_Mid.SetX(v9);
//   v10 = m_Mid.GetWidth();
//   v11 = m_Mid.GetX();
//   m_Right.SetX(v11 + v10);
// ==========================
void CControlCount::SetImage(unsigned int group, int idLeft, int idMid, int idRight)
{
    m_Left.SetImage(group, static_cast<uint16_t>(idLeft));
    m_Mid.SetImage(group, static_cast<uint16_t>(idMid));
    m_Right.SetImage(group, static_cast<uint16_t>(idRight));

    uint16_t wL = m_Left.GetWidth();
    m_Mid.SetX(wL);

    int midX = m_Mid.GetX();
    uint16_t wM = m_Mid.GetWidth();
    m_Right.SetX(midX + wM);
}

// ==========================
// SetCountRange
// 反編譯：
//   QWORD[87] = a4 (min)
//   QWORD[88] = a2 (cur)
//   QWORD[89] = a3 (max)
//   SetupCurCount();
// ==========================
void CControlCount::SetCountRange(int64_t cur, int64_t max, int64_t min)
{
    m_Min = min;
    m_Cur = cur;
    m_Max = max;
    SetupCurCount();
}

// ==========================
// SetupCurCount
// 反編譯完全對照
// ==========================
void CControlCount::SetupCurCount()
{
    // 反編譯：夾制 m_Cur
    if (m_Cur > m_Max)
        m_Cur = m_Max;
    else if (m_Cur < m_Min)
        m_Cur = m_Min;

    uint16_t rightW = m_Right.GetWidth();
    uint16_t leftW = m_Left.GetWidth();
    int caps = leftW + rightW;

    uint16_t ctrlW = GetWidth();

    if (m_Max)
    {
        // 對齊反編譯：第二次呼叫 GetWidth()（ground truth 呼叫兩次）
        double desired = (double)GetWidth() * ((double)m_Cur / (double)m_Max);
        // 對齊反編譯：ground truth 截斷為 float 再比較
        float v14 = static_cast<float>(desired);

        if (desired > 1.0)
        {
            if ((double)ctrlW >= (double)v14)
            {
                // 反編譯：Show 三張圖
                m_Left.Show();
                m_Mid.Show();
                m_Right.Show();

                double v12 = (double)v14 - (double)caps;
                if (v12 <= 0.0)
                    v12 = 0.0;
                // 對齊反編譯：float[87] = m_Mid.m_fScaleX
                m_Mid.SetScaleX((float)v12);
            }
            else
            {
                // 反編譯：Show 三張圖，用 ctrlW - caps
                m_Left.Show();
                m_Mid.Show();
                m_Right.Show();

                double v12 = (double)(ctrlW - caps);
                m_Mid.SetScaleX((float)v12);
            }
        }
        else
        {
            // 反編譯：Hide 三張圖
            m_Left.Hide();
            m_Mid.Hide();
            m_Right.Hide();
        }

        // 反編譯：
        //   v16 = float[87];  → m_Mid.m_fScaleX
        //   v17 = (float)(m_Mid.GetWidth()) * v16;
        //   v13 = (double)m_Mid.GetAbsX() + v17;
        //   m_Right.SetAbsX((__int64)v13);
        float scale = m_Mid.GetScaleX();
        float advance = (float)m_Mid.GetWidth() * scale;
        double rightAbsX = (double)m_Mid.GetAbsX() + (double)advance;
        m_Right.SetAbsX(static_cast<int>(static_cast<long long>(rightAbsX)));
    }
}
