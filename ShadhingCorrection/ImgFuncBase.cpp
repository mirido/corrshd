#include "stdafx.h"
#include "IImgFunc.h"
#include "ImgFuncBase.h"

#include "../libimaging/geometryutil.h"
#include "pathutil.h"

// [CONF] Default image file extension.
#define DEFAULT_IMG_EXT		".bmp"

//
//	For DEBUG
//

unsigned long ImgFuncBase::m_imgDumpCnt = C_ULONG(0);

ImgFuncBase::ImgFuncBase()
{
	/*pass*/
}

/// Reset image dump count.
void ImgFuncBase::resetImgDumpCnt()
{
	m_imgDumpCnt = C_ULONG(0);
}

/// Dump intermediate image. (For DEBUG.)
void ImgFuncBase::dumpImg(const cv::Mat& image, const char* const caption, const char* const dstDir)
{
#ifdef NDEBUG
	(void)(image, caption, dstDir);
#else
	m_imgDumpCnt++;

	cv::String numStr = std::to_string(m_imgDumpCnt);
	if (numStr.length() < 2) {
		numStr = cv::String("0") + numStr;
	}

	// Display the image on the screen.
	cv::String cap = numStr + cv::String("_") + caption;
	cv::imshow(cap, image);

	// Save image to specified directory.
	if (dstDir != NULL) {
		// Make directory path string.
		cv::String dir(dstDir);
		if (strchr("\\/", CHAR_TO_INT(dir.back())) == NULL) {
			dir += cv::String("/");
		}

		// Replace space to '_' in cap.
		std::string cap2;
		for (auto it = cap.begin(); it != cap.end(); it++) {
			int c = CHAR_TO_INT(*it);
			if (isspace(c)) {
				c = '_';
			}
			cap2.push_back(C_UCHAR(c));
		}
		cap = cap2;

		// Decide filename.
		// If cap have no extension, add DEFAULT_IMG_EXT as extension.
		std::string dummy, filenameMajor, ext;
		parse_file_name(cap.c_str(), dummy, filenameMajor, ext);
		cv::String filename(cap);
		if (ext.empty()) {
			filename += cv::String(DEFAULT_IMG_EXT);
		}

		// Save image.
		cv::imwrite(dir + filename, image);
	}
#endif
}
