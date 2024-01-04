#include "stdafx.h"

#include "ImagingCanvas.h"
#include "ClickedPointList.h"
#include "ImagingContext.h"

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

	/// 画像を画面に収まるように表示する。
	/// 戻り値は縮小率。
	/// (表示画像上で拾ったマウスクリック座標に対し、戻り値を掛けた値がソース画像上の座標となる。)
	void show_image(
		const cv::String& winname,
		cv::Mat mat,
		const bool bAsSameMag,
		const cv::Point& centerPt,
		cv::Rect& srcArea,
		int& dispWidth
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

		// 表示
		if (bAsSameMag) {
			// 画像ウィンドウが画面に治まるサイズを算出
			const int sx = centerPt.x - workAreaWidth / 2;
			const int sy = centerPt.y - workAreaHeight / 2;
			const int ex = sx + workAreaWidth;
			const int ey = sy + workAreaHeight;
			srcArea = cv::Rect(sx, sy, ex - sx, ey - sy);
			srcArea = clip_rect_into_image(srcArea, mat.cols, mat.rows);

			// 切り抜き
			cv::Mat ROISrc = cv::Mat(mat, srcArea);
			dispWidth = ROISrc.cols;

			// 表示
			cv::imshow(winname, ROISrc);
		}
		else {
			// 画像ウィンドウが画面に治まる倍率を算出
			const double ratioHorz = (double)mat.cols / (double)workAreaWidth;
			const double ratioVert = (double)mat.rows / (double)workAreaHeight;
			const double ratio = std::max(1.0, std::max(ratioHorz, ratioVert));
			srcArea = cv::Rect(0, 0, mat.cols, mat.rows);

			// リサイズ
			cv::Mat resized;
			cv::resize(mat, resized, cv::Size(), 1.0 / ratio, 1.0 / ratio);
			dispWidth = resized.cols;

			// 表示
			cv::imshow(winname, resized);
		}
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

		cv::Point centerPt;
		pCtx->getCurPt(centerPt);

		cv::Rect srcArea;
		int dispWidth;
		switch (event) {
		case cv::EVENT_LBUTTONDOWN:
			cout << "Left mouse button is pressed. (x=" << x << ", y=" << y << ")" << endl;
			pCtx->selectOrAdd(x, y);
			pCtx->refreshCanvas();
			show_image(IMAGE_WND_NAME, pCtx->refCanvas(), g_bShowAsSameMag, centerPt, srcArea, dispWidth);
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

	// 表示
	g_bShowAsSameMag = false;		// 最初の表示は全体表示(centerPtが無いため)
	cv::Point centerPt;
	cv::Rect srcArea;
	int dispWidth;
	show_image(IMAGE_WND_NAME, *pSrcImage, g_bShowAsSameMag, centerPt, srcArea, dispWidth);

	// 画像操作準備
	ImagingContext ctx;
	ctx.setSrcImage(pSrcImage);
	ctx.setDispGeometry(srcArea, dispWidth);

	// マウスイベントコールバック登録
	g_mainThreadID = get_thread_id();	// チェック用
	cv::setMouseCallback(IMAGE_WND_NAME, mouse_callback, (void*)&ctx);

	int prevKeyIn = '\0';
	for (;;) {
		// 終了判定
		// "#<CR>" で終了
		const int c = cv::waitKey(0);
		if (c < 0) {
			// (画像ウィンドウが閉じられた)
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

		// キーボードコマンド処理
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
			/*pass*/
			break;
		}

		// 再表示
		if (!ctx.getCurPt(centerPt)) {
			g_bShowAsSameMag = false;
		};
		show_image(IMAGE_WND_NAME, ctx.refCanvas(), g_bShowAsSameMag, centerPt, srcArea, dispWidth);
		ctx.setDispGeometry(srcArea, dispWidth);
		ctx.refreshCanvas();
	}

	return 0;
}
