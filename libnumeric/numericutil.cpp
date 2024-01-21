#include "stdafx.h"
#include "numericutil.h"

// [DBGSW] �q�X�g�O�������_���v����B
//#define DUMP_HIST

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
