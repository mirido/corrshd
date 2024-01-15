#pragma once

class ImagingCanvas
{
	cv::Ptr<cv::Mat> m_pSrcImage;
	cv::Mat m_canvas;
	cv::Rect m_dirtyArea;

public:
	ImagingCanvas();

	/// �\�[�X�摜��ݒ肷��B
	bool setSrcImage(cv::Ptr<cv::Mat> pSrcImage);

	/// �L�����o�X�ɑ��p�`��`�悷��B
	void drawPolylines(const std::vector<cv::Point>& vertexes, const int vtxMarkerRadius, const double magToDisp);

	/// �L�����o�X�ւ̕`�����������B
	void cleanup();

	/// �L�����o�X��90����]����B(���W�n�͍���n�O��)
	void rotate(const int dir, cv::Point& ofsAfterRot);

	/// �\�[�X�摜���Q�Ƃ���B
	cv::Ptr<cv::Mat> getSrcImagePtr();

	/// �L�����o�X���Q�Ƃ���B
	cv::Mat& refCanvas();

};
