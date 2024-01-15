#include "stdafx.h"
#include "osal.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

/// ��ʂ̎���Ɨ̈�T�C�Y���擾����B
void get_primary_monitor_work_area_size(int& width, int& height)
{
	RECT rect;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);

	width = rect.right - rect.left;
	height = rect.bottom - rect.top;
}

/// �X���b�hID���擾����B
void* get_thread_id()
{
	return (void*)(INT_PTR)GetCurrentThreadId();
}
