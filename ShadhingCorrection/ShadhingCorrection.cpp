#include "stdafx.h"

#include <opencv2/highgui/highgui_c.h>	/* cvGetWindowHandle()導入目的 */

#include "ImagingCanvas.h"
#include "ClickedPointList.h"
#include "ImagingContext.h"

#include "cv_keycode.h"
#include "geometryutil.h"
#include "osal.h"

#ifdef NDEBUG
#pragma comment(lib, "opencv_world480.lib")
#else
#pragma comment(lib, "opencv_world480d.lib")
#endif

// [CONF] コマンド名
#define PROG_NAME					"corrshd"

// [CONF] 画像ウィンドウ名
#define IMAGE_WND_NAME				"Input image"

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
		get_primary_monitor_work_area_size(workAreaWidth, workAreaHeight);
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
	void refresh_disp(ImagingContext& ctx, bool& bShowAsSameMag)
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
		if (!(get_thread_id() == g_mainThreadID)) {
			throw std::logic_error("*** ERR ***");
		}

		ImagingContext* const pCtx = static_cast<ImagingContext*>(userdata);

		switch (event) {
		case cv::EVENT_LBUTTONDOWN:
			cout << "Left mouse button is pressed. (x=" << x << ", y=" << y << ")" << endl;
			pCtx->addOrMovePoint(x, y);
			pCtx->refreshCanvas();
			cv::imshow(IMAGE_WND_NAME, pCtx->refCanvas());
			break;
		default:
			/*pass*/
			break;
		}
	}

}	// namespace

int main(const int argc, char* argv[])
{
	if (argc < 4) {
		show_usage();
		return 1;
	}
	const char* const imageFile = argv[1];
	const double tgRelWidth = atof(argv[2]);
	const double tgRelHeight = atof(argv[3]);
	const int outputWidth = (argc >= 5) ? atoi(argv[4]) : 0;
	cout << "Speciied argument: \"" << imageFile << "\" " << tgRelWidth << " " << tgRelHeight << " " << outputWidth << endl;

	// 画像読み込み
	cv::Ptr<cv::Mat> pSrcImage(new cv::Mat());
	*pSrcImage = cv::imread(imageFile);

	// 画像操作準備
	ImagingContext ctx;
	ctx.setSrcImage(pSrcImage);

	// 表示
	g_bShowAsSameMag = false;		// 最初の表示は全体表示(centerPtが無いため)
	refresh_disp(ctx, g_bShowAsSameMag);

	// マウスイベントコールバック登録
	g_mainThreadID = get_thread_id();	// チェック用
	cv::setMouseCallback(IMAGE_WND_NAME, mouse_callback, (void*)&ctx);

	int prevKeyIn = '\0';
	int keyWait = 0;
	const int KEY_WAIT_TIMEOUT = 20;
	for (;;) {
		// 終了判定
		// "#<CR>" で終了
		const int c = cv::waitKeyEx(keyWait);
		cout << "Key code=0x" << std::setbase(16) << std::setfill('0') << std::setw(8) << c << endl;

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
			void* wndHandle = NULL;
			try {
				wndHandle = cvGetWindowHandle(IMAGE_WND_NAME);
			}
			catch (cv::Exception& e) {
				const char* const err_msg = e.what();
				cerr << err_msg << endl;
				wndHandle = NULL;
			}
			if (wndHandle == NULL) {
				// (画像ウィンドウが閉じられた)
				break;
			}
			/*
				(NOTE)
				画像ウィンドウが閉じられたか否かを判定するにあたり、
				C言語インターフェースであるcvGetWindowHandle()が将来廃止になった場合は
				cv::waitKeyEx()が-1を返したのが即座だったか否かをもって
				判定する方法を検討する。
			*/
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
		switch (c) {
		case 'r':
			// 時計回りに90度回転
			ctx.rotate(1);
			break;
		case 'R':
			// 反時計回りに90度回転
			ctx.rotate(-1);
			break;
		case 'z':
			g_bShowAsSameMag = !g_bShowAsSameMag;
			break;
		default:
			cout << "Unknown key code. (0x" << std::setbase(16) << std::setfill('0') << std::setw(8) << c << ")" << endl;
			break;
		}

		// 画像表示更新
		refresh_disp(ctx, g_bShowAsSameMag);
	}

	return 0;
}
