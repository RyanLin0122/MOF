#ifndef NFS_H
#define NFS_H

#include <windows.h> // For DWORD, SYSTEM_INFO
#include <stdio.h>   // For FILE
#include <stdlib.h>  // For size_t
#include <stdint.h>  // For uintptr_t

// --- Struct Definitions ---

/**
 * @brief IIO �֨��������c
 * �x�s��ڪ���ƽw�İϥH�ά��������ƾڡC
 */
struct NfsIioCachePage {
	// ��l C �{���X���A������x�s�w�İϫ��СA�ñN��̧C���Ħ� (LSB) �Χ@�P�B�X�СC
	uintptr_t buffer_ptr_and_syncflag_storage;
	int disk_block_position;        // ���֨������������Ϻа϶���m
	int last_access_time;           // �̫�s���������������� (�Ӧ� nfs_iio_CLOCK)
};

/**
 * @brief IIO �q�D���֨��޲z��
 */
struct NfsIioCache {
	NfsIioCachePage** pages;        // ���V NfsIioCachePage ���Ъ��}�C
	int num_pages_allocated;        // pages �}�C���w���t���`�Ѽ�
	int num_pages_active;           // �ثe���D/�w�ϥΪ��֨������ƶq
};

/**
 * @brief IIO �q�D���c
 */
struct NfsIioChannel {
	int blocks_per_stripe;          // ���q�D���C�� "stripe" (���a/�򥻳��) �]�t���϶���
	int current_size_bytes;         // �q�D��ƪ��ثe�޿�j�p (�줸��)
	int current_seek_position;      // �b�q�D�����ثeŪ�g��m
	NfsIioCache* cache_header;      // ���V���q�D���֨��޲z��
};

/**
 * @brief IIO �ɮ׵��c
 */
struct NfsIioFile {
	FILE* file_handle;              // C �з��ɮ׫���
	char* file_name;                // IIO �ɮת��W�� (�ʺA���t)
	short num_channels;             // �� IIO �ɮפ����q�D�ƶq
	NfsIioChannel** channels;       // ���V NfsIioChannel ���Ъ��}�C
};

/**
 * @brief ��Ƨ֨������Y���c�C
 * �x�s����֨��w�İϥ�����������ơC
 */
struct NfsDataCacheHeader {
	size_t buffer_capacity;         // �֨��w�İϪ��ثe�e�q (�줸��)
	void* buffer;                   // ���V��ڧ֨���ƪ�����
	int cache_window_start_offset;  // ���֨������������ɮפ����_�l�줸�հ����q
	int is_synced_flag;             // �X�СG1 ��ܧ֨��P�ϺЦP�B (clean)�A0 ��ܤw�ק� (dirty)
};

/**
 * @brief ����ɮױ���N�X�A�]�t�ɮ׫��ЩM�������֨��C
 * ������l�X���� _nfs_data_Data�C
 */
struct NfsDataHandle {
	FILE* file_ptr;                 // ���V�w�}���ɮת�����
	char* file_name;                // �ɮצW�� (�ʺA���t)
	NfsDataCacheHeader* cache;      // ���V���ɮ׸�Ƨ֨�������
};

// --- NfsFatHandle ���c��w�q ---
struct NfsFatHandle { // ������l�X�� _nfs_fat_FAT
	NfsIioFile* iio_file;           // ���V IIO �ɮ׵��c������
	int fat_iio_channel_id;         // �Ω��x�s FAT ��ƪ� IIO �q�D ID
	int next_free_search_start_idx; // �U�@�ӪŶ� FAT �����_�l�j�M����
};

/**
 * @brief �`�I�� (NT) �����@�Ӹ`�I�A�x�s�ɮ�/�ؿ������ƾڡC
 * �j�p�� 16 �줸�աC
 */
struct NfsNode { // ���� _nfs_nt_Node
	int ref_count;                  // �ޥέp�� (offset +0)
	int file_size_bytes;            // �ɮפj�p (�줸��) (offset +4)
	int fat_chain_start_idx;        // �b FAT �����϶���_�l���� (offset +8)
	int user_flags_or_type;         // �ϥΪ̩w�q���X�Щ������� (offset +12)
};

/**
 * @brief �`�I�� (NT) ������N�X�C
 */
struct NfsNtHandle { // ���� _nfs_nt_NT
	NfsIioFile* iio_file;                // ���V IIO �ɮ׵��c������
	int nt_iio_channel_id;               // �Ω��x�s NT ��ƪ� IIO �q�D ID
	int next_free_node_search_start_idx; // �U�@�ӪŶ� NT �`�I���_�l�j�M����
};

/**
 * @brief Trie �`�I���c (�Ω� Patricia Trie)�C
 * �j�p�� 16 �줸�աC
 */
struct NfsTrieNode {				// ���� _nfs_dt_TrieNode
	short nt_idx;					// ���V�`�I�� (NT) �����ޡA�Ψ�L���ƾڡC
	short b_index;					// �Ω� Patricia Trie ������줸���ޡC
	int k_index;					// �Y MSB �� 1 (�Y < 0): (���V KeyNode ������ | 0x80000000)�C
	// �Y MSB �� 0 (�Y >= 0): ��ܦ� TrieNode ���Ŷ��C0 �O�嫬���Ŷ�/�w�M���ȡC
	int left_child_idx;				// ���l TrieNode �����ޡC
	int right_child_idx;			// �k�l TrieNode �����ޡC
};

/**
 * @brief ��`�I���c�A�Ω��x�s�ɦW (��) �����q�C
 * �j�p�� 64 �줸�աC
 */
struct NfsKeyNode {					// ���� _nfs_dt_KeyNode
	int next_fragment_idx_flags;	// �Y MSB �� 1 (�Y < 0): (���V�U�@�� NfsKeyNode ���q������ | 0x80000000)�A
	// �ζȬ� 0x80000000 ��ܦ����̫���q�C
	// �Y MSB �� 0 (�Y >= 0): ��ܦ� KeyNode ���Ŷ��C0 �O�嫬���Ŷ�/�w�M���ȡC
	char key_fragment[60];			// �x�s��W���@�Ӥ��q�C
};

/**
 * @brief �ؿ��� (DT) ������N�X�C
 */
struct NfsDtHandle { // ���� _nfs_dt_DT
	NfsIioFile* iio_file;
	int trienode_channel_id;		// �x�s NfsTrieNode �}�C�� IIO �q�D ID�C
	int keynode_channel_id;			// �x�s NfsKeyNode �}�C�� IIO �q�D ID�C
	int next_free_trienode_idx;		// �Ω�j�M�U�@�ӪŶ� TrieNode �����ܯ��ޡC
	int next_free_keynode_idx;		// �Ω�j�M�U�@�ӪŶ� KeyNode �����ܯ��ޡC
};

// Glob ���G���c�� (�����зǪ� glob_t)
struct NfsGlobResults {
	size_t gl_pathc;				// �ǰt���|���ƶq
	char** gl_pathv;				// ���V�ǰt���|�r��}�C������
	size_t gl_offs;					// �b gl_pathv �}�Y�O�d���Ѧ�ƶq (�Y FNM_DOOFFS)
	// �����ϥΡA�Ω�l�ܦ^�I�禡���O�_�o�Ϳ��~
	int internal_callback_error_flag;
};

struct NfsOpenFileHandle {
	int dt_trienode_idx;
	int nt_node_idx;
	int fat_chain_start_idx;		// �ɮ׸����bFAT�����_�l�϶�����
	int current_byte_offset;
	int open_mode_flags;
};

struct NfsHandle {
	NfsIioFile* iio_handle;
	NfsDtHandle* dt_handle;
	NfsNtHandle* nt_handle;
	NfsFatHandle* fat_handle;
	NfsDataHandle* data_handle;
	NfsOpenFileHandle** open_files_array;
	int open_files_array_capacity;
	int num_currently_open_files;
	void* unknown_ptr_or_padding1;
	char* base_vfs_name_for_lock;
};

// --- Global Variable Declarations (extern) ---
// �o���ܼƦb nfs.cpp ���w�q�A���B�ŧi�� extern �H�K��L�ɮרϥ�
extern int nfs_iio_BLOCK_SIZEv;
extern int nfs_iio_CLOCK;
extern int nfs_iio_IOMODE;
extern int nfs_data_IOMODE;
extern int nfs_errno;
extern char nfs_glob_key_buffer[4096]; // �p�G�n�b�h���ɮפ��@�ΡA�h���ŧi�� extern

// --- Public Function Prototypes ---

// Layer 0/1 (Basic Utilities)
DWORD get_page_size(void);
int file_exists(const char* fileName);
int bit_get(const char* byteValue, int bitIndex);
int bitfirst_different(const char* str1, const char* str2);
int nblocks(int size);
int blockno(int offset);
void nfs_perror(void* unusedHandle, const char* prefixMessage); // void* for NfsHandle
int lock_remove(const char* baseName);
int lock_check(const char* baseName, int accessMode);
int lock_leave(const char* baseName);


// Layer 2 (IIO - Indexed I/O with Cache)
NfsIioFile* nfs_iio_create(const char* fileName);
NfsIioFile* nfs_iio_open(const char* fileName);
void nfs_iio_close(NfsIioFile* file_to_close);
void nfs_iio_destroy(NfsIioFile* file_to_destroy);
int nfs_iio_read(NfsIioFile* file, int channel_idx, void* buffer, int bytes_to_read);
int nfs_iio_write(NfsIioFile* file, int channel_idx, const void* buffer, int bytes_to_write);
int nfs_iio_seek(NfsIioFile* file, int channel_idx, int seek_position);
int nfs_iio_allocate_channel(NfsIioFile* file, int blocks_per_stripefor_new_channel);
NfsIioChannel* nfs_iio_get_channel(NfsIioFile* file, int channel_idx);
int nfs_iio_channel_size(NfsIioChannel* channel);
int nfs_iio_channel_blocks(NfsIioChannel* channel);
int nfs_iio_channel_truncate(NfsIioFile* file, int channel_idx);
int channel_block_to_absolute_block(NfsIioFile* file, int channel_idx, int channel_relative_block_idx);
int channel_pos_to_absolute_block(NfsIioFile* file, int channel_idx, int channel_relative_byte_pos);
int cache_page_dump(NfsIioFile* file, int channel_idx, int page_array_idx);
void cache_pageflush(NfsIioFile* file, int channel_idx, int page_array_idx);
void cache_page_create(NfsIioFile* file, int channel_idx, int page_array_idx_to_create_at);
int cache_page_refresh(NfsIioFile* file, int channel_idx, int channel_relative_byte_pos);
int cache_create(NfsIioFile* file, int channel_idx);
int cache_destroy(NfsIioCache* cache_to_destroy);
int is_in_cache(NfsIioFile* file, int channel_idx, int channel_relative_block_idx);
void cache_update(NfsIioFile* file, int channel_idx, int channel_relative_byte_pos);
void cacheflush(NfsIioFile* file, int channel_idx);
void flush_data(NfsIioFile* file);
int cache_read_channel_block(NfsIioFile* file, int channel_idx, int channel_relative_block_idx, void* buffer);
unsigned int cache_read_partial_channel_block(NfsIioFile* file, int channel_idx, int channel_relative_block_idx,
	int offset_in_block, int end_offset_in_block, void* buffer);
int cache_write_channel_block(NfsIioFile* file, int channel_idx, int channel_relative_block_idx, const void* buffer);
int cache_write_partial_channel_block(NfsIioFile* file, int channel_idx, int channel_relative_block_idx,
	int offset_in_block, int end_offset_in_block, const void* buffer);
int read_header(NfsIioFile* file);
void auto_truncate(NfsIioFile* file);
int nfs_iio_blocks_per_chunk(NfsIioFile* file); // Computes total blocks in one interleaved stripe
int cache_expand(NfsIioFile* file, int channel_idx, int required_page_array_idx);
int header_size(NfsIioFile* file);
void* cache_page_get_buffer(NfsIioCachePage* page);
void cache_page_set_sync(NfsIioCachePage* page, int sync_status);
int cache_page_get_sync(NfsIioCachePage* page);
int cache_page_choose_best_to_reuse(NfsIioFile* file, int channel_idx, int exclude_page_idx);
int write_header(NfsIioFile* file);
int write_absolute_block_n(NfsIioFile* file, int absolute_block_index, int num_blocks_to_write, const void* buffer);
int read_absolute_block_n(NfsIioFile* file, int absolute_block_index, int num_blocks_to_read, void* buffer);


// Layer 3 (Data File I/O with Cache - .pak files)
int is_in_cache(NfsDataHandle* handle, int block_index_to_check);
int cache_flush(NfsDataHandle* handle);
int cache_slide(NfsDataHandle* handle, int new_desired_start_offset);
int cache_create(NfsDataHandle* handle);
int cache_resize(NfsDataHandle* handle, size_t new_capacity);
int cache_destroy(NfsDataHandle* handle);
int cache_get(NfsDataHandle* handle, int file_offset_to_read_from, int num_bytes_to_read, void* output_buffer);
int cache_put(NfsDataHandle* handle, int file_offset_to_write_to, int num_bytes_to_write, const void* input_buffer);

NfsDataHandle* nfs_data_create(const char* fileName);
NfsDataHandle* nfs_data_open(const char* fileName);
void nfs_data_close(NfsDataHandle* handle);
void nfs_data_destroy(NfsDataHandle* handle);
int nfs_data_read(NfsDataHandle* handle, int block_index, void* buffer);
int nfs_data_write(NfsDataHandle* handle, int block_index, const void* buffer);
int nfs_data_read_contiguous(NfsDataHandle* handle, int start_block_index, int num_blocks, void* buffer);
int nfs_data_write_contiguous(NfsDataHandle* handle, int start_block_index, int num_blocks, const void* buffer);
int nfs_data_flush_cache(NfsDataHandle* handle);
int nfs_data_set_cache_size(NfsDataHandle* handle, size_t new_size);


// Layer 4 (FAT - File Allocation Table)
int node_get_value(NfsFatHandle* fat_handle, int fat_entry_index);
void node_set_value(NfsFatHandle* fat_handle, int fat_entry_index, int value_to_set);
int next_free(NfsFatHandle* fat_handle, int start_search_idx);
int find_last_in_chain(NfsFatHandle* fat_handle, int start_of_chain_idx);
int node_recover(NfsFatHandle* fat_handle, int fat_entry_idx_to_free);
NfsFatHandle* nfs_fat_create(NfsIioFile* iio_file, int num_iio_blocks_for_fat_channel);
NfsFatHandle* nfs_fat_open(NfsIioFile* iio_file, int fat_iio_channel_id);
int nfs_fat_close(NfsFatHandle* fat_handle);
// nfs_fat_destroy is not present in source, close is used.
int nfs_fat_create_chain(NfsFatHandle* fat_handle);
void nfs_fat_chain_for_each(NfsFatHandle* fat_handle, int start_of_chain_idx, int (*callback)(NfsFatHandle*, int));
int nfs_fat_destroy_chain(NfsFatHandle* fat_handle, int start_of_chain_idx);
int nfs_fat_chain_get_nth(NfsFatHandle* fat_handle, int start_of_chain_idx, int n);
int nfs_fat_chain_truncate(NfsFatHandle* fat_handle, int entry_idx_to_become_new_eoc);
int nfs_fat_chain_extend(NfsFatHandle* fat_handle, int chain_to_extend);
int nfs_fat_chain_shrink(NfsFatHandle* fat_handle, int start_of_chain_idx, int num_blocks_to_keep);
void nfs_fat_chain_get(NfsFatHandle* fat_handle, int start_of_chain_idx, void* output_buffer);
void nfs_fat_chain_get_first_n(NfsFatHandle* fat_handle, int start_of_chain_idx, int num_entries_to_get, void* output_buffer);


// Layer 5 (NT - Node Table)
int node_get(NfsNtHandle* nt_handle, int node_index, NfsNode* node_buffer);
int node_set(NfsNtHandle* nt_handle, int node_index, const NfsNode* node_data);
int find_first_free(NfsNtHandle* nt_handle, int start_search_idx);
int node_recover(NfsNtHandle* nt_handle, int node_idx_to_free);
NfsNtHandle* nfs_nt_create(NfsIioFile* iio_file, int num_iio_blocks_for_nt_channel);
NfsNtHandle* nfs_nt_open(NfsIioFile* iio_file, int nt_iio_channel_id);
void nfs_nt_close(NfsNtHandle* nt_handle);
void nfs_nt_destroy(NfsNtHandle* nt_handle);
int nfs_nt_get_node(NfsNtHandle* nt_handle, int node_index, NfsNode* node_buffer);
int nfs_nt_set_node(NfsNtHandle* nt_handle, int node_index, const NfsNode* node_data);
int nfs_nt_allocate_node(NfsNtHandle* nt_handle);
void nfs_nt_refcount_incr(NfsNtHandle* nt_handle, int node_index);
int nfs_nt_refcount_decr(NfsNtHandle* nt_handle, int node_index);
int nfs_nt_node_get_size(NfsNtHandle* nt_handle, int node_index);
void nfs_nt_node_set_size(NfsNtHandle* nt_handle, int node_index, int new_size);
int nfs_nt_node_get_chain(NfsNtHandle* nt_handle, int node_index);
void nfs_nt_node_set_chain(NfsNtHandle* nt_handle, int node_index, int new_fat_chain_start_idx);


// Layer 6 (DT - Directory Table with Patricia Trie)
int trienode_get(NfsDtHandle* dt_handle, int tn_idx, NfsTrieNode* out_node);
int trienode_set(NfsDtHandle* dt_handle, int tn_idx, NfsTrieNode* node_to_set);
int keynode_get(NfsDtHandle* dt_handle, int kn_idx, NfsKeyNode* out_node);
int keynode_set(NfsDtHandle* dt_handle, int kn_idx, const NfsKeyNode* node_to_set);
int trienode_is_free(NfsDtHandle* dt_handle, int tn_idx);
int trienode_find_first_free(NfsDtHandle* dt_handle, int start_idx);
int trienode_clear(NfsDtHandle* dt_handle, int tn_idx);
int trienode_recover(NfsDtHandle* dt_handle, int tn_idx);
int keynode_is_free(NfsDtHandle* dt_handle, int kn_idx);
int keynode_find_first_free(NfsDtHandle* dt_handle, int start_idx);
int keynode_clear(NfsDtHandle* dt_handle, int kn_idx);
unsigned int trienode_get_left(NfsDtHandle* dt_handle, int tn_idx);
unsigned int trienode_get_right(NfsDtHandle* dt_handle, int tn_idx);
short trienode_get_bindex(NfsDtHandle* dt_handle, int tn_idx);
unsigned int trienode_get_kindex(NfsDtHandle* dt_handle, int tn_idx);
short trienode_get_nt(NfsDtHandle* dt_handle, int tn_idx);
int trienode_set_left(NfsDtHandle* dt_handle, int tn_idx, int left_child_idx);
int trienode_set_right(NfsDtHandle* dt_handle, int tn_idx, int right_child_idx);
int trienode_set_nt(NfsDtHandle* dt_handle, int tn_idx, short nt_idx);
int fnode_extract_key(NfsDtHandle* dt_handle, int start_keynode_idx, char* output_buffer);
int fnode_free(NfsDtHandle* dt_handle, int start_keynode_idx_to_free);
int fnode_allocate(NfsDtHandle* dt_handle, const char* source_string);
int node_allocate(NfsDtHandle* dt_handle, const char* key_string, short nt_idx_for_trienode, short b_index_for_trienode);
int node_copy_key(NfsDtHandle* dt_handle, int source_trienode_idx, int dest_trienode_idx);
int p_get_head();
int p_init_head(NfsDtHandle* dt_handle);
int p_compare_keys(NfsDtHandle* dt_handle, const char* key_to_compare, int external_trienode_idx);
int p_find_first_different_bit(NfsDtHandle* dt_handle, const char* key1, int trienode_idx_for_key2);
int p_insert_key(NfsDtHandle* dt_handle, const char* key_to_insert, short nt_idx);
int p_remove_key(NfsDtHandle* dt_handle, const char* key_to_remove);
int p_lookup_key(NfsDtHandle* dt_handle, const char* key_to_lookup);
int p_lookup_key_n(NfsDtHandle* dt_handle, const char* key_to_lookup, int num_bits_to_match);
char* find_prefix(const char* pattern, char* output_buffer);
int nfs_pmatch(const char* pattern, const char* string_to_test, int flags);
int p_node_iterate(NfsDtHandle* dt_handle, int current_trienode_idx, int parent_b_index,
	const char* glob_pattern, int pmatch_flags,
	int (*callback)(NfsDtHandle*, char*, int, void*), void* callback_context);

NfsDtHandle* nfs_dt_create(NfsIioFile* iio_file, int tn_chan_blocks, int kn_chan_blocks);
NfsDtHandle* nfs_dt_open(NfsIioFile* iio_file, int trienode_channel_id, int keynode_channel_id);
int nfs_dt_close(NfsDtHandle* dt_handle);
int nfs_dt_destroy(NfsDtHandle* dt_handle);
int nfs_dt_filename_add(NfsDtHandle* dt_handle, const char* filename_to_add);
int nfs_dt_filename_delete(NfsDtHandle* dt_handle, const char* filename_to_delete);
int nfs_dt_filename_lookup(NfsDtHandle* dt_handle, const char* filename_to_lookup);
short nfs_dt_filename_get_nt_index(NfsDtHandle* dt_handle, int trienode_idx);
int nfs_dt_filename_set_nt_index(NfsDtHandle* dt_handle, int trienode_idx, short new_nt_idx);
int nfs_dt_filename_get_name(NfsDtHandle* dt_handle, int trienode_idx, char* output_name_buffer);
int nfs_dt_filename_glob(NfsDtHandle* dt_handle, const char* pattern, int flags,
	int (*errfunc)(const char* epath, int eerrno),
	NfsGlobResults* glob_results_output);

// (Pattern Matching - used by glob, could be utility)
int nfs_pmatch(const char* pattern, const char* string_to_test, int flags);


// Layer 7 (NFS API - File System Level)
int allocate_file_descriptor(NfsHandle* handle);
int deallocate_file_descriptor(NfsHandle* handle, int fd);
void file_truncate(NfsHandle* handle, NfsOpenFileHandle* fh, int new_size);

NfsHandle* nfs_start(const char* base_vfs_name, int access_mode);
void nfs_end(NfsHandle* handle, int destroy_files_flag);
int nfs_exists(const char* base_vfs_name); // Returns bool (int)
int nfs_file_exists(NfsHandle* handle, const char* filename); // Returns bool (int)
int nfs_file_open(NfsHandle* handle, const char* filename, int open_flags);
int nfs_file_create(NfsHandle* handle, const char* filename);
int nfs_file_close(NfsHandle* handle, int fd);
int nfs_file_lseek(NfsHandle* handle, int fd, int offset, int whence);
int nfs_file_read(NfsHandle* handle, int fd, void* buffer, int bytes_to_read);
int nfs_file_write(NfsHandle* handle, int fd, const void* buffer, int bytes_to_write);
int nfs_file_inc_refcount(NfsHandle* handle, const char* filename);
int nfs_file_dec_refcount(NfsHandle* handle, const char* filename);
int nfs_file_link(NfsHandle* handle, const char* old_name, const char* new_name);
int nfs_file_unlink(NfsHandle* handle, const char* filename_to_unlink);
int nfs_glob(NfsHandle* handle, const char* pattern, int flags,
	int (*errfunc)(const char* epath, int eerrno),
	NfsGlobResults* glob_results);
void nfs_glob_free(NfsGlobResults* glob_results);

#endif // NFS_H