#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include <windows.h>
#include <map>
#include <string>

// 包含所有已還原的輔助類別
#include "Image/GIVertex.h"
#include "Font/TextInfoListMgr.h"
#include "Font/CMoFFontTextureManager.h"
#include "Font/FileCrypt.h"
#include "Font/stFontInfo.h"

/**
 * @class MoFFont
 * @brief 核心字型渲染引擎。
 *
 * 這是對 MOFFont.cpp 內所有函式進行完整還原的最終成果。
 * 此類別整合了 GDI 字型處理、Direct3D 渲染、紋理池管理以及渲染結果快取，
 * 構成一個高效能的遊戲字型系統。
 */
class MoFFont {
public:
    MoFFont();
    ~MoFFont();

    // --- 初始化與設定 ---
    bool InitFontInfo(const char* fileName);
    bool CreateMoFFont(IDirect3DDevice9* pDevice, const char* fontKey);
    bool CreateMoFFont(IDirect3DDevice9* pDevice, int height, int width, const wchar_t* faceName, int weight);
    void SetFont(const char* fontKey);
    void SetFont(int height, const wchar_t* faceName, int weight);
    void ResetFont();

    // --- 渲染 API ---
    void SetBlendType(unsigned char blendType);
    void SetTextLine(int x, int y, DWORD color, const char* text, char alignment = 0, int clipLeft = -1, int clipRight = -1);
    void SetTextLineA(int x, int y, DWORD color, const char* text, char alignment = 0, int clipLeft = -1, int clipRight = -1);
    void SetTextBox(RECT* pRect, DWORD color, const char* text, int lineSpacing = 0, char alignment = 0, int unknownFlag = 0);
    void SetTextBoxA(RECT* pRect, DWORD color, const char* text, int lineSpacing = 0, char alignment = 0, int unknownFlag = 0);
    void SetTextLineShadow(int x, int y, DWORD shadowColor, const char* text, char alignment = 0);

    // --- 資訊查詢 ---
    void GetTextLength(int* pWidth, int* pHeight, const char* fontKey, const char* text);
    void GetTextLength(int* pWidth, int* pHeight, int height, const wchar_t* faceName, const char* text, int weight);
    int GetCharByteByLine(unsigned short lineWidth, const char* text, unsigned char* lineBreakBytes, int maxLines);
    stFontInfo* GetFontInfo(const char* keyName);
    int GetNationCode();

private:
    // --- 內部輔助函式 ---
    HFONT GetCachedOrCreateFont(int height, int width, const wchar_t* faceName, int weight);
    bool IsAnsiCharSet();

    // --- 小工具：確保/釋放 Quad VB ---
    bool EnsureQuadVB();
    void ReleaseQuadVB();

    void DrawSegments(int x, int y, DWORD color, int align, int clipL, int clipR, TILInfo* pLine,
        /* 你的渲染資源 */ void* pDevice, void* pVB);
public:
    // --- 資源管理 ---
    // 此函式應每幀呼叫一次，以清理未使用的快取
    void MoFFontResourceManager();

private:
    // --- 成員變數 (對應反編譯碼中的記憶體位移) ---
    IDirect3DDevice9* m_pDevice;             // +0
    IDirect3DVertexBuffer9* m_pVertexBuffer;       // +4
    GIVertex                 m_QuadVertices[4];     // +8 (112 bytes)

    FTInfo* m_pCurrentFontTexture; // +120
    HFONT                    m_hFont;               // +124 (當前 GDI 字型控制代碼)

    // GDI HFONT 快取，避免重複創建
    std::map<unsigned int, HFONT> m_FontCacheMap;   // +128

    // 當前字型屬性
    wchar_t                  m_wszFaceName[32];      // +144 (根據 strcpy 推斷)
    int                      m_nFontHeight;         // +316
    int                      m_nFontWeight;         // +320

    HDC                      m_hDC;                 // +164 (當前紋理的 Device Context)
    TextInfoListMgr          m_TextCacheMgr;        // +168 (頂層快取管理器)
    SIZE                     m_CharSize;            // +180 (用於 GDI GetTextExtentPoint32A)

    // --- 字型圖集 (Font Atlas) 填充狀態 ---
    int                      m_nStringIndex;        // +188 (當前處理字串的索引)
    int                      m_nCurrentLineX;       // +192 (紋理上當前行的 X 座標)
    float                    m_fTexCoordX;          // +200
    int                      m_nLineCount;          // +204
    float                    m_fTexCoordY;          // +208
    int                      m_nAtlasY;             // +212 (紋理上 Y 座標)
    int                      m_nPrevAtlasY;         // +216
    int                      m_nMaxHeightInLine;    // +220
    int                      m_nTotalLines;         // +224
    float                    m_fTexU;               // +228
    float                    m_fTexV;               // +232
    int                      m_nUnusedState1;       // +236
    bool                     m_bShadowFlag;         // +240

    // --- 字型設定檔資訊 ---
    stFontInfo* m_pFontInfoArray;      // +244
    int                      m_nFontInfoCount;      // +248
    char                     m_szNationCode[16];    // +252 (根據 strcpy 推斷)
};