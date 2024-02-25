#include "stdafx.h"
#include "PhysicalSize.h"
#include "PointWrp.h"
#include "DstImgSizeFunc.h"
#include "AppParam.h"

#include "IImgFunc.h"
#include "ImgFuncBase.h"	/*introduce DBG_IMG_DIR*/

#include "osal.h"
#include "pathutil.h"
#include "streamutil.h"

namespace
{
	/// Parse string as DstImgFunc.
	bool parse_as_DstImgFunc(const char* const str, DstImgSizeFunc& dszfunc)
	{
		std::istringstream ist(str);
		ist.setf(std::ios::skipws);
		ist >> dszfunc;
		return !!ist;
	}

	/// Parse string as std::vector<PointWrp>.
	bool parse_as_PointWrp_list(const char* const str, std::vector<cv::Point>& ptls)
	{
		PointWrp pt;
		char c;

		std::istringstream ist(str);
		ist.setf(std::ios::skipws);

		ptls.clear();
		for (int i = 0; i < 4; i++) {
			ist >> pt;
			if (!ist) {
				return false;
			}
			ptls.push_back(pt);

			if (i + 1 < 4) {
				ist >> c;
				if (ist.eof()) {
					return true;
				}
				if (!ist || c != ',') {
					return false;
				}
			}
		}

		return !!ist;
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
		std::ostringstream ost;
		ost
			<< PROG_NAME << " -- " << SUMMARY << endl
			<< COPYRIGHT << endl
			<< "Using OpenCV version: " << CV_VERSION << endl
			<< endl
			<< "Description:" << endl
			<< "This program captures a specified input-image ROI (region of interest)" << endl
			<< "and corrects for perspective distortion, uneven brightness, and line shading." << endl
			<< "The correcting algorithm is particularly suitable to capturing hand-drawn line drawings." << endl;
		return ost.str().c_str();
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
		"{ @roi-size        | B5    | physical size of ROI (or magnification ratio of src image to dst image) }"
		"{ dpi              | 96.0  | output resolution in dot per inch }"
		"{ outfile          |       | output image file name }"
		"{ cutoffonly       |       | do nothing other than perspective correction }"
		"{ rot              |  0    | rotation angle }"
		"{ corners          |       | corner pointlist of ROI }"
		"{ algorithm        |       | image processing algorithm }"
		;

}	// namespace

AppParam::AppParam()
	: m_bCutoffOnly(false), m_rotAngle(0), m_bDumpItmImg(false), m_dbgImgDir(DBG_IMG_DIR)
{
#ifndef NDEBUG
	m_bDumpItmImg = true;
#endif
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
	const double dpi = parser.get<double>("dpi");
	if (!parser.check() || dpi <= 0.0) {
		if (parser.check()) {
			cerr << "ERROR: Illegal dpi value. (-dpi=\"" << parser.get<cv::String>("dpi") << "\")" << endl;
		}
		return -1;
	}
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
	m_dstImgSizeFunc.setDpi(dpi);
	if (!parse_as_DstImgFunc(ROISizeStr.c_str(), m_dstImgSizeFunc)) {
		cerr << "ERROR: Illegal image size specification. (roi-size=" << ROISizeStr << ")" << endl;
		return 1;
	}

	// Parser step 3: Update outfile path.
	updateOutfilePath();

	return 0;
}

/// Update output file path (m_outfile).
void AppParam::updateOutfilePath()
{
	std::string dir, fnameMajor, ext;
	parse_file_name(m_imageFile.c_str(), dir, fnameMajor, ext);

	if (!m_outfileOrg.empty()) {
		// (Parser already has output file name)
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
		// (Parser does not have output file name)
		// If no outfile specified, output with modified image-file name. 
		const std::string algname = (!m_imgAlgorithm.empty()) ? m_imgAlgorithm : "xxx";
		m_outfile = dir + fnameMajor + "_" + algname + ext;
	}
}

/// Input dialogue.
bool AppParam::inputDialogue(std::ostream& os, std::istream& is)
{
	std::istringstream ist;
	std::string strTmp;
	double fTmp;
	DstImgSizeFunc dszFuncTmp;
	bool bRet;

	os << endl;

	//	Major items.
	do {
		os << "*" << endl;
		os << "*   <<Basic settings>>" << endl;
		os << "*" << endl;
		os << endl;

		// m_imageFile
		do {
			// Input
			os << "[Required] Source (input) image file:" << endl;
			if (!m_imageFile.empty()) {
				os << "(current=\"" << m_imageFile << "\")" << endl;
			}
			os << ">";
			strTmp = get_line_from_istream(is);
			if (!is) { return false; }

			// Check
			if (strTmp.empty() && !m_imageFile.empty()) {
				strTmp = m_imageFile;
			}
			if (strTmp.empty() || !osal_is_regular_file(strTmp.c_str())) {
				os << "ERROR: File not found." << endl;
				continue;
			}

			// Pass
			m_imageFile = strTmp;
			break;
		} while (1);
		os << endl;

		// m_dstImgSizeFunc
		do {
			// Input
			os << "[Required] Physical size of target (or magnification of output image to target):" << endl;
			if (!m_dstImgSizeFunc.empty()) {
				os << "(current=" << m_dstImgSizeFunc << ")" << endl;
			}
			os << "Physical size Example: \"B5\" means B5 size, \"LB5\" means landscape B5 size." << endl;
			os << "Magnification Example: \"x0.5\" means the maginification 0.5 of output image to target." << endl;
			os << ">";
			strTmp = get_line_from_istream(is);
			if (!is) { return false; }

			// Check
			ist.clear();
			ist.str(strTmp);
			ist >> dszFuncTmp;
			if (!ist) {
				os << "ERROR: Syntax error." << endl;
				continue;
			}

			// Pass
			m_dstImgSizeFunc = dszFuncTmp;
			break;
		} while (1);
		os << endl;

		// dpi
		if (!m_dstImgSizeFunc.isMagnificationMode()) {
			do {
				// Input
				os << "[Required] dpi of output image:" << endl;
				if (!m_dstImgSizeFunc.empty()) {
					os << "(current=" << m_dstImgSizeFunc.getDpi() << " dpi)" << endl;
				}
				os << ">";
				is >> fTmp;
				if (!is) { return false; }
				is.ignore();

				// Check
				if (fTmp <= 0.0) {
					os << "ERROR: Illegal dpi value. (" << fTmp << " dpi)" << endl;
					continue;
				}

				// Pass
				m_dstImgSizeFunc.setDpi(fTmp);
				break;
			} while (1);
			os << endl;
		}

		// m_outfileOrg
		do {
			// Input
			os << "[Optional] Output image file:" << endl;
			if (!m_outfileOrg.empty()) {
				os << "(current=\"" << m_outfileOrg << "\")" << endl;
			}
			os << ">";
			strTmp = get_line_from_istream(is);
			if (!is) { return false; }

			// Check
			/*pass*/

			// Pass
			m_outfileOrg = strTmp;
			break;
		} while (1);
		os << endl;

		// m_bCutoffOnly
		do {
			// Input
			os << "[Optional] Do you want to shading correction after cut out? ";
			bRet = get_selection_from_istream(os, is, "Yes No", strTmp, true);

			// Check
			if (!bRet) {
				os << "ERROR: Syntax error. (input=\"" << strTmp << "\")" << endl;
				continue;
			}

			// Pass
			m_bCutoffOnly = (strTmp == "NO");
			break;
		} while (1);
		os << endl;

		// m_rotAngle -- Excluded from dialogue.
		/*pass*/

		// m_cornerPoints -- Excluded from dialogue.
		/*pass*/

		// m_imgAlgorithm -- Excluded from dialogue.
		/*pass*/

		// Show output file path.
		updateOutfilePath();
		os << "Result image file path=\"" << m_outfile << "\")" << endl;
		os << endl;

		// Confirm
		do {
			// Input
			os << "Confirm above setting. ";
			bRet = get_selection_from_istream(os, is, "Yes No Retry", strTmp, false);

			// Check
			if (!bRet) {
				os << "ERROR: Syntax error. (input=\"" << strTmp << "\")" << endl;
				continue;
			}

			// Pass
			break;
		} while (1);
		cout << endl;

		// Check
		if (strTmp != "YES") {
			// (Retry)
			continue;
		}

		// Pass
		break;
	} while (1);

	// Minor items.
	do {
		os << "*" << endl;
		os << "*   <<Minor settings>>" << endl;
		os << "*" << endl;
		os << endl;

		// Show current settings.
		os << "Current settings:" << endl;
		os << "- Dump intermediate image: " << ((m_bDumpItmImg) ? "YES" : "NO") << endl;
		if (m_bDumpItmImg) {
			os << "- Directory to save intermediate images: \"" << m_dbgImgDir << "\"" << endl;
		}
		os << endl;

		// Query go / nogo.
		os << "Proceed with above setting(s)? ";
		bRet = get_selection_from_istream(os, is, "Yes No", strTmp, true);
		
		// Check
		if (!bRet) {
			os << "ERROR: Syntax error. (input=\"" << strTmp << "\")" << endl;
			continue;
		}
		if (strTmp == "YES") {
			// Pass
			break;
		}
		
		// m_bDumpItmImg
		do {
			// Input
			os << "[Optional] Do you want to dump intermediate image? ";
			bRet = get_selection_from_istream(os, is, "Yes No", strTmp, false);

			// Check
			if (!bRet) {
				os << "ERROR: Syntax error. (input=\"" << strTmp << "\")" << endl;
				continue;
			}

			// Pass
			m_bDumpItmImg = (strTmp == "YES");
			break;
		} while (1);
		os << endl;

		// m_dbgImgDir
		if (m_bDumpItmImg) {
			do {
				// Input
				os << "[Required] Input directory to save intermediate image." << endl;
				if (!m_dbgImgDir.empty()) {
					os << "(current=\"" << m_dbgImgDir << "\")" << endl;
				}
				os << ">";
				strTmp = get_line_from_istream(is);
				if (!is) { return false; }

				// Check
				if (strTmp.empty() && !m_dbgImgDir.empty()) {
					strTmp = m_dbgImgDir;
				}
				if (strTmp.empty() || !osal_ensure_dir_exists(strTmp.c_str())) {
					os << "ERROR: Directory creation failed." << endl;
					continue;
				}

				// Pass
				m_imageFile = strTmp;
				break;
			} while (1);
			os << endl;
		}
	} while (1);
	os << endl;

	return true;
}

std::istream& AppParam::input(std::istream& is)
{
	// TODO: Create.
	return is;
}

std::ostream& AppParam::output(std::ostream& os) const
{
	os << "\"" << m_imageFile << "\"";
	os << " ";
	os << m_dstImgSizeFunc;
	if (!m_dstImgSizeFunc.isMagnificationMode()) {
		os << " ";
		os << "-dpi=" << m_dstImgSizeFunc.getDpi();
	}
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
