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
	DstImgSizeFunc m_dstImgSizeFunc;	// The function to determine destination image size.

private:
	std::string m_outfileOrg;		// Specified output image file or file path
public:
	std::string m_outfile;			// Output image file path
	bool m_bCutoffOnly;				// Cut off only or not flag

	int m_rotAngle;					// Rotation angle of image.
	std::vector<cv::Point> m_cornerPoints;	// Corner point list of ROI.

	std::string m_imgAlgorithm;		// Image processing algorithm.

	// Minor items.
	bool m_bDumpItmImg;				// Dump intermediate image or not.
	std::string m_dbgImgDir;		// Directry to save intermediate images.

public:
	AppParam();

	/// Parse command arguments.
	int parse(int argc, char* argv[]);

	/// Update output file path (m_outfile).
	void updateOutfilePath();

	/// Input dialogue.
	bool inputDialogue(std::ostream& os, std::istream& is);

	std::istream& input(std::istream& is);
	std::ostream& output(std::ostream& os) const;
	std::string str() const;
};

std::istream& operator>>(std::istream& is, AppParam& param);
std::ostream& operator<<(std::ostream& os, const AppParam& param);
