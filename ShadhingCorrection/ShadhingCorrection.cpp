#include "stdafx.h"

#include "ClickedPointList.h"
#include "ImagingCanvas.h"

#include "IImgFunc.h"
#include "ImagingContext.h"

#include "cv_keycode.h"
#include "geometryutil.h"
#include "osal.h"
#include "pathutil.h"
#include "PhysicalSize.h"

#ifdef USE_OPENCV_WORDL_DLL
#ifdef NDEBUG
#pragma comment(lib, "opencv_world480.lib")
#else
#pragma comment(lib, "opencv_world480d.lib")
#endif
#else
#pragma comment(lib, "ippicvmt.lib")
#ifdef NDEBUG
#pragma comment(lib, "IlmImf.lib")
#pragma comment(lib, "ippiw.lib")
#pragma comment(lib, "ittnotify.lib")
#pragma comment(lib, "libjpeg-turbo.lib")
#pragma comment(lib, "libopenjp2.lib")
#pragma comment(lib, "libpng.lib")
#pragma comment(lib, "libtiff.lib")
#pragma comment(lib, "libwebp.lib")
#pragma comment(lib, "opencv_core480.lib")
#pragma comment(lib, "opencv_highgui480.lib")
#pragma comment(lib, "opencv_imgcodecs480.lib")
#pragma comment(lib, "opencv_imgproc480.lib")
#pragma comment(lib, "zlib.lib")
#else
#pragma comment(lib, "IlmImfd.lib")
#pragma comment(lib, "ippiwd.lib")
#pragma comment(lib, "ittnotifyd.lib")
#pragma comment(lib, "libjpeg-turbod.lib")
#pragma comment(lib, "libopenjp2d.lib")
#pragma comment(lib, "libpngd.lib")
#pragma comment(lib, "libtiffd.lib")
#pragma comment(lib, "libwebpd.lib")
#pragma comment(lib, "opencv_core480d.lib")
#pragma comment(lib, "opencv_highgui480d.lib")
#pragma comment(lib, "opencv_imgcodecs480d.lib")
#pragma comment(lib, "opencv_imgproc480d.lib")
#pragma comment(lib, "zlibd.lib")
#endif
#endif	/*OPENCV_BY_DLL*/

// Program name (Automatically acquired from argv[0].)
std::string PROG_NAME;

// [CONF] Program summary
#define SUMMARY		"image capture and correct program suitable for hand-drawn line drawings"

// [CONF] Copyright
#define COPYRIGHT	"Copyright 2024 mirido"

// [CONF] OpenCV distributer URL
#define OPENCV_URL	"https://opencv.org/"

// [CONF] 画像ウィンドウ名
#define IMAGE_WND_NAME				"Input image"
#define OUTPUT_WND_NAME				"Result image"

// [CONF] 画像表示のマージン
#define IMAGE_WND_MARGIN_HORZ		16
#define IMAGE_WND_MARGIN_VERT		32

// Factor to convert 1 inch to mm
const double mm_per_inch = 25.4;

namespace
{
	/// parse string as cv::Size2d
	bool parse_as_Size2d(const char* const str, cv::Size2d& size)
	{
		size = cv::Size2d();

		std::istringstream is(str);
		PhysicalSize psz;
		is >> psz;
		if (!is) {
			return false;
		}
		size = cv::Size2d(psz.width(), psz.height());
		return true;
	}

	/// Get the approximate size of the ROI.
	cv::Size get_approximate_size(const std::vector<cv::Point>& roi_corners)
	{
		if (roi_corners.size() != 4) {
			throw std::logic_error("*** ERR ***");
		}

		cv::Point midpoints[4];
		midpoints[0] = (roi_corners[0] + roi_corners[1]) / 2;
		midpoints[1] = (roi_corners[1] + roi_corners[2]) / 2;
		midpoints[2] = (roi_corners[2] + roi_corners[3]) / 2;
		midpoints[3] = (roi_corners[3] + roi_corners[0]) / 2;

		const int width = (int)std::round(cv::norm(midpoints[1] - midpoints[3]));
		const int height = (int)std::round(cv::norm(midpoints[2] - midpoints[0]));

		return cv::Size(width, height);
	}

	/// 画面に収まるソース画像範囲と表示サイズを取得する。
	void get_src_area_and_disp_size(
		const cv::Size& srcImgSize,
		const bool bAsSameMag,
		const cv::Point& centerPt,
		cv::Rect& srcArea,
		cv::Size& dispSize
	)
	{
		// プライマリモニターの実作業領域サイズ取得
		int workAreaWidth, workAreaHeight;
		osal_get_primary_monitor_work_area_size(workAreaWidth, workAreaHeight);
		if (!(workAreaWidth > 0 && workAreaHeight > 0)) {
			throw std::logic_error("*** ERR ***");
		}

		// マージンを減算
		workAreaWidth -= 2 * IMAGE_WND_MARGIN_HORZ;
		workAreaHeight -= 2 * IMAGE_WND_MARGIN_VERT;

		// 表示範囲とサイズ決定
		if (bAsSameMag) {
			// (等倍表示モード)
			// 画像ウィンドウが画面に治まるサイズを算出
			const int sx = centerPt.x - workAreaWidth / 2;
			const int sy = centerPt.y - workAreaHeight / 2;
			const int ex = sx + workAreaWidth;
			const int ey = sy + workAreaHeight;
			srcArea = cv::Rect(sx, sy, ex - sx, ey - sy);
			srcArea = clip_rect_into_image(srcArea, srcImgSize.width, srcImgSize.height);

			// 表示サイズ決定
			dispSize = cv::Size(srcArea.width, srcArea.height);
		}
		else {
			// (全体表示モード)
			// 画像ウィンドウが画面に治まる倍率を算出
			const double ratioHorz = (double)srcImgSize.width / (double)workAreaWidth;
			const double ratioVert = (double)srcImgSize.height / (double)workAreaHeight;
			const double ratio = std::max(1.0, std::max(ratioHorz, ratioVert));
			srcArea = cv::Rect(0, 0, srcImgSize.width, srcImgSize.height);

			// 表示サイズ決定
			const double dispWidth = (double)srcArea.width / ratio;
			const double dispHeight = (double)srcArea.height / ratio;
			dispSize = cv::Size((int)dispWidth, (int)dispHeight);
		}
	}

	/// 画像表示を更新する。
	void refresh_input_image_disp(ImagingContext& ctx, bool& bShowAsSameMag)
	{
		// 最新のソース画像サイズ取得
		// 90°回転等の操作で変化し得るため、改めて取得し直す必要がある。
		cv::Ptr<cv::Mat> pSrcImage = ctx.refSrcImage();
		const cv::Size srcImgSize = cv::Size(pSrcImage->cols, pSrcImage->rows);

		// 再描画
		cv::Point centerPt;
		if (!ctx.getCurPoint(centerPt)) {
			bShowAsSameMag = false;
		};
		cv::Rect srcArea;
		cv::Size dispSize;
		get_src_area_and_disp_size(srcImgSize, bShowAsSameMag, centerPt, srcArea, dispSize);
		ctx.setupCanvas(srcArea, dispSize);
		ctx.refreshCanvas();

		// 再表示
		cv::imshow(IMAGE_WND_NAME, ctx.refCanvas());
	}

	/// 処理結果画像を表示する。
	void show_output_image(const cv::Mat& outputImg)
	{
		const cv::Size outputImgSz = cv::Size(outputImg.cols, outputImg.rows);

		cv::Rect srcArea2;
		cv::Size dispSize2;
		get_src_area_and_disp_size(outputImgSz, false, cv::Point(), srcArea2, dispSize2);

		cv::Mat outDispImg;
		cv::resize(outputImg, outDispImg, dispSize2);

		cv::imshow(OUTPUT_WND_NAME, outDispImg);
	}

}	// namespace

namespace
{
	void* g_mainThreadID;
	bool g_bShowAsSameMag = false;

	/// マウスイベントのコールバック関数
	void mouse_callback(int event, int x, int y, int flags, void* userdata)
	{
		(void)(flags);

		// main()と同じスレッドコンテキストで呼ばれるはず
		if (!(osal_get_thread_id() == g_mainThreadID)) {
			throw std::logic_error("*** ERR ***");
		}

		ImagingContext* const pCtx = static_cast<ImagingContext*>(userdata);

		switch (event) {
		case cv::EVENT_LBUTTONDOWN:
#ifndef NDEBUG
			cout << "Left mouse button is pressed. (x=" << x << ", y=" << y << ")" << endl;
#endif
			if ((flags & flags & cv::EVENT_FLAG_SHIFTKEY) == 0) {
				// (SHIFTキー押下無し)
				// 任意の箇所を頂点として選択する
				pCtx->addOrMovePoint(x, y);
				pCtx->refreshCanvas();
				cv::imshow(IMAGE_WND_NAME, pCtx->refCanvas());
			}
			else {
				// (SHIFTキー押下有り)
				// 近傍の頂点をcurrent pointと選択
				if (pCtx->selectExistingPointIF(x, y)) {
					pCtx->refreshCanvas();
					cv::imshow(IMAGE_WND_NAME, pCtx->refCanvas());
				}
			}
			break;
		default:
			/*pass*/
			break;
		}
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

	struct AppParam
	{
		std::string m_imageFile;		// Input image file path
		cv::Size2d m_ROISize;			// ROI physical size in mm
		double m_dpi;					// Resolution in dpi
		std::string m_outfile;			// Output image file path
		bool m_bCutoffOnly;				// Cut off only or not flag

		cv::Size m_outputImgSz;			// Output image size (calculated based on m_ROISize and m_dpi)

		AppParam() : m_dpi(0.0), m_bCutoffOnly(false) { }

		/// Parse command arguments.
		int parse(int argc, char* argv[]) {
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
			cv::Size2d size2d;
			if (!parse_as_Size2d(ROISizeStr.c_str(), size2d)) {
				cerr << "ERROR: Illegal ROI size. (roi-size=" << ROISizeStr << ")" << endl;
				return 1;
			}
			const int widthInPx = (int)std::round((m_dpi * size2d.width) / mm_per_inch);
			const int heightInPx = (int)std::round((m_dpi * size2d.height) / mm_per_inch);
			m_ROISize = size2d;
			m_outputImgSz = cv::Size(widthInPx, heightInPx);

			return 0;
		}
	};

}	// namespace

int main(const int argc, char* argv[])
{
	osal_setup_locale();

	// Get program name.
	PROG_NAME = get_prog_name(argv[0]);

	// Parse commandline arguments.
	AppParam param;
	const int parseResult = param.parse(argc, argv);
	if (parseResult < 0) {
		// (Usage printed.)
		return 0;
	}
	if (parseResult != 0) {
		// (Error occured.)
		return 1;
	}

	// Print specified arguments for check.
	cout
		<< "Speciied argument: \"" << param.m_imageFile << "\" "
		<< param.m_ROISize.width << "x" << param.m_ROISize.height << " "
		<< "-dpi " << param.m_dpi << " "
		<< "\"" << param.m_outfile << "\"";
	if (param.m_bCutoffOnly) {
		cout << "-cutoffonly";
	}
	cout << endl;
	cout << "Output image size=" << param.m_outputImgSz << " (in pixel)" << endl;

	// 画像読み込み
	cv::Ptr<cv::Mat> pSrcImage(new cv::Mat());
	*pSrcImage = cv::imread(static_cast<cv::String>(param.m_imageFile));
	if (pSrcImage->data == NULL) {
		cout << "ERROR: cv::imread() failed. (file name=\"" << param.m_imageFile << "\")" << endl;
		return 1;
	}

	// 画像操作準備
	ImagingContext ctx;
	ctx.setSrcImage(pSrcImage);

	// 表示
	g_bShowAsSameMag = false;		// 最初の表示は全体表示(centerPtが無いため)
	refresh_input_image_disp(ctx, g_bShowAsSameMag);

	// マウスイベントコールバック登録
	g_mainThreadID = osal_get_thread_id();	// チェック用
	cv::namedWindow(IMAGE_WND_NAME, cv::WINDOW_AUTOSIZE);
	cv::setMouseCallback(IMAGE_WND_NAME, mouse_callback, (void*)&ctx);

	int prevKeyIn = '\0';
	int keyWait = 0;
	const int KEY_WAIT_TIMEOUT = 20;
	for (;;) {
		// 終了判定
		// "#<CR>" で終了
		const int c = cv::waitKeyEx(keyWait);
#ifndef NDEBUG
		cout << "Key code=0x" << std::setbase(16) << std::setfill('0') << std::setw(8) << c << endl;
		cout << std::setbase(10);
#endif

		// カーソルキー押下の処理
		switch (c) {
		case (int)CVKEY::CURSOR_LEFT:
			ctx.moveCurPoint(-1, 0);
			keyWait = KEY_WAIT_TIMEOUT;
			break;
		case (int)CVKEY::CURSOR_UP:
			ctx.moveCurPoint(0, -1);
			keyWait = KEY_WAIT_TIMEOUT;
			break;
		case (int)CVKEY::CURSOR_RIGHT:
			ctx.moveCurPoint(1, 0);
			keyWait = KEY_WAIT_TIMEOUT;
			break;
		case (int)CVKEY::CURSOR_DOWN:
			ctx.moveCurPoint(0, 1);
			keyWait = KEY_WAIT_TIMEOUT;
			break;
		default:
			keyWait = 0;
			break;
		}
		if (keyWait > 0) {
			// カーソルキー押下中は画像表示を更新しないこととする。
			continue;
		}

		// 終了判定
		if (c < 0) {
			// (キー入力待ちがタイムアウトしたか、画像ウィンドウが閉じられた)
			double prop_val = -1;
			try {
				prop_val = cv::getWindowProperty(IMAGE_WND_NAME, cv::WND_PROP_ASPECT_RATIO);
			}
			catch (cv::Exception& e) {
				const char* const msg = e.what();
				cout << "Warning: " << msg << endl;
				prop_val = -1;
			}
			if (prop_val < 0) {
				// (入力画像ウィンドウが閉じられた)
				// 終了
				break;
			}
			/*
				(NOTE)
				ここでウィンドウ存在判定をcv::getWindowProperty()ではなく
				C言語インターフェースであるcvGetWindowHandle()で行おうとすると
				ハンドルされない例外が発生し、クラッシュする。
				(おそらくtry { } catch (...) { } で捕捉できない構造化例外を生じている。)
				C++インターフェースであるcv::imgshow()で表示した画像ウィンドウは、あくまで
				C++インターフェースの関数で取り扱わねばならない模様。
			*/
		}
		if (c == '\x1b') {
			// (ESC押下)
			// 終了
			break;
		}
		if (strchr("\r\n", c) == NULL) {
			prevKeyIn = c;
		}
		else {
			if (prevKeyIn == '#') {
				break;
			}
			else {
				prevKeyIn = '\0';
			}
		}

		// カーソルキー以外のキーボードコマンド処理
		cv::Mat outputImg;
		switch (c) {
		case '\t':
			// Current point切り替え
			ctx.changeCurrentPointToNext();
			break;
		case 'r':
			// 時計回りに90度回転
			ctx.rotate(1);
			break;
		case 'R':
			// 反時計回りに90度回転
			ctx.rotate(-1);
			break;
		case 'z':
			// 等倍表示 <--> 全体表示 トグル
			g_bShowAsSameMag = !g_bShowAsSameMag;
			break;
		case 's':
			// 頂点自動登録
			// デバッグ時のiteration効率を上げるため、頂点が全く未設定の場合は
			// 画像の4隅を自動的に設定する。
			if (ctx.isPointListEmpty()) {
				cv::Mat& canvas = ctx.refCanvas();		// Alias
				ctx.addOrMovePoint(0, 0);
				ctx.addOrMovePoint(canvas.cols - 1, 0);
				ctx.addOrMovePoint(canvas.cols - 1, canvas.rows - 1);
				ctx.addOrMovePoint(0, canvas.rows - 1);
			}
			// シェーディング補正 (or cutoff only)
			{
				// Check that 4 corners are specified.
				const std::vector<cv::Point> corners = ctx.getClockwiseList();
				if (corners.size() != 4) {
					cout << "ERROR: Correction cannot be started because the number of corners is insufficient." << endl;
					break;
				}

				// Print the size before and after conversion.
				cv::Size appxROISize = get_approximate_size(corners);
				if (!(appxROISize.width > 0 && appxROISize.height > 0)) {
					cout << "ERROR: Illegal corner selection. (width=" << appxROISize.width << ", height=" << appxROISize.height << ")" << endl;
					break;
				}
				cout << "Convert about " << appxROISize << " image to " << param.m_outputImgSz << " image. (in pixel)" << endl;
				cout << "horizontal ratio=" << (100.0 * (double)param.m_outputImgSz.width) / (double)appxROISize.width << "%" << endl;
				cout << "  vertical ratio=" << (100.0 * (double)param.m_outputImgSz.height) / (double)appxROISize.height << "%" << endl;

				// Do correction.
				bool bSuc;
				const char* caption;
				if (param.m_bCutoffOnly) {
					bSuc = ctx.correctDistortion(param.m_outputImgSz, outputImg);
					caption = "Distortion correction";
				}
				else {
					bSuc = ctx.doShadingCorrection(param.m_outputImgSz, outputImg);
					caption = " Shading correction";
				}
				if (bSuc) {
					cv::namedWindow(OUTPUT_WND_NAME, cv::WINDOW_AUTOSIZE);
						show_output_image(outputImg);

						// 結果画像保存
						if (!cv::imwrite(static_cast<cv::String>(param.m_outfile), outputImg)) {
							cout << "ERROR: cv::imwrite() failed. (file name=\"" << param.m_outfile << "\")" << endl;
						}
				}
				else {
					cout << "Info: " << caption << " failed." << endl;
				}
			}
			break;
		default:
#ifndef NDEBUG
			cout << "Unknown key code. (0x" << std::setbase(16) << std::setfill('0') << std::setw(8) << c << ")" << endl;
#endif
			break;
		}

		// 画像表示更新
		refresh_input_image_disp(ctx, g_bShowAsSameMag);
	}

	return 0;
}
