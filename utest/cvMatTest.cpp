#include "stdafx.h"

TEST(cvMatTest, normal_subtract) {
	const cv::Size imgSz(100, 100);

	const cv::Mat imgA = cv::Mat::ones(imgSz, CV_8UC1) * 30.0;
	const cv::Mat imgB = cv::Mat::ones(imgSz, CV_8UC1) * 20.0;
	ASSERT_EQ(C_UCHAR(30), imgA.at<uchar>(50, 50));
	ASSERT_EQ(C_UCHAR(20), imgB.at<uchar>(50, 50));

	cv::Mat imgC = imgA - imgB;

	EXPECT_EQ(C_UCHAR(10), imgC.at<uchar>(0, 0));
	EXPECT_EQ(C_UCHAR(10), imgC.at<uchar>(50, 50));
	EXPECT_EQ(C_UCHAR(10), imgC.at<uchar>(99, 99));
}

TEST(cvMatTest, normal_addition) {
	const cv::Size imgSz(100, 100);

	const cv::Mat imgA = cv::Mat::ones(imgSz, CV_8UC1) * 230.0;
	const cv::Mat imgB = cv::Mat::ones(imgSz, CV_8UC1) * 20.0;
	ASSERT_EQ(C_UCHAR(230), imgA.at<uchar>(50, 50));
	ASSERT_EQ(C_UCHAR(20), imgB.at<uchar>(50, 50));

	cv::Mat imgC = imgA + imgB;

	EXPECT_EQ(C_UCHAR(250), imgC.at<uchar>(0, 0));
	EXPECT_EQ(C_UCHAR(250), imgC.at<uchar>(50, 50));
	EXPECT_EQ(C_UCHAR(250), imgC.at<uchar>(99, 99));
}

TEST(cvMatTest, underflow_by_subtract) {
	const cv::Size imgSz(100, 100);

	const cv::Mat imgA = cv::Mat::ones(imgSz, CV_8UC1) * 10.0;
	const cv::Mat imgB = cv::Mat::ones(imgSz, CV_8UC1) * 20.0;
	ASSERT_EQ(C_UCHAR(10), imgA.at<uchar>(50, 50));
	ASSERT_EQ(C_UCHAR(20), imgB.at<uchar>(50, 50));

	cv::Mat imgC = imgA - imgB;

	// Above subtraction is achieved as saturation operation.
	EXPECT_EQ(C_UCHAR(0), imgC.at<uchar>(0, 0));
	EXPECT_EQ(C_UCHAR(0), imgC.at<uchar>(50, 50));
	EXPECT_EQ(C_UCHAR(0), imgC.at<uchar>(99, 99));
}

TEST(cvMatTest, overflow_by_add) {
	const cv::Size imgSz(100, 100);

	const cv::Mat imgA = cv::Mat::ones(imgSz, CV_8UC1) * 250.0;
	const cv::Mat imgB = cv::Mat::ones(imgSz, CV_8UC1) * 20.0;
	ASSERT_EQ(C_UCHAR(250), imgA.at<uchar>(50, 50));
	ASSERT_EQ(C_UCHAR(20), imgB.at<uchar>(50, 50));

	cv::Mat imgC = imgA + imgB;

	// Above addition is achieved as saturation operation.
	EXPECT_EQ(C_UCHAR(255), imgC.at<uchar>(0, 0));
	EXPECT_EQ(C_UCHAR(255), imgC.at<uchar>(50, 50));
	EXPECT_EQ(C_UCHAR(255), imgC.at<uchar>(99, 99));
}
