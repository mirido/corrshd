#include "stdafx.h"

#include "ImagingCanvas.h"
#include "ClickedPointList.h"
#include "ImagingContext.h"

#include "cv_keycode.h"
#include "geometryutil.h"
#include "osal.h"

#ifdef NDEBUG
#pragma comment(lib, "opencv_world460.lib")
#else
#pragma comment(lib, "opencv_world460d.lib")
#endif

// [CONF] コマンド名
#define PROG_NAME					"corrshd"

// [CONF] 画像ウィンドウ名
#define IMAGE_WND_NAME				"Input image"
#define OUTPUT_WND_NAME				"Result image"

// [CONF] デフォルトの画像出力幅
#define DEFAULT_OUTPUT_WIDTH		800

// [CONF] 画像表示のマージン
#define IMAGE_WND_MARGIN_HORZ		16
#define IMAGE_WND_MARGIN_VERT		32

namespace
{
	void show_usage()
	{
		cerr << "Usage: " << PROG_NAME << " <image-file> <target-relative-width> <target-relative-height> [ <output-width> ]" << endl;
	}

	/// ファイル名をディレクトリおよび拡張子の前後に分解する。
	void parse_file_name(const char* const fpath, std::string& dir, std::string& fnameMajor, std::string& ext)
	{
		const size_t len = strlen(fpath);

		// ファイル名の先頭位置取得
		size_t k1 = len;
		while (k1 > 0 && strchr("\\/", fpath[k1 - 1]) == NULL) {
			k1--;
		}

		// 拡張子の先頭位置取得
		size_t k2 = len;
		while (k2 > k1 && fpath[k2 - 1] != '.') {
			k2--;
		}
		if (k2 <= k1) {
			// (ファイル名部分に '.' 無し = 拡張子無し)
			k2 = len;
		}

		dir = fpath;
		dir = (k1 > 0) ? dir.substr(0, k1) : "";

		fnameMajor = &(fpath[k1]);
		fnameMajor = (k2 - k1 > 0) ? fnameMajor.substr(0, k2 - k1) : "";

		ext = &(fpath[k2]);

		if (fnameMajor.length() > 0 && fnameMajor.back() == '.') {
			fnameMajor.pop_back();
			ext = std::string(".") + ext;
		}
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

}	// namespace

int main(const int argc, char* argv[])
{
	osal_setup_locale();

	if (argc < 4) {
		show_usage();
		return 1;
	}
	const char* const imageFile = argv[1];
	const double tgRelWidth = atof(argv[2]);
	const double tgRelHeight = atof(argv[3]);
	const double outputWidth = (argc >= 5) ? atof(argv[4]) : tgRelWidth;
	cout << "Speciied argument: \"" << imageFile << "\" " << tgRelWidth << " " << tgRelHeight << " " << outputWidth << endl;

	// 出力ファイル名のメジャー名と拡張子決定
	std::string dir, fnameMajor, ext;
	parse_file_name(imageFile, dir, fnameMajor, ext);

	// 出力画像サイズ決定
	const double outputHeight = (outputWidth * tgRelHeight) / tgRelWidth;
	const cv::Size outputImgSz = cv::Size((int)std::round(outputWidth), (int)std::round(outputHeight));

	// 画像読み込み
	cv::Ptr<cv::Mat> pSrcImage(new cv::Mat());
	*pSrcImage = cv::imread(imageFile);
	if (pSrcImage->data == NULL) {
		cout << "ERROR: cv::imread() failed. (file name=\"" << imageFile << "\")" << endl;
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
			// シェーディング補正
			if (ctx.doShadingCorrection(outputImgSz, outputImg)) {
				cv::namedWindow(OUTPUT_WND_NAME, cv::WINDOW_AUTOSIZE);
				show_output_image(outputImg);

				// 結果画像保存
				std::string dstFileName = dir + fnameMajor + "_mod" + ext;
				if (!cv::imwrite(dstFileName, outputImg)) {
					cout << "ERROR: cv::imwrite() failed. (file name=\"" << dstFileName << "\")" << endl;
				}
			}
			else {
				cout << "Info: Shading correction failed." << endl;
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
