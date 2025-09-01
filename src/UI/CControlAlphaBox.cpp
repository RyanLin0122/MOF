#include "UI/CControlAlphaBox.h"
#include "Image/CDeviceResetManager.h"
#include "Image/CDeviceManager.h"
#include <d3d9.h>
#include <cstring> // memcpy

// ==== 顏色工具 =========================================================
unsigned char CControlAlphaBox::ToByte(float v)
{
	if (v >= 1.0f) return 255;
	if (v <= 0.0f) return 0;
	return static_cast<unsigned char>(v * 255.0f + 0.5f);
}

DWORD CControlAlphaBox::PackColor(float r, float g, float b, float a)
{
	return (static_cast<DWORD>(ToByte(a)) << 24) |
		(static_cast<DWORD>(ToByte(r)) << 16) |
		(static_cast<DWORD>(ToByte(g)) << 8) |
		(static_cast<DWORD>(ToByte(b)));
}

// ==== 建構 / 解構 ======================================================
CControlAlphaBox::CControlAlphaBox()
{
	// 既定：白色不透明
	SetColor(1.f, 1.f, 1.f, 1.f);
}

CControlAlphaBox::~CControlAlphaBox()
{
	// 與基準一致：由 Reset Manager 刪除 VB
	if (m_pVBData)
	{
		CDeviceResetManager::GetInstance()->DeleteVertexBuffer(m_pVBData);
		m_pVBData = nullptr;
	}
}

// ==== Create 多載 ======================================================
void CControlAlphaBox::Create(CControlBase* pParent)
{
	if (m_bCreated) return;
	m_bCreated = true;

	m_pVBData = CDeviceResetManager::GetInstance()->CreateVertexBuffer(4u, 1u);
	if (m_pVBData)
	{
		CControlBase::Create(pParent);
	}
}

void CControlAlphaBox::Create(int x, int y, unsigned short w, unsigned short h, CControlBase* pParent)
{
	if (m_bCreated) return;
	m_bCreated = true;

	m_pVBData = CDeviceResetManager::GetInstance()->CreateVertexBuffer(4u, 1u);
	if (m_pVBData)
	{
		CControlBase::Create(x, y, w, h, pParent);
	}
}

void CControlAlphaBox::Create(int x, int y, unsigned short w, unsigned short h,
	float r, float g, float b, float a, CControlBase* pParent)
{
	if (m_bCreated) return;

	// 先掛到樹上（一定要）
	SetColor(r, g, b, a);
	CControlBase::Create(x, y, w, h, pParent);

	// 再嘗試拿 VB；拿不到也沒關係，之後可再補
	m_pVBData = CDeviceResetManager::GetInstance()->CreateVertexBuffer(4u, 1u);

	m_bCreated = true;
}

// ==== 幾何更新 =========================================================
void CControlAlphaBox::UpdateVerticesFromRect()
{
	// 與基準一致：半像素對齊（-0.5f）
	const float left = static_cast<float>(GetAbsX()) - 0.5f;
	const float top = static_cast<float>(GetAbsY()) - 0.5f;
	const float right = left + static_cast<float>(GetWidth());
	const float bottom = top + static_cast<float>(GetHeight());

	// TriangleFan：v0=LT, v1=RT, v2=RB, v3=LB
	const float z = 0.0f;
	const float rhw = 1.0f;

	m_vtx[0].x = left;  m_vtx[0].y = top;    m_vtx[0].z = z; m_vtx[0].rhw = rhw;
	m_vtx[1].x = right; m_vtx[1].y = top;    m_vtx[1].z = z; m_vtx[1].rhw = rhw;
	m_vtx[2].x = right; m_vtx[2].y = bottom; m_vtx[2].z = z; m_vtx[2].rhw = rhw;
	m_vtx[3].x = left;  m_vtx[3].y = bottom; m_vtx[3].z = z; m_vtx[3].rhw = rhw;
}

// ==== 顏色 / 透明度 ====================================================
void CControlAlphaBox::SetColor(float r, float g, float b, float a)
{
	const DWORD c = PackColor(r, g, b, a);
	for (int i = 0; i < 4; ++i) m_vtx[i].diffuse = c;
}

void CControlAlphaBox::SetColor(float r1, float g1, float b1, float a1,
	float r2, float g2, float b2, float a2,
	float r3, float g3, float b3, float a3,
	float r4, float g4, float b4, float a4)
{
	m_vtx[0].diffuse = PackColor(r1, g1, b1, a1);
	m_vtx[1].diffuse = PackColor(r2, g2, b2, a2);
	m_vtx[2].diffuse = PackColor(r3, g3, b3, a3);
	m_vtx[3].diffuse = PackColor(r4, g4, b4, a4);
}

void CControlAlphaBox::SetAlpha(unsigned char a)
{
	for (int i = 0; i < 4; ++i)
	{
		// 只換 A，保留 RGB
		m_vtx[i].diffuse = (m_vtx[i].diffuse & 0x00FFFFFFu) | (static_cast<DWORD>(a) << 24);
	}
}

// ==== 其他工具 =========================================================
void CControlAlphaBox::SetAttr(int x, int y, unsigned short w, unsigned short h,
	float r, float g, float b, float a)
{
	CControlBase::SetAbsPos(x, y);
	m_usWidth = w;
	m_usHeight = h;
	SetColor(r, g, b, a);
}

void CControlAlphaBox::SetRectInParent()
{
	if (m_pParent)
	{
		m_usWidth = m_pParent->GetWidth();
		m_usHeight = m_pParent->GetHeight();
	}
}

// ==== 繪製流程（與基準一致） ===========================================
void CControlAlphaBox::PrepareDrawing()
{
	if (!m_bIsVisible) return;
	if (!m_pVBData)    return;

	auto* mgr = CDeviceResetManager::GetInstance();
	if (mgr && mgr->IsDeviceReady() && m_pVBData) {
		UpdateVerticesFromRect();
		mgr->UpdateVertexBuffer(m_pVBData, m_vtx, sizeof(m_vtx));
	}
	CControlBase::PrepareDrawing();
}

void CControlAlphaBox::Draw()
{
	if (!m_bIsVisible) return;
	if (!m_pVBData)    return;
	if (!Device)       return;

	IDirect3DVertexBuffer9* vb = CDeviceResetManager::GetInstance()->GetVertexBuffer(m_pVBData);
	if (!vb) return;

	// 0 號貼圖設為空（基準）
	CDeviceManager::GetInstance()->SetTexture(0, nullptr);
	// 綁 VB：stream 0, stride = 20 bytes
	CDeviceManager::GetInstance()->SetStreamSource(0, vb, 0, sizeof(D3DVertex));
	// FVF = XYZRHW | DIFFUSE
	CDeviceManager::GetInstance()->SetFVF(kFVF);

	// TriangleFan 畫 2 個三角形 = 1 個矩形
	Device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);

	// 繼續畫子控制
	CControlBase::Draw();
}
