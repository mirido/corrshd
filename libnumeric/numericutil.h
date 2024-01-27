#pragma once

/// Get number of grid points included in half-open section [0, ub).
int get_num_grid_points(const int ub, const int cyc);

/// Approximate equal operator on double type values. 
bool can_equal(const double a, const double b);

/// ‘å’Ã‚Ì•û–@‚É‚æ‚é”»•Ê•ªÍ
/// http://ithat.me/2016/02/05/opencv-discriminant-analysis-method-otsu-cpp
int discriminant_analysis_by_otsu(const std::vector<uchar>& data);
