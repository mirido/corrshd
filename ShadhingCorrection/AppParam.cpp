#include "stdafx.h"
#include "PhysicalSize.h"
#include "PointWrp.h"
#include "AppParam.h"

#include "pathutil.h"
#include "PhysicalSize.h"

// [CONF] Factor to convert 1 inch to mm
const double mm_per_inch = 25.4;

namespace
{
	/// Parse string as PhysicalSize.
	bool parse_as_Size2d(const char* const str, PhysicalSize& psz)
	{
		std::istringstream is(str);
		is >> psz;
		return !!is;
	}

	/// Parse string as std::vector<PointWrp>.
	bool parse_as_PointWrp_list(const char* const str, std::vector<cv::Point>& ptls)
	{
		PointWrp pt;
		char c;

		std::istringstream is(str);
		is.setf(std::ios::skipws);

		ptls.clear();
		for (int i = 0; i < 4; i++) {
			is >> pt;
			if (!is) {
				return false;
			}
			ptls.push_back(pt);

			if (i + 1 < 4) {
				is >> c;
				if (is.eof()) {
					return true;
				}
				if (!is || c != ',') {
					return false;
				}
			}
		}

		return !!is;
	}

	/// Stringization method.
	std::string str_op_PointWrp_list(const std::vector<cv::Point>& ptls)
	{
		std::ostringstream ost;
		for (size_t i = ZT(0); i < ZT(4); i++) {
			ost << PointWrp(ptls[i]);
			if (i + ZT(1) < ZT(4)) {
				ost << ",";
			}
		}
		return ost.str();
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
			<< "Rotation angle is discretized in units of 90 degrees." << endl
			<< "(Ex. -1=-90 deg (CCW), 0=0 deg, 1=90 deg (CW), 2=180 deg)" << endl
			<< "Corner pointlist of ROI is a list of points separated by commas." << endl
			<< "(Ex. \"(774, 3653),(2901, 3702),(2877, 580),(728, 629)\")" << endl;
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
		"{ cutoffonly       |       | do nothing other than perspective correction }"
		"{ rot              |  0    | rotation angle }"
		"{ corners          |       | corner pointlist of ROI }"
		"{ algorithm        |       | image processing algorithm }"
		;

}	// namespace

AppParam::AppParam()
	: m_dpi(0.0), m_bCutoffOnly(false), m_rotAngle(0)
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
	m_outfileOrg.clear();
	if (parser.has("outfile")) {
		tmp = parser.get<cv::String>("outfile");
		if (parser.check()) {
			m_outfileOrg = static_cast<std::string>(tmp);
		}
	}
	// -cutoffonly
	m_bCutoffOnly = parser.has("cutoffonly");
	// Error check
	if (!parser.check()) {
		parser.printErrors();
		return 1;
	}
	// (Rotation angle)
	m_rotAngle = 0;
	if (parser.has("rot")) {
		const int rot = parser.get<int>("rot");
		if (parser.check()) {
			m_rotAngle = rot;
		}
	}
	// (Corner points)
	if (parser.has("corners")) {
		const std::string ptls = parser.get<std::string>("corners");
		if (!parser.check() || !parse_as_PointWrp_list(ptls.c_str(), m_cornerPoints)) {
			cerr << "ERROR: Illegal corner points. (\"" << ptls << "\")" << endl;
		}
	}
	// (Image processing algorithm)
	m_imgAlgorithm.clear();
	if (parser.has("algorithm")) {
		const std::string algname = parser.get<std::string>("algorithm");
		if (!parser.check()) {
			cerr << "ERROR: Illegal image processing algorithm. (\"" << algname << "\")" << endl;
		}
		m_imgAlgorithm = algname;
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

	// Parser step 3: Update outfile path.
	update_outfile_path();

	return 0;
}

/// Update output file path (m_outfile).
void AppParam::update_outfile_path()
{
	std::string dir, fnameMajor, ext;
	parse_file_name(m_imageFile.c_str(), dir, fnameMajor, ext);

	if (!m_outfileOrg.empty()) {
		// (Parser have been "outfile")
		// If outfile specified, output with that file name.
		// In this case, if the output file name does not have a directory,
		// it will be output to the same directory as the input file name.
		const std::string tmp2 = static_cast<std::string>(m_outfileOrg);
		std::string dir2, fnameMajor2, ext2;
		parse_file_name(tmp2.c_str(), dir2, fnameMajor2, ext2);
		if (dir2.empty()) {
			m_outfile = dir + tmp2;
		}
		else {
			m_outfile = tmp2;
		}
	}
	else {
		// (Parser have not been "outfile")
		// If no outfile specified, output with modified image-file name. 
		const std::string algname = (!m_imgAlgorithm.empty()) ? m_imgAlgorithm : "mod";
		m_outfile = dir + fnameMajor + "_" + algname + ext;
	}
}

std::istream& AppParam::input(std::istream& is)
{




	return is;
}

std::ostream& AppParam::output(std::ostream& os) const
{
	os << "\"" << m_imageFile << "\"";
	os << " ";
	os << m_ROISize.str();
	os << " ";
	os << "-dpi=" << m_dpi;
	if (!m_outfileOrg.empty()) {
		os << " ";
		os << "-outfile=\"" << m_outfile << "\"";
	}
	if (m_bCutoffOnly) {
		os << " ";
		os << "-cutoff";
	}
	os << " ";
	os << "-rot=" << m_rotAngle;
	if (!m_cornerPoints.empty()) {
		os << " ";
		os << "-corners=\"" << str_op_PointWrp_list(m_cornerPoints) << "\"";
	}
	if (!m_imgAlgorithm.empty()) {
		os << " ";
		os << "-algorithm=\"" << m_imgAlgorithm << "\"";
	}

	return os;
}

std::string AppParam::str() const
{
	std::ostringstream ost;
	output(ost);
	return ost.str();
}

std::istream& operator>>(std::istream& is, AppParam& param)
{
	return param.input(is);
}

std::ostream& operator<<(std::ostream& os, const AppParam& param)
{
	return param.output(os);
}
