#pragma once

/// ���P�[����ݒ肷��B
void osal_setup_locale();

/// ��ʂ̎���Ɨ̈�T�C�Y���擾����B
void osal_get_primary_monitor_work_area_size(int& width, int& height);

/// �X���b�hID���擾����B
void* osal_get_thread_id();
