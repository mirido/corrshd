#pragma once

/// ロケールを設定する。
void osal_setup_locale();

/// 画面の実作業領域サイズを取得する。
void osal_get_primary_monitor_work_area_size(int& width, int& height);

/// スレッドIDを取得する。
void* osal_get_thread_id();
