
// LabelMeWinDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "LabelMeWin.h"
#include "LabelMeWinDlg.h"
#include "afxdialogex.h"
#include "BrowseDir.h"
using namespace cv;


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CLabelMeWinDlg 对话框



CLabelMeWinDlg::CLabelMeWinDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_LABELMEWIN_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_FRAME);
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
END_MESSAGE_MAP()


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

	mBtnOpenDir.SetIcon(IDI_ICON_FILE);
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
	int lw = rect.Width() / 6;
	mListFiles.InsertColumn(0, _T(""), LVCFMT_LEFT, 0);
	mListFiles.InsertColumn(1, _T("序号"), LVCFMT_LEFT, lw);
	mListFiles.InsertColumn(2, _T("名称"), LVCFMT_LEFT, lw * 5);


	

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

#pragma region 界面按钮
void CLabelMeWinDlg::OnBnClickedBtnOpen()
{
	// TODO: 打开文件对话框
	//1.
	//打开文件
	CString filePath = _T("");
	BOOL isOpen = TRUE;     //是否打开(否则为保存) 
	CString defaultDir = _T("./");   //默认打开的文件路径 
	CString fileName = _T("");         //默认打开的文件名 
	CString filter = _T("图像 (*.bmp; *.jpg; *.png; *.tiff)|*.bmp;*.jpg;*.png;*.tiff||");   //文件过虑的类型 
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

	//读取文件到内存
	LoadImageAndShow();
}

void CLabelMeWinDlg::OnBnClickedBtnOpenDir()
{
	// TODO: 选择目录
	mRootDir = SelectFolder();
	if (mRootDir == _T(""))
	{
		MessageBox(_T("没有选择文件夹！"));
		return;
	}
	//查询目录下所有的图片
	std::string strRoot;
	CBrowseDir br;
	char* pr = cstring_to_char(mRootDir);
	br.SetInitDir(pr);
	strRoot = pr;
	delete[] pr;

	auto r1 = br.BeginBrowseFilenames("*.jpg");
	auto r2 = br.BeginBrowseFilenames("*.bmp");
	auto r3 = br.BeginBrowseFilenames("*.tiff");
	auto r4 = br.BeginBrowseFilenames("*.png");

	mvFiles.clear();
	mvFiles.insert(mvFiles.end(), r1.begin(), r1.end());
	mvFiles.insert(mvFiles.end(), r2.begin(), r2.end());
	mvFiles.insert(mvFiles.end(), r3.begin(), r3.end());
	mvFiles.insert(mvFiles.end(), r4.begin(), r4.end());

	//
	if (mvFiles.size() <= 0)
	{
		MessageBox(_T("选择的文件夹中没有图片文件！"));
		return;
	}
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

	//加载图片
	LoadImageAndShow();
}


void CLabelMeWinDlg::OnBnClickedBtnNextImage()
{
	// TODO: 下一张
}


void CLabelMeWinDlg::OnBnClickedBtnPrevImage()
{
	// TODO: 前一张

}

void CLabelMeWinDlg::OnBnClickedBtnSave()
{
	// TODO: 保存
}


void CLabelMeWinDlg::OnBnClickedBtnCreatePoly()
{
	// TODO: 创建多边形
}


void CLabelMeWinDlg::OnBnClickedBtnDeletePoly()
{
	// TODO: 删除多边形
}


void CLabelMeWinDlg::OnBnClickedBtnEditPoly()
{
	// TODO: 编辑多边形
}
#pragma endregion

#pragma region 图像显示
void CLabelMeWinDlg::LoadImageAndShow()
{
	auto* p = cstring_to_char(mCurrentFile);
	mSrc = imread(p);
	if (mSrc.empty())
	{
		MessageBox(_T("无法读取文件：\r\n") + mCurrentFile);
		return;
	}

	//显示
	MakeShowingImage(mSrc, mShow, IDC_PIC);
	ConvertMatToCImage(mShow, mCimg);
	DrawCImageCenter(mCimg, GetDlgItem(IDC_PIC), mRectShow);
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
	resize(src, dst, Size(w, h), 0.0, 0.0, INTER_CUBIC);
	if (dst.channels() == 1)
		cvtColor(dst, dst, COLOR_GRAY2BGR);
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