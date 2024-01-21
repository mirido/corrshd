#include "stdafx.h"
#include "numericutil.h"

// [DBGSW] ヒストグラムをダンプする。
//#define DUMP_HIST

/// 大津の方法による判別分析
/// http://ithat.me/2016/02/05/opencv-discriminant-analysis-method-otsu-cpp
int discriminant_analysis_by_otsu(const std::vector<uchar>& data)
{
	/* ヒストグラム作成 */
	std::vector<int> hist(256, 0);  // 0-255の256段階のヒストグラム(要素数256、全て0で初期化)
	for (auto it = data.begin(); it != data.end(); it++) {
		assert(0 <= *it && *it < 256);
		hist[static_cast<int>(*it)]++;  // 輝度値を集計
	}
#ifdef DUMP_HIST
	for (int i = 0; i < 256; i++) {
		cout << std::setbase(10) << "hist[" << i << "]," << i << "," << hist[i] << endl;
	}
#endif

	/* 判別分析法 */
	int t = 0;  // 閾値
	double max = 0.0;  // w1 * w2 * (m1 - m2)^2 の最大値

	for (int i = 0; i < 256; ++i) {
		int w1 = 0;  // クラス１の画素数
		int w2 = 0;  // クラス２の画素数
		long sum1 = 0;  // クラス１の平均を出すための合計値
		long sum2 = 0;  // クラス２の平均を出すための合計値
		double m1 = 0.0;  // クラス１の平均
		double m2 = 0.0;  // クラス２の平均

		for (int j = 0; j <= i; ++j) {
			w1 += hist[j];
			sum1 += j * hist[j];
		}

		for (int j = i + 1; j < 256; ++j) {
			w2 += hist[j];
			sum2 += j * hist[j];
		}

		if (w1 != 0) {
			m1 = (double)sum1 / w1;
		}

		if (w2 != 0) {
			m2 = (double)sum2 / w2;
		}

		double tmp = ((double)w1 * w2 * (m1 - m2) * (m1 - m2));

		if (tmp > max) {
			max = tmp;
			t = i;
		}
	}

	return t;
}
