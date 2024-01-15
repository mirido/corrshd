#pragma once

struct ImagingContext
{
private:
	ImagingCanvas m_imagingCanvas;
	ClickedPointList m_clickedPointList;

	cv::Rect m_srcArea;
	int m_dispWidth;

public:
	ImagingContext();

	/// �\�[�X�摜�ݒ�
	void setSrcImage(cv::Ptr<cv::Mat> pSrcImage);

	/// �\���d�l�ݒ�
	void setDispGeometry(const cv::Rect& srcArea, const int dispWidth);

	/// ���W������
	void clearPointList();

	/// ���W�ǉ�
	void selectOrAdd(const int x, const int y);

	/// �L�����o�X�ĕ`��
	void refreshCanvas();

	/// �L�����o�X�ƃ\�[�X�摜��]
	void rotate(const int dir);

	/// �c�ݕ␳
	bool correctDistortion(const double relWidth, const double relHeight, const int outputWidth);

	/// �؂蔲��
	bool cutOff();

	/// �V�F�[�f�B���O�␳
	bool shadingCorrection();

	/// �q�X�g�O�����ϓ���
	bool equalizeHist();

	/// �L�����o�X���Q�Ƃ���B
	cv::Mat& refCanvas();

	/// Current point���擾����B
	bool getCurPt(cv::Point& pt) const;

};
