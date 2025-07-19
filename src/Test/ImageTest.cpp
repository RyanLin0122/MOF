#include "Test/ImageTest.h"
#include "Image/GIVertex.h"
#include "Image/CDeviceManager.h"
#include "Image/ImageResource.h"
#include "Image/LoadingThread.h"
#include "Image/ResourceMgr.h"
#include "CMOFPacking.h" // For integration test
#include <fstream>
#include <windows.h> // For Sleep

extern LPDIRECT3DDEVICE9 Device;

// ���� ImageResource.cpp ���� D3DXCreateTextureFromFileInMemoryEx
// �ޥ��G�ڭ̦b�o�̩w�q���A�s�����|�u���ϥγo�Ӫ����Ӥ��O D3DX �禡�w���������C
// �o�ݭn�b�sĶ�ɯS�O�]�w�A�Ϊ̽T�O�o�� obj ���u���Q�s���C
// �@�ӧ�w�����@�k�O�ϥ� function hooking �禡�w(�p MinHook)�έק��l�X�Ӫ`�J�̿�C
// ���D²��A�ڭ̥����]�i�H�o���л\�C
HRESULT D3DXCreateTextureFromFileInMemoryEx(
    LPDIRECT3DDEVICE9 pDevice,
    LPCVOID pSrcData,
    UINT SrcDataSize,
    UINT Width,
    UINT Height,
    UINT MipLevels,
    DWORD Usage,
    D3DFORMAT Format,
    D3DPOOL Pool,
    DWORD Filter,
    DWORD MipFilter,
    D3DCOLOR ColorKey,
    D3DXIMAGE_INFO* pSrcInfo,
    PALETTEENTRY* pPalette,
    LPDIRECT3DTEXTURE9* ppTexture)
{
    // �N�I�s��o���ڭ̪� MockDevice
    MockD3DDevice* mockDevice = static_cast<MockD3DDevice*>(pDevice);
    return mockDevice->Mock_D3DXCreateTextureFromFileInMemoryEx(Width, Height, Format, ppTexture);
}

// ���� LoadingThread.cpp ���� ResourceMgr::GetInstance()->LoadingImageResource
// �o�O����Ƥ⪺�����A�]����l�X�����g���F�C�z�Q���p�U�A��l�X���`�J ResourceMgr ���̿�C
// �b�o�̡A�ڭ̱N�b��X���դ����Ѥ@�ӯS�� Mock ResourceMgr �����Ҧ欰�C
// ���椸���աA�ڭ̫h���]�i�H������欰�C

ImageSystemTester::ImageSystemTester() {
    m_mockDevice = new MockD3DDevice();
    // �ƥ���l�� Device ���� (���M�b main ���O nullptr)
    m_originalDevicePtr = Device;
    // �N���� Device ���Ы��V�ڭ̪� Mock ����
    Device = m_mockDevice;
}

ImageSystemTester::~ImageSystemTester() {
    // �٭���� Device ����
    Device = m_originalDevicePtr;
    delete m_mockDevice;
    m_mockDevice = nullptr;
}

void ImageSystemTester::RunImageTests() {
    std::cout << "===== Running All Image System Tests =====" << std::endl;
    RunUnitTests();
    RunIntegrationTests();
    std::cout << "===== All Tests Completed =====" << std::endl;
}

void ImageSystemTester::Setup() {
    m_mockDevice->ResetCounters();
    // �M�z�i��s�b����Ҫ��󪬺A
    if (CDeviceManager::GetInstance()) {
        // (�L�k�����R���A�]���غc�禡�O�p�����C���ծج[�q�`�� friend class ����)
        // �o�̧ڭ̰��]�i�H���]�䤺�����A
    }
    MockResourceMgr::GetInstance()->Reset();
}

void ImageSystemTester::Teardown() {
    // �i�H�b���B�R�����դ��إߪ��ɮ�
    remove("test_uncompressed.gi");
    remove("test_compressed.gi");
    remove("test_anim.gi");
}


// ----------------------------------------
// --- �椸���� (UNIT TESTS) ---
// ----------------------------------------
void ImageSystemTester::RunUnitTests() {
    std::cout << "\n--- Running Unit Tests ---" << std::endl;

    Setup(); RUN_TEST(Test_GIVertex_Constructor); Teardown();
    Setup(); RUN_TEST(Test_CDeviceManager_Singleton); Teardown();
    Setup(); RUN_TEST(Test_CDeviceManager_StateCaching); Teardown();
    Setup(); RUN_TEST(Test_CDeviceManager_ResetRenderState); Teardown();
    Setup(); RUN_TEST(Test_ImageResource_LoadGI_Success); Teardown();
    Setup(); RUN_TEST(Test_ImageResource_LoadGI_FileNotFound); Teardown();
    //Setup(); RUN_TEST(Test_ImageResource_LoadTexture_Success); Teardown();
    //Setup(); RUN_TEST(Test_ImageResource_LoadTexture_Failure); Teardown();
    Setup(); RUN_TEST(Test_LoadingThread_QueueOperations); Teardown();
    Setup(); RUN_TEST(Test_LoadingThread_ThreadProcessing); Teardown();

    std::cout << "--- Unit Tests Finished ---" << std::endl;
}

void ImageSystemTester::Test_GIVertex_Constructor() {
    GIVertex v;
    assert(v.position_x == 0.0f);
    assert(v.position_y == 0.0f);
    assert(v.position_z_or_rhw == 0.5f);
    assert(v.rhw_value == 1.0f);
    assert(v.diffuse_color == 0xFFFFFFFF);
    assert(v.texture_u == 0.0f);
    assert(v.texture_v == 0.0f);
}

void ImageSystemTester::Test_CDeviceManager_Singleton() {
    CDeviceManager* p1 = CDeviceManager::GetInstance();
    CDeviceManager* p2 = CDeviceManager::GetInstance();
    assert(p1 != nullptr);
    assert(p1 == p2);
}

void ImageSystemTester::Test_CDeviceManager_StateCaching() {
    CDeviceManager* dm = CDeviceManager::GetInstance();

    // ���� FVF
    dm->SetFVF(D3DFVF_XYZ);
    assert(m_mockDevice->SetFVF_count == 1);
    dm->SetFVF(D3DFVF_XYZ); // ���ƩI�s
    assert(m_mockDevice->SetFVF_count == 1); // �p�ƾ������W�[
    dm->SetFVF(D3DFVF_NORMAL);
    assert(m_mockDevice->SetFVF_count == 2); // �p�ƾ����W�[

    // ���� Texture
    IDirect3DBaseTexture9* tex1 = reinterpret_cast<IDirect3DBaseTexture9*>(new char[1]);
    IDirect3DBaseTexture9* tex2 = reinterpret_cast<IDirect3DBaseTexture9*>(new char[1]);
    dm->SetTexture(0, tex1);
    assert(m_mockDevice->SetTexture_count == 1);
    dm->SetTexture(0, tex1);
    assert(m_mockDevice->SetTexture_count == 1);
    dm->SetTexture(0, tex2);
    assert(m_mockDevice->SetTexture_count == 2);
    delete[] tex1;
    delete[] tex2;

    // ���� Render State
    dm->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    assert(m_mockDevice->SetRenderState_count == 1);
    dm->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    assert(m_mockDevice->SetRenderState_count == 1);
    dm->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    assert(m_mockDevice->SetRenderState_count == 2);
    assert(m_mockDevice->last_render_state_value == FALSE);

    // ���յL�֨��� SetTextureStageState
    dm->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    dm->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    assert(m_mockDevice->SetTextureStageState_count == 2);
}

void ImageSystemTester::Test_CDeviceManager_ResetRenderState() {
    CDeviceManager* dm = CDeviceManager::GetInstance();
    m_mockDevice->ResetCounters();

    dm->ResetRenderState();
    // ���I�s 3 �� SetRenderState
    assert(m_mockDevice->SetRenderState_count == 3);
}

void ImageSystemTester::Test_ImageResource_LoadGI_Success() {
    CreateDummyGIFile("test_uncompressed.gi", false, true);
    ImageResource res;
    bool success = res.LoadGI("test_uncompressed.gi", 0);

    assert(success == true);
    assert(res.m_version == 10);
    assert(res.m_width == 64);
    assert(res.m_height == 32);
    assert(res.m_d3dFormat == D3DFMT_A8R8G8B8);
    assert(res.m_animationFrameCount == 2);
    assert(res.m_pAnimationFrames != nullptr);
    assert(res.m_pImageData != nullptr);
    assert(res.m_imageDataSize == 64 * 32 * 4);

    // �ˬd�ʵe�v���ƬO�_���TŪ��
    assert(res.m_pAnimationFrames[1].width == 123);
}

void ImageSystemTester::Test_ImageResource_LoadGI_FileNotFound() {
    ImageResource res;
    bool success = res.LoadGI("non_existent_file.gi", 0);
    assert(success == false);
}

void ImageSystemTester::Test_ImageResource_LoadTexture_Success() {
    CreateDummyGIFile("test_uncompressed.gi", false, false);
    ImageResource res;
    res.LoadGI("test_uncompressed.gi", 0);

    m_mockDevice->force_texture_creation_failure = false;
    bool success = res.LoadTexture();

    assert(success == true);
    assert(m_mockDevice->CreateTexture_count == 1);
    assert(res.m_pTexture != nullptr);
    assert(res.m_pImageData == nullptr); // �O�������Q����

    delete[] res.m_pTexture; // �M�z fake texture
}

void ImageSystemTester::Test_ImageResource_LoadTexture_Failure() {
    CreateDummyGIFile("test_uncompressed.gi", false, false);
    ImageResource res;
    res.LoadGI("test_uncompressed.gi", 0);

    m_mockDevice->force_texture_creation_failure = true; // �j���
    bool success = res.LoadTexture();

    assert(success == false);
    assert(m_mockDevice->CreateTexture_count == 1);
    assert(res.m_pTexture == nullptr);
    assert(res.m_pImageData == nullptr); // �O����L�צ��\���ѳ�������
}

void ImageSystemTester::Test_LoadingThread_QueueOperations() {
    LoadingThread lt;
    tResInfo res1 = { 101, 1, 0 };
    tResInfo res2 = { 102, 1, 0 };

    assert(lt.FindInResLoadingList(101) == false);

    lt.AddBackGroundLoadingRes(res1);
    lt.AddBackGroundLoadingRes(res2);
    assert(lt.FindInResLoadingList(101) == true);
    assert(lt.FindInResLoadingList(102) == true);

    lt.DelResInLoadingList(101);
    assert(lt.FindInResLoadingList(101) == false);
    assert(lt.FindInResLoadingList(102) == true);

    lt.ClearLodingList();
    assert(lt.FindInResLoadingList(102) == false);
}

void ImageSystemTester::Test_LoadingThread_ThreadProcessing() {
    LoadingThread lt;

    lt.AddBackGroundLoadingRes({184550388, 2, 1});
    lt.AddBackGroundLoadingRes({251658410, 2, 2});

    lt.Poll(); // �Ұʰ����

    // --- �i�a������ ---
    // �ˬd���������N�X�O�_����
    if (lt.m_hThread) {
        // ���ݡA����� m_hThread �ҥN������������C
        // INFINITE ��ܵL�������ݡA���쥦��������C
        WaitForSingleObject(lt.m_hThread, INFINITE);

        // �J�M����������F�A�̦n�N�䱱��N�X����
        CloseHandle(lt.m_hThread);
        lt.m_hThread = NULL; // �קK��������
    }

    // �{�b�ڭ̥i�H 100% �T�w������w�g���槹��
    assert(lt.m_bIsRunning == false);
}


// ----------------------------------------
// --- ��X���� (INTEGRATION TESTS) ---
// ----------------------------------------
void ImageSystemTester::RunIntegrationTests() {
    std::cout << "\n--- Running Integration Tests ---" << std::endl;

    Setup(); RUN_TEST(Test_ImageResource_LoadGIInPack_Success); Teardown();
    Setup(); RUN_TEST(Test_Integration_AsyncLoadAndVerify); Teardown();

    std::cout << "--- Integration Tests Finished ---" << std::endl;
}


void ImageSystemTester::Test_ImageResource_LoadGIInPack_Success() {
    // �o�Ӵ��ը̿�� main.cpp ���� create_vfs_archive() �w�g�Q�I�s
    // �B "D:\\VFS_Source\\" �ؿ��U���@�ӦW�� "test_pack.gi" ���ɮ�

    ImageResource res;
    // LoadGIInPack �� packerType �ѼƦb�����]�� 0 (CMofPacking)
    bool success = res.LoadGIInPack("D:/VFS_Source/1f000386_sky-middle-01.gi", 0, 0);

    assert(success == true);
    assert(res.m_width == 64);
    assert(res.m_height == 32);
    assert(res.m_pImageData != nullptr);
}

void ImageSystemTester::Test_ImageResource_LoadGI_RealFileRelativePath() {
    std::cout << "\n  [PRE-REQUISITE] This test requires the file '1f000386_sky-middle-01.gi' to exist in the execution directory." << std::endl;

    // Arrange
    const char* filename = "1f000386_sky-middle-01.gi";
    ImageResource res;

    // Act
    bool success = res.LoadGI(filename, 0);

    // Assert
    // �p�G�ɮפ��s�b�A���շ|���ѡA�o�O�w���欰�C
    assert(success == true && "Failed to load '1f000386_sky-middle-01.gi'. Make sure it exists in the execution path.");
    assert(res.m_width > 0);
    assert(res.m_height > 0);
    assert(res.m_imageDataSize > 0);
    assert(res.m_pImageData != nullptr || res.m_pTexture != nullptr); // ���J��ܤ֭n��������Ʃί��z
}

/// @brief ���ըϥε�����|Ū���@�ӯu��s�b�� GI �ɮ�
void ImageSystemTester::Test_ImageResource_LoadGI_RealFileAbsolutePath() {
    std::cout << "\n  [PRE-REQUISITE] This test requires the file '200000a1_static_m0002_1.gi' to exist at 'D:\\VFS_Source\\'." << std::endl;

    // Arrange
    const char* filename = "D:\\VFS_Source\\200000a1_static_m0002_1.gi";
    ImageResource res;

    // Act
    bool success = res.LoadGI(filename, 0);

    // Assert
    assert(success == true && "Failed to load 'D:\\VFS_Source\\200000a1_static_m0002_1.gi'. Make sure the file exists at this absolute path.");
    assert(res.m_width > 0);
    assert(res.m_height > 0);
    assert(res.m_imageDataSize > 0);
    assert(res.m_pImageData != nullptr || res.m_pTexture != nullptr); // ���J��ܤ֭n��������Ʃί��z
}

// --- ��X���եΪ��S�� ResourceMgr ---
// �o�Ӫ����|�u���h���J ImageResource
class RealLoadingResourceMgr {
public:
    std::map<unsigned int, std::unique_ptr<ImageResource>> loaded_resources;
    std::mutex mtx;

    static RealLoadingResourceMgr* GetInstance() {
        static RealLoadingResourceMgr instance;
        return &instance;
    }

    void LoadingImageResource(unsigned int groupID, unsigned int resourceID, int a3, char priority) {
        // �o�Ө禡�N�Q�I��������I�s
        auto res = std::make_unique<ImageResource>();

        // ���] resourceID ���������ɦW
        std::string filename = "res_" + std::to_string(resourceID) + ".gi";

        if (res->LoadGI(filename.c_str(), 0)) {
            if (res->LoadTexture()) {
                std::lock_guard<std::mutex> lock(mtx);
                loaded_resources[resourceID] = std::move(res);
            }
        }
    }
    void Reset() { loaded_resources.clear(); }
};

// ���� LoadingThread.cpp ���� ResourceMgr::GetInstance()->LoadingImageResource
// �o�O�����������C�b�@�ӯu�ꪺ�M�פ��A�A�|�ϥΨ̿�`�J�C
// �o�̧ڭ̰��] LoadingThread.cpp �Q�ק令�o�ˡG
/*
// In LoadingThread.cpp
#ifdef ENABLE_TESTING
#include "ImageTest.h" // For RealLoadingResourceMgr
#else
#include "Image/ResourceMgr.h"
#endif

// ... in OnBackgroundLoadUpdate ...
#ifdef ENABLE_TESTING
    RealLoadingResourceMgr::GetInstance()->LoadingImageResource(...)
#else
    ResourceMgr::GetInstance()->LoadingImageResource(...)
#endif
*/
// �ѩ�ڭ̤���ק��l�X�A�o�Ӿ�X���ժ���@�|���ǧx���A���ڭ̮i�ܨ��޿�C

void ImageSystemTester::Test_Integration_AsyncLoadAndVerify() {
    std::cout << "  (Note: This test assumes LoadingThread can be re-wired to use a test-specific resource manager.)" << std::endl;

    // 1. �ǳ�����
    RealLoadingResourceMgr::GetInstance()->Reset();
    CreateDummyGIFile("res_301.gi", false, false);
    CreateDummyGIFile("res_302.gi", true, false);

    // 2. �]�w LoadingThread
    LoadingThread lt;
    lt.AddBackGroundLoadingRes({ 301, 3, 0 });
    lt.AddBackGroundLoadingRes({ 302, 3, 0 });

    // 3. ����
    lt.Poll();
    Sleep(200); // ���ݰ��������

    // 4. ����
    assert(lt.m_bIsRunning == false);
    auto& loaded = RealLoadingResourceMgr::GetInstance()->loaded_resources;
    assert(loaded.size() == 2);
    assert(loaded.count(301) == 1);
    assert(loaded.count(302) == 1);

    // ���ҲĤ@�Ӹ귽
    ImageResource* res1 = loaded[301].get();
    assert(res1 != nullptr);
    assert(res1->m_pTexture != nullptr);
    assert(res1->m_width == 64);

    // ���ҲĤG�Ӹ귽 (�q���Y�ɸ��J)
    ImageResource* res2 = loaded[302].get();
    assert(res2 != nullptr);
    assert(res2->m_pTexture != nullptr);
    assert(res2->m_version == 20); // �������Y����
    assert(res2->m_height == 32);

    // �M�z�����z
    delete[] res1->m_pTexture;
    delete[] res2->m_pTexture;
}

// ----------------------------------------
// --- ���ջ��U�禡��@ ---
// ----------------------------------------
void ImageSystemTester::CreateDummyGIFile(const std::string& filename, bool compressed, bool with_anim) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) return;

    // Header
    int version = compressed ? 20 : 10;
    unsigned short width = 64, height = 32;
    unsigned int imageDataSize = width * height * 4; // 32-bit RGBA
    D3DFORMAT format = D3DFMT_A8R8G8B8;
    unsigned short anim_count = with_anim ? 2 : 0;

    file.write(reinterpret_cast<const char*>(&version), sizeof(version));
    file.write(reinterpret_cast<const char*>(&width), sizeof(width));
    file.write(reinterpret_cast<const char*>(&height), sizeof(height));
    file.write(reinterpret_cast<const char*>(&imageDataSize), sizeof(imageDataSize));
    file.write(reinterpret_cast<const char*>(&format), sizeof(format));
    file.write(reinterpret_cast<const char*>(&anim_count), sizeof(anim_count));

    if (with_anim) {
        AnimationFrameData anim_frame[2] = {};
        anim_frame[0].width = 64;
        anim_frame[1].width = 123; // �S�x��
        file.write(reinterpret_cast<const char*>(anim_frame), sizeof(AnimationFrameData) * 2);
    }

    unsigned char unknown_flag = 0xAB;
    file.write(reinterpret_cast<const char*>(&unknown_flag), sizeof(unknown_flag));

    if (compressed) {
        // RLE ���Y (���B�Ȭ��ܷN�A����@�u�����Y�t��k)
        unsigned int decompressed_size = imageDataSize;
        unsigned int compressed_size = 10; // �������Y���ܤp�F

        // --- �ץ��I�b�o�� ---
        // ���N sizeof �����G (unsigned size_t) �૬�� signed �� std::streamoff
        // �M��A���t���A�o�ˤ~��o��@�ӭt�Ʀ첾�C
        std::streamoff offset = -static_cast<std::streamoff>(sizeof(imageDataSize));

        // �q�ɮ׬y���ثe��m���e���� offset ���Z��
        file.seekp(offset, std::ios_base::cur); // �^�Y�ק� imageDataSize

        file.write(reinterpret_cast<const char*>(&compressed_size), sizeof(compressed_size));
        file.seekp(0, std::ios_base::end); // ���^�ɮ׵����~��g�J

        file.write(reinterpret_cast<const char*>(&decompressed_size), sizeof(decompressed_size));
        std::vector<char> dummy_compressed_data(compressed_size, 'c');
        file.write(dummy_compressed_data.data(), compressed_size);
    }
    else {
        std::vector<char> dummy_pixel_data(imageDataSize, 'p');
        file.write(dummy_pixel_data.data(), imageDataSize);
    }
    file.close();
}