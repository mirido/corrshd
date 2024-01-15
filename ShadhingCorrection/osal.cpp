#include "stdafx.h"
#include "osal.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <locale.h>

/// ���P�[����ݒ肷��B
void osal_setup_locale()
{
	setlocale(LC_ALL, "");
	_wsetlocale(LC_ALL, L"");

	cin.imbue(std::locale(""));
	cout.imbue(std::locale(""));
	cerr.imbue(std::locale(""));

	std::wcin.imbue(std::locale(""));
	std::wcout.imbue(std::locale(""));
}

/// ��ʂ̎���Ɨ̈�T�C�Y���擾����B
void osal_get_primary_monitor_work_area_size(int& width, int& height)
{
	RECT rect;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);

	width = rect.right - rect.left;
	height = rect.bottom - rect.top;
}

/// �X���b�hID���擾����B
void* osal_get_thread_id()
{
	return (void*)(INT_PTR)GetCurrentThreadId();
}
