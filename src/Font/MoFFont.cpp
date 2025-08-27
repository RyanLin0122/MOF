#include "Font/MoFFont.h"
#include "Image/CDeviceManager.h" // 假設的外部依賴
#include "global.h"
#include <vector>
#include <cstdio>  // for FILE ops
#include <cstring> // for strtok_s

extern bool g_bRenderStateLocked;
extern LPDIRECT3DDEVICE9 Device;

// DJB2 hash 函式，用於從字串產生金鑰
unsigned int HashString(const char* str) {
    unsigned int hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash;
}


// 對應反編譯碼: 0x0051B1B0
MoFFont::MoFFont()
    : m_pDevice(nullptr),
    m_pVertexBuffer(nullptr),
    m_pCurrentFontTexture(nullptr),
    m_hFont(NULL),
    m_hDC(NULL),
    m_nFontHeight(0),
    m_nFontWeight(0),
    m_CharSize{ 0, 0 },
    m_nStringIndex(0),
    m_nCurrentLineX(0),
    m_fTexCoordX(0.0f),
    m_nLineCount(0),
    m_fTexCoordY(0.0f),
    m_nAtlasY(0),
    m_nPrevAtlasY(0),
    m_nMaxHeightInLine(0),
    m_nTotalLines(0),
    m_fTexU(0.0f),
    m_fTexV(0.0f),
    m_nUnusedState1(0),
    m_bShadowFlag(false),
    m_pFontInfoArray(nullptr),
    m_nFontInfoCount(0)
{
    // 1) 初始化四個頂點 (共 112 bytes)
    //    x=0, y=0, z=0.5, rhw=1.0, color=0xFFFFFFFF, u=v=0
    for (int i = 0; i < 4; ++i) {
        m_QuadVertices[i].position_x = 0.0f;
        m_QuadVertices[i].position_y = 0.0f;
        m_QuadVertices[i].position_z_or_rhw = 0.5f;
        m_QuadVertices[i].rhw_value = 1.0f;
        m_QuadVertices[i].diffuse_color = 0xFFFFFFFF;
        m_QuadVertices[i].texture_u = 0.0f;
        m_QuadVertices[i].texture_v = 0.0f;
    }

    // 2) 其他字串欄位清空
    m_szFaceName[0] = '\0';
    m_szNationCode[0] = '\0';

    // 3) STL 結構（如 m_FontCacheMap、m_TextCacheMgr）依 C++ 預設建構
    //    （對應反編譯裡的 _Nil 節點初始化；以現代 C++ 無需手動處理）
    //    參考：標準碼中的 map/_Nil 初始化片段。:contentReference[oaicite:3]{index=3}
}

// 對應反編譯碼: 0x0051B4D0
MoFFont::~MoFFont() {
    // 1) 釋放 Vertex Buffer
    if (m_pVertexBuffer) {
        m_pVertexBuffer->Release();
        m_pVertexBuffer = nullptr;
    }

    // 2) 刪除 GDI HFONT 快取中所有字型
    for (auto& kv : m_FontCacheMap) {
        if (kv.second) {
            DeleteObject(kv.second);
        }
    }
    m_FontCacheMap.clear();

    // 3) 清理字型設定表
    delete[] m_pFontInfoArray;
    m_pFontInfoArray = nullptr;

    // 備註：
    // - 標準碼中也會把 m_hFont 歸零、map 哨兵節點回收等；
    //   在現代 C++/STL 下由 RAII 處理（功能等價）。:contentReference[oaicite:4]{index=4}
}


bool MoFFont::EnsureQuadVB() {
    if (m_pVertexBuffer) return true;
    IDirect3DDevice9* dev = m_pDevice;
    if (!dev) return false;

    // 動態+只寫入，容量只需 4 顆頂點
    HRESULT hr = dev->CreateVertexBuffer(
        sizeof(GIVertex) * 4,
        D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC,
        GIVertex::FVF,
        D3DPOOL_DEFAULT,
        &m_pVertexBuffer,
        nullptr
    );
    return SUCCEEDED(hr);
}

void MoFFont::ReleaseQuadVB() {
    if (m_pVertexBuffer) {
        m_pVertexBuffer->Release();
        m_pVertexBuffer = nullptr;
    }
}

// 對應反編譯碼: 0x0051B430
void MoFFont::ResetFont() {
    if (m_pVertexBuffer) {
        m_pVertexBuffer->Release();
        m_pVertexBuffer = nullptr;
    }
    m_pDevice = nullptr;
    m_pCurrentFontTexture = nullptr;
    m_hFont = NULL;
    m_hDC = NULL;
    m_TextCacheMgr.DeleteAll();
    // ...重設所有狀態變數...
    m_nFontHeight = 0;
    m_nFontWeight = 0;
    m_szFaceName[0] = '\0';
    m_bShadowFlag = false;
}

bool MoFFont::InitFontInfo(const char* fileName) {
    auto ParsePlain = [&](const char* path) -> bool {
        FILE* p = nullptr;
        if (fopen_s(&p, path, "rb") != 0 || !p) return false;

        char buffer[2048];

        // --- 跳過前置抬頭行，直到遇到真正的表頭行 ---
        // 期待的表頭關鍵字（寬鬆判斷）
        auto is_header = [&](const char* s)->bool {
            return strstr(s, "Nation") && strstr(s, "Name") && strstr(s, "Size")
                && strstr(s, "Font") && strstr(s, "Thick");
            };

        // 讀首行，去除 UTF-8 BOM（若存在）
        if (!fgets(buffer, sizeof(buffer), p)) { fclose(p); return false; }
        if ((unsigned char)buffer[0] == 0xEF &&
            (unsigned char)buffer[1] == 0xBB &&
            (unsigned char)buffer[2] == 0xBF) {
            // 去掉BOM
            memmove(buffer, buffer + 3, strlen(buffer + 3) + 1);
        }

        // 若首行不是表頭，持續讀到表頭
        if (!is_header(buffer)) {
            bool found = false;
            while (fgets(buffer, sizeof(buffer), p)) {
                if (is_header(buffer)) { found = true; break; }
            }
            if (!found) { fclose(p); return false; }
        }

        // 到這裡，buffer 裡是表頭行；記錄資料起始位置
        long dataStart = ftell(p);

        // --- 第一次掃描：計數（跳過空行/註解） ---
        int count = 0;
        while (fgets(buffer, sizeof(buffer), p)) {
            // 跳過空行或以 '#' 開頭的註解（可依需求調整）
            char* s = buffer;
            while (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n') ++s;
            if (*s == '\0' || *s == '#') continue;
            ++count;
        }
        if (count <= 0) { fclose(p); return false; }

        // --- 配置陣列並第二次掃描：實際解析 ---
        m_pFontInfoArray = new stFontInfo[count]();
        m_nFontInfoCount = count;

        fseek(p, dataStart, SEEK_SET);

        int idx = 0;
        bool nationSet = false;

        while (idx < count && fgets(buffer, sizeof(buffer), p)) {
            // 跳過空/註解
            char* s = buffer;
            while (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n') ++s;
            if (*s == '\0' || *s == '#') continue;

            // 依欄位順序：Nation, Name, Size, Font, Thick, Comment
            char* context = nullptr;
            char* tok = strtok_s(s, "\t\r\n", &context); // token0: Nation
            if (!tok) continue;
            const char* nation = tok;

            tok = strtok_s(nullptr, "\t\r\n", &context); // token1: Name -> KeyName
            if (tok) strncpy_s(m_pFontInfoArray[idx].szKeyName, tok, _TRUNCATE);

            tok = strtok_s(nullptr, "\t\r\n", &context); // token2: Size -> Height
            if (tok) m_pFontInfoArray[idx].nHeight = atoi(tok);

            tok = strtok_s(nullptr, "\t\r\n", &context); // token3: Font -> FaceName
            if (tok) strncpy_s(m_pFontInfoArray[idx].szFaceName, tok, _TRUNCATE);

            tok = strtok_s(nullptr, "\t\r\n", &context); // token4: Thick -> Weight
            if (tok) m_pFontInfoArray[idx].nWeight = atoi(tok);

            // （如需 Comment 可再讀下一個 token）
            // tok = strtok_s(nullptr, "\t\r\n", &context); // token5: Comment (optional)

            // 第一次遇到 Nation，存為 m_szNationCode
            if (!nationSet && nation && *nation) {
                strncpy_s(m_szNationCode, nation, _TRUNCATE);
                nationSet = true;
            }

            ++idx;
        }

        // 若實際解析到的筆數少於計數，縮減 m_nFontInfoCount
        if (idx < m_nFontInfoCount) {
            m_nFontInfoCount = idx;
        }

        fclose(p);
        return m_nFontInfoCount > 0;
        };

    // 1) 先試純文字
    if (ParsePlain(fileName)) return true;

    // 2) 純文字失敗，再試解密 -> 解析
    const char* TMP = "tmp.dat";
    unsigned int ok = FileCrypt::GetInstance()->DecoderFileCrypt(fileName, TMP);
    if (ok) {
        bool parsed = ParsePlain(TMP);
        DeleteFileA(TMP);
        if (parsed) return true;
    }
    else {
        // 解密失敗也要確保 tmp 被清掉（以防前一次殘留）
        DeleteFileA(TMP);
    }

    MessageBoxA(0, "FontInfo Error", "Master of Fantasy", 0);
    return false;
}


// 對應反編譯碼: 0x0051B980
stFontInfo* MoFFont::GetFontInfo(const char* keyName) {
    if (!m_pFontInfoArray || m_nFontInfoCount <= 0) {
        return nullptr;
    }
    for (int i = 0; i < m_nFontInfoCount; ++i) {
        if (_stricmp(m_pFontInfoArray[i].szKeyName, keyName) == 0) {
            return &m_pFontInfoArray[i];
        }
    }
    return nullptr;
}

// 對應反編譯碼: 0x0051BA00
bool MoFFont::CreateMoFFont(IDirect3DDevice9* pDevice, int height, int width, const char* faceName, int weight) {
    m_pDevice = pDevice;
    if (!m_pDevice) return false;

    if (m_pVertexBuffer) m_pVertexBuffer->Release();
    m_pDevice->CreateVertexBuffer(
        sizeof(GIVertex) * 4,
        D3DUSAGE_WRITEONLY,
        GIVertex::FVF,
        D3DPOOL_MANAGED,
        &m_pVertexBuffer,
        nullptr
    );

    SetFont(height, faceName, weight);
    m_pCurrentFontTexture = CMoFFontTextureManager::GetInstance()->GetFontTexture();
    return true;
}

// 對應反編譯碼: 0x0051BBB0
bool MoFFont::CreateMoFFont(IDirect3DDevice9* pDevice, const char* fontKey) {
    stFontInfo* pInfo = GetFontInfo(fontKey);
    if (pInfo) {
        return CreateMoFFont(pDevice, pInfo->nHeight, 0, pInfo->szFaceName, pInfo->nWeight);
    }
    return false;
}

// 對應反編譯碼: 0x0051BDF0
void MoFFont::SetFont(const char* fontKey) {
    stFontInfo* pInfo = GetFontInfo(fontKey);
    if (pInfo) {
        SetFont(pInfo->nHeight, pInfo->szFaceName, pInfo->nWeight);
    }
}

// 對應反編譯碼: 0x0051BBF0
void MoFFont::SetFont(int height, const char* faceName, int weight) {
    // 檢查字型屬性是否與當前完全相同，若是則無需更換
    if (m_nFontHeight == height && m_nFontWeight == weight && _stricmp(m_szFaceName, faceName) == 0) {
        return;
    }

    m_nFontHeight = height;
    m_nFontWeight = weight;
    strncpy_s(m_szFaceName, faceName, sizeof(m_szFaceName) - 1);

    // 從快取或 GDI 取得 HFONT 控制代碼
    m_hFont = GetCachedOrCreateFont(height, 0, faceName, weight);
}

// 對應反編譯碼: 0x0051BE30
void MoFFont::SetBlendType(unsigned char blendType) {
    if (g_bRenderStateLocked) {
        return;
    }
    // 假設 g_clDeviceManager 是已還原的 CDeviceManager 全域實例
    switch (blendType) {
    case 0: // 一般 Alpha 混合
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);  // 5
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA); // 6
        break;
    case 1: // 加法混合
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE); // 2
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE); // 2
        break;
    case 2: // 乘法混合 (原始碼為 SRC:1, DEST:4)
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE); // 1
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR); // 4
        break;
    default:
        // 預設恢復為一般 Alpha 混合
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        break;
    }
}

// 對應反編譯碼: 0x0051C880
void MoFFont::SetTextLineA(int x, int y, DWORD color, const char* text, char alignment, int clipLeft, int clipRight) {
    if (color == 0xFFFFFFFF) { // 純白，不透明
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE); // 2
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE); // 2
    }
    else if (color == 0xFF000000) { // 純黑，不透明
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE); // 1
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO); // 4 in original -> D3DBLEND_SRCCOLOR
    }
    else {
        // 其他顏色，先畫一層底，再疊加顏色
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);
        SetTextLine(x, y, 0xFFFFFFFF, text, alignment, clipLeft, clipRight);

        CDeviceManager::GetInstance()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
    }
    SetTextLine(x, y, color, text, alignment, clipLeft, clipRight);

    // 恢復預設混合模式
    CDeviceManager::GetInstance()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    CDeviceManager::GetInstance()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
}

// 對應反編譯碼: 0x0051D000
void MoFFont::SetTextLineShadow(int x, int y, DWORD shadowColor, const char* text, char alignment) {
    // 八個方向繪製陰影
    SetTextLineA(x - 1, y, shadowColor, text, alignment, -1, -1);
    SetTextLineA(x - 1, y - 1, shadowColor, text, alignment, -1, -1);
    SetTextLineA(x, y - 1, shadowColor, text, alignment, -1, -1);
    SetTextLineA(x + 1, y - 1, shadowColor, text, alignment, -1, -1);
    SetTextLineA(x + 1, y, shadowColor, text, alignment, -1, -1);
    SetTextLineA(x + 1, y + 1, shadowColor, text, alignment, -1, -1);
    SetTextLineA(x, y + 1, shadowColor, text, alignment, -1, -1);
    SetTextLineA(x - 1, y + 1, shadowColor, text, alignment, -1, -1);
}

// 對應反編譯碼: 0x0051C9A0
void MoFFont::SetTextBox(RECT* pRect, DWORD color, const char* text, int lineSpacing, char alignment, int unknownFlag) {
    if (!text || *text == '\0' || !pRect) return;

    int currentX = pRect->left;
    int currentY = pRect->top;
    int boxWidth = pRect->right; // 在原始碼中 right 被當作寬度使用

    int lineStartIdx = 0; // 當前行第一個字元的索引
    int currentIdx = 0;   // 當前處理到的字元索引
    int lastSpaceIdx = -1; // 最後一個空格的索引，用於優雅換行

    char lineBuffer[1024] = { 0 };
    int lineBufferIdx = 0;
    int currentLineWidth = 0;

    HDC hdc = GetWindowDC(NULL);
    HGDIOBJ oldFont = SelectObject(hdc, m_hFont);

    while (text[currentIdx] != '\0') {
        int charLen = IsDBCSLeadByte(text[currentIdx]) ? 2 : 1;

        if (text[currentIdx] == '\n') {
            // 手動換行
            strncpy_s(lineBuffer, &text[lineStartIdx], currentIdx - lineStartIdx);
            lineBuffer[currentIdx - lineStartIdx] = '\0';
            SetTextLine(currentX, currentY, color, lineBuffer, alignment);

            currentY += m_nFontHeight + lineSpacing;
            currentIdx++;
            lineStartIdx = currentIdx;
            lastSpaceIdx = -1;
            currentLineWidth = 0;
            continue;
        }

        // 測量加入此字元後的寬度
        SIZE charSize;
        GetTextExtentPoint32A(hdc, &text[currentIdx], charLen, &charSize);

        if (currentLineWidth + charSize.cx > boxWidth) {
            // 超出寬度，需要換行
            int breakIdx = (lastSpaceIdx != -1) ? lastSpaceIdx : currentIdx;

            strncpy_s(lineBuffer, &text[lineStartIdx], breakIdx - lineStartIdx);
            lineBuffer[breakIdx - lineStartIdx] = '\0';
            SetTextLine(currentX, currentY, color, lineBuffer, alignment);

            currentY += m_nFontHeight + lineSpacing;
            currentIdx = (lastSpaceIdx != -1) ? breakIdx + 1 : breakIdx;
            lineStartIdx = currentIdx;
            lastSpaceIdx = -1;
            currentLineWidth = 0;
        }
        else {
            if (text[currentIdx] == ' ') {
                lastSpaceIdx = currentIdx;
            }
            currentLineWidth += charSize.cx;
            currentIdx += charLen;
        }
    }

    // 繪製剩餘的最後一行
    if (lineStartIdx < currentIdx) {
        strncpy_s(lineBuffer, &text[lineStartIdx], currentIdx - lineStartIdx);
        lineBuffer[currentIdx - lineStartIdx] = '\0';
        SetTextLine(currentX, currentY, color, lineBuffer, alignment);
    }

    SelectObject(hdc, oldFont);
    ReleaseDC(NULL, hdc);
}


// 對應反編譯碼: 0x0051CE60
void MoFFont::SetTextBoxA(RECT* pRect, DWORD color, const char* text, int lineSpacing, char alignment, int unknownFlag) {
    // 此函式是 SetTextBox 的 Wrapper，處理不同的混合模式
    // 邏輯與 SetTextLineA 相似
    if (color == 0xFFFFFFFF || color == 0xFF000000) {
        // ... 設定對應的 Blend State ...
        SetTextBox(pRect, color, text, lineSpacing, alignment, unknownFlag);
    }
    else {
        // ... 先畫底，再疊加顏色 ...
        SetTextBox(pRect, 0xFFFFFFFF, text, lineSpacing, alignment, unknownFlag); // 畫底
        SetTextBox(pRect, color, text, lineSpacing, alignment, unknownFlag); // 疊色
    }
    // ... 恢復預設 Blend State ...
}

// 對應反編譯碼: 0x0051D270
void MoFFont::GetTextLength(int* pWidth, int* pHeight, const char* fontKey, const char* text) {
    stFontInfo* pInfo = GetFontInfo(fontKey);
    if (pInfo) {
        GetTextLength(pWidth, pHeight, pInfo->nHeight, pInfo->szFaceName, text, pInfo->nWeight);
    }
    else {
        if (pWidth) *pWidth = 0;
        if (pHeight) *pHeight = 0;
    }
}

// 對應反編譯碼: 0x0051D100
void MoFFont::GetTextLength(int* pWidth, int* pHeight, int height, const char* faceName, const char* text, int weight) {
    HFONT hFont = GetCachedOrCreateFont(height, 0, faceName, weight);

    HDC hdc = GetWindowDC(NULL);
    HGDIOBJ oldFont = SelectObject(hdc, hFont);

    SIZE size;
    GetTextExtentPoint32A(hdc, text, strlen(text), &size);

    SelectObject(hdc, oldFont);
    ReleaseDC(NULL, hdc);

    if (pWidth) *pWidth = size.cx;
    if (pHeight) *pHeight = size.cy;
}

// 對應反編譯碼: 0x0051CCC0
int MoFFont::GetCharByteByLine(unsigned short lineWidth, const char* text, unsigned char* lineBreakBytes, int maxLines) {
    if (!text || !lineBreakBytes) return 0;

    HDC hdc = GetWindowDC(NULL);
    HGDIOBJ oldFont = SelectObject(hdc, m_hFont);

    int lineCount = 0;
    int currentByte = 0;
    int lineStartByte = 0;
    int currentLineWidth = 0;

    while (text[currentByte] != '\0' && lineCount < maxLines) {
        int charLen = IsDBCSLeadByte(text[currentByte]) ? 2 : 1;
        SIZE charSize;
        GetTextExtentPoint32A(hdc, &text[currentByte], charLen, &charSize);

        if (text[currentByte] == '\n' || currentLineWidth + charSize.cx > lineWidth) {
            lineBreakBytes[lineCount] = currentByte;
            lineCount++;
            lineStartByte = (text[currentByte] == '\n') ? currentByte + 1 : currentByte;
            currentLineWidth = 0;
            if (text[currentByte] == '\n') currentByte++;
        }
        else {
            currentLineWidth += charSize.cx;
            currentByte += charLen;
        }
    }

    if (lineCount < maxLines) {
        lineBreakBytes[lineCount] = currentByte;
        lineCount++;
    }

    SelectObject(hdc, oldFont);
    ReleaseDC(NULL, hdc);
    return lineCount;
}

#include <cstdio> // for sprintf_s

/**
 * @brief 產生一個 DJB2 雜湊值。
 * @param str 要計算雜湊值的字串。
 * @return 32 位元的雜湊值。
 */
static unsigned int GenerateFontHash(const char* str) {
    unsigned int hash = 5381;
    int c;
    while ((c = *str++)) {
        // hash = hash * 33 + c
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

/**
 * @brief 從快取中獲取 HFONT，如果不存在則創建一個新的並加入快取。
 * * @param height 字型高度。
 * @param width 字型寬度。
 * @param faceName 字型名稱。
 * @param weight 字型粗細。
 * @return HFONT 控制代碼。
 *
 * 對應反編譯碼: 邏輯分散在 0x0051BA00 (CreateMoFFont) 和 0x0051BBF0 (SetFont) 中。
 */
HFONT MoFFont::GetCachedOrCreateFont(int height, int width, const char* faceName, int weight) {
    // 1. 根據字型屬性組合出一個唯一的字串作為 Key。
    char keyBuffer[256];
    sprintf_s(keyBuffer, sizeof(keyBuffer), "%d_%d_%s_%d", height, width, faceName, weight);

    // 2. 計算字串的雜湊值。
    unsigned int fontHash = GenerateFontHash(keyBuffer);

    // 3. 在 map 中查找這個雜湊值。
    auto it = m_FontCacheMap.find(fontHash);

    if (it != m_FontCacheMap.end()) {
        // 4a. 快取命中：直接返回已有的 HFONT。
        return it->second;
    }
    else {
        // 4b. 快取未命中：呼叫 GDI 創建新的 HFONT。
        HFONT hNewFont = CreateFontA(
            height,          // nHeight
            width,           // nWidth
            0,               // nEscapement
            0,               // nOrientation
            weight,          // fnWeight
            FALSE,           // fdwItalic
            FALSE,           // fdwUnderline
            FALSE,           // fdwStrikeOut
            DEFAULT_CHARSET, // fdwCharSet (原始碼使用 1)
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, // 原始碼使用 3
            DEFAULT_PITCH | FF_DONTCARE,
            faceName
        );

        // 5. 將新創建的 HFONT 存入快取中，然後返回。
        m_FontCacheMap[fontHash] = hNewFont;
        return hNewFont;
    }
}

/**
 * @brief 判斷當前的地區設定是否為 ANSI 字元集。
 * * @return 如果地區碼為 4 ("INDONESIA")，返回 true，否則返回 false。
 *
 * 對應反編譯碼: 0x0051D850
 */
bool MoFFont::IsAnsiCharSet() {
    // 此函式直接依賴 GetNationCode() 的返回值。
    // 在 GetNationCode() 中，傳回值 4 對應 "INDONESIA"。
    return this->GetNationCode() == 4;
}

static const int ATLAS_W = 128;   // 規格限定
static const int ATLAS_H = 128;   // 規格限定

// 依 Font.txt：0 = Left, 1 = Center, 2 = Right
static inline int AlignCode(char a) { return (a < 0 || a > 2) ? 0 : int(a); }

// 以 DBCS 判斷一個「字」佔幾個位元組（1 或 2）
static inline int CharBytesDBCS(unsigned char c) { return ::IsDBCSLeadByte(c) ? 2 : 1; }

// 將像素座標轉為 UV（0~1）
static inline float U(int x) { return x * (1.0f / ATLAS_W); }
static inline float V(int y) { return y * (1.0f / ATLAS_H); }

// 依照 Font.txt 規格，在 Surface HDC 上「畫字到圖集」
static inline void BlitGlyphToAtlas(HDC hdc, int atlasX, int atlasY, const char* s, int nBytes) {
    // 規格行為：直接 TextOutA 寫進圖集 surface（A8R8G8B8）
    ::TextOutA(hdc, atlasX, atlasY, s, nBytes);
}

// 以 TILInfo 內的 TLILInfo 片段，提交到 VB / Draw（此處僅示意，請接上你的渲染封裝）
void MoFFont::DrawSegments(int x, int y, DWORD color, int align, int clipL, int clipR, TILInfo* pLine,
    /* 你的渲染資源 */ void* pDevice, void* pVB) {
    // 計算對齊位移：Left/Center/Right
    int totalWidth = pLine->m_nTextWidth;
    float xOffset = 0.0f;
    if (align == 1)      xOffset = float(-totalWidth / 2);
    else if (align == 2) xOffset = float(-totalWidth);

    // 走訪每個 TLILInfo 片段，套用裁切並畫
    for (TLILInfo* seg = pLine->m_LineMgr.m_pHead; seg; seg = seg->m_pNext) {
        if (!seg->m_pFTInfo || !seg->m_pFTInfo->m_pTexture) continue;

        // 片段螢幕座標（左上）
        float sx = x + xOffset + seg->m_fPosX;
        float sy = y + seg->m_fPosY;
        float sw = seg->m_fWidth;

        float u1 = seg->m_fTexU1, v1 = seg->m_fTexV1;
        float u2 = seg->m_fTexU2, v2 = seg->m_fTexV2;

        // --- 裁切（只有 clipL/clipR 都有效才啟用，依 Font.txt） ---
        if (clipL != -1 && clipR != -1) {
            // 左裁切
            if (sx < clipL) {
                float cut = (clipL - sx);
                sx = float(clipL) - 0.5f;         // 與規格一致的 -0.5 取整補償
                u1 += (cut / ATLAS_W);
                sw -= cut;
            }
            // 右裁切
            if (sx + sw > clipR) {
                float cut = (sx + sw - clipR);
                // sx 不動，縮寬
                sw -= cut;
                u2 -= (cut / ATLAS_W);
            }
            if (sw <= 0.0f) continue; // 完全被裁掉
        }

        // --- 設定貼圖 / RenderState（g_bRenderStateLocked==0 才改狀態） ---
        if (g_bRenderStateLocked) {
            // 規格：此旗標為真時，不進行任何狀態變更與繪製
            return;
        }

        // 1) 計算四個頂點的座標與 UV（此時 sx/sy/sw、u1/v1/u2/v2 已含對齊與裁切修正）
        float left = sx;
        float top = sy;
        float right = sx + sw;
        float bottom = sy + m_nFontHeight;

        // 2) 填入 4 個 GIVertex（順序採 TRIANGLESTRIP：LT, RT, LB, RB）
        m_QuadVertices[0].position_x = left;
        m_QuadVertices[0].position_y = top;
        m_QuadVertices[0].position_z_or_rhw = 0.5f;
        m_QuadVertices[0].rhw_value = 1.0f;
        m_QuadVertices[0].diffuse_color = color;
        m_QuadVertices[0].texture_u = u1;
        m_QuadVertices[0].texture_v = v1;

        m_QuadVertices[1].position_x = right;
        m_QuadVertices[1].position_y = top;
        m_QuadVertices[1].position_z_or_rhw = 0.5f;
        m_QuadVertices[1].rhw_value = 1.0f;
        m_QuadVertices[1].diffuse_color = color;
        m_QuadVertices[1].texture_u = u2;
        m_QuadVertices[1].texture_v = v1;

        m_QuadVertices[2].position_x = left;
        m_QuadVertices[2].position_y = bottom;
        m_QuadVertices[2].position_z_or_rhw = 0.5f;
        m_QuadVertices[2].rhw_value = 1.0f;
        m_QuadVertices[2].diffuse_color = color;
        m_QuadVertices[2].texture_u = u1;
        m_QuadVertices[2].texture_v = v2;

        m_QuadVertices[3].position_x = right;
        m_QuadVertices[3].position_y = bottom;
        m_QuadVertices[3].position_z_or_rhw = 0.5f;
        m_QuadVertices[3].rhw_value = 1.0f;
        m_QuadVertices[3].diffuse_color = color;
        m_QuadVertices[3].texture_u = u2;
        m_QuadVertices[3].texture_v = v2;

        // 3) 將 4 個頂點（總計 112 bytes = 0x70）寫入預先建立好的 VertexBuffer
        void* pVBData = nullptr;
        m_pVertexBuffer->Lock(0, 0, &pVBData, 0);
        std::memcpy(pVBData, m_QuadVertices, sizeof(m_QuadVertices)); // 0x70 bytes
        m_pVertexBuffer->Unlock();

        // 4) 綁定貼圖與頂點格式/串流（stride 0x1C = 28 bytes）
        CDeviceManager::GetInstance()->SetTexture(0, seg->m_pFTInfo->m_pTexture);
        CDeviceManager::GetInstance()->SetFVF(GIVertex::FVF);
        CDeviceManager::GetInstance()->SetStreamSource(0, m_pVertexBuffer, 0, sizeof(GIVertex));

        // 5) 繪製：四點 TRIANGLESTRIP -> 2 個三角形
        Device->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
    }
}

void MoFFont::SetTextLine(int x, int y, DWORD color, const char* text, char alignment, int clipLeft, int clipRight)
{
    if (!text || text[0] == '\0') return;

    // ---------- 1) 快取命中路徑：相同字串 + 同字型屬性 (Face/Height/Weight) ----------
    TILInfo* pCached = nullptr;
    for (TILInfo* it = m_TextCacheMgr.m_pHead; it; it = it->m_pNext) {
        if (!it->m_pszString) continue;
        if (_strcmpi(it->m_pszString, text) == 0 &&
            std::strcmp(it->m_szFaceName, m_szFaceName) == 0 &&          // m_szFaceName 需是 MoFFont 當前 face name
            it->m_nFontHeight == m_nFontHeight &&
            it->m_nFontWeight == m_nFontWeight) {
            pCached = it;
            break;
        }
    }

    if (pCached) {
        // 命中：提升 TILInfo 引用、標記本幀使用，並把其所有片段的 FTInfo 引用 +1（規格行為）
        pCached->m_wRefCnt++;
        pCached->m_bIsUsedThisFrame = 1;
        for (TLILInfo* seg = pCached->m_LineMgr.m_pHead; seg; seg = seg->m_pNext) {
            if (seg->m_pFTInfo) {
                seg->m_pFTInfo->m_wRefCnt++;
            }
        }

        // 直接依照規格進行對齊 / 裁切 / 繪製
        DrawSegments(x, y, color, AlignCode(alignment), clipLeft, clipRight, pCached, /*pDevice*/nullptr, /*pVB*/nullptr);
        return;
    }

    // ---------- 2) 快取未命中：建立新的 TILInfo，逐字排版並寫入圖集 ----------
    // 2.1 取第一頁 FTInfo 與 HDC
    FTInfo* pPage = CMoFFontTextureManager::GetInstance()->GetFontTexture();
    if (!pPage || !pPage->m_pSurface) return;

    HDC hdc = nullptr;
    pPage->m_pSurface->GetDC(&hdc);
    if (!hdc) return;

    // Select 當前 HFONT，設定前景/背景（規格：BkMode/顏色）
    HGDIOBJ hOldFont = ::SelectObject(hdc, m_hFont);
    ::SetBkMode(hdc, TRANSPARENT);
    ::SetTextColor(hdc, RGB(255, 255, 255)); // 實際顏色不重要，渲染時用貼圖 + color

    // 2.2 配置一個新的 TILInfo 進快取
    TILInfo* pLine = m_TextCacheMgr.Add();
    pLine->m_bIsUsedThisFrame = 1;
    pLine->m_wRefCnt = 1;
    // 複製字串與字型屬性
    {
        size_t n = std::strlen(text);
        pLine->m_pszString = new char[n + 1];
        std::memcpy(pLine->m_pszString, text, n + 1);
        std::strcpy(pLine->m_szFaceName, m_szFaceName);
        pLine->m_nFontHeight = m_nFontHeight;
        pLine->m_nFontWeight = m_nFontWeight;
    }

    // 2.3 圖集排版狀態
    int penX = 0;                // 本行在圖集的 X 游標
    int penY = m_nAtlasY;        // 當前圖集 Y 起點（this->m_nAtlasY），初始 0
    int lineH = m_nFontHeight;   // 規格：以字高為行高，必要時更新本行最大高度
    int usedW = 0;               // 此字串渲染總寬（用於對齊）
    int totalLines = 1;          // 本字串行數

    TLILInfo* seg = pLine->m_LineMgr.Add(); // 建立第一個片段
    seg->m_pFTInfo = pPage;                 // 初始綁定本頁
    pPage->m_wRefCnt++;                     // 片段引用紋理頁（規格）

    // 片段的 UV 起點綁現在的 penX/penY
    seg->m_fTexU1 = U(penX);
    seg->m_fTexV1 = V(penY);
    seg->m_fTexU2 = seg->m_fTexU1;
    seg->m_fTexV2 = seg->m_fTexV1;
    seg->m_fPosX = 0.0f;          // 從螢幕起點（尚未加上對齊/呼叫座標）開始
    seg->m_fPosY = float((totalLines - 1) * lineH);
    seg->m_fWidth = 0.0f;

    // 2.4 逐字量測與寫入（DBCS 支援）
    SIZE tm = { 0, 0 };
    int i = 0;
    const int N = int(std::strlen(text));
    while (i < N) {
        int cbytes = CharBytesDBCS((unsigned char)text[i]);
        if (i + cbytes > N) cbytes = 1; // 防呆
        // 強制換行（若規格支援 '\n'）
        if (text[i] == '\n') {
            // 結束本片段，換行
            seg->m_fTexU2 = U(penX);
            seg->m_fTexV2 = V(penY + lineH);
            seg->m_fWidth = float(penX - int(seg->m_fPosX));

            // 開新片段
            seg = pLine->m_LineMgr.Add();
            // 換行：X 歸零、Y 前進、更新行數
            penX = 0;
            penY += lineH;
            totalLines++;

            // 若超頁（Y + lineH > 128）→ 換頁
            if (penY + lineH > ATLAS_H) {
                pPage->m_pSurface->ReleaseDC(hdc);
                hdc = nullptr;

                pPage = CMoFFontTextureManager::GetInstance()->GetFontTexture();
                if (!pPage || !pPage->m_pSurface) return;
                pPage->m_pSurface->GetDC(&hdc);
                if (!hdc) return;
                ::SelectObject(hdc, m_hFont);
                ::SetBkMode(hdc, TRANSPARENT);
                ::SetTextColor(hdc, RGB(255, 255, 255));

                // 新頁：筆 Y 從 0 開始
                penY = 0;
            }

            // 新片段綁定新位置/頁
            seg->m_pFTInfo = pPage;
            pPage->m_wRefCnt++;
            seg->m_fTexU1 = U(penX);
            seg->m_fTexV1 = V(penY);
            seg->m_fTexU2 = seg->m_fTexU1;
            seg->m_fTexV2 = seg->m_fTexV1;
            seg->m_fPosX = 0.0f;
            seg->m_fPosY = float((totalLines - 1) * lineH);
            seg->m_fWidth = 0.0f;

            ++i;
            continue;
        }

        // 量測
        ::GetTextExtentPoint32A(hdc, &text[i], cbytes, &tm);
        int cw = tm.cx; // 字寬
        // 是否需要換行（X + cw >= 128）
        if (penX + cw > ATLAS_W) {
            // 結束當前片段，更新 UV2 與寬度
            seg->m_fTexU2 = U(penX);
            seg->m_fTexV2 = V(penY + lineH);
            seg->m_fWidth = float(penX - int(seg->m_fPosX));

            // 換行
            penX = 0;
            penY += lineH;
            totalLines++;

            // 超頁則換頁
            if (penY + lineH > ATLAS_H) {
                pPage->m_pSurface->ReleaseDC(hdc);
                hdc = nullptr;

                pPage = CMoFFontTextureManager::GetInstance()->GetFontTexture();
                if (!pPage || !pPage->m_pSurface) return;
                pPage->m_pSurface->GetDC(&hdc);
                if (!hdc) return;
                ::SelectObject(hdc, m_hFont);
                ::SetBkMode(hdc, TRANSPARENT);
                ::SetTextColor(hdc, RGB(255, 255, 255));

                penY = 0;
            }

            // 新片段綁定
            seg = pLine->m_LineMgr.Add();
            seg->m_pFTInfo = pPage;
            pPage->m_wRefCnt++;
            seg->m_fTexU1 = U(penX);
            seg->m_fTexV1 = V(penY);
            seg->m_fTexU2 = seg->m_fTexU1;
            seg->m_fTexV2 = seg->m_fTexV1;
            seg->m_fPosX = float(usedW);        // 螢幕 X 起點接續（用於對齊）
            seg->m_fPosY = float((totalLines - 1) * lineH);
            seg->m_fWidth = 0.0f;
        }

        // 把字畫進圖集（在 penX, penY）
        BlitGlyphToAtlas(hdc, penX, penY, &text[i], cbytes);

        // 前進 atlas X 與總寬
        penX += cw;
        usedW += cw;

        // 片段暫存 U2
        seg->m_fTexU2 = U(penX);
        seg->m_fTexV2 = V(penY + lineH);

        i += cbytes;
    }

    // 結尾：補上最後片段寬度
    seg->m_fWidth = float(penX - int(seg->m_fPosX));

    // 釋放 DC
    if (hdc) {
        pPage->m_pSurface->ReleaseDC(hdc);
        hdc = nullptr;
    }

    // 更新 TILInfo 的總寬與字高（供對齊/裁切）
    pLine->m_nTextWidth = usedW;
    pLine->m_nFontHeight = lineH;

    // ---------- 3) 對齊 / 裁切 / 繪製 ----------
    DrawSegments(x, y, color, AlignCode(alignment), clipLeft, clipRight, pLine, /*pDevice*/nullptr, /*pVB*/nullptr);
}

// 對應反編譯碼: 0x0051CFA0
void MoFFont::MoFFontResourceManager() {
    std::vector<TILInfo*> toDelete;
    for (TILInfo* pNode = m_TextCacheMgr.m_pHead; pNode; pNode = pNode->m_pNext) {
        if (!pNode->m_bIsUsedThisFrame) {
            if (--pNode->m_wRefCnt <= 0) {
                toDelete.push_back(pNode);
            }
        }
        pNode->m_bIsUsedThisFrame = FALSE;
    }

    for (TILInfo* pNode : toDelete) {
        for (TLILInfo* pLine = pNode->m_LineMgr.m_pHead; pLine; pLine = pLine->m_pNext) {
            CMoFFontTextureManager::GetInstance()->DeleteFontTexture(pLine->m_pFTInfo);
        }
        m_TextCacheMgr.Delete(pNode);
    }
}

// 對應反編譯碼: 0x0051D700
int MoFFont::GetNationCode() {
    if (_stricmp(m_szNationCode, "KOREA") == 0) return 1;
    if (_stricmp(m_szNationCode, "JAPAN") == 0) return 2;
    if (_stricmp(m_szNationCode, "TAIWAN") == 0) return 3;
    if (_stricmp(m_szNationCode, "INDONESIA") == 0) return 4;
    if (_stricmp(m_szNationCode, "HONGKONG") == 0) return 5;
    return 0;
}