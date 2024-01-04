#include "stdafx.h"
#include "osal.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

/// 画面の実作業領域サイズを取得する。
void get_primary_monitor_work_area_size(int& width, int& height)
{
	RECT rect;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);

	width = rect.right - rect.left;
	height = rect.bottom - rect.top;
}

/// スレッドIDを取得する。
void* get_thread_id()
{
	return (void*)(INT_PTR)GetCurrentThreadId();
}
