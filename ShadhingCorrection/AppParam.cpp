#include "stdafx.h"
#include "PhysicalSize.h"
#include "AppParam.h"

#include "pathutil.h"
#include "PhysicalSize.h"

// [CONF] Factor to convert 1 inch to mm
const double mm_per_inch = 25.4;

namespace
{
	/// parse string as PhysicalSize
	bool parse_as_Size2d(const char* const str, PhysicalSize& psz)
	{
		std::istringstream is(str);
		is >> psz;
		return !!is;
	}

	/// Generate about message.
	cv::String gen_about_msg()
	{
		std::ostringstream os;
		os
			<< PROG_NAME << " -- " << SUMMARY << endl
			<< COPYRIGHT << endl
			<< "Using OpenCV version: " << CV_VERSION << endl
			<< endl
			<< "Description:" << endl
			<< "This program captures a specified input-image ROI (region of interest)" << endl
			<< "and corrects for perspective distortion, uneven brightness, and line shading." << endl
			<< "The correcting algorithm is particularly suitable to capturing hand-drawn line drawings." << endl;
		return os.str().c_str();
	}

	/// Print usage.
	void print_usage(const cv::CommandLineParser& parser)
	{
		parser.printMessage();
		cout << endl
			<< "Hot keys:" << endl
			<< "\tESC          - quit the program" << endl
			<< "\tr            - rotate input-image 90 degrees clockwise" << endl
			<< "\tR            - rotate input-image 90 degrees counterclockwise" << endl
			<< "\tz            - zoom in or out the input-image (toggled)" << endl
			<< "\t(cursor key) - move the current corner point of ROI (range of interest) in pixel-wise" << endl
			<< "\tTAB          - switch the current corner point of ROI to neighbor" << endl
			<< "\ts            - correct the input-image ROI and save result" << endl;
		cout << endl
			<< "Use your mouse to click the 4 corner points of ROI (range of interest) on the image." << endl
			<< "Then you press the hotkey \"s\" will output the corrected image within the ROI." << endl
			<< "(If pressing the hot key \"s\" without clicking any corner points," << endl
			<< " the entire image will be corrected.)" << endl;
	}

	// Definition of command line arguments.
	const cv::String keys =
		"{ h ?              |       | print this message }"
		"{ @image-file      |       | image file to be corrected }"
		"{ @roi-size        | B5    | physical size of ROI }"
		"{ dpi              | 96.0  | output resolution in dot per inch }"
		"{ outfile          |       | output image file name }"
		"{ cutoffonly       |       | do nothing other than perspective correction }";

}	// namespace

AppParam::AppParam()
	: m_dpi(0.0), m_bCutoffOnly(false)
{
	/*pass*/
}

/// Parse command arguments.
int AppParam::parse(int argc, char* argv[]) {
	cv::String tmp;

	// Prepare CommandlineParser
	cv::CommandLineParser parser(argc, argv, keys);
	parser.about(gen_about_msg());

	// Print usage if help specified.
	if (argc <= 1 || parser.has("h")) {
		print_usage(parser);
		return -1;
	}

	// Parse step 1: get and check with parse object.
	// (image file name)
	tmp = parser.get<cv::String>("@image-file");
	m_imageFile = static_cast<std::string>(tmp);
	// (ROI size)
	const cv::String ROISizeStr = parser.get<cv::String>("@roi-size");
	// -dpi=<x>
	m_dpi = parser.get<double>("dpi");
	// -outfile=<output-file>
	std::string dir, fnameMajor, ext;
	parse_file_name(m_imageFile.c_str(), dir, fnameMajor, ext);
	if (parser.has("outfile")) {
		// If outfile specified, output with that file name.
		// In this case, if the output file name does not have a directory,
		// it will be output to the same directory as the input file name.
		tmp = parser.get<cv::String>("outfile");
		if (parser.check()) {
			const std::string tmp2 = static_cast<std::string>(tmp);
			std::string dir2, fnameMajor2, ext2;
			parse_file_name(tmp2.c_str(), dir2, fnameMajor2, ext2);
			if (dir2.empty()) {
				m_outfile = dir + tmp2;
			}
			else {
				m_outfile = tmp2;
			}
		}
	}
	else {
		// If no outfile specified, output with modified image-file name. 
		m_outfile = dir + fnameMajor + "_mod" + ext;
	}
	// -cutoffonly
	m_bCutoffOnly = parser.has("cutoffonly");
	// Error check
	if (!parser.check()) {
		parser.printErrors();
		return 1;
	}

	// Parse step 2: Do detailed conversion.
	if (m_dpi <= 0.0) {
		cerr << "ERROR: Illegal dpi value. (-dpi=" << m_dpi << ")" << endl;
		return 1;
	}
	PhysicalSize psz;
	if (!parse_as_Size2d(ROISizeStr.c_str(), psz)) {
		cerr << "ERROR: Illegal ROI size. (roi-size=" << ROISizeStr << ")" << endl;
		return 1;
	}
	const int widthInPx = (int)std::round((m_dpi * psz.width()) / mm_per_inch);
	const int heightInPx = (int)std::round((m_dpi * psz.height()) / mm_per_inch);
	m_ROISize = psz;
	m_outputImgSz = cv::Size(widthInPx, heightInPx);

	return 0;
}
