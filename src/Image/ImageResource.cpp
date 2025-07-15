#include "Image/ImageResource.h"
#include <cassert> // 用於 assert

// 假設這些外部相依性在連結時可用
extern LPDIRECT3DDEVICE9 Device;
void* g_pMsgBoxMgr = nullptr;

// 輔助函式，用於安全地釋放 COM 物件
template<typename T>
void SafeRelease(T*& p) {
    if (p) {
        p->Release();
        p = nullptr;
    }
}


ImageResource::ImageResource() {
    m_pTexture = nullptr;
    m_pAnimationFrames = nullptr;
    m_pImageData = nullptr;
    ResetGIData(); // 使用 ResetGIData 來初始化所有成員
}

ImageResource::~ImageResource() {
    ResetGIData();
    // m_pTexture 是在 ResetGIData 中釋放的
}

bool ImageResource::LoadGIInPack(char* fileData, int packerType, unsigned char a4) {
    if (!fileData) {
        char buffer[128];
        sprintf_s(buffer, sizeof(buffer), "Image file is not found in pack");
        //CMessageBoxManager::AddOK(g_pMsgBoxMgr, buffer, 0, 0, 0, -1);
        return false;
    }
    
    // 使用一個指標來遍歷記憶體中的檔案資料
    char* current_ptr = fileData;
    bool isCompressed = false;

    // 從記憶體中讀取標頭
    m_version = *reinterpret_cast<int*>(current_ptr);
    current_ptr += sizeof(int);
    if (m_version == 20) {
        isCompressed = true;
    }

    m_width = *reinterpret_cast<unsigned short*>(current_ptr);
    current_ptr += sizeof(unsigned short);
    m_height = *reinterpret_cast<unsigned short*>(current_ptr);
    current_ptr += sizeof(unsigned short);
    m_imageDataSize = *reinterpret_cast<unsigned int*>(current_ptr);
    current_ptr += sizeof(unsigned int);
    m_d3dFormat = *reinterpret_cast<D3DFORMAT*>(current_ptr);
    current_ptr += sizeof(D3DFORMAT);
    m_animationFrameCount = *reinterpret_cast<unsigned short*>(current_ptr);
    current_ptr += sizeof(unsigned short);

    // 複製動畫影格資料
    if (m_animationFrameCount > 0) {
        m_pAnimationFrames = new (std::nothrow) AnimationFrameData[m_animationFrameCount];
        if (!m_pAnimationFrames) return false;
        
        size_t animDataSize = sizeof(AnimationFrameData) * m_animationFrameCount;
        memcpy(m_pAnimationFrames, current_ptr, animDataSize);
        current_ptr += animDataSize;
    }

    m_unknownFlag = *reinterpret_cast<unsigned char*>(current_ptr);
    current_ptr += sizeof(unsigned char);

    // 處理像素資料
    if (isCompressed) {
        m_decompressedSize = *reinterpret_cast<unsigned int*>(current_ptr);
        current_ptr += sizeof(unsigned int);
        
        unsigned char* compressedBuffer = reinterpret_cast<unsigned char*>(current_ptr);
        
        m_pImageData = new (std::nothrow) unsigned char[m_decompressedSize];
        if (!m_pImageData) return false;

        unsigned char pixelDepth = GetPixelDepth(m_d3dFormat);
        run_length_decomp(compressedBuffer, m_imageDataSize, m_pImageData, m_decompressedSize, pixelDepth);
        
        m_imageDataSize = m_decompressedSize; // 更新大小為解壓後的大小

    } else { // 未壓縮
        if (m_imageDataSize > 0 && m_imageDataSize < 0x20000000) {
            m_pImageData = new (std::nothrow) unsigned char[m_imageDataSize];
            if (!m_pImageData) return false;
            
            memcpy(m_pImageData, current_ptr, m_imageDataSize);
        }
    }

    return true;
}

bool ImageResource::LoadGI(const char* fileName, unsigned char a3) {
    // 使用 C++ 的 RAII (Resource Acquisition Is Initialization) 風格來管理檔案指標
    FILE* pFile = nullptr;
    fopen_s(&pFile, fileName, "rb");
    if (!pFile) {
        char buffer[128];
        sprintf_s(buffer, sizeof(buffer), "%s Image file is not found", fileName);
        //CMessageBoxManager::AddOK(g_pMsgBoxMgr, buffer, 0, 0, 0, -1);
        return false;
    }

    bool isCompressed = false;
    
    // 讀取檔案標頭
    fread(&m_version, 4, 1, pFile);
    if (m_version == 20) {
        isCompressed = true;
    }
    fread(&m_width, 2, 1, pFile);
    fread(&m_height, 2, 1, pFile);
    fread(&m_imageDataSize, 4, 1, pFile);
    fread(&m_d3dFormat, 4, 1, pFile);
    fread(&m_animationFrameCount, 2, 1, pFile);

    // 如果有動畫影格，讀取它們的資料
    if (m_animationFrameCount > 0) {
        m_pAnimationFrames = new (std::nothrow) AnimationFrameData[m_animationFrameCount];
        if (!m_pAnimationFrames) {
            fclose(pFile);
            return false;
        }
        fread(m_pAnimationFrames, sizeof(AnimationFrameData) * m_animationFrameCount, 1, pFile);
    }
    
    fread(&m_unknownFlag, 1, 1, pFile);

    // 根據是否壓縮來處理像素資料
    if (isCompressed) {
        // 讀取壓縮前的原始大小
        fread(&m_decompressedSize, 4, 1, pFile);

        // 分配用於儲存壓縮資料的臨時緩衝區
        unsigned char* compressedBuffer = new (std::nothrow) unsigned char[m_imageDataSize];
        if (!compressedBuffer) {
            fclose(pFile);
            return false;
        }
        fread(compressedBuffer, m_imageDataSize, 1, pFile);

        // 分配用於儲存解壓縮後資料的最終緩衝區
        m_pImageData = new (std::nothrow) unsigned char[m_decompressedSize];
        if (!m_pImageData) {
            delete[] compressedBuffer;
            fclose(pFile);
            return false;
        }
        
        // 執行解壓縮
        unsigned char pixelDepth = GetPixelDepth(m_d3dFormat);
        run_length_decomp(compressedBuffer, m_imageDataSize, m_pImageData, m_decompressedSize, pixelDepth);
        
        // 更新資料大小為解壓縮後的大小，並釋放臨時緩衝區
        m_imageDataSize = m_decompressedSize;
        delete[] compressedBuffer;

    } else { // 未壓縮
        if (m_imageDataSize > 0 && m_imageDataSize < 0x20000000) { // 基本的大小檢查
            m_pImageData = new (std::nothrow) unsigned char[m_imageDataSize];
            if (!m_pImageData) {
                fclose(pFile);
                return false;
            }
            fread(m_pImageData, m_imageDataSize, 1, pFile);
        }
    }

    fclose(pFile);
    return true;
}

bool ImageResource::LoadTexture() {
    // 如果沒有圖片資料，或紋理已經存在，則直接返回
    if (!m_pImageData) {
        return false;
    }
    if (m_pTexture) {
        return true;
    }

    assert(Device != nullptr && "Direct3D Device has not been initialized!");
    IDirect3DTexture9* tempTex = nullptr;
    // D3DXCreateTextureFromFileInMemoryEx 是一個強大的函式，可以從記憶體直接建立紋理
    HRESULT hr = D3DXCreateTextureFromFileInMemoryEx(
        Device,                 // D3D 裝置
        m_pImageData,           // 包含圖片資料的記憶體指標
        m_imageDataSize,        // 資料大小
        m_width,                // 寬度
        m_height,               // 高度
        1,                      // MipLevels (1 表示不生成 mipmap)
        0,                      // Usage
        m_d3dFormat,            // 像素格式
        D3DPOOL_MANAGED,        // 記憶體池 (MANAGED 表示由 D3D 自動管理)
        D3DX_FILTER_POINT,      // Filter (1u 對應點採樣)
        D3DX_FILTER_POINT,      // MipFilter (1u 對應點採樣)
        0,                      // ColorKey (0 表示禁用)
        nullptr,                // pSrcInfo
        nullptr,                // pPalette
        &tempTex);           // 接收紋理指標的位址

    if (SUCCEEDED(hr)) {
        // 紋理建立成功後，可以釋放記憶體中的像素資料以節省記憶體
        delete[] m_pImageData;
        m_pImageData = nullptr;
        m_pTexture = tempTex;
        return true;
    } else {
        // PrintDXError(hr, "Failed to create texture", 0);
        // 建立失敗，同樣釋放記憶體
        delete[] m_pImageData;
        m_pImageData = nullptr;
        SafeRelease(tempTex);
        return false;
    }
}

void ImageResource::ResetGIData() {
    // 釋放動畫影格資料
    if (m_pAnimationFrames) {
        delete[] m_pAnimationFrames;
        m_pAnimationFrames = nullptr;
    }
    // 釋放像素資料
    if (m_pImageData) {
        delete[] m_pImageData;
        m_pImageData = nullptr;
    }
    // 釋放 D3D 紋理資源
    SafeRelease(m_pTexture);

    // 重設所有成員變數為初始狀態
    m_version = 10;
    m_width = 0;
    m_height = 0;
    m_imageDataSize = 0;
    m_animationFrameCount = 0;
    m_unknownFlag = 0;
}

unsigned char ImageResource::GetPixelDepth(D3DFORMAT format) {
    unsigned char result; // al

    if (format <= D3DFMT_L16) {
        if (format < D3DFMT_D16) {
            // 處理 D3DFMT_D16 以下的格式
            switch (format) {
                // 4 位元組格式
                case D3DFMT_A8R8G8B8:
                case D3DFMT_X8R8G8B8:
                case D3DFMT_A2B10G10R10:
                case D3DFMT_A8B8G8R8:
                case D3DFMT_X8B8G8R8:
                case D3DFMT_G16R16:
                case D3DFMT_A2R10G10B10:
                case D3DFMT_X8L8V8U8:
                case D3DFMT_Q8W8V8U8:
                case D3DFMT_V16U16:
                case D3DFMT_A2W10V10U10:
                case D3DFMT_D32:
                case D3DFMT_D24S8:
                case D3DFMT_D24X8:
                case D3DFMT_D24X4S4:
                    return 4;
                
                // 2 位元組格式
                case D3DFMT_R5G6B5:
                case D3DFMT_X1R5G5B5:
                case D3DFMT_A1R5G5B5:
                case D3DFMT_A4R4G4B4:
                case D3DFMT_A8R3G3B2:
                case D3DFMT_X4R4G4B4:
                case D3DFMT_A8P8:
                case D3DFMT_A8L8:
                case D3DFMT_V8U8:
                case D3DFMT_L6V5U5:
                case D3DFMT_D16_LOCKABLE:
                case D3DFMT_D15S1:
                    return 2;
                
                // 1 位元組格式
                case D3DFMT_R3G3B2:
                case D3DFMT_A8:
                case D3DFMT_P8:
                case D3DFMT_L8:
                case D3DFMT_A4L4:
                    return 1;
                
                // 8 位元組格式
                case D3DFMT_A16B16G16R16:
                    goto LABEL_8; // 原始碼中的 goto 邏輯，跳轉到 8 位元組返回點
                
                default:
                    return 0; // 未知格式返回 0
            }
        }
        // 如果格式在 D3DFMT_D16 到 D3DFMT_L16 之間
        return 2;
    }

    // 處理 D3DFMT_L16 以上的格式
    if (format > D3DFMT_MULTI2_ARGB8) {
        if (format > D3DFMT_DXT4) {
            if (format > D3DFMT_R8G8_B8G8) {
                // 如果格式不是 D3DFMT_UYVY，返回 0
                if (format != D3DFMT_UYVY)
                    return 0;
                return 4; // D3DFMT_UYVY 返回 4
            }
            if (format == D3DFMT_R8G8_B8G8)
                return 4; // D3DFMT_R8G8_B8G8 返回 4

            // 如果格式不是 D3DFMT_DXT5，返回 0
            if (format != D3DFMT_DXT5) {
                if (format != D3DFMT_G8R8_G8B8)
                    return 0;
                return 4; // D3DFMT_G8R8_G8B8 返回 4
            }
        } else if (format != D3DFMT_DXT4) {
            if (format > D3DFMT_YUY2) {
                if (format != D3DFMT_DXT3)
                    return 0; // 不是 DXT3 返回 0
            } else {
                if (format == D3DFMT_YUY2)
                    return 4; // D3DFMT_YUY2 返回 4
                // 檢查 DXT1 和 DXT2
                if (format != D3DFMT_DXT1 && format != D3DFMT_DXT2)
                    return 0;
            }
        }
        // DXT 1-5 和 DXT4 的情況返回 1
        return 1; 
    }

    // D3DFMT_L16 < format <= D3DFMT_MULTI2_ARGB8 範圍內的格式
    if (format == D3DFMT_MULTI2_ARGB8)
        return 4;

    switch (format) {
        // 4 位元組格式
        case D3DFMT_D32F_LOCKABLE:
        case D3DFMT_D24FS8:
        case D3DFMT_INDEX32:
        case D3DFMT_G16R16F:
        case D3DFMT_R32F:
            return 4;
            
        // 2 位元組格式
        case D3DFMT_INDEX16:
        case D3DFMT_R16F:
            return 2;
        
        // 8 位元組格式
        case D3DFMT_Q16W16V16U16:
        case D3DFMT_A16B16G16R16F:
        case D3DFMT_G32R32F:
        LABEL_8: // 原始碼中的 goto 目標
            result = 8;
            break;
            
        // 16 位元組格式
        case D3DFMT_A32B32G32R32F:
            result = 16;
            break;
            
        default:
            return 0; // 未知格式返回 0
    }
    return result;
}