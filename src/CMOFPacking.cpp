#include "CMOFPacking.h"
#include <string> // �i�Ω��w�����r��ާ@�A�����B�ɶq����C����H�ŦX��l�X

// �qCMOFPacking.c���[��쪺����r�� (��l�X���ϥΪ��O�r��r���q)
const char STR_DOT[] = ".";
const char STR_DOTDOT[] = "..";
// const char STR_MOF_INI[] = "mof.ini"; // �bDataPacking�������ϥ� "mof.ini"

// �غc�禡
CMofPacking::CMofPacking() {
    // *(_DWORD *)this = &CMofPacking::`vftable'; // vfptr�ѽsĶ���۰ʳ]�w
    m_pNfsHandle = nullptr;                     // *((_DWORD *)this + 1) = 0;
    m_pReadBuffer = nullptr;                    // *((_DWORD *)this + 2) = 0;
    m_pBuffer1 = nullptr;                       // *((_DWORD *)this + 3) = 0;
    m_nReadBytes = 0;                           // *((_DWORD *)this + 68) = 0;

    // ��l�� m_globResults ������
    // *((_DWORD *)this + 72) = 0; // -> m_globResults.internal_callback_error_flag
    m_globResults.gl_pathc = 0;
    m_globResults.gl_pathv = nullptr;
    m_globResults.gl_offs = 0;
    m_globResults.internal_callback_error_flag = 0;

    m_isLoadingFlag = false;                    // byte_7A1324[(_DWORD)this] = 0;

    // memset((char *)this + 292, 0, 0x7A1200u);
    memset(m_backgroundLoadBufferField, 0, sizeof(m_backgroundLoadBufferField));

    // memset((char *)this + 16, 0, 0x100u);
    memset(m_tempPathBuffer, 0, sizeof(m_tempPathBuffer));
}

// �Ѻc�禡
CMofPacking::~CMofPacking() {
    // *(_DWORD *)this = &CMofPacking::`vftable'; // vfptr�ѽsĶ���۰ʳ]�w
    DeleteBuffer();  // �M�z m_pReadBuffer
    // DeleteBuffer1(); // �]���ӲM�z m_pBuffer1�A�p�G���Q�ϥΤF
    PackFileClose(); // ����NFS�ɮ�
}

// ��l�� (���G�S���S�O����)
bool CMofPacking::Init() {
    return true; // ��l�X��^1
}

// �}��NFS�ʸ��ɮ�
bool CMofPacking::PackFileOpen(const char* packFileName) {
    m_pNfsHandle = nfs_start(packFileName, 3); // �Ҧ�3�b��l�X���Q�ϥ�
    return (m_pNfsHandle != nullptr);
}

// ����NFS�ʸ��ɮ�
bool CMofPacking::PackFileClose() {
    if (m_pNfsHandle) {
        nfs_end(m_pNfsHandle, 0); // �Ѽ�0��ܤ��P�������ɮ�
        m_pNfsHandle = nullptr;
    }
    return true; // ��l�X��^1
}

// �s�W�ɮצܫʸ�
bool CMofPacking::AddFile(const char* filePathInPack) {
    if (!m_pNfsHandle) return false;

    if (nfs_file_exists(m_pNfsHandle, filePathInPack)) {
        RemoveFile(filePathInPack); // �p�G�ɮפw�s�b�A������
    }

    int fd = nfs_file_create(m_pNfsHandle, filePathInPack);
    if (fd < 0) {
        return false; // �إ��ɮץ���
    }

    // ��l�X���g�J�F2048�줸�ժ�����l�ư��|�ƾ� v5�C
    // �o�q�`�O��bug�Ϊ̦��S��ت��]�Ҧp�w���t�Ŷ��A���g�J����l�ƼƾګܦM�I�^�C
    // ���F�欰�@�P�A�ڭ̼����g�J�@�ǼƾڡA���ϥ�0��R���ƾڥH�קK�w�����I�C
    // �p�G�Y��n�D�g�J"�U��"�ƾڡA�h�ݭnchar uninitialized_buffer[2048];
    char zero_buffer[2048];
    memset(zero_buffer, 0, sizeof(zero_buffer)); // �ϥ�0��R���w�İ�

    if (nfs_file_write(m_pNfsHandle, fd, zero_buffer, sizeof(zero_buffer)) != sizeof(zero_buffer)) {
        nfs_file_close(m_pNfsHandle, fd);
        return false; // �g�J����
    }

    nfs_file_close(m_pNfsHandle, fd);
    return true;
}

// �q�ʸˤ������ɮ�
bool CMofPacking::RemoveFile(const char* filePathInPack) {
    if (!m_pNfsHandle) return false;
    nfs_file_unlink(m_pNfsHandle, filePathInPack);
    return true; // ��l�X��^1�A�Y��unlink���Ѥ]�p�� (nfs_file_unlink��������^��)
}

// �N���w�ؿ��U���Ҧ��ɮץ��]�iNFS
int CMofPacking::DataPacking(const char* directoryPath) {
    if (!m_pNfsHandle || !directoryPath) return 0;

    WIN32_FIND_DATAA findFileData;
    char searchPath[MAX_PATH];
    char fullFilePath[MAX_PATH];
    char subDirectoryPath[MAX_PATH];

    // �c�طj�M���|�A�Ҧp "C:\\Data\\*.*"
    sprintf_s(searchPath, sizeof(searchPath), "%s*.*", directoryPath);

    HANDLE hFindFile = FindFirstFileA(searchPath, &findFileData);
    if (hFindFile == INVALID_HANDLE_VALUE) {
        return 0; // FindFirstFileA ����
    }

    do {
        if (lstrcmpA(findFileData.cFileName, STR_DOT) == 0 || lstrcmpA(findFileData.cFileName, STR_DOTDOT) == 0) {
            continue; // ���L "." �M ".."
        }

        // ��l�X������ "mof.ini" ���S��B�z�A���r�ꤣ�ǰt ('&`string`')�A�i��O�ϽsĶ���~
        // `if ( !lstrcmpA(FindFileData.cFileName, &`string`) ) return 1;` -> `&`string`` ���ӹ�
        // ���]�O�ˬd�S�w�ɮצW�A�Ҧp "mof.ini" (���`���Ҧ�)
        if (lstrcmpA(findFileData.cFileName, "mof.ini") == 0) { // ���]�ˬd "mof.ini"
            FindClose(hFindFile);
            return 1; // ��� "mof.ini"�A����B�z
        }

        sprintf_s(fullFilePath, sizeof(fullFilePath), "%s%s", directoryPath, findFileData.cFileName);

        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            // �p�G�O�l�ؿ��A���k�B�z
            sprintf_s(subDirectoryPath, sizeof(subDirectoryPath), "%s\\", fullFilePath);
            int packing_result = DataPacking(subDirectoryPath);
            if (packing_result == 1) { // �p�G�b�l�ؿ������ "mof.ini"
                FindClose(hFindFile);
                return 1;
            }
            // �p�G packing_result == 0 (�l�ؿ��B�z����)�A��l�X���G�|�~��B�z��e�ؿ�����L�ɮ�
            // �Ӥ��O������^0�C��l�X�޿�G`v4 = CMofPacking::DataPacking(this, v13), v4 != 1) && v4`
            // �p�G v4 (packing_result) �O0�A`(v4 != 1 && v4)` �����A�N�|����᭱���ɮ׳B�z�޿�A�o����C
            // ���ӬO: if (packing_result == 0 && packing_result != 1) { /* handle error or continue */ }
            // �Ϊ�: if (packing_result != 1 && packing_result != 2) { /* error in subdir */ }
            // ��l�X�� `v4 != 1 && v4` �N��O `packing_result != 1 && packing_result != 0` -> `packing_result == 2`
            // �ҥH�u���l�ؿ����\��^2�ɡA�~���L�����ɮ׳B�z�C
            // ²�ơG�p�G���k�I�s���O1�]mof.ini�^�A�]���O0�]���~�^�A����N�~��C
            // �p�G���k�I�s�O0�A�����l�X�� `v4` (packing_result) ��0�A`v4 != 1 && v4` �����A
            // �|�ɭP����᭱���ɮ׫����޿�A�o�O���諸�A�]���l�ؿ��O�ӥؿ��C
            // ���T���޿������G�p�G (FindFileData.dwFileAttributes != 16) then process file
            // else (it's a directory) recurse; if recurse_result == 1 return 1; (if recurse_result == 0, maybe error)
            // �ϽsĶ�X `if ( FindFileData.dwFileAttributes != 16 || (..., v4 = DataPacking(...), v4 != 1) && v4 )`
            // ������ `if (IS_FILE || (IS_DIR && DataPacking_Returns_NOT_MOF_INI && DataPacking_Returns_NOT_ERROR_OR_EMPTY))`
            // ��ڤW�A�p�G DataPacking ��^ 0 (���~) �� 1 (���mof.ini)�A�h `(v4 != 1 && v4)` �����C
            // ���� if ������M�� `FindFileData.dwFileAttributes != 16`�C
            // �o��ܦp�G�O�@���ɮסA�Ϊ̬O�@�ӥؿ��B���k���]���ѩΧ��mof.ini�A�h����᭱���ɮ׽ƻs�C�o��M�O�����C
            // ���T�޿������G�Y�O�ɮסA�h���]�C�Y�O�ؿ��A�h���k�C
            // �ڱN�ץ����޿謰��X�z���Φ��C
        }
        else {
            // �p�G�O�ɮסAŪ���üg�JNFS
            FILE* pFile = nullptr;
            errno_t err = fopen_s(&pFile, fullFilePath, "rb");
            if (err == 0 && pFile) {
                fseek(pFile, 0, SEEK_END);
                long fileSize = ftell(pFile);
                fseek(pFile, 0, SEEK_SET);

                if (fileSize > 0) {
                    char* buffer = new (std::nothrow) char[fileSize];
                    if (buffer) {
                        fread(buffer, 1, fileSize, pFile);

                        // �N�ɮצW�ର�p�g�]NFS�����q�`�O�j�p�g���ӷP�βΤ@�p�g�^
                        char lowerCaseFileName[MAX_PATH];
                        strcpy_s(lowerCaseFileName, sizeof(lowerCaseFileName), fullFilePath);
                        _strlwr_s(lowerCaseFileName, sizeof(lowerCaseFileName)); // �ϥΦw������

                        int fd = nfs_file_create(m_pNfsHandle, lowerCaseFileName);
                        if (fd >= 0) {
                            nfs_file_write(m_pNfsHandle, fd, buffer, fileSize);
                            nfs_file_close(m_pNfsHandle, fd);
                        }
                        delete[] buffer;
                    }
                }
                fclose(pFile);
            }
        }
    } while (FindNextFileA(hFindFile, &findFileData));

    FindClose(hFindFile);
    return 2; // ���`����
}


// �I�����J��Ū���ɮ�
void CMofPacking::FileReadBackGroundLoading(const char* filePathInPack) {
    if (!m_pNfsHandle) return;

    int fd = nfs_file_open(m_pNfsHandle, filePathInPack, 2); // �Ҧ�2�A�i��O��Ū��Ū�g
    if (fd >= 0) {
        m_isLoadingFlag = true;
        int totalBytesReadThisCall = 0; // �Ω�֭p�����I�����J���`�줸�ռ�
        int bytesReadThisChunk;
        do {
            bytesReadThisChunk = nfs_file_read(m_pNfsHandle, fd, m_backgroundLoadBufferField, sizeof(m_backgroundLoadBufferField));
            if (bytesReadThisChunk > 0) {
                totalBytesReadThisCall += bytesReadThisChunk;
                // �o�̥i�H�B�zŪ���쪺 m_backgroundLoadBufferField �����ƾ�
                // ��l�X�N��� chunk ��Ū���q�s�x�b m_nReadBytes (`*((_DWORD *)this + 68) = v7;`)
                // �æb�@�ӧ����ܼ� v6 ���֥[�C���B�ڭ̶ȧ�s m_nReadBytes ���̫�@�����j�p�C
                m_nReadBytes = bytesReadThisChunk;
            }
            else {
                m_nReadBytes = 0; // �p�GŪ�����ѩΨ��ɮק���
            }
        } while (bytesReadThisChunk >= static_cast<int>(sizeof(m_backgroundLoadBufferField))); // �p�GŪ���F�w�İϡA�i���٦���h

        nfs_file_close(m_pNfsHandle, fd);
        m_isLoadingFlag = false;
        // ��l�X�� v6 �֥[�F�`�q�A���̲� m_nReadBytes �u�O�d�̫�@�����j�p�C
        // �p�G�ݭn�`�q�A���Ӧ��@�ӥt�~�������ܼƨ��x�s totalBytesReadThisCall�C
        // �ھ� GetBufferSize() ���欰�A����^ m_nReadBytes�A�ҥH�ڭ̫O�� m_nReadBytes ���̫�@�����j�p�C
    }
    else {
        m_nReadBytes = 0;
    }
}

// Ū���ɮצܰʺA���t���w�İ�
char* CMofPacking::FileRead(const char* filePathInPack) {
    if (!m_pNfsHandle) return nullptr;

    m_nReadBytes = 0;
    int fd = nfs_file_open(m_pNfsHandle, filePathInPack, 2); // �Ҧ�2
    if (fd >= 0) {
        m_isLoadingFlag = true;

        // �qNfsHandle���OpenFileHandle�A�A���nt_node_idx
        // �o�O��nfs�������c�����]�A�����ϥ�API���
        // int fileSize = nfs_nt_node_get_size(m_pNfsHandle->nt_handle, m_pNfsHandle->open_files_array[fd]->nt_node_idx);
        // ��w�������k�O��lseek�쥽������j�p�A�Ϊ�nfs API���Ѫ�������j�p����k�C
        // nfs.cpp ���� nfs_file_lseek �|��s�Ϊ�^�j�p�A���S�������� nfs_file_size(fd)�C
        // nfs_nt_node_get_size �ݭn nt_handle �M nt_node_idx�C
        // open_files_array[fd]->nt_node_idx �O���T���C
        NfsOpenFileHandle* fh = m_pNfsHandle->open_files_array[fd]; // �ݭn�T�O fd ���ĥB fh �w�Q��R
        int fileSize = -1;
        if (fh) { // �T�O fh ����
            fileSize = nfs_nt_node_get_size(m_pNfsHandle->nt_handle, fh->nt_node_idx);
        }

        if (fileSize >= 0) {
            DeleteBuffer(); // �����ª� m_pReadBuffer
            m_pReadBuffer = new (std::nothrow) char[fileSize + 1]; // +1 for null terminator if needed, though binary read
            if (m_pReadBuffer) {
                m_nReadBytes = nfs_file_read(m_pNfsHandle, fd, m_pReadBuffer, fileSize);
                if (m_nReadBytes != fileSize) { // Ū��������
                    DeleteBuffer(); // �M�z�ê�^nullptr
                }
                else {
                    // ((char*)m_pReadBuffer)[m_nReadBytes] = '\0'; // �p�G�O�奻�A�T�O�ŵ��� (�i��)
                }
            }
            else {
                m_nReadBytes = 0; // ���t����
            }
        }
        else {
            m_nReadBytes = 0; // ����j�p����
        }
        nfs_file_close(m_pNfsHandle, fd);
        m_isLoadingFlag = false;
    }
    return (char*)m_pReadBuffer; // �p�G���ѡAm_pReadBuffer �|�O nullptr
}

// ���� m_pReadBuffer
void CMofPacking::DeleteBuffer() {
    if (m_pReadBuffer) {
        delete[](char*)m_pReadBuffer;
        m_pReadBuffer = nullptr;
    }
}

// ���� m_pBuffer1
void CMofPacking::DeleteBuffer1() {
    if (m_pBuffer1) {
        delete[](char*)m_pBuffer1;
        m_pBuffer1 = nullptr;
    }
}

// ���o�W��Ū���ާ@���줸�ռ�
int CMofPacking::GetBufferSize() const {
    return m_nReadBytes;
}

// �bNFS���j�M�ŦX�Ҧ����ɮצW��
NfsGlobResults* CMofPacking::SearchString(const char* pattern) {
    if (!m_pNfsHandle) return nullptr;

    // nfs_glob �� flags �ѼƦbC�X���O4�C
    // ���] 4 ���� NFS_FNM_NOSORT (���Ƨǵ��G)
    // ���~�^�I�禡�� nullptr (0)
    int result = nfs_glob(m_pNfsHandle, pattern, 4 /*NFS_FNM_NOSORT?*/, nullptr, &m_globResults);

    if (result == 0 && m_globResults.gl_pathc > 0) { // ���\�B���ǰt
        return &m_globResults;
    }
    // �p�G result != 0 (�Ҧp GLOB_NOMATCH) �� gl_pathc == 0�A�h��^ nullptr
    // ��l�X�޿� `*v2 <= 0 ? 0 : (unsigned int)v2`�Av2 ���V gl_pathc ���a�}�A����^���O m_globResults ���a�}�C
    // �ҥH�O `m_globResults.gl_pathc > 0 ? &m_globResults : nullptr;`
    return nullptr;
}

// ���� SearchString �����G
void CMofPacking::DeleteSearchData() {
    if (!m_pNfsHandle) return;
    // �ھ� nfs.h, nfs_glob_free �u�ݭn NfsGlobResults*
    // �ϽsĶ��C�X `nfs_glob_free(m_pNfsHandle, &m_globResults)` �O���~���C
    nfs_glob_free(&m_globResults);
}

// �N�r���ର�p�g
char* CMofPacking::ChangeString(char* str) {
    if (str) {
        return _strlwr(str); // _strlwr �ק��r��ê�^��
        // �Ϊ̨ϥ�C++�зǤ�k�G
        // std::string temp_str = str;
        // std::transform(temp_str.begin(), temp_str.end(), temp_str.begin(), ::tolower);
        // strcpy(str, temp_str.c_str()); // �p�G�ݭn�ק��char*
        // return str;
    }
    return nullptr;
}