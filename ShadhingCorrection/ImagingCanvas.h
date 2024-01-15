#pragma once

class ImagingCanvas
{
	cv::Ptr<cv::Mat> m_pSrcImage;
	cv::Mat m_canvas;
	cv::Mat m_canvasMaster;
	cv::Rect m_srcArea;
	cv::Size m_dispSize;

	cv::Rect m_dirtyArea;

public:
	ImagingCanvas();

	/// �\�[�X�摜��ݒ肷��B
	void setSrcImage(cv::Ptr<cv::Mat> pSrcImage);
	
	/// �L�����o�X���X�V����B
	bool setupCanvas(const cv::Rect& srcArea, const cv::Size& dispSize);

	/// �}�E�X�N���b�N�ʒu���\�[�X�摜��̍��W�ɕϊ�����B
	cv::Point convToSrcPoint(const int dispX, const int dispY);

	/// �\�[�X�摜��̍��W���}�E�X�N���b�N�ʒu�ɕϊ�����B
	void convToDispPoint(const cv::Point& srcPt, int& dispX, int& dispY);

	/// �L�����o�X�ɑ��p�`��`�悷��B
	void drawPolylines(const std::vector<cv::Point>& vertexes, const int vtxMarkerRadius, const int curIdx);

	/// �L�����o�X�ւ̕`�����������B
	void cleanup();

	/// �L�����o�X��90����]����B(���W�n�͍��㌴�_�O��)
	void rotate(const int dir, cv::Point& ofsAfterRot);

	/// �L�����o�X���Q�Ƃ���B
	cv::Mat& refCanvas();

private:
	/// �\�[�X�摜��̍��W���L�����o�X��̍��W�ɕϊ�
	cv::Point convToCanvasPoint(const cv::Point& point) const;

	/// �\�[�X�摜��̍��W���L�����o�X��̍��W�Ɉꊇ�ϊ�
	std::vector<cv::Point> convToCanvasPointInBulk(const std::vector<cv::Point>& points) const;

};
