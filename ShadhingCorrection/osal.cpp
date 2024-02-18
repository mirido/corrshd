#include "stdafx.h"
#include "osal.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <imagehlp.h>
#include <Shlwapi.h>

#include <locale.h>

/// ロケールを設定する。
void osal_setup_locale()
{
	setlocale(LC_ALL, "C");
	_wsetlocale(LC_ALL, L"C");

	//cin.imbue(std::locale("C", std::locale::all));
	//cout.imbue(std::locale("C", std::locale::all));
	//cerr.imbue(std::locale("C", std::locale::all));

	//std::wcin.imbue(std::locale("C", std::locale::all));
	//std::wcout.imbue(std::locale("C", std::locale::all));

	std::locale::global(std::locale("C", std::locale::all));
}

/// 画面の実作業領域サイズを取得する。
void osal_get_primary_monitor_work_area_size(int& width, int& height)
{
	RECT rect;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);

	width = rect.right - rect.left;
	height = rect.bottom - rect.top;
}

/// スレッドIDを取得する。
void* osal_get_thread_id()
{
	return (void*)(INT_PTR)GetCurrentThreadId();
}

/// Check if given file path corresponds to a regular file.
bool osal_is_regular_file(const char* const fpath)
{
	return !!PathFileExistsA(fpath);
}

/// Ensure specified directry exists.
bool osal_ensure_dir_exists(const char* const fpath)
{
	return !!MakeSureDirectoryPathExists(fpath);
}

#pragma comment(lib, "imagehlp.lib")
#pragma comment(lib, "Shlwapi.lib")
