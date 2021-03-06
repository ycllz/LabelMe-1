
// LabelMeWinDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "LabelMeWin.h"
#include "LabelMeWinDlg.h"
#include "afxdialogex.h"
#include "BrowseDir.h"
#include "DlgAddedLabel.h"
#include <fstream>
#include <rapidjson\document.h>
#include <regex>
#include <direct.h>
#include <io.h>
#define  IsKeyPressed(nVirtKey)     ((GetKeyState(nVirtKey) & (1<<(sizeof(SHORT)*8-1))) != 0)
using namespace cv;

cv::RNG rng(time(0));
//最多放大倍率
#define MAX_SCALE_RATIO 4
#define SCALE_STEP 0.5

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
#define INDICATOR_EDIT 0
#define INDICATOR_CREATE_POLY 1
#define INDICATOR_CREATE_RECT 2

//多边形模式至少要5个点
#define MIN_POLY_POINTS 5

//窗宽窗位设置
#define WINDOW_POS_MIN -100 
#define WINDOW_POS_MAX 500
#define WINDOW_POS_TIC 10

#define WINDOW_WIDTH_MIN 100
#define WINDOW_WIDTH_MAX 800
#define WINDOW_WIDTH_TIC 10

//#define WINDOW_INF 1000000
#define WINDOW_INF (double)0x3f3f3f3f


// 标签颜色
static std::vector<cv::Scalar> gvColors = {
	{ 25, 217, 217 },
	{ 50, 115, 184 },
	{ 107, 35, 142 },
	{ 204, 153, 50 },
	{ 153, 50, 204 },
	{ 142, 35, 35 },
	{ 0, 127, 255 },
	{ 47, 79, 79 }
};



// CLabelMeWinDlg 对话框

//for dcm
#pragma comment(lib, "DMHelper.lib")


CLabelMeWinDlg::CLabelMeWinDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_LABELMEWIN_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_FRAME);
	mCurrentPolyIdx = -1;
}

void CLabelMeWinDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_OPEN, mBtnOpen);
	DDX_Control(pDX, IDC_BTN_OPEN_DIR, mBtnOpenDir);
	DDX_Control(pDX, IDC_BTN_NEXT_IMAGE, mBtnNextImage);
	DDX_Control(pDX, IDC_BTN_PREV_IMAGE, mBtnPrevImage);
	DDX_Control(pDX, IDC_BTN_SAVE, mBtnSave);
	DDX_Control(pDX, IDC_BTN_CREATE_POLY, mBtnCreatePoly);
	DDX_Control(pDX, IDC_BTN_DELETE_POLY, mBtnDeletePoly);
	DDX_Control(pDX, IDC_BTN_EDIT_POLY, mBtnEditPoly);
	DDX_Control(pDX, IDC_BTN_LOAD_LABEL, mBtnLoadLabel);
	DDX_Control(pDX, IDC_LIST_FILES, mListFiles);
	DDX_Control(pDX, IDC_LIST_POLYS, mListROIs);
	DDX_Control(pDX, IDC_LIST_LABELS, mListLabels);
	DDX_Control(pDX, IDC_BTN_CREATE_RECT, mBtnCreateRect);
	DDX_Control(pDX, IDC_SLIDER_WINDOW, mSliderWindow);
}

BEGIN_MESSAGE_MAP(CLabelMeWinDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CLabelMeWinDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BTN_OPEN, &CLabelMeWinDlg::OnBnClickedBtnOpen)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BTN_OPEN_DIR, &CLabelMeWinDlg::OnBnClickedBtnOpenDir)
	ON_BN_CLICKED(IDC_BTN_NEXT_IMAGE, &CLabelMeWinDlg::OnBnClickedBtnNextImage)
	ON_BN_CLICKED(IDC_BTN_PREV_IMAGE, &CLabelMeWinDlg::OnBnClickedBtnPrevImage)
	ON_BN_CLICKED(IDC_BTN_SAVE, &CLabelMeWinDlg::OnBnClickedBtnSave)
	ON_BN_CLICKED(IDC_BTN_CREATE_POLY, &CLabelMeWinDlg::OnBnClickedBtnCreatePoly)
	ON_BN_CLICKED(IDC_BTN_DELETE_POLY, &CLabelMeWinDlg::OnBnClickedBtnDeletePoly)
	ON_BN_CLICKED(IDC_BTN_EDIT_POLY, &CLabelMeWinDlg::OnBnClickedBtnEditPoly)
	ON_NOTIFY(NM_CLICK, IDC_LIST_FILES, &CLabelMeWinDlg::OnNMClickListFiles)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDC_BTN_LOAD_LABEL, &CLabelMeWinDlg::OnBnClickedBtnLoadLabel)
	ON_BN_CLICKED(IDC_CHECK_AUTOSAVE, &CLabelMeWinDlg::OnBnClickedCheckAutosave)
	ON_NOTIFY(NM_CLICK, IDC_LIST_POLYS, &CLabelMeWinDlg::OnNMClickListPolys)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_POLYS, &CLabelMeWinDlg::OnNMDblclkListPolys)
	ON_BN_CLICKED(IDC_CHECK_ZOOM, &CLabelMeWinDlg::OnBnClickedCheckZoom)
	ON_BN_CLICKED(IDC_BTN_ZOOM_ORIGIN, &CLabelMeWinDlg::OnBnClickedBtnZoomOrigin)
	ON_BN_CLICKED(IDC_BTN_ZOOM_UP, &CLabelMeWinDlg::OnBnClickedBtnZoomUp)
	ON_BN_CLICKED(IDC_BTN_ZOOM_DOWN, &CLabelMeWinDlg::OnBnClickedBtnZoomDown)
	ON_BN_CLICKED(IDC_BTN_CREATE_RECT, &CLabelMeWinDlg::OnBnClickedBtnCreateRect)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_RADIO_WINDOW_POS, &CLabelMeWinDlg::OnBnClickedRadioWindowPos)
	ON_BN_CLICKED(IDC_RADIO_WINDOW_WIDTH, &CLabelMeWinDlg::OnBnClickedRadioWindowWidth)
	ON_BN_CLICKED(IDC_CHECK_SHOW, &CLabelMeWinDlg::OnBnClickedCheckShow)
	ON_BN_CLICKED(IDC_BTN_DELETE_FILE, &CLabelMeWinDlg::OnBnClickedBtnDeleteFile)
	ON_WM_RBUTTONDOWN()
	ON_BN_CLICKED(IDC_BTN_DELETE_FILE2, &CLabelMeWinDlg::OnBnClickedBtnDeleteFile2)
	ON_BN_CLICKED(IDC_BTN_DELETE_FILE3, &CLabelMeWinDlg::OnBnClickedBtnDeleteFile3)
	ON_WM_CLOSE()
END_MESSAGE_MAP()

double PolygonTest(std::vector<cv::Point2f>& c, Point2f pt, bool measureDist)
{


	double result = 0;
	int i, total = c.size(), counter = 0;
	//CV_Assert(total >= 0 && (depth == CV_32S || depth == CV_32F));

	bool is_float = false;
	double min_dist_num = FLT_MAX, min_dist_denom = 1;
	Point ip(cvRound(pt.x), cvRound(pt.y));

	if (total == 0)
		return measureDist ? -DBL_MAX : -1;

	const Point2f* cnt = &c[0];
	const Point2f* cntf = (const Point2f*)cnt;

	if (!is_float && !measureDist && ip.x == pt.x && ip.y == pt.y)
	{
		// the fastest "purely integer" branch
		Point v0, v = cnt[total - 1];

		for (i = 0; i < total; i++)
		{
			int dist;
			v0 = v;
			v = cnt[i];

			if ((v0.y <= ip.y && v.y <= ip.y) ||
				(v0.y > ip.y && v.y > ip.y) ||
				(v0.x < ip.x && v.x < ip.x))
			{
				if (ip.y == v.y && (ip.x == v.x || (ip.y == v0.y &&
					((v0.x <= ip.x && ip.x <= v.x) || (v.x <= ip.x && ip.x <= v0.x)))))
					return 0;
				continue;
			}

			dist = (ip.y - v0.y)*(v.x - v0.x) - (ip.x - v0.x)*(v.y - v0.y);
			if (dist == 0)
				return 0;
			if (v.y < v0.y)
				dist = -dist;
			counter += dist > 0;
		}

		result = counter % 2 == 0 ? -1 : 1;
	}
	else
	{
		Point2f v0, v;
		Point2f iv;

		if (is_float)
		{
			v = cntf[total - 1];
		}
		else
		{
			v = cnt[total - 1];
		}

		if (!measureDist)
		{
			for (i = 0; i < total; i++)
			{
				double dist;
				v0 = v;
				if (is_float)
					v = cntf[i];
				else
					v = cnt[i];

				if ((v0.y <= pt.y && v.y <= pt.y) ||
					(v0.y > pt.y && v.y > pt.y) ||
					(v0.x < pt.x && v.x < pt.x))
				{
					if (pt.y == v.y && (pt.x == v.x || (pt.y == v0.y &&
						((v0.x <= pt.x && pt.x <= v.x) || (v.x <= pt.x && pt.x <= v0.x)))))
						return 0;
					continue;
				}

				dist = (double)(pt.y - v0.y)*(v.x - v0.x) - (double)(pt.x - v0.x)*(v.y - v0.y);
				if (dist == 0)
					return 0;
				if (v.y < v0.y)
					dist = -dist;
				counter += dist > 0;
			}

			result = counter % 2 == 0 ? -1 : 1;
		}
		else
		{
			for (i = 0; i < total; i++)
			{
				double dx, dy, dx1, dy1, dx2, dy2, dist_num, dist_denom = 1;

				v0 = v;
				if (is_float)
					v = cntf[i];
				else
					v = cnt[i];

				dx = v.x - v0.x; dy = v.y - v0.y;
				dx1 = pt.x - v0.x; dy1 = pt.y - v0.y;
				dx2 = pt.x - v.x; dy2 = pt.y - v.y;

				if (dx1*dx + dy1*dy <= 0)
					dist_num = dx1*dx1 + dy1*dy1;
				else if (dx2*dx + dy2*dy >= 0)
					dist_num = dx2*dx2 + dy2*dy2;
				else
				{
					dist_num = (dy1*dx - dx1*dy);
					dist_num *= dist_num;
					dist_denom = dx*dx + dy*dy;
				}

				if (dist_num*min_dist_denom < min_dist_num*dist_denom)
				{
					min_dist_num = dist_num;
					min_dist_denom = dist_denom;
					if (min_dist_num == 0)
						break;
				}

				if ((v0.y <= pt.y && v.y <= pt.y) ||
					(v0.y > pt.y && v.y > pt.y) ||
					(v0.x < pt.x && v.x < pt.x))
					continue;

				dist_num = dy1*dx - dx1*dy;
				if (dy < 0)
					dist_num = -dist_num;
				counter += dist_num > 0;
			}

			result = std::sqrt(min_dist_num / min_dist_denom);
			if (counter % 2 == 0)
				result = -result;
		}
	}

	return result;
}


// CLabelMeWinDlg 消息处理程序

BOOL CLabelMeWinDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	mBtnOpen.SetIcon(IDI_ICON_FILE);
	mBtnOpen.SetAlign(CButtonST::ST_ALIGN_VERT);
	mBtnOpen.SetTooltipText(_T("打开一个图片"));

	mBtnOpenDir.SetIcon(IDI_ICON_OPEN_DIR);
	mBtnOpenDir.SetAlign(CButtonST::ST_ALIGN_VERT);
	mBtnOpenDir.SetTooltipText(_T("打开一个目录"));

	mBtnNextImage.SetIcon(IDI_ICON_NEXT);
	mBtnNextImage.SetAlign(CButtonST::ST_ALIGN_VERT);
	mBtnNextImage.SetTooltipText(_T("下一张图片"));

	mBtnPrevImage.SetIcon(IDI_ICON_PREV);
	mBtnPrevImage.SetAlign(CButtonST::ST_ALIGN_VERT);
	mBtnPrevImage.SetTooltipText(_T("上一张图片"));

	mBtnSave.SetIcon(IDI_ICON_SAVE);
	mBtnSave.SetAlign(CButtonST::ST_ALIGN_VERT);
	mBtnSave.SetTooltipText(_T("保存标注"));

	mBtnCreatePoly.SetIcon(IDI_ICON_OBJECTS);
	mBtnCreatePoly.SetAlign(CButtonST::ST_ALIGN_VERT);
	mBtnCreatePoly.SetTooltipText(_T("创建多边形"));

	mBtnCreateRect.SetIcon(IDI_ICON_RECT);
	mBtnCreateRect.SetAlign(CButtonST::ST_ALIGN_VERT);
	mBtnCreateRect.SetTooltipText(_T("创建矩形标注"));


	mBtnDeletePoly.SetIcon(IDI_ICON_CANCEL);
	mBtnDeletePoly.SetAlign(CButtonST::ST_ALIGN_VERT);
	mBtnDeletePoly.SetTooltipText(_T("删除多边形"));

	mBtnEditPoly.SetIcon(IDI_ICON_COLOR_LINE);
	mBtnEditPoly.SetAlign(CButtonST::ST_ALIGN_VERT);
	mBtnEditPoly.SetTooltipText(_T("编辑多边形"));

	mBtnLoadLabel.SetIcon(IDI_ICON_LABELS);
	mBtnLoadLabel.SetAlign(CButtonST::ST_ALIGN_VERT);
	mBtnLoadLabel.SetTooltipText(_T("加载标签文件"));

	//文件列表
	CRect rect;
	mListFiles.GetClientRect(rect);
	mListFiles.SetExtendedStyle(mListFiles.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	int lw = rect.Width() / 15;
	mListFiles.InsertColumn(0, _T(""), LVCFMT_LEFT, 0);
	mListFiles.InsertColumn(1, _T("序号"), LVCFMT_LEFT, lw * 3);
	mListFiles.InsertColumn(2, _T("名称"), LVCFMT_LEFT, lw * 11);

	//标签列表
	mListLabels.GetClientRect(rect);
	mListLabels.SetExtendedStyle(mListFiles.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	lw = rect.Width() / 15;
	mListLabels.InsertColumn(0, _T(""), LVCFMT_LEFT, 0);
	mListLabels.InsertColumn(1, _T("序号"), LVCFMT_LEFT, lw * 3);
	mListLabels.InsertColumn(2, _T("名称"), LVCFMT_LEFT, lw * 11);

	//当前标签列表
	mListROIs.GetClientRect(rect);
	mListROIs.SetExtendedStyle(mListFiles.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_CHECKBOXES);
	lw = rect.Width() / 15;
	mListROIs.InsertColumn(0, _T(""), LVCFMT_LEFT, 0);
	mListROIs.InsertColumn(1, _T("序号"), LVCFMT_LEFT, lw * 3);
	mListROIs.InsertColumn(2, _T("标签"), LVCFMT_LEFT, lw * 11);

	//自动保存
	mbAutoSave = true;
	((CButton *)GetDlgItem(IDC_CHECK_AUTOSAVE))->SetCheck(1);

	// 显示 标注
	((CButton*)GetDlgItem(IDC_CHECK_SHOW))->SetCheck(1);

	//默认是创建多边形模式
	mnStatusIndicator = INDICATOR_CREATE_POLY;
	GetDlgItem(IDC_BTN_CREATE_POLY)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_CREATE_RECT)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_DELETE_POLY)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_EDIT_POLY)->EnableWindow(TRUE);

	// 缩放使能
	((CButton*)GetDlgItem(IDC_CHECK_ZOOM))->SetCheck(0);
	GetDlgItem(IDC_BTN_ZOOM_UP)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_ZOOM_DOWN)->EnableWindow(FALSE);
	mfScalor = 1.0f;
	mbZoom = false;

	//状态栏
	mStatusBar.Create(WS_CHILD | WS_VISIBLE | SBT_OWNERDRAW, CRect(0, 0, 0, 0), this, 0);
	mStatusBar.GetClientRect(rect);
	
	int strPartDim[5] = { rect.Width() / 5,rect.Width() / 5 * 2,rect.Width() / 5 * 3 ,rect.Width() / 5 * 4, -1 };
	mStatusBar.SetParts(5, strPartDim);
	
	//设置状态栏文本
	mStatusBar.SetText(_T(""), 0, 0);
	mStatusBar.SetText(_T(""), 1, 0);
	mStatusBar.SetText(_T(""), 2, 0);
	mStatusBar.SetText(_T(""), 3, 0);
	mStatusBar.SetText(_T(""), 4, 0);

	mStatusBar.SetText(_T("创建多边模式"), 0, 0);

	//dcm
	mpDCMReader = std::shared_ptr<IDiReader>(GetDiReaderObj(), [](IDiReader* p) {
		if (p) DestroyDiReaderObj(&p); });

	((CButton *)GetDlgItem(IDC_RADIO_JPG))->SetCheck(TRUE);//选上

	SetVisableWindow(FALSE);

	LOGD("Init dialog...");
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CLabelMeWinDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CLabelMeWinDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



float CLabelMeWinDlg::GetPtDistI2(cv::Point2f& p1, cv::Point2f& p2)
{
	float dist = (p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y);
	dist = sqrt(dist);
	return dist;
}

void CLabelMeWinDlg::SaveLabels()
{
	//TODO: 保存标注
	std::string strRoot;
	char* p = cstring_to_char(mRootDir);
	strRoot = p;
	delete[] p;

	// 标签文件
	std::string pathLabel = strRoot + "\\" + "label.txt";
	std::ofstream ofs;
	ofs.open(pathLabel, std::ios::out);
	if (!ofs.is_open())
	{
		MessageBox(_T("请注意：标签文件并未保存，但这并不影响标注文件的保存"));
	}
	else
	{
		for (auto& i : msLabels)
		{
			ofs << i << std::endl;
		}
		ofs.close();
	}
	//标注文件
	std::vector<TImgAnno> va;

	for (int i = 0; i < mvPolys.size(); i++)
	{
		auto& k = mvPolys[i];
		TImgAnno ta;
		ta.label = k.first;
		ta.pos = mvWindows[i].first;
		ta.width = mvWindows[i].second;
		for (auto& j : k.second)
		{
			TPtAnno pt;
			pt.x = j.x;
			pt.y = j.y;
			ta.pts.push_back(pt);
		}
		va.push_back(ta);
	}
/*
	for (auto& i : mvPolys)
	{
		TImgAnno ta;
		ta.label = i.first;
		for (auto& j : i.second)
		{
			TPtAnno pt;
			pt.x = j.x;
			pt.y = j.y;
			ta.pts.push_back(pt);
		}
		va.push_back(ta);
	}*/
	TAnnoInfo outInfo;
	outInfo.va = std::move(va);
	outInfo.width = mSrc.cols;
	outInfo.height = mSrc.rows;
	//
	char drive[4096] = { 0 };
	char dir[4096] = { 0 };
	char filename[4096] = { 0 };
	p = cstring_to_char(mCurrentFile);
	_splitpath(p, drive, dir, filename, NULL);
	delete[] p;
	std::string pathJson = std::string(drive) + dir + filename + ".json";
	iguana::string_stream ss;
	iguana::json::to_json(ss, outInfo);
	
	ofs.open(pathJson, std::ios::out);
	if (!ofs.is_open())
	{
		MessageBox(_T("无法保存该图片的标注文件！"));
	}
	else
	{
		ofs << ss.str() << std::endl;
		ofs.close();
	}
}

cv::Point2f CLabelMeWinDlg::CanvasPt2SrcPt(cv::Point2f pt)
{
	//先缩放再平移
	if (mSrc.empty())
	{
		return{ -1,-1 };
	}

	Point2f rt;
	int dx = mptCurrentOrigin.x - mSrc.cols / 2;
	int dy = mptCurrentOrigin.y - mSrc.rows / 2;
	rt.x = float(mCurrentSrcSize.x) / float(mShow.cols);
	rt.y = float(mCurrentSrcSize.y) / float(mShow.rows);

	//缩放
	pt.x *= rt.x;
	pt.y *= rt.y;

	//平移到ROI起始点
	pt.x += mCurrentSrcRoi.x;
	pt.y += mCurrentSrcRoi.y;

	return pt;
}

cv::Point2f CLabelMeWinDlg::SourcePt2CanvasPt(cv::Point2f pt)
{
	//先平移再缩放
	Point2f rt;
	int dx = mptCurrentOrigin.x - mSrc.cols / 2;
	int dy = mptCurrentOrigin.y - mSrc.rows / 2;

	//平移到ROI坐标
	pt.x -= mCurrentSrcRoi.x;
	pt.y -= mCurrentSrcRoi.y;

	//缩放：
	rt.x = float(mCurrentSrcSize.x) / float(mShow.cols);
	rt.y = float(mCurrentSrcSize.y) / float(mShow.rows);

	pt.x /= rt.x;
	pt.y /= rt.y;
	return pt;
}

cv::Point2f CLabelMeWinDlg::GetCurrentScaler()
{
	Point2f rt;
	//if (mScaleRatio == 0)
	//{
		rt.x = float(mSrc.cols) / float(mShow.cols);
		rt.y = float(mSrc.rows) / float(mShow.rows);
	/*}
	else
	{
		rt.x = float(mroiScale.width) / float(mShow.cols);
		rt.y = float(mroiScale.height) / float(mShow.rows);
	}*/
	return rt;
}

void CLabelMeWinDlg::ShowPosAndWidth()
{
	CString str;
	str.Format(_T("窗位：%d"), (int)mDCMPos);
	GetDlgItem(IDC_RADIO_WINDOW_POS)->SetWindowText(str);
	str.Format(_T("窗宽：%d"), (int)mDCMWidth);
	GetDlgItem(IDC_RADIO_WINDOW_WIDTH)->SetWindowText(str);
}

void CLabelMeWinDlg::SetVisableWindow(BOOL flag)
{
	//GetDlgItem(IDC_RADIO_WINDOW_POS)->ShowWindow(flag);
	//GetDlgItem(IDC_RADIO_WINDOW_WIDTH)->ShowWindow(flag);
	//mSliderWindow.ShowWindow(flag);
	GetDlgItem(IDC_RADIO_WINDOW_POS)->EnableWindow(flag);
	GetDlgItem(IDC_RADIO_WINDOW_WIDTH)->EnableWindow(flag);
	mSliderWindow.EnableWindow(flag);
}

void CLabelMeWinDlg::SetWindowPosControl()
{
	mpDCMReader->GetWindow(&mDCMWidth, &mDCMPos);
	ShowPosAndWidth();
	SetVisableWindow(TRUE);
	//slider bar 
	//初值化为Pos 
	((CButton *)GetDlgItem(IDC_RADIO_WINDOW_POS))->SetCheck(TRUE);//选上
	mSliderWindow.SetRange(WINDOW_POS_MIN, WINDOW_POS_MAX);
	mSliderWindow.SetTicFreq(WINDOW_POS_TIC);
	mSliderWindow.SetPos(int(mDCMPos));
}

void CLabelMeWinDlg::OnBnClickedOk()
{
	// TODO: 这里屏蔽掉enter
	return;
	CDialogEx::OnOK();
}


HBRUSH CLabelMeWinDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  List 控件背景

	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}


CString CLabelMeWinDlg::SelectFolder()
{
	TCHAR           szFolderPath[MAX_PATH] = { 0 };
	CString         strFolderPath = _T("");

	BROWSEINFO      sInfo;
	::ZeroMemory(&sInfo, sizeof(BROWSEINFO));
	sInfo.pidlRoot = 0;
	sInfo.lpszTitle = _T("请选择一个文件夹：");
	sInfo.ulFlags = BIF_DONTGOBELOWDOMAIN | BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_EDITBOX;
	sInfo.lpfn = NULL;

	// 显示文件夹选择对话框  
	LPITEMIDLIST lpidlBrowse = ::SHBrowseForFolder(&sInfo);
	if (lpidlBrowse != NULL)
	{
		// 取得文件夹名  
		if (::SHGetPathFromIDList(lpidlBrowse, szFolderPath))
		{
			strFolderPath = szFolderPath;
		}
	}
	if (lpidlBrowse != NULL)
	{
		::CoTaskMemFree(lpidlBrowse);
	}

	return strFolderPath;
}

#pragma region 文件列表
void CLabelMeWinDlg::OnNMClickListFiles(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;
	// TODO: 查询点击了第几行，并加载相应的图片
	int idx = pNMItemActivate->iItem;
	if (mvFiles.size() <= idx)
		return;

	//保存
	bool bSave = false;
	if (mvPolys.size() != 0)
	{
		if (!mbAutoSave)
		{
			int ret = MessageBox(_T("是否保存当前图片的标注？"), _T("提示"), MB_OKCANCEL);
			if (ret == IDOK)
			{
				//保存
				bSave = true;
			}
		}
		else
		{
			//自动保存
			bSave = true;
		}
	}

	if (bSave)
	{
		// 执行保存
		SaveLabels();
	}
	//清空
	mvPolys.clear();
	mvWindows.clear();
	mvRoi.clear();
	RefreshROILists();


	mCurrentFile = mRootDir + _T("\\") + CString(mvFiles[idx].c_str());
	mCurrentIndex = idx;
	FindCurrentLabels();
	LoadImageAndShow();
	
	//

}

void CLabelMeWinDlg::FindCurrentLabels()
{
	mDCMPos = WINDOW_INF + 1;
	mDCMWidth = WINDOW_INF + 1;
	if (mCurrentFile.IsEmpty())
		return;
	char drive[4096] = { 0 };
	char dir[4096] = { 0 };
	char filename[4096] = { 0 };
	char* p = cstring_to_char(mCurrentFile);
	_splitpath(p, drive, dir, filename, NULL);
	delete[] p;

	std::string fn = std::string(drive) + dir + filename + ".json";

	//load json
	mvPolys.clear();
	mvWindows.clear();
	using namespace rapidjson;
	Document d;
	char* pd = ReadWholeFile(fn.c_str());
	if (pd == NULL)
		return;
	if (d.Parse(pd).HasParseError())
	{
		delete[] pd;
		return;
	}
	delete[] pd;
	//parse
	Value& v = d["va"];
	for (SizeType i = 0; i < v.Size(); i++)
	{
		std::string label;
		std::vector<cv::Point2f> pts;
		auto& json_ia = v[i];

		if (json_ia.HasMember("label"))
		{
			label = json_ia["label"].GetString();

			if (json_ia.HasMember("pts"))
			{
				auto& cell = json_ia["pts"].GetArray();
				for (SizeType j = 0; j < cell.Size(); j++)
				{
					int x = cell[j]["x"].GetInt();
					int y = cell[j]["y"].GetInt();
					pts.push_back(cv::Point2f(x, y));
				}

				if (mmapColor.find(label) == mmapColor.end())
				{
					int idx = mmapColor.size();
					mmapColor[label] = idx;
					gvColors.push_back(cv::Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255)));
					//mmapColor[label] = cv::Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
				}
				mvPolys.emplace_back(std::pair<std::string, std::vector<cv::Point2f>>(std::move(label), std::move(pts)));
			}
			if (json_ia.HasMember("pos") && json_ia.HasMember("width"))
			{
				auto win_pos = json_ia["pos"].GetInt();
				auto win_width = json_ia["width"].GetInt();
				mvWindows.emplace_back(std::pair<int, int>(win_pos, win_width));
				mDCMPos = win_pos;
				mDCMWidth = win_width;
			}
			else
			{
				mvWindows.emplace_back(std::pair<int, int>(mDCMPos, mDCMWidth));
			}
		}

		//label = json_ia["label"].GetString();

		//if (json_ia.HasMember("pts") && json_ia.HasMember("pos") && json_ia.HasMember("width"))
		//{
		//	auto& cell = json_ia["pts"].GetArray();
		//	for (SizeType j = 0; j < cell.Size(); j++)
		//	{
		//		int x = cell[j]["x"].GetInt();
		//		int y = cell[j]["y"].GetInt();
		//		pts.push_back(cv::Point2f(x,y));
		//	}
		//	
		//	if (mmapColor.find(label) == mmapColor.end())
		//	{
		//		int idx = mmapColor.size();
		//		mmapColor[label] = idx;
		//		gvColors.push_back(cv::Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255)));
		//		//mmapColor[label] = cv::Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		//	}
		//	mvPolys.emplace_back(std::pair<std::string, std::vector<cv::Point2f>>(std::move(label), std::move(pts)));
		//	auto win_pos = json_ia["pos"].GetInt();
		//	auto win_width = json_ia["width"].GetInt();
		//	mvWindows.emplace_back(std::pair<int, int>(win_pos, win_width));
		//	mDCMPos = win_pos;
		//	mDCMWidth = win_width;
		//	
		//}
	}

	// 标签
	for (auto& i : mvPolys)
	{
		msLabels.insert(i.first);
	}

	//refresh list
	RefreshLabelLists();
	RefreshROILists();

	//显示 
	//Mat tmp = mShow.clone();
	//DrawPolys(tmp);
	//ConvertMatToCImage(tmp, mCimg);
	//DrawCImageCenter(mCimg, GetDlgItem(IDC_PIC), mRectShow);
}

void CLabelMeWinDlg::RefreshFileLists()
{
	mListFiles.DeleteAllItems();
	for (int i = 0; i < mvFiles.size(); i++)
	{
		mListFiles.InsertItem(i, _T(""));
		mListFiles.SetItemText(i, 1, CString(std::to_string(i).c_str()));
		mListFiles.SetItemText(i, 2, CString(mvFiles[i].c_str()));
	}
}

void CLabelMeWinDlg::RefreshLabelLists()
{
	mListLabels.DeleteAllItems();
	int i = 0;
	for (auto& ele : msLabels)
	{
		mListLabels.InsertItem(i, _T(""));
		mListLabels.SetItemText(i, 1, CString(std::to_string(i).c_str()));
		mListLabels.SetItemText(i, 2, CString(ele.c_str()));
		i++;
	}
}

void CLabelMeWinDlg::RefreshROILists()
{
	mListROIs.DeleteAllItems();
	int i = 0;
	for (auto& ele : mvPolys)
	{
		mListROIs.InsertItem(i, _T(""));
		mListROIs.SetItemText(i, 1, CString(std::to_string(i).c_str()));
		mListROIs.SetItemText(i, 2, CString(ele.first.c_str()));
		i++;
	}
}

void CLabelMeWinDlg::ItemHighLight(int idx, CListCtrl& list)
{
	//List 
	list.SetFocus();
	auto a = list.GetItemCount();
	for (int i = 0; i < a; i++)
	{
		list.SetItemState(i, 0, LVIS_SELECTED | LVIS_FOCUSED);
	}
	if (idx >= 0)
	{
		list.SetFocus();
		list.SetItemState(idx, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		list.EnsureVisible(idx, FALSE);
	}
}
#pragma endregion

#pragma region 界面按钮
void CLabelMeWinDlg::OnBnClickedBtnOpen()
{
	// TODO: 打开文件对话框
	ENTER_FUNC;
	//1.
	//打开文件
	CString filePath = _T("");
	BOOL isOpen = TRUE;     //是否打开(否则为保存) 
	CString defaultDir = _T("./");   //默认打开的文件路径 
	CString fileName = _T("");         //默认打开的文件名 
	CString filter;
	if (((CButton *)GetDlgItem(IDC_RADIO_JPG))->GetCheck() == 1)
	{
		filter = _T("图像 (*.bmp; *.jpg; *.png; *.tiff)|*.bmp;*.jpg;*.png;*.tiff||");
	}
	else
	{
		filter = _T("医学图像(*.dcm)|*.dcm||");
	}
	//CString filter = _T("图像 (*.bmp; *.jpg; *.png; *.tiff)|*.bmp;*.jpg;*.png;*.tiff||");   //文件过虑的类型 
	//CString filter = _T("医学图像(*.dcm)|*.dcm||");
	CFileDialog openFileDlg(isOpen, defaultDir, fileName, OFN_HIDEREADONLY | OFN_READONLY, filter, NULL);
	if (openFileDlg.DoModal() == IDOK)
	{
		filePath = openFileDlg.GetPathName();
	}
	//
	if (filePath == _T(""))
	{
		MessageBox(_T("没有选择文件"));
		return;
	}

	mCurrentFile = filePath;
	mCurrentIndex = 0;

	//
	//查找是否有本文件的标注文件，读入并显示
	int ret = LoadImageAndShow();
	if (ret < 0)
	{
		return;
	}


	char dir[4096] = { 0 };
	char drive[4096] = { 0 };
	char filename[4096] = { 0 };
	char ext[4096] = { 0 };
	char* p = cstring_to_char(mCurrentFile);
	LOGD("file path is: {}", p);
	_splitpath(p, drive, dir, filename, ext);
	delete[] p;

	//
	mRootDir = CString(drive) + CString(dir);
	mvFiles.push_back(std::string(filename) + std::string(ext));
	mRootDir = mRootDir.Left(mRootDir.GetLength() - 1);

	//读取文件到内存
	
	RefreshFileLists();
	mvPolys.clear();
	mvWindows.clear();
	mvRoi.clear();
	RefreshROILists();
	ItemHighLight(0, mListFiles);
	mbLButtonDown = false;

	mbZoom = false;
	mfScalor = 1.0f;
	((CButton*)GetDlgItem(IDC_CHECK_ZOOM))->SetCheck(0);

	FindCurrentLabels();
	LoadImageAndShow();
	LEAVE_FUNC;
}

void CLabelMeWinDlg::OnBnClickedBtnOpenDir()
{
	// TODO: 选择目录
	ENTER_FUNC;
	auto&& root_dir = SelectFolder();
	if (root_dir == _T(""))
	{
		MessageBox(_T("没有选择文件夹！"));
		return;
	}
	mRootDir = root_dir;
	//查询目录下所有的图片
	std::string strRoot;
	CBrowseDir br;
	char* pr = cstring_to_char(mRootDir);
	LOGD("root folder is: {}", pr);
	br.SetInitDir(pr);
	strRoot = pr;
	delete[] pr;

	if (((CButton *)GetDlgItem(IDC_RADIO_JPG))->GetCheck() == 1)
	{
		auto r1 = br.BeginBrowseFilenames("*.jpg");
		auto r2 = br.BeginBrowseFilenames("*.bmp");
		auto r3 = br.BeginBrowseFilenames("*.tiff");
		auto r4 = br.BeginBrowseFilenames("*.png");

		LOGD("find jpg in root folder, num is: -> {}", r1.size());
		LOGD("find bmp in root folder, num is: -> {}", r2.size());
		LOGD("find tiff in root folder, num is: -> {}", r3.size());
		LOGD("find png in root folder, num is: -> {}", r4.size());

		mvFiles.clear();
		mvFiles.insert(mvFiles.end(), r1.begin(), r1.end());
		mvFiles.insert(mvFiles.end(), r2.begin(), r2.end());
		mvFiles.insert(mvFiles.end(), r3.begin(), r3.end());
		mvFiles.insert(mvFiles.end(), r4.begin(), r4.end());
	}
	else
	{
		auto r1 = br.BeginBrowseFilenames("*.dcm");
		mvFiles.clear();
		mvFiles.insert(mvFiles.end(), r1.begin(), r1.end());
	}
	

	//
	if (mvFiles.size() <= 0)
	{
		MessageBox(_T("选择的文件夹中没有图片文件！"));
		return;
	}
	//排序, value-index

	std::sort(mvFiles.begin(), mvFiles.end(), [](std::string& left, std::string& right) {
		int ret = 0;
		if (left.length() > right.length())
			return false;
		if (left.length() < right.length())
			return true;
		ret = left.compare(right);
		if (ret > 0)
			return false;
		else
			return true;
	});


	//std::vector<std::pair<int, int>> sortIndex;
	//std::regex re(R"((\d)+)");
	//std::cmatch match;
	//char filename[4096];
	//for (int i = 0; i < mvFiles.size(); i++)
	//{
	//	auto ele = std::pair<int, int>(-1, i);
	//	memset(filename, 0, sizeof(filename));
	//	_splitpath(mvFiles[i].c_str(), NULL, NULL, filename, NULL);

	//	if (std::regex_search(filename, match, re))
	//	{
	//		//ele.first = std::atoi(match[0].str());
	//		ele.first = std::stoi(match[0].str());
	//	}
	//	sortIndex.emplace_back(ele);
	//}
	//std::sort(sortIndex.begin(), sortIndex.end(), [](const std::pair<int, int>& left, const std::pair<int, int>& right) {
	//	return left.first < right.first;
	//});
	////
	//auto tmpFiles = std::move(mvFiles);
	//for (int i = 0; i < sortIndex.size(); i++)
	//{
	//	mvFiles.emplace_back(tmpFiles[sortIndex[i].second]);
	//}


	mCurrentFile = CString(mvFiles[0].c_str());
	mCurrentIndex = 0;
	//把mvFiles里面的路径，去掉根目录
	for (auto& i : mvFiles)
	{
		auto j = i.substr(strRoot.length() + 1);
		i = j;
	}

	//列表显示图片
	RefreshFileLists();
	mvPolys.clear();
	mvWindows.clear();
	mvRoi.clear();
	RefreshROILists();

	//加载图片
	FindCurrentLabels();

	mbZoom = false;
	mfScalor = 1.0f;
	((CButton*)GetDlgItem(IDC_CHECK_ZOOM))->SetCheck(0);
	LoadImageAndShow();
	
	//List 
	ItemHighLight(0, mListFiles);
	mbLButtonDown = false;

	LEAVE_FUNC;
}

void CLabelMeWinDlg::ClickedNextImpl(int selfHightlight)
{
	// TODO: 下一张
	if (mCurrentIndex == mvFiles.size() - 1 || mvFiles.size() <= 0)
	{
		MessageBox(_T("已经是最后一张了"));
		return;
	}
	//保存
	bool bSave = false;
	if (mvPolys.size() != 0)
	{
		if (!mbAutoSave)
		{
			int ret = MessageBox(_T("是否保存当前图片的标注？"), _T("提示"), MB_OKCANCEL);
			if (ret == IDOK)
			{
				//保存
				bSave = true;
			}
		}
		else
		{
			//自动保存
			bSave = true;
		}
	}

	if (bSave)
	{
		// 执行保存
		SaveLabels();
	}
	//清空
	mvPolys.clear();
	mvRoi.clear();
	mvWindows.clear();
	RefreshROILists();

	mCurrentIndex++;
	mCurrentFile = mRootDir + _T("\\") + CString(mvFiles[mCurrentIndex].c_str());
	FindCurrentLabels();

	mbZoom = false;
	mfScalor = 1.0f;
	((CButton*)GetDlgItem(IDC_CHECK_ZOOM))->SetCheck(0);
	LoadImageAndShow();

	//List 
	if(selfHightlight)
		ItemHighLight(mCurrentIndex, mListFiles);
}

void CLabelMeWinDlg::OnBnClickedBtnNextImage()
{
	return ClickedNextImpl(1);
}

void CLabelMeWinDlg::ClickedPrevImpl(int selfHightlight)
{
	if (mCurrentIndex == 0)
	{
		MessageBox(_T("已经是第一张了"));
		return;
	}
	//保存
	bool bSave = false;
	if (mvPolys.size() != 0)
	{
		if (!mbAutoSave)
		{
			int ret = MessageBox(_T("是否保存当前图片的标注？"), _T("提示"), MB_OKCANCEL);
			if (ret == IDOK)
			{
				//保存
				bSave = true;
			}
		}
		else
		{
			//自动保存
			bSave = true;
		}
	}

	if (bSave)
	{
		// 执行保存
		SaveLabels();
	}
	//清空
	mvPolys.clear();
	mvWindows.clear();
	mvRoi.clear();
	RefreshROILists();

	mCurrentIndex--;
	mCurrentFile = mRootDir + _T("\\") + CString(mvFiles[mCurrentIndex].c_str());
	FindCurrentLabels();

	mbZoom = false;
	mfScalor = 1.0f;
	((CButton*)GetDlgItem(IDC_CHECK_ZOOM))->SetCheck(0);
	LoadImageAndShow();

	//List 
	if(selfHightlight)
		ItemHighLight(mCurrentIndex, mListFiles);
}
void CLabelMeWinDlg::OnBnClickedBtnPrevImage()
{
	// TODO: 前一张
	ClickedPrevImpl(1);
}

void CLabelMeWinDlg::OnBnClickedBtnSave()
{
	// TODO: 保存
	if (mvPolys.size() > 0)
	{
		// 执行保存
		SaveLabels();
	}
}


void CLabelMeWinDlg::OnBnClickedBtnCreatePoly()
{
	// TODO: 创建多边形
	mnStatusIndicator = INDICATOR_CREATE_POLY;
	GetDlgItem(IDC_BTN_CREATE_POLY)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_CREATE_RECT)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_DELETE_POLY)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_EDIT_POLY)->EnableWindow(TRUE);
	mStatusBar.SetText(_T("创建多边形模式"), 0, 0);
}

void CLabelMeWinDlg::OnBnClickedBtnCreateRect()
{
	// TODO: 创建矩形
	mnStatusIndicator = INDICATOR_CREATE_RECT;
	GetDlgItem(IDC_BTN_CREATE_RECT)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_CREATE_POLY)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_DELETE_POLY)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_EDIT_POLY)->EnableWindow(TRUE);
	mStatusBar.SetText(_T("创建矩形模式"), 0, 0);
}


void CLabelMeWinDlg::OnBnClickedBtnDeletePoly()
{
	// TODO: 删除多边形
	if (mCurrentPolyIdx < 0)
		return;
	if (mCurrentPolyIdx >= mvPolys.size())
		return;

	//
	mvPolys.erase(mvPolys.begin() + mCurrentPolyIdx);
	mvWindows.erase(mvWindows.begin() + mCurrentPolyIdx);
	RefreshROILists();

	Mat tmp = mShow.clone();
	DrawCurrentPoly(tmp);
	DrawPolys(tmp);

	ConvertMatToCImage(tmp, mCimg);
	DrawCImageCenter(mCimg, GetDlgItem(IDC_PIC), mRectShow);
}


void CLabelMeWinDlg::OnBnClickedBtnEditPoly()
{
	// TODO: 编辑多边形
	mnStatusIndicator = INDICATOR_EDIT;
	mvRoi.clear();
	mbLButtonDown = false;
	Mat tmp = mShow.clone();
	DrawPolys(tmp);
	ConvertMatToCImage(tmp, mCimg);
	DrawCImageCenter(mCimg, GetDlgItem(IDC_PIC), mRectShow);
	GetDlgItem(IDC_BTN_CREATE_POLY)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_CREATE_RECT)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_DELETE_POLY)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_EDIT_POLY)->EnableWindow(FALSE);
	mStatusBar.SetText(_T("编辑模式"), 0, 0);
}
#pragma endregion

#pragma region 图像显示
int CLabelMeWinDlg::LoadImageAndShow()
{
	auto* p = cstring_to_char(mCurrentFile);

	char ext[4096] = { 0 };
	_splitpath(p, NULL, NULL, NULL, ext);

	std::string str_ext = ext;
	std::transform(str_ext.begin(), str_ext.end(), str_ext.begin(),
		[](unsigned char c) { return std::tolower(c); });

	if (str_ext == ".dcm")
	{
		//for dcm 
		int imgNum = 0;
		mpDCMReader->LoadDCMFile(p);
		if (mDCMPos < WINDOW_INF && mDCMWidth < WINDOW_INF)
		{
			mpDCMReader->SetWindow(mDCMWidth, mDCMPos);
		}
		TDiImage* imgs;
		mpDCMReader->Decode2Img(&imgs, &imgNum);
		delete[] p;

		if (imgNum <= 0)
		{
			MessageBox(_T("无法读取文件：\r\n") + mCurrentFile);
			return -1;
		}

		//
		int type = CV_8UC3;
		if (imgs[0].channels == 1)
			type = CV_8UC1;
		Mat m(imgs[0].height, imgs[0].width, type, imgs[0].data, imgs[0].step);
		mSrc = m.clone();
		ReleaseDiImage(imgs, imgNum);
		SetWindowPosControl();
		mbDCM = true;
	}
	else
	{
		mSrc = imread(p);
		delete[] p;
		SetVisableWindow(FALSE);
		mbDCM = false;
	}

	if (mSrc.empty())
	{
		MessageBox(_T("无法读取文件：\r\n") + mCurrentFile);
		return -1;
	}

	//显示
	MakeShowingImage(mSrc, mShow, IDC_PIC);
	auto tmp = mShow.clone();
	DrawPolys(tmp);
	ConvertMatToCImage(tmp, mCimg);
	DrawCImageCenter(mCimg, GetDlgItem(IDC_PIC), mRectShow);

	//
	mptCurrentOrigin.x = mSrc.cols / 2;
	mptCurrentOrigin.y = mSrc.rows / 2;

	//
	mbZoom = false;
	GetDlgItem(IDC_BTN_ZOOM_UP)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_ZOOM_DOWN)->EnableWindow(FALSE);

	//
	CString msg;
	msg.Format(_T("第 %d 张/共 %d 张"), mCurrentIndex + 1, mvFiles.size());
	mStatusBar.SetText(msg, 1, 0);

	return 0;
}

void CLabelMeWinDlg::ConvertMatToCImage(cv::Mat & src, CImage & cimg)
{
	void* data = src.data;
	int width = src.cols;
	int height = src.rows;
	int channels = src.channels();
	int step = src.step1();
	if (data == nullptr)
		return;
	if (cimg.IsNull())
		cimg.Create(width, height, 8 * channels);
	else if (cimg.GetWidth() != width ||
		cimg.GetHeight() != height ||
		cimg.GetBPP() / 8 != channels)
	{
		cimg.Destroy();
		cimg.Create(width, height, 8 * channels);
	}
	unsigned char* ps;
	unsigned char* pd = (unsigned char*)data;
	unsigned char* pimg = (unsigned char*)cimg.GetBits(); //获取CImage的像素存贮区的指针 
	int cimg_step = cimg.GetPitch();//每行的字节数,注意这个返回值有正有负

									// 如果是1个通道的图像(灰度图像) DIB格式才需要对调色板设置   
									// CImage中内置了调色板，我们要对他进行赋值： 
	if (1 == channels)
	{
		RGBQUAD* ColorTable;
		int MaxColors = 256;
		//这里可以通过CI.GetMaxColorTableEntries()得到大小(如果你是CI.Load读入图像的话)   
		ColorTable = new RGBQUAD[MaxColors];
		cimg.GetColorTable(0, MaxColors, ColorTable);//这里是取得指针   
		for (int i = 0; i< MaxColors; i++)
		{
			ColorTable[i].rgbBlue = (BYTE)i;
			//BYTE和uchar一回事，但MFC中都用它   
			ColorTable[i].rgbGreen = (BYTE)i;
			ColorTable[i].rgbRed = (BYTE)i;
		}
		cimg.SetColorTable(0, MaxColors, ColorTable);
		delete[]ColorTable;
	}
	for (int y = 0; y < height; y++)
	{
		ps = pd + y * step;
		for (int x = 0; x < width; x++)
		{
			if (1 == channels)
			{
				*(pimg + y * cimg_step + x) = ps[x];
			}
			else if (3 == channels)
			{
				*(pimg + y* cimg_step + x * 3 + 0) = ps[x * 3];
				*(pimg + y* cimg_step + x * 3 + 1) = ps[x * 3 + 1];
				*(pimg + y* cimg_step + x * 3 + 2) = ps[x * 3 + 2];
			}
		}
	}
}

void CLabelMeWinDlg::MakeShowingImage(cv::Mat & src, cv::Mat & dst, UINT id)
{
	// 缩放图片
	if (src.empty())
		return;
	CWnd* pwnd = GetDlgItem(id);
	CRect rect;
	pwnd->GetClientRect(rect);

	int h, w;
	float ratio_h = src.rows * 1.0 / rect.Height();
	float ratio_w = src.cols * 1.0 / rect.Width();
	if (ratio_h > ratio_w)
	{
		//按H的压缩比例计算
		h = rect.bottom;
		w = src.cols / ratio_h;
	}
	else
	{
		w = rect.right;
		h = src.rows / ratio_w;
	}
	mCurrentSrcSize.x = src.cols;
	mCurrentSrcSize.y = src.rows;
	mCurrentSrcRoi = { 0,0,src.cols, src.rows };
	resize(src, dst, Size(w, h), 0.0, 0.0, INTER_CUBIC);
	if (dst.channels() == 1)
		cvtColor(dst, dst, COLOR_GRAY2BGR);
	mStatusBar.SetText(_T("100%"), 4, 0);
	CString msg;
	msg.Format(_T("当前: (%d, %d, %d, %d)"), mCurrentSrcRoi.x, mCurrentSrcRoi.y, mCurrentSrcRoi.width, mCurrentSrcRoi.height);
	mStatusBar.SetText(msg, 3, 0);
	msg.Format(_T("图像: %d x %d"), src.cols, src.rows);
	mStatusBar.SetText(msg, 2, 0);
}

void CLabelMeWinDlg::DrawCImageCenter(ATL::CImage & image, CWnd * pwnd, CRect & dstRect, COLORREF bkColor)
{
	if (image.IsNull())
		return;
	CRect rect;
	pwnd->GetClientRect(&rect);
	int img_width = image.GetWidth();
	int img_height = image.GetHeight();
	//计算水平点
	int cx0 = (rect.right - img_width) / 2;
	int cy0 = (rect.bottom - img_height) / 2;
	CClientDC dc(pwnd);
	HDC hdc = dc.GetSafeHdc();
	SetStretchBltMode(hdc, HALFTONE);
	SetBrushOrgEx(hdc, 0, 0, NULL);
	dstRect = rect;
	if (cx0 >= 0 && cy0 >= 0)
	{
		dstRect.left = cx0;
		dstRect.top = cy0;
		dstRect.right = cx0 + img_width;
		dstRect.bottom = cy0 + img_height;
	}
	//填充背景色
	FillRect(hdc, &rect, CBrush(bkColor));
	image.Draw(hdc, dstRect, CRect(0, 0, image.GetWidth(), image.GetHeight()));
	return;
}

#pragma endregion


#pragma region 鼠标事件

void CLabelMeWinDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (IsKeyPressed(VK_CONTROL) && !IsKeyPressed(VK_MENU))
	{
		OnBnClickedBtnZoomDown();
	}
	CDialogEx::OnRButtonDown(nFlags, point);
}

void CLabelMeWinDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 左键按下
	CRect rect, rectRoi;
	if (mShow.empty() || mSrc.empty())
		goto RETURN;

	//判断是否在控件内
	GetDlgItem(IDC_PIC)->GetWindowRect(rect);
	ScreenToClient(rect);
	//判断是否在控件内
	rectRoi = mRectShow + POINT{ rect.left, rect.top };
	if (point.x < rectRoi.left || point.x > rectRoi.right || point.y < rectRoi.top || point.y > rectRoi.bottom)
	{
		goto RETURN;
	}
	// 判断control 键是否被按下
	if (IsKeyPressed(VK_CONTROL) && !IsKeyPressed(VK_MENU))
	{
		OnBnClickedBtnZoomUp();
		goto RETURN;
	}

	mbLButtonDown = true;
	mptButtonDown.x = point.x;
	mptButtonDown.y = point.y;
	//
	GetDlgItem(IDC_PIC)->GetWindowRect(rect);
	ClipCursor(rect);
RETURN:
	CDialogEx::OnLButtonDown(nFlags, point);
}


void CLabelMeWinDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 左键弹起
	ClipCursor(NULL);
	if(!mbLButtonDown)
		return CDialogEx::OnLButtonUp(nFlags, point);
	mbLButtonDown = false;
	
	//如果是在缩放模式下，什么也不做
	if (mbZoom || (IsKeyPressed(VK_CONTROL) && IsKeyPressed(VK_MENU)))
	{
		//记录下偏移量：
		int dx = point.x - mptButtonDown.x;
		int dy = point.y - mptButtonDown.y;

		//转换为原图的偏移量
		float scale_x = mCurrentSrcSize.x * 1.0f / mShow.cols;
		float scale_y = mCurrentSrcSize.y * 1.0f / mShow.rows;
		dx *= scale_x;
		dy *= scale_y;

		//
		mptCurrentOrigin.x -= dx;
		mptCurrentOrigin.y -= dy;

		if (MakeScaleImage(mSrc, mShow, IDC_PIC) < 0)
		{
			mptCurrentOrigin.x += dx;
			mptCurrentOrigin.y += dy;
			return CDialogEx::OnLButtonUp(nFlags, point);
		}

		cv::Mat tmp = mShow.clone();

		DrawCross(tmp);
		DrawPolys(tmp);
		DrawCurrentPoly(tmp);
		ConvertMatToCImage(tmp, mCimg);
		DrawCImageCenter(mCimg, GetDlgItem(IDC_PIC), mRectShow);
		return CDialogEx::OnLButtonUp(nFlags, point);
	}
		
	//记录下点
	CRect rect;
	GetDlgItem(IDC_PIC)->GetWindowRect(rect);
	ScreenToClient(rect);
	//判断是否在控件内
	CRect rectRoi = mRectShow + POINT{ rect.left, rect.top };
	if (point.x < rectRoi.left || point.x > rectRoi.right || point.y < rectRoi.top || point.y > rectRoi.bottom)
	{
		return CDialogEx::OnLButtonDown(nFlags, point);;
	}
	//
		
	mptStart.x = point.x - rectRoi.left;
	mptStart.y = point.y - rectRoi.top;

	mptStart = CanvasPt2SrcPt(mptStart);

	//如果画到图像边界外面了，就不要了
	if(mptStart.x < -1 || mptStart.y < -1)
		return CDialogEx::OnLButtonDown(nFlags, point);;


	if (mnStatusIndicator == INDICATOR_EDIT)
	{ //## 编辑模式
		//查看当前点，在哪个Poly里面
		std::vector<std::pair<int, float>> vidx;
		for (int i = 0; i < mvPolys.size(); i++)
		{
			auto& v = mvPolys[i].second;
			auto ret = PolygonTest(v, Point2f(mptStart.x, mptStart.y), false);
			if (ret >= 0)
			{
				float area = cv::contourArea(v, false);
				vidx.emplace_back(std::pair<int, float>(i, area));
			}
		}

		if(vidx.size() <= 0)
			return CDialogEx::OnLButtonUp(nFlags, point);
		//sort
		std::sort(vidx.begin(), vidx.end(), [](const std::pair<int, float>& left, const std::pair<int, float>& right)
		{
			return left.second < right.second;
		});
		//画
		int idx = vidx[0].first;
		ItemHighLight(idx, mListROIs);
		mCurrentPolyIdx = idx;
		DrawIdxRedPolys(idx);
		return CDialogEx::OnLButtonUp(nFlags, point);
	}
	
	//## 创建多边形模式
	if (mnStatusIndicator == INDICATOR_CREATE_POLY)
	{
		//保存
		mvRoi.push_back(mptStart);
		if (mvRoi.size() > MIN_POLY_POINTS)
		{
			//判断是否闭合
			auto pt1 = SourcePt2CanvasPt(mvRoi[0]);
			auto pt2 = SourcePt2CanvasPt(mptStart);
			auto dist = GetPtDistI2(pt1, pt2);
			//auto scaler = GetCurrentScaler();
			if (dist < MIN_NEIGBOR )
			{
				//说明是闭合了
				//1. 对话框，选择或输入标签
				std::string label;
				CDlgAddedLabel dlg(msLabels);
				if (dlg.DoModal() == IDOK)
				{
					auto txt = dlg.mEditLabel;
					char* p = cstring_to_char(txt);
					//
					msLabels.insert(std::string(p));
					label = p;
					delete[] p;
				}
				else
				{
					mvRoi.clear();
					MakeScaleImage(mSrc, mShow, IDC_PIC);
					Mat tmp = mShow.clone();
					DrawPolys(tmp);
					DrawCurrentPoly(tmp);
					ConvertMatToCImage(tmp, mCimg);
					DrawCImageCenter(mCimg, GetDlgItem(IDC_PIC), mRectShow);
					return CDialogEx::OnLButtonUp(nFlags, point);;
				}

				//2. 保存进行下一个
				if (mmapColor.find(label) == mmapColor.end())
				{
					int idx = mmapColor.size();
					mmapColor[label] = idx;
					gvColors.push_back(cv::Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255)));
					//mmapColor[label] = cv::Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
				}
				auto pa = std::pair<std::string, std::vector<cv::Point2f>>(std::move(label), std::move(mvRoi));
				auto pw = std::pair<int, int>(int(mDCMPos), int(mDCMWidth));
				mvPolys.emplace_back(std::move(pa));
				mvWindows.emplace_back(pw);
				//3. 刷新标签列表
				RefreshLabelLists();
				RefreshROILists();
			}
		}

		//画一个小圆
		MakeScaleImage(mSrc, mShow, IDC_PIC);
		Mat tmp = mShow.clone();
		DrawPolys(tmp);
		DrawCurrentPoly(tmp);
		ConvertMatToCImage(tmp, mCimg);
		DrawCImageCenter(mCimg, GetDlgItem(IDC_PIC), mRectShow);
	}

	if (mnStatusIndicator == INDICATOR_CREATE_RECT)
	{
		if (mptButtonDown.x == point.x && mptButtonDown.y == point.y)
		{
			return CDialogEx::OnLButtonUp(nFlags, point);
		}
		std::string label;
		CDlgAddedLabel dlg(msLabels);
		if (dlg.DoModal() == IDOK)
		{
			auto txt = dlg.mEditLabel;
			char* p = cstring_to_char(txt);
			msLabels.insert(std::string(p));
			label = p;
			delete[] p;

			//保存矩形有四个点
			cv::Point2f pt1;
			pt1.x = mptButtonDown.x - rectRoi.left;
			pt1.y = mptButtonDown.y - rectRoi.top;
			pt1 = CanvasPt2SrcPt(pt1);

			mvRoi.clear();
			mvRoi.push_back(pt1);
			mvRoi.push_back({ pt1.x, mptStart.y });
			mvRoi.push_back(mptStart);
			mvRoi.push_back({ mptStart.x, pt1.y });
			//mvRoi.push_back(pt1);

			//2. 保存进行下一个
			if (mmapColor.find(label) == mmapColor.end())
			{
				int idx = mmapColor.size();
				mmapColor[label] = idx;
				gvColors.push_back(cv::Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255)));
				//mmapColor[label] = cv::Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
			}
			auto pa = std::pair<std::string, std::vector<cv::Point2f>>(std::move(label), std::move(mvRoi));
			auto pw = std::pair<int, int>(int(mDCMPos), int(mDCMWidth));
			mvPolys.emplace_back(std::move(pa));
			mvWindows.emplace_back(pw);
			//3. 刷新标签列表
			RefreshLabelLists();
			RefreshROILists();
		}
		mvRoi.clear();
		MakeScaleImage(mSrc, mShow, IDC_PIC);
		Mat tmp = mShow.clone();
		DrawPolys(tmp);
		DrawCurrentPoly(tmp);
		ConvertMatToCImage(tmp, mCimg);
		DrawCImageCenter(mCimg, GetDlgItem(IDC_PIC), mRectShow);

	}
	

	CDialogEx::OnLButtonUp(nFlags, point);
}


void CLabelMeWinDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 鼠标移动
	if (mSrc.empty())
		return CDialogEx::OnMouseMove(nFlags, point);
	if(!mbLButtonDown)
		return CDialogEx::OnMouseMove(nFlags, point);

	
	//按下鼠标左键，拖动图片
	//记录下点
	CRect rect;
	GetDlgItem(IDC_PIC)->GetWindowRect(rect);
	ScreenToClient(rect);
	//判断是否在控件内
	CRect rectRoi = mRectShow + POINT{ rect.left, rect.top };
	if (point.x < rectRoi.left || point.x > rectRoi.right || point.y < rectRoi.top || point.y > rectRoi.bottom)
	{
		return CDialogEx::OnMouseMove(nFlags, point);
	}

	//1.先判断是否是缩放模式下
	if (mbZoom || (IsKeyPressed(VK_CONTROL) && IsKeyPressed(VK_MENU)))
	{
		static int drag_interval = 0;
		drag_interval++;
		if (drag_interval % 12 != 0)
			return CDialogEx::OnMouseMove(nFlags, point);
		drag_interval = 0;
		//记录下偏移量：
		int dx = point.x - mptButtonDown.x;
		int dy = point.y - mptButtonDown.y;
		mptButtonDown.x = point.x;
		mptButtonDown.y = point.y;

		//转换为原图的偏移量
		float scale_x = mCurrentSrcSize.x * 1.0f / mShow.cols;
		float scale_y = mCurrentSrcSize.y * 1.0f / mShow.rows;
		dx *= scale_x;
		dy *= scale_y;

		//
		mptCurrentOrigin.x -= dx;
		mptCurrentOrigin.y -= dy;

		if (MakeScaleImage(mSrc, mShow, IDC_PIC) < 0)
		{
			mptCurrentOrigin.x += dx;
			mptCurrentOrigin.y += dy;
			return CDialogEx::OnLButtonUp(nFlags, point);
		}

		cv::Mat tmp = mShow.clone();

		DrawCross(tmp);
		DrawPolys(tmp);
		DrawCurrentPoly(tmp);
		ConvertMatToCImage(tmp, mCimg);
		DrawCImageCenter(mCimg, GetDlgItem(IDC_PIC), mRectShow);
		return CDialogEx::OnMouseMove(nFlags, point);
	}

	// 2.1不是缩放模式, 如果是 编辑模式，直接退出
	if (mnStatusIndicator == INDICATOR_EDIT)
	{
		return CDialogEx::OnMouseMove(nFlags, point);
	}


	//2.2, 不是缩放模式,也不是编辑模式，进行正常标注
	mptStart.x = point.x - rectRoi.left;
	mptStart.y = point.y - rectRoi.top;

	mptStart = CanvasPt2SrcPt(mptStart);

	//如果画到图像边界外面了，就不要了
	if (mptStart.x < -1 || mptStart.y < -1)
		return CDialogEx::OnLButtonDown(nFlags, point);;

	//2.3 判断是不是多边形模式
	if (mnStatusIndicator == INDICATOR_CREATE_POLY)
	{
		mvRoi.push_back(mptStart);
		//画一个小圆
		static int draw_intervel = 0;
		if (draw_intervel == 8)
		{
			draw_intervel = 0;
			MakeScaleImage(mSrc, mShow, IDC_PIC);
			Mat tmp = mShow.clone();
			DrawPolys(tmp);
			DrawCurrentPoly(tmp);
			ConvertMatToCImage(tmp, mCimg);
			DrawCImageCenter(mCimg, GetDlgItem(IDC_PIC), mRectShow);
		}
		draw_intervel++;
	}

	//2.4 判断是不是 矩形框模型
	if (mnStatusIndicator == INDICATOR_CREATE_RECT)
	{
		//左键点下的点
		static int draw_rect_interval = 0;
		if (draw_rect_interval == 8)
		{
			cv::Point2f pt1;
			pt1.x = mptButtonDown.x - rectRoi.left;
			pt1.y = mptButtonDown.y - rectRoi.top;
			pt1 = CanvasPt2SrcPt(pt1);

			mvRoi.clear();
			mvRoi.push_back(pt1);
			mvRoi.push_back({ pt1.x, mptStart.y });
			mvRoi.push_back(mptStart);
			mvRoi.push_back({ mptStart.x, pt1.y });
			mvRoi.push_back(pt1);

			MakeScaleImage(mSrc, mShow, IDC_PIC);
			Mat tmp = mShow.clone();
			DrawPolys(tmp);
			DrawCurrentPoly(tmp);
			ConvertMatToCImage(tmp, mCimg);
			DrawCImageCenter(mCimg, GetDlgItem(IDC_PIC), mRectShow);

			draw_rect_interval = 0;

		}
		draw_rect_interval++;
	}
	
	
	return CDialogEx::OnMouseMove(nFlags, point);
}

void CLabelMeWinDlg::DrawPolys(cv::Mat& canvas)
{
	if (((CButton *)GetDlgItem(IDC_CHECK_SHOW))->GetCheck() == 0)
		return;
	Point pt1, pt2;
	for (auto& i : mvPolys)
	{
		auto& v = i.second;
		auto color = gvColors[mmapColor[i.first]];
		for (int k = 0; k < v.size(); k++)
		{
			pt1 = v[k];
			pt1 = SourcePt2CanvasPt(pt1);
			circle(canvas, pt1, POINT_CIRCLE_R, color, -1);
			if (k == v.size() - 1)
			{
				pt2 = SourcePt2CanvasPt(v[0]);
				line(canvas, pt1, pt2, color, POINT_LINE_R);
			}
			else
			{
				pt2 = SourcePt2CanvasPt(v[k + 1]);
				line(canvas, pt1, pt2, color, POINT_LINE_R);
			}
		}
	}
}
void CLabelMeWinDlg::DrawCurrentPoly(cv::Mat& canvas)
{
	if (((CButton *)GetDlgItem(IDC_CHECK_SHOW))->GetCheck() == 0)
		return;
	Point pt1, pt2;
	for (int i = 0; i < mvRoi.size(); i++)
	{
		pt1 = SourcePt2CanvasPt(mvRoi[i]);
		circle(canvas, pt1, POINT_CIRCLE_R, COLOR_GREEN, -1);
		if (i != mvRoi.size() - 1)
		{
			pt2 = SourcePt2CanvasPt(mvRoi[i + 1]);
			line(canvas, pt1, pt2 , COLOR_GREEN, POINT_LINE_R);
		}	
	}
}

#pragma endregion


void CLabelMeWinDlg::OnBnClickedBtnLoadLabel()
{
	// TODO: 加载标签文件
	CString filePath = _T("");
	BOOL isOpen = TRUE;     //是否打开(否则为保存) 
	CString defaultDir = _T("./");   //默认打开的文件路径 
	CString fileName = _T("");         //默认打开的文件名 
	CString filter = _T("普通文件 (*.txt)|*.txt||");   //文件过虑的类型 
	CFileDialog openFileDlg(isOpen, defaultDir, fileName, OFN_HIDEREADONLY | OFN_READONLY, filter, NULL);
	if (openFileDlg.DoModal() == IDOK)
	{
		filePath = openFileDlg.GetPathName();
	}
	//
	if (filePath == _T(""))
	{
		MessageBox(_T("没有选择文件"));
		return;
	}
	//按行读入
	std::ifstream ifs;
	char* p = cstring_to_char(filePath);
	ifs.open(p, std::ios::in);
	delete[] p;
	if (!ifs.is_open())
	{
		MessageBox(_T("打开标签文件失败！"));
		return;
	}
	std::string line;
	while (getline(ifs, line))
	{
		msLabels.insert(line);
	}
	ifs.close();

	//刷新标签列表
	RefreshLabelLists();
}

 
void CLabelMeWinDlg::OnBnClickedCheckAutosave()
{
	// TODO: 自动保存?
	if (((CButton *)GetDlgItem(IDC_CHECK_AUTOSAVE))->GetCheck() == 1)
		mbAutoSave = true;
	else
		mbAutoSave = false;
}

void CLabelMeWinDlg::OnNMDblclkListPolys(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;

	int index = pNMItemActivate->iItem;
	if (index > mvPolys.size() - 1 || index < 0)
		return;

	// 修改当前选定的 多边形的标签
	std::string newLabel;
	CDlgAddedLabel* dlg = new CDlgAddedLabel(msLabels);
	if (dlg->DoModal() == IDOK)
	{
		auto txt = dlg->mEditLabel;
		char* p = cstring_to_char(txt);
		//
		msLabels.insert(std::string(p));
		newLabel = p;
		delete[] p;

		//
		
		mCurrentPolyIdx = index;
		mvPolys[index].first = newLabel;
		//
		DrawIdxRedPolys(index);

		//标注列表标签列表都要刷新 
		RefreshLabelLists();
		RefreshROILists();
	}
	else
	{
		//MessageBox(_T("你没有选定新的标签，标签将不会修改！"));
	}
	delete dlg;

}

void CLabelMeWinDlg::OnNMClickListPolys(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 红色显示当前选中的行
	*pResult = 0;

	//
	int index = pNMItemActivate->iItem;
	if (index > mvPolys.size() - 1 || index < 0)
		return;
	mCurrentPolyIdx = index;
	//
	DrawIdxRedPolys(index);
}

void CLabelMeWinDlg::DrawIdxRedPolys(int idx)
{
	Mat tmp = mShow.clone();
	DrawCurrentPoly(tmp);
	DrawPolys(tmp);
	auto& v = mvPolys[idx].second;

	for (int k = 0; k < v.size(); k++)
	{
		auto & pt = v[k];
		circle(tmp, SourcePt2CanvasPt(pt), POINT_CIRCLE_R, COLOR_RED, -1);
		if (k == v.size() - 1)
		{
			line(tmp, SourcePt2CanvasPt(pt), SourcePt2CanvasPt(v[0]), COLOR_RED, POINT_LINE_R);
		}
		else
		{
			line(tmp, SourcePt2CanvasPt(pt), SourcePt2CanvasPt(v[k + 1]), COLOR_RED, POINT_LINE_R);
		}
	}

	//
	ConvertMatToCImage(tmp, mCimg);
	DrawCImageCenter(mCimg, GetDlgItem(IDC_PIC), mRectShow);
}

BOOL CLabelMeWinDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_ESCAPE)
		{
			//捕获ESC事件

			//
			if (!mbLButtonDown && !mSrc.empty())
			{
				//如果鼠标不是在按下状态，并且已经加载了图片
				//那么，取消最后一个点的编辑
				//画一个小圆
				if (mvRoi.size() > 0)
				{
					mvRoi.pop_back();
					Mat tmp = mShow.clone();
					DrawPolys(tmp);
					DrawCurrentPoly(tmp);
					ConvertMatToCImage(tmp, mCimg);
					DrawCImageCenter(mCimg, GetDlgItem(IDC_PIC), mRectShow);
				}
			}

			//取消默认关闭窗体事件
			return TRUE;
		}
		if (pMsg->wParam == VK_LEFT || pMsg->wParam == VK_RIGHT || pMsg->wParam == VK_UP || pMsg->wParam == VK_DOWN)
		{
			return TRUE;
		}
		//if (pMsg->hwnd == ::GetDlgItem(this->m_hWnd, IDC_LIST_FILES))
		//{
		//	if (pMsg->wParam == VK_DOWN)
		//	{
		//		//OnBnClickedBtnNextImage();
		//		ClickedNextImpl(0);
		//	}
		//	if (pMsg->wParam == VK_UP)
		//	{
		//		//OnBnClickedBtnPrevImage();
		//		ClickedPrevImpl(0);
		//	}
		//	if (pMsg->wParam == VK_LEFT)
		//	{
		//		if (((CButton *)GetDlgItem(IDC_CHECK_SHOW))->GetCheck() == 0)
		//			((CButton *)GetDlgItem(IDC_CHECK_SHOW))->SetCheck(1);
		//		else
		//			((CButton *)GetDlgItem(IDC_CHECK_SHOW))->SetCheck(0);
		//		OnBnClickedCheckShow();
		//	}
		//	//if (pMsg->wParam == VK_RIGHT)
		//	//{
		//	//	OnBnClickedBtnDeleteFile();
		//	//}
		//}

		if (pMsg->hwnd == ::GetDlgItem(this->m_hWnd, IDC_LIST_POLYS))
		{
			//按下字母d(D)
			if (pMsg->wParam == 0xe5 || pMsg->wParam == 0x44)
			{
				//删除当前标注
				OnBnClickedBtnDeletePoly();
			}
			//if (pMsg->wParam == VK_DOWN)
			//{
			//	if (mCurrentPolyIdx < mvPolys.size())
			//	{
			//		mCurrentPolyIdx++;
			//		DrawIdxRedPolys(mCurrentPolyIdx);
			//	}
			//}
			//if (pMsg->wParam == VK_UP)
			//{
			//	if (mCurrentPolyIdx > 0)
			//	{
			//		mCurrentPolyIdx--;
			//		DrawIdxRedPolys(mCurrentPolyIdx);
			//	}
			//}
		}

		//上一张,a,A 0x41
		if (pMsg->wParam == 0x41)
		{
			//ClickedPrevImpl(0);
			OnBnClickedBtnPrevImage();
		}
		// 下一张， Ff
		if (pMsg->wParam == 0x46)
		{
			OnBnClickedBtnNextImage();
		}
		if (pMsg->wParam == 0x53)
		{
			if (((CButton *)GetDlgItem(IDC_CHECK_SHOW))->GetCheck() == 0)
				((CButton *)GetDlgItem(IDC_CHECK_SHOW))->SetCheck(1);
			else
				((CButton *)GetDlgItem(IDC_CHECK_SHOW))->SetCheck(0);
			OnBnClickedCheckShow();
		}
		



	}

	return CDialogEx::PreTranslateMessage(pMsg);
}

#pragma region 缩放功能

/*缩放使能*/
void CLabelMeWinDlg::OnBnClickedCheckZoom()
{
	// TODO: 缩放使能
	if (mSrc.empty())
	{
		MessageBox(_T("没有读取图片！"));
		((CButton*)GetDlgItem(IDC_CHECK_ZOOM))->SetCheck(0);
		return;
	}
	if (((CButton*)GetDlgItem(IDC_CHECK_ZOOM))->GetCheck() == 1)
	{
		mbZoom = true;
		GetDlgItem(IDC_BTN_ZOOM_UP)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_ZOOM_DOWN)->EnableWindow(TRUE);
		if(mbDCM)
			SetVisableWindow(FALSE);

		//画十字线，允许鼠标拖动图片
		MakeScaleImage(mSrc, mShow, IDC_PIC);
		Mat tmp = mShow.clone();
		DrawCurrentPoly(tmp);
		DrawCross(tmp);
		DrawPolys(tmp);
		DrawCurrentPoly(tmp);
		ConvertMatToCImage(tmp, mCimg);
		DrawCImageCenter(mCimg, GetDlgItem(IDC_PIC), mRectShow);
	}
	else
	{
		mbZoom = false;
		GetDlgItem(IDC_BTN_ZOOM_UP)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_ZOOM_DOWN)->EnableWindow(FALSE);
		if (mbDCM)
			SetVisableWindow(TRUE);

		//取消画十字线
		MakeScaleImage(mSrc, mShow, IDC_PIC);
		Mat tmp = mShow.clone();
		DrawCurrentPoly(tmp);
		DrawPolys(tmp);
		DrawCurrentPoly(tmp);
		ConvertMatToCImage(tmp, mCimg);
		DrawCImageCenter(mCimg, GetDlgItem(IDC_PIC), mRectShow);
	}

}


void CLabelMeWinDlg::OnBnClickedBtnZoomOrigin()
{
	// TODO: 还原
	mfScalor = 1.0f;
	((CButton*)GetDlgItem(IDC_CHECK_ZOOM))->SetCheck(0);
	GetDlgItem(IDC_BTN_ZOOM_DOWN)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_ZOOM_UP)->EnableWindow(FALSE);
	mbZoom = false;
	mCurrentSrcSize.x = mSrc.cols;
	mCurrentSrcSize.y = mSrc.rows;
	mCurrentSrcRoi = { 0, 0, mSrc.cols, mSrc.rows };
	mptCurrentOrigin.x = mSrc.cols / 2;
	mptCurrentOrigin.y = mSrc.rows / 2;
	MakeShowingImage(mSrc, mShow, IDC_PIC);
	Mat tmp = mShow.clone();
	DrawPolys(tmp);
	DrawCurrentPoly(tmp);
	ConvertMatToCImage(tmp, mCimg);
	DrawCImageCenter(mCimg, GetDlgItem(IDC_PIC), mRectShow);
}


void CLabelMeWinDlg::OnBnClickedBtnZoomUp()
{
	// TODO: 放大
	mfScalor += SCALE_STEP;
	if (mfScalor > MAX_SCALE_RATIO + 0.00001)
	{
		mfScalor = MAX_SCALE_RATIO;
		return;
	}

	//
	MakeScaleImage(mSrc, mShow, IDC_PIC);
	Mat tmp = mShow.clone();
	DrawCross(tmp);
	DrawPolys(tmp);
	DrawCurrentPoly(tmp);
	ConvertMatToCImage(tmp, mCimg);
	DrawCImageCenter(mCimg, GetDlgItem(IDC_PIC), mRectShow);

}


void CLabelMeWinDlg::OnBnClickedBtnZoomDown()
{
	// TODO: 缩小
	mfScalor -= SCALE_STEP;
	if (mfScalor < 1.0f)
	{
		mfScalor = 1.0f;
		return;
	}
	if (std::abs(mfScalor - 1.0f) < 0.0001)
	{
		mfScalor = 1.0f;
		mCurrentSrcSize.x = mSrc.cols;
		mCurrentSrcSize.y = mSrc.rows;
		mCurrentSrcRoi = { 0, 0, mSrc.cols, mSrc.rows };
		mptCurrentOrigin.x = mSrc.cols / 2;
		mptCurrentOrigin.y = mSrc.rows / 2;
		MakeShowingImage(mSrc, mShow, IDC_PIC);
		Mat tmp = mShow.clone();
		DrawCross(tmp);
		DrawPolys(tmp);
		DrawCurrentPoly(tmp);
		ConvertMatToCImage(tmp, mCimg);
		DrawCImageCenter(mCimg, GetDlgItem(IDC_PIC), mRectShow);
		return;
	}
	//
	MakeScaleImage(mSrc, mShow, IDC_PIC);
	Mat tmp = mShow.clone();
	DrawCross(tmp);
	DrawPolys(tmp);
	DrawCurrentPoly(tmp);
	ConvertMatToCImage(tmp, mCimg);
	DrawCImageCenter(mCimg, GetDlgItem(IDC_PIC), mRectShow);
}

void CLabelMeWinDlg::DrawCross(cv::Mat src)
{
	auto color = cv::Scalar{ 255,255,255 };
	//九宫格
	int h3 = src.rows / 3;
	int w3 = src.cols / 3;
	line(src, { 0,h3 }, { src.cols - 1, h3 }, color, 1);
	line(src, { 0,h3 * 2 }, { src.cols - 1, h3 * 2 }, color, 1);
	line(src, { w3, 0 }, { w3, src.rows - 1 }, color, 1);
	line(src, { w3 * 2, 0 }, { w3 * 2, src.rows - 1 }, color, 1);

}

int CLabelMeWinDlg::MakeScaleImage(cv::Mat& src, cv::Mat& dst, UINT id)
{
	// 缩放图片
	if (src.empty())
		return -1;
	CWnd* pwnd = GetDlgItem(id);
	CRect rect;
	pwnd->GetClientRect(rect);


	//缩放对应的原图的位置
	int sh = src.rows * 1.0f / mfScalor;
	int sw = src.cols * 1.0f / mfScalor;

	float ratio_h = sh * 1.0 / rect.Height();
	float ratio_w = sw * 1.0 / rect.Width();

	int h, w;
	if (ratio_h > ratio_w)
	{
		//按H的压缩比例计算
		h = rect.bottom;
		w = sw / ratio_h;
	}
	else
	{
		w = rect.right;
		h = sh / ratio_w;
	}

	//w,h就是最终的要缩放的尺寸
	//先计算src中的roi
	int x0 = (src.cols - sw) / 2 ;
	int y0 = (src.rows - sh) / 2 ;
	int dx = src.cols / 2 - mptCurrentOrigin.x;
	int dy = src.rows / 2 - mptCurrentOrigin.y;
	x0 -= dx;
	y0 -= dy;
	cv::Mat mroi = cv::Mat::zeros({ sw, sh }, CV_8UC3);

	cv::Rect srcRoi{ x0,y0,sw, sh };
	cv::Rect dstRoi{ 0,0,sw, sh };

	mCurrentSrcSize.x = sw;
	mCurrentSrcSize.y = sh;
	mCurrentSrcRoi = srcRoi;

	if (srcRoi.x < 0)
	{
		dstRoi.x = -srcRoi.x;
		srcRoi.x = 0;
	}
	if (srcRoi.y < 0)
	{
		dstRoi.y = -srcRoi.y;
		srcRoi.y = 0;
	}
	if (srcRoi.x + srcRoi.width > mSrc.cols)
	{
		srcRoi.width = mSrc.cols - 1 - srcRoi.x;
		dstRoi.width = srcRoi.width;
	}
	if (srcRoi.y + srcRoi.height > mSrc.rows)
	{
		srcRoi.height = mSrc.rows - 1 - srcRoi.y;
		dstRoi.height = srcRoi.height;
	}

	if (dstRoi.x + dstRoi.width > sw)
	{
		dstRoi.width = sw - dstRoi.x - 1;
		srcRoi.width = dstRoi.width;
	}

	if (dstRoi.y + dstRoi.height > sh)
	{
		dstRoi.height = sh - dstRoi.y - 1;
		srcRoi.height = dstRoi.height;
	}


	if (srcRoi.width <= 0 || srcRoi.height <= 0 || srcRoi.x < 0 || srcRoi.y < 0 ||
		dstRoi.width <= 0 || dstRoi.height <= 0 || dstRoi.x < 0 || dstRoi.y < 0)
	{
		return -1;
	}

	src(srcRoi).copyTo(mroi(dstRoi));
	

	resize(mroi, dst, Size(w, h), 0.0, 0.0, INTER_CUBIC);
	if (dst.channels() == 1)
		cvtColor(dst, dst, COLOR_GRAY2BGR);

	CString msg;
	msg.Format(_T("%d"), int(mfScalor * 100));
	msg += _T("%");
	mStatusBar.SetText(msg, 4, 0);
	msg.Format(_T("当前: (%d, %d, %d, %d)"), mCurrentSrcRoi.x, mCurrentSrcRoi.y, mCurrentSrcRoi.width, mCurrentSrcRoi.height);
	mStatusBar.SetText(msg, 3, 0);

	return 0;
}

#pragma endregion



void CLabelMeWinDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (*pScrollBar == mSliderWindow)
	{
		//get pos 
		//判断当前模式
		int pos = mSliderWindow.GetPos();
		CString str;
		if (((CButton *)GetDlgItem(IDC_RADIO_WINDOW_POS))->GetCheck() == 1)
		{
			mDCMPos = pos;
			str.Format(_T("窗位：%d"), pos);
			GetDlgItem(IDC_RADIO_WINDOW_POS)->SetWindowText(str);
		}
		else
		{
			mDCMWidth = pos;
			str.Format(_T("窗宽：%d"), pos);
			GetDlgItem(IDC_RADIO_WINDOW_WIDTH)->SetWindowText(str);
		}
		//
		mpDCMReader->SetWindow(mDCMWidth, mDCMPos);
		//for dcm 
		int imgNum = 0;
		TDiImage* imgs;
		mpDCMReader->Decode2Img(&imgs, &imgNum);

		if (imgNum <= 0)
		{
			//MessageBox(_T("无法读取文件：\r\n") + mCurrentFile);
			return CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
		}

		//
		int type = CV_8UC3;
		if (imgs[0].channels == 1)
			type = CV_8UC1;
		Mat m(imgs[0].height, imgs[0].width, type, imgs[0].data, imgs[0].step);
		mSrc = m.clone();
		ReleaseDiImage(imgs, imgNum);
		MakeScaleImage(mSrc, mShow, IDC_PIC);
		Mat tmp = mShow.clone();
		DrawCurrentPoly(tmp);
		DrawPolys(tmp);
		DrawCurrentPoly(tmp);
		ConvertMatToCImage(tmp, mCimg);
		DrawCImageCenter(mCimg, GetDlgItem(IDC_PIC), mRectShow);

		int x = 0;
		x++;
	}

	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}


void CLabelMeWinDlg::OnBnClickedRadioWindowPos()
{
	// TODO: 点击了窗位按钮
	mSliderWindow.SetRange(WINDOW_POS_MIN, WINDOW_POS_MAX);
	mSliderWindow.SetTicFreq(WINDOW_POS_TIC);
	mSliderWindow.SetPos(int(mDCMPos));

	//
	mpDCMReader->SetWindow(mDCMWidth, mDCMPos);
	//for dcm 
	int imgNum = 0;
	TDiImage* imgs;
	mpDCMReader->Decode2Img(&imgs, &imgNum);

	if (imgNum <= 0)
	{
		MessageBox(_T("无法读取文件：\r\n") + mCurrentFile);
		return ;
	}

	//
	int type = CV_8UC3;
	if (imgs[0].channels == 1)
		type = CV_8UC1;
	Mat m(imgs[0].height, imgs[0].width, type, imgs[0].data, imgs[0].step);
	mSrc = m.clone();
	ReleaseDiImage(imgs, imgNum);
	MakeScaleImage(mSrc, mShow, IDC_PIC);
	Mat tmp = mShow.clone();
	DrawCurrentPoly(tmp);
	DrawPolys(tmp);
	DrawCurrentPoly(tmp);
	ConvertMatToCImage(tmp, mCimg);
	DrawCImageCenter(mCimg, GetDlgItem(IDC_PIC), mRectShow);
}


void CLabelMeWinDlg::OnBnClickedRadioWindowWidth()
{
	// TODO: 点击了窗宽按钮
	mSliderWindow.SetRange(WINDOW_WIDTH_MIN, WINDOW_WIDTH_MAX);
	mSliderWindow.SetTicFreq(WINDOW_WIDTH_TIC);
	mSliderWindow.SetPos(int(mDCMWidth));

	//
	mpDCMReader->SetWindow(mDCMWidth, mDCMPos);
	//for dcm 
	int imgNum = 0;
	TDiImage* imgs;
	mpDCMReader->Decode2Img(&imgs, &imgNum);

	if (imgNum <= 0)
	{
		MessageBox(_T("无法读取文件：\r\n") + mCurrentFile);
		return;
	}

	//
	int type = CV_8UC3;
	if (imgs[0].channels == 1)
		type = CV_8UC1;
	Mat m(imgs[0].height, imgs[0].width, type, imgs[0].data, imgs[0].step);
	mSrc = m.clone();
	ReleaseDiImage(imgs, imgNum);
	MakeScaleImage(mSrc, mShow, IDC_PIC);
	Mat tmp = mShow.clone();
	DrawCurrentPoly(tmp);
	DrawPolys(tmp);
	DrawCurrentPoly(tmp);
	ConvertMatToCImage(tmp, mCimg);
	DrawCImageCenter(mCimg, GetDlgItem(IDC_PIC), mRectShow);
}



void CLabelMeWinDlg::OnBnClickedCheckShow()
{
	// TODO: 
	MakeScaleImage(mSrc, mShow, IDC_PIC);
	Mat tmp = mShow.clone();
	DrawCurrentPoly(tmp);
	DrawPolys(tmp);
	ConvertMatToCImage(tmp, mCimg);
	DrawCImageCenter(mCimg, GetDlgItem(IDC_PIC), mRectShow);
}

inline std::string backslash2slash(std::string in)
{
	std::regex vr("(\\\\)");
	std::string out = std::regex_replace(in, vr, "/");
	return out;
}
/*查看文件或目录是否存在*/
inline bool isFileOrDirEists(const char* path)
{
#ifdef _MSC_VER
	if (0 != _access(path, 0))
#else
	if (0 != access(path, 0))
#endif
	{
		return false;
	}
	return true;
}

/*创建目录*/
inline void createDirectory(const char* path)
{
#ifdef _MSC_VER
	int ret = _mkdir(path);
#else
	mkdir(path, 0755);
#endif
}

// 递归创建目录
inline bool createDirectoryRecursive(const char* path)
{
	std::vector<std::string> dirs;
	dirs.push_back(std::string(path));
	std::string s = path;
	while (true)
	{
		int pos = s.rfind('/');
		if (pos < 0)
			break;
		std::string p = s.substr(0, pos);
		dirs.push_back(p);
		s = p;
	}

	for (int i = dirs.size() - 1; i >= 0; i--)
	{
		if (!isFileOrDirEists(dirs[i].c_str()))
			createDirectory(dirs[i].c_str());
	}
	return true;
}

/*获取当前路径*/
std::string getExePath()
{
	char szFilePath[MAX_PATH + 1] = { 0 };
	GetModuleFileNameA(NULL, szFilePath, MAX_PATH);
	(strrchr(szFilePath, '\\'))[0] = 0;
	std::string path = szFilePath;

	return path;
}

void CLabelMeWinDlg::DeleteFileImpl(std::string DELETE_PATH)
{
	if (mCurrentIndex < 0 || mCurrentIndex >= mvFiles.size())
	{
		MessageBox(_T("索引超出范围！"));
		return;
	}
	// 将对应的文件及json拷贝到当前目录中
	//1. 创建保存目录
	// 删除文件的保存目录
	//const std::string DELETE_PATH = "/deleted";
	std::string delete_dir = getExePath() + DELETE_PATH;
	delete_dir = backslash2slash(delete_dir);
	if (!isFileOrDirEists(delete_dir.c_str()))
	{
		createDirectory(delete_dir.c_str());
	}

	std::string file_path = mvFiles[mCurrentIndex];
	char* p = cstring_to_char(mRootDir);
	std::string root_path = std::string(p);
	delete[] p;

	root_path = backslash2slash(root_path);
	if (root_path[root_path.length() - 1] != '/')
		root_path += "/";

	file_path = backslash2slash(file_path);
	int pos = file_path.rfind('/');
	std::string dst_dir = delete_dir + "/";
	if (pos < 0)
	{
		// 没有父目录，直接移动就行
	}
	else
	{
		std::string parent_dir = dst_dir + file_path.substr(0, pos);
		if (!isFileOrDirEists(parent_dir.c_str()))
			createDirectoryRecursive(parent_dir.c_str());
	}

	// image 
	std::string src_image_path = root_path + file_path;
	std::string dst_image_path = dst_dir + file_path;
	MoveFile(CString(src_image_path.c_str()), CString(dst_image_path.c_str()));
	// json 
	pos = file_path.rfind('.');
	std::string json_path = file_path.substr(0, pos) + ".json";
	std::string src_json_path = root_path + json_path;
	if (isFileOrDirEists(src_json_path.c_str()))
	{
		std::string dst_json_path = dst_dir + json_path;
		MoveFile(CString(src_json_path.c_str()), CString(dst_json_path.c_str()));
	}

	mvFiles.erase(mvFiles.begin() + mCurrentIndex);
	if (mCurrentIndex >= mvFiles.size())
		mCurrentIndex = mvFiles.size() - 1;
	RefreshFileLists();

	//清空
	mvPolys.clear();
	mvRoi.clear();
	mvWindows.clear();
	RefreshROILists();

	mCurrentFile = mRootDir + _T("\\") + CString(mvFiles[mCurrentIndex].c_str());
	FindCurrentLabels();

	mbZoom = false;
	mfScalor = 1.0f;
	((CButton*)GetDlgItem(IDC_CHECK_ZOOM))->SetCheck(0);
	LoadImageAndShow();

	//List 
	ItemHighLight(mCurrentIndex, mListFiles);
}

void CLabelMeWinDlg::OnBnClickedBtnDeleteFile()
{
	// TODO: 删除当前文件及标注
	DeleteFileImpl("/OK");
}


void CLabelMeWinDlg::OnBnClickedBtnDeleteFile2()
{
	// TODO: 在此添加控件通知处理程序代码
	DeleteFileImpl("/VAGUE_A");
}


void CLabelMeWinDlg::OnBnClickedBtnDeleteFile3()
{
	// TODO: 在此添加控件通知处理程序代码
	DeleteFileImpl("/VAGUE_B");
}

void CLabelMeWinDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	int ret = MessageBox(_T("确定退出吗？"), _T("提示"), MB_OKCANCEL);
	if(ret == IDOK)
		CDialogEx::OnClose();
}
