#pragma once

class ImagingContext
{
	cv::Ptr<cv::Mat> m_pSrcImage;
	ImagingCanvas m_imagingCanvas;
	ClickedPointList m_clickedPointList;
	int m_nImgRotAngle;

	// Image processing algorithm after perspective correction
	std::shared_ptr<IImgFunc> m_pImgFunc;
	std::vector<std::string> m_imgFuncNames;
	std::map<std::string, std::shared_ptr<IImgFunc> > m_imgFuncDic;

public:
	ImagingContext();

	/// �\�[�X�摜�ݒ�
	void setSrcImage(cv::Ptr<cv::Mat> pSrcImage);

	/// �\�[�X�摜�Q��
	cv::Ptr<cv::Mat> refSrcImage();

	/// �L�����o�X�ݒ�
	bool setupCanvas(const cv::Rect& srcArea, const cv::Size& dispSize);

	/// �L�����o�X�Q��
	cv::Mat& refCanvas();

	/// ���W������
	void clearPointList();

	/// Set state at once.
	void setState(const int nImgRotAngle, const std::vector<cv::Point>& points);

	/// �������W�񂪋󂩔ۂ���Ԃ��B
	bool isPointListEmpty() const;

#if 0
	/// �������W�̌����擾����B
	int pointListSize() const;
#endif

#if 0
	/// �������W����擾����B
	int getPointList(std::vector<cv::Point>& points) const;
#endif

	/// �ł����ォ�玞�v���̏��̃��X�g���擾����B
	std::vector<cv::Point> getClockwiseList() const;

	/// �������W�I��
	bool selectExistingPointIF(const int dispX, const int dispY);

	/// ���W�̒ǉ��܂��͈ړ�
	void addOrMovePoint(const int dispX, const int dispY);

	/// Current point�擾
	bool getCurPoint(cv::Point& curPt) const;

	/// Current point�ړ�
	void moveCurPoint(const int dx, const int dy);

	/// Current point�؂�ւ�
	void changeCurrentPointToNext();

	/// �L�����o�X�ĕ`��
	void refreshCanvas();

	/// �L�����o�X�ƃ\�[�X�摜��]
	void rotate(const int dir);
	
	/// Get rotation angle of canvas and source image.
	int getImgRotAngle() const;

	/// Select image processing algorithm.
	bool selectImagingAlgorithmByName(const std::string& name);

	/// Get current image processing algorithm name.
	std::string getCurImagingAlgorithmName() const;

	/// Select image processing algorithm.
	bool selectImagingAlgorithmByIdx(const int idx);

	/// Get current image processing algorithm index.
	int getCurImagingAlgorithmIdx() const;

	/// Get number of image processing algorithms.
	int getNumImagingAlgorithms() const;

	/// �c�ݕ␳
	bool correctDistortion(const cv::Size& dstSz, cv::Mat& dstImg);

	/// �V�F�[�f�B���O�␳
	bool doShadingCorrection(const cv::Size& dstSz, cv::Mat& dstImg);

};
