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

bool ImageResource::LoadGIInPack(const char* filePathInPack, int /*packerType*/, unsigned char /*a4*/)
{
    CMofPacking* pPacker = CMofPacking::GetInstance();
    if (!pPacker) return false;

    char* fileData = pPacker->FileRead(filePathInPack);
    if (!fileData) return false;

    unsigned char* cur = reinterpret_cast<unsigned char*>(fileData);

    // 讀 Header
    m_version = *reinterpret_cast<int*>(cur);               cur += 4;
    const bool isV20 = (m_version == 20);
    const bool isCompressed = isV20; // 目前資源：v20 皆為壓縮

    m_width = *reinterpret_cast<unsigned short*>(cur);    cur += 2;
    m_height = *reinterpret_cast<unsigned short*>(cur);    cur += 2;
    m_imageDataSize = *reinterpret_cast<unsigned int*>(cur);      cur += 4; // v20: 解壓後大小
    m_d3dFormat = *reinterpret_cast<D3DFORMAT*>(cur);         cur += sizeof(D3DFORMAT);
    m_animationFrameCount = *reinterpret_cast<unsigned short*>(cur); cur += 2;

    // 影格表
    if (m_animationFrameCount > 0) {
        size_t bytes = sizeof(AnimationFrameData) * m_animationFrameCount;
        m_pAnimationFrames = new(std::nothrow) AnimationFrameData[m_animationFrameCount];
        if (!m_pAnimationFrames) return false;
        std::memcpy(m_pAnimationFrames, cur, bytes);
        cur += bytes;
    }

    if (isCompressed && isV20) {
        // v20: 影格表後立即是壓縮資料，m_imageDataSize = 解壓後大小
        unsigned char* compressed = cur;                 // 起點即為壓縮資料
        m_decompressedSize = m_imageDataSize;            // 目標大小

        m_pImageData = new(std::nothrow) unsigned char[m_decompressedSize];
        if (!m_pImageData) return false;

        unsigned char unit = GetPixelDepth(m_d3dFormat); // DXT* 會回 1
        // 注意：a4（壓縮長度）給個上限即可，邏輯以 a6（輸出）為主
        run_length_decomp(compressed, m_decompressedSize /*上限*/,
            m_pImageData, m_decompressedSize, unit);
        m_imageDataSize = m_decompressedSize;
        return true;
    }

    // 非 v20（或未壓縮）— 保留原本行為
    // 讀 unknownFlag
    m_unknownFlag = *reinterpret_cast<unsigned char*>(cur); cur += 1;

    if (isCompressed) {
        // 舊版：檔內跟著存了解壓後大小
        m_decompressedSize = *reinterpret_cast<unsigned int*>(cur); cur += 4;
        unsigned char* compressed = cur;

        m_pImageData = new(std::nothrow) unsigned char[m_decompressedSize];
        if (!m_pImageData) return false;

        unsigned char unit = GetPixelDepth(m_d3dFormat);
        run_length_decomp(compressed, m_imageDataSize /*舊版此欄位為壓縮長度*/,
            m_pImageData, m_decompressedSize, unit);
        m_imageDataSize = m_decompressedSize;
        return true;
    }
    else {
        // 未壓縮：直接拷貝 m_imageDataSize bytes
        if (m_imageDataSize == 0 || m_imageDataSize > 0x20000000) return false;
        m_pImageData = new(std::nothrow) unsigned char[m_imageDataSize];
        if (!m_pImageData) return false;
        std::memcpy(m_pImageData, cur, m_imageDataSize);
        return true;
    }
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
    
    fread(&m_unknownFlag, 1, 1, pFile); //有可能要多讀32Bytes

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

bool IsCompressedFormat(D3DFORMAT format) {
    return format == D3DFMT_DXT1 ||
        format == D3DFMT_DXT2 ||
        format == D3DFMT_DXT3 ||
        format == D3DFMT_DXT4 ||
        format == D3DFMT_DXT5;
}

bool ImageResource::LoadTexture() {
    if (m_pTexture) {
        return true;
    }

    if (!m_pImageData) {
        return false;
    }

    assert(Device != nullptr && "Direct3D Device has not been initialized!");

    // 步驟 1: 建立一個空的 D3D 紋理
    HRESULT hr = Device->CreateTexture(
        m_width, m_height, 1, 0, m_d3dFormat, D3DPOOL_MANAGED, &m_pTexture, NULL);

    if (FAILED(hr)) {
        SafeRelease(m_pTexture);
        delete[] m_pImageData;
        m_pImageData = nullptr;
        return false;
    }

    // 步驟 2: 鎖定紋理以準備寫入資料
    D3DLOCKED_RECT lockedRect;
    hr = m_pTexture->LockRect(0, &lockedRect, NULL, 0);

    if (FAILED(hr)) {
        SafeRelease(m_pTexture);
        delete[] m_pImageData;
        m_pImageData = nullptr;
        return false;
    }

    // 步驟 3: 偵測資料類型並準備複製參數
    bool isDDS = (m_imageDataSize > 4 && *reinterpret_cast<DWORD*>(m_pImageData) == 0x20534444); // "DDS "
    bool isCompressed = IsCompressedFormat(m_d3dFormat);

    unsigned char* pSrcData = m_pImageData;
    size_t dataToCopySize = m_imageDataSize;

    // 如果是 DDS 檔案，來源指標需要向後移動 128 位元組以跳過其標頭
    if (isDDS) {
        const size_t DDS_HEADER_SIZE = 128;
        pSrcData += DDS_HEADER_SIZE;
        dataToCopySize -= DDS_HEADER_SIZE;
    }

    // 步驟 4: 根據是否為壓縮格式，選擇不同的複製策略
    if (isCompressed) {
        // 對於 DXT1-5 等塊壓縮格式，資料是連續的塊，可以直接進行一次性複製。
        // D3D 會處理好 Pitch/Stride。
        memcpy(lockedRect.pBits, pSrcData, dataToCopySize);
    }
    else {
        // 對於 A8R8G8B8 等非壓縮格式，必須逐行複製以應對 Pitch。
        // Pitch 是 D3D 在記憶體中為一行像素分配的實際寬度，可能大於圖像的視覺寬度。
        unsigned int bytesPerRow = m_width * GetPixelDepth(m_d3dFormat);
        unsigned char* pDestRow = static_cast<unsigned char*>(lockedRect.pBits);
        unsigned char* pSrcRow = pSrcData;

        for (unsigned int y = 0; y < m_height; ++y) {
            memcpy(pDestRow, pSrcRow, bytesPerRow); // 複製一行的資料
            pDestRow += lockedRect.Pitch;          // 目標指標移動到下一行的起始位置
            pSrcRow += bytesPerRow;                // 來源指標也移動一行的長度
        }
    }

    // 步驟 5: 解鎖紋理，完成所有操作
    m_pTexture->UnlockRect(0);

    // 釋放記憶體中的原始資料
    delete[] m_pImageData;
    m_pImageData = nullptr;

    return true;
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