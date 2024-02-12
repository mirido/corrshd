#pragma once

// Program name (Automatically acquired from argv[0].)
extern std::string PROG_NAME;

// [CONF] Program summary
extern std::string SUMMARY;

// [CONF] Copyright
extern std::string COPYRIGHT;

// [CONF] OpenCV distributer URL
extern std::string OPENCV_URL;

struct AppParam
{
	std::string m_imageFile;		// Input image file path
	PhysicalSize m_ROISize;			// ROI physical size in mm (or standart paper size name)
	double m_dpi;					// Resolution in dpi
private:
	std::string m_outfileOrg;		// Specified output image file or file path
public:
	std::string m_outfile;			// Output image file path
	bool m_bCutoffOnly;				// Cut off only or not flag

	cv::Size m_outputImgSz;			// Output image size (calculated based on m_ROISize and m_dpi)

	int m_rotAngle;					// Rotation angle of image.
	std::vector<cv::Point> m_cornerPoints;	// Corner point list of ROI.

	std::string m_imgAlgorithm;		// Image processing algorithm.

	AppParam();

	/// Parse command arguments.
	int parse(int argc, char* argv[]);

	/// Update output file path (m_outfile).
	void update_outfile_path();

	std::istream& input(std::istream& is);
	std::ostream& output(std::ostream& os) const;
	std::string str() const;
};

std::istream& operator>>(std::istream& is, AppParam& param);
std::ostream& operator<<(std::ostream& os, const AppParam& param);
