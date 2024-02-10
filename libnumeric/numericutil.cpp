#include "stdafx.h"
#include "numericutil.h"

// [DBGSW] �q�X�g�O�������_���v����B
//#define DUMP_HIST

/// Get number of grid points included in half-open section [0, ub).
int get_num_grid_points(const int ub, const int cyc)
{
	return (ub + (cyc - 1)) / cyc;
}

/// Approximate equal operator on double type values. 
bool can_equal(const double a, const double b)
{
	if (std::isnan(a) || std::isnan(b)) {
		return false;
	}
	if (std::isinf(a) || std::isinf(b)) {
		return (a == b);
	}

	const auto min_expn = std::numeric_limits<double>::min_exponent;
	const auto digits = std::numeric_limits<double>::digits;

	int expn_a, expn_b;
	std::frexp(a, &expn_a);
	std::frexp(b, &expn_b);

	const int expn = std::max(min_expn, std::max(expn_a, expn_b));
	assert(expn >= min_expn);

	const double mod_epsilon = std::ldexp(0.5, expn - digits);
	assert(mod_epsilon > 0.0);

	return (std::abs(a - b) <= mod_epsilon);
}

/// Generates floating point number consisting of consecutive 1's.
double gen_consecutive_1s(const int cnt, const int expn)
{
	// Make 2^(expn)
	double r = 1.0;
	if (expn > 0) {
		for (int i = 0; i < expn; i++) {
			r *= 2.0;
		}
	}
	else {
		for (int i = 0; i > expn; i--) {
			r /= 2.0;
		}
	}

	// Make 1.111...*2^(expn)
	double x = 0.0;
	for (int i = 0; i < cnt; i++) {
		x += r;
		r /= 2.0;
	}

	return x;
}

/// ��Â̕��@�ɂ�锻�ʕ���
/// http://ithat.me/2016/02/05/opencv-discriminant-analysis-method-otsu-cpp
int discriminant_analysis_by_otsu(const std::vector<uchar>& data)
{
	/* �q�X�g�O�����쐬 */
	std::vector<int> hist(256, 0);  // 0-255��256�i�K�̃q�X�g�O����(�v�f��256�A�S��0�ŏ�����)
	for (auto it = data.begin(); it != data.end(); it++) {
		assert(0 <= *it && *it < 256);
		hist[static_cast<int>(*it)]++;  // �P�x�l���W�v
	}
#ifdef DUMP_HIST
	for (int i = 0; i < 256; i++) {
		cout << std::setbase(10) << "hist[" << i << "]," << i << "," << hist[i] << endl;
	}
#endif

	/* ���ʕ��͖@ */
	int t = 0;  // 臒l
	double max = 0.0;  // w1 * w2 * (m1 - m2)^2 �̍ő�l

	for (int i = 0; i < 256; ++i) {
		int w1 = 0;  // �N���X�P�̉�f��
		int w2 = 0;  // �N���X�Q�̉�f��
		long sum1 = 0;  // �N���X�P�̕��ς��o�����߂̍��v�l
		long sum2 = 0;  // �N���X�Q�̕��ς��o�����߂̍��v�l
		double m1 = 0.0;  // �N���X�P�̕���
		double m2 = 0.0;  // �N���X�Q�̕���

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
