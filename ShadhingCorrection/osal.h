#pragma once

/// ���P�[����ݒ肷��B
void osal_setup_locale();

/// ��ʂ̎���Ɨ̈�T�C�Y���擾����B
void osal_get_primary_monitor_work_area_size(int& width, int& height);

/// �X���b�hID���擾����B
void* osal_get_thread_id();

/// Check if given file path corresponds to a regular file.
bool osal_is_regular_file(const char* const fpath);

/// Ensure specified directry exists.
bool osal_ensure_dir_exists(const char* const fpath);
