
// sm3MFCDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "sm3MFC.h"
#include "sm3MFCDlg.h"
#include "afxdialogex.h"
#include <string>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

constexpr auto BUF_SIZE = (1024*64);		//缓冲区大小
#define LeftRotate(X, n) ((X) << (n % 32) | (X) >> (32 - (n % 32)))	//循环左移
#define P0(X) ((X) ^ (((X) << 9) | ((X) >> 23)) ^ (((X) << 17) | ((X) >> 15)))
#define P1(X) ((X) ^ (((X) << 15) | ((X) >> 17)) ^ (((X) << 23) | ((X) >> 9)))
#define T(j) ((j) <= 15 ? 0x79cc4519 : 0x7a879d8a)
#define FF(j, X, Y, Z) ((j) <= 15 ? ((X)^(Y)^(Z)) : (((X) & (Y)) | ((X) & (Z)) | ((Y) & (Z))))
#define GG(j, X, Y, Z) ((j) <= 15 ? ((X)^(Y)^(Z)) : (((X) & (Y)) | ((~(X)) & (Z))))

//全局变量
CString VFinal;	//最终杂凑值
CString VFinalWeb; //从网页端获取的杂凑值
CString path; //文件路径
DWORD V[8] = { 0x7380166f, 0x4914b2b9, 0x172442d7, 0xda8a0600,
						  0xa96f30bc, 0x163138aa, 0xe38dee4d, 0xb0fb0e4e };

void SM3Full(char* message)
{
	int block = BUF_SIZE / 64;		//分组数

	//W和W'拓展
	DWORD* word;
	word = new DWORD[68 * block];
	for (int i = 0; i < block; i++)
	{
		for (int j = 0; j < 16; j++)
		{
			word[i * 68 + j] = (unsigned char)message[i * 64 + j * 4] << 24 | (unsigned char)message[i * 64 + j * 4 + 1] << 16 | (unsigned char)message[i * 64 + j * 4 + 2] << 8 | (unsigned char)message[i * 64 + j * 4 + 3];
		}
		for (int k = 16; k < 68; k++)
		{
			word[i * 68 + k] = P1(word[i * 68 + k - 16] ^ word[i * 68 + k - 9] ^ (LeftRotate(word[i * 68 + k - 3], 15))) ^ (LeftRotate(word[i * 68 + k - 13], 7)) ^ word[i * 68 + k - 6];
		}
	}

	DWORD* wordPrime;
	wordPrime = new DWORD[64 * block];
	for (int i = 0; i < block; i++)
	{
		for (int j = 0; j < 64; j++)
		{
			wordPrime[i * 64 + j] = word[i * 68 + j] ^ word[i * 68 + j + 4];
		}
	}

	//CF函数
	DWORD SS1, SS2, TT1, TT2;
	DWORD A, B, C, D, E, F, G, H;
	for (int i = 0; i < block; i++)
	{
		A = V[0];
		B = V[1];
		C = V[2];
		D = V[3];
		E = V[4];
		F = V[5];
		G = V[6];
		H = V[7];

		for (int j = 0; j < 64; j++)
		{
			SS1 = LeftRotate((LeftRotate(A, 12) + E + LeftRotate(T(j), j)), 7);
			SS2 = SS1 ^ (LeftRotate(A, 12));
			TT1 = FF(j, A, B, C) + D + SS2 + wordPrime[i * 64 + j];
			TT2 = GG(j, E, F, G) + H + SS1 + word[i * 68 + j];

			D = C;
			C = LeftRotate(B, 9);
			B = A;
			A = TT1;
			H = G;
			G = LeftRotate(F, 19);
			F = E;
			E = P0(TT2);
		}

		V[0] = V[0] ^ A;
		V[1] = V[1] ^ B;
		V[2] = V[2] ^ C;
		V[3] = V[3] ^ D;
		V[4] = V[4] ^ E;
		V[5] = V[5] ^ F;
		V[6] = V[6] ^ G;
		V[7] = V[7] ^ H;
	}

	delete[] word;
	delete[] wordPrime;
}

void SM3CompletelyFill(char* message)
{
	DWORD word[68];
	for (int i = 0; i < 16; i++)
	{
		word[i] = (unsigned char)message[i * 4] << 24 | (unsigned char)message[i * 4 + 1] << 16 | (unsigned char)message[i * 4 + 2] << 8 | (unsigned char)(unsigned char)message[i * 4 + 3];
	}
	for (int i = 16; i < 68; i++)
	{
		word[i] = P1(word[i - 16] ^ word[i - 9] ^ (LeftRotate(word[i - 3], 15))) ^ (LeftRotate(word[i - 13], 7)) ^ word[i - 6];
	}

	DWORD wordPrime[64];
	for (int i = 0; i < 64; i++)
	{
		wordPrime[i] = word[i] ^ word[i + 4];
	}

	//CF函数
	DWORD SS1, SS2, TT1, TT2;
	DWORD A, B, C, D, E, F, G, H;
	A = V[0];
	B = V[1];
	C = V[2];
	D = V[3];
	E = V[4];
	F = V[5];
	G = V[6];
	H = V[7];

	for (int j = 0; j < 64; j++)
	{
		SS1 = LeftRotate((LeftRotate(A, 12) + E + LeftRotate(T(j), j)), 7);
		SS2 = SS1 ^ (LeftRotate(A, 12));
		TT1 = FF(j, A, B, C) + D + SS2 + wordPrime[j];
		TT2 = GG(j, E, F, G) + H + SS1 + word[j];

		D = C;
		C = LeftRotate(B, 9);
		B = A;
		A = TT1;
		H = G;
		G = LeftRotate(F, 19);
		F = E;
		E = P0(TT2);
	}

	V[0] = V[0] ^ A;
	V[1] = V[1] ^ B;
	V[2] = V[2] ^ C;
	V[3] = V[3] ^ D;
	V[4] = V[4] ^ E;
	V[5] = V[5] ^ F;
	V[6] = V[6] ^ G;
	V[7] = V[7] ^ H;
}

void SM3Remain(char* message, DWORD64 fileLength, DWORD fileRemainLength)
{
	int block = 0;

	if (fileRemainLength % 64 >= 56)		//确定分组个数
	{
		block = fileRemainLength / 64 + 2;
	}
	else
	{
		block = fileRemainLength / 64 + 1;
	}

	//消息填充
	message[fileRemainLength] = { char(0x80) };
	for (int i = fileRemainLength + 1; i < 64 * (block - 1) + 56; i++)
	{
		message[i] = { 0x00 };
	}
	fileRemainLength = 64 * (block - 1) + 56;
	for (int i = 0; i < 8; i++)
	{
		message[fileRemainLength + i] = (fileLength * 8) >> (64 - 8 * (i + 1));
	}

	//W和W'拓展
	DWORD* word;
	word = new DWORD[68 * block];
	for (int i = 0; i < block; i++)
	{
		for (int j = 0; j < 16; j++)
		{

			word[i * 68 + j] = (unsigned char)message[i * 64 + j * 4] << 24 | (unsigned char)message[i * 64 + j * 4 + 1] << 16 | (unsigned char)message[i * 64 + j * 4 + 2] << 8 | (unsigned char)message[i * 64 + j * 4 + 3];

		}
		for (int k = 16; k < 68; k++)
		{
			word[i * 68 + k] = P1(word[i * 68 + k - 16] ^ word[i * 68 + k - 9] ^ (LeftRotate(word[i * 68 + k - 3], 15))) ^ (LeftRotate(word[i * 68 + k - 13], 7)) ^ word[i * 68 + k - 6];
		}
	}

	DWORD* wordPrime;
	wordPrime = new DWORD[64 * block];
	for (int i = 0; i < block; i++)
	{
		for (int j = 0; j < 64; j++)
		{
			wordPrime[i * 64 + j] = word[i * 68 + j] ^ word[i * 68 + j + 4];
		}
	}

	//CF函数
	DWORD SS1, SS2, TT1, TT2;
	DWORD A, B, C, D, E, F, G, H;
	for (int i = 0; i < block; i++)
	{
		A = V[0];
		B = V[1];
		C = V[2];
		D = V[3];
		E = V[4];
		F = V[5];
		G = V[6];
		H = V[7];

		for (int j = 0; j < 64; j++)
		{
			SS1 = LeftRotate((LeftRotate(A, 12) + E + LeftRotate(T(j), j)), 7);
			SS2 = SS1 ^ (LeftRotate(A, 12));
			TT1 = FF(j, A, B, C) + D + SS2 + wordPrime[i * 64 + j];
			TT2 = GG(j, E, F, G) + H + SS1 + word[i * 68 + j];

			D = C;
			C = LeftRotate(B, 9);
			B = A;
			A = TT1;
			H = G;
			G = LeftRotate(F, 19);
			F = E;
			E = P0(TT2);
		}

		V[0] = V[0] ^ A;
		V[1] = V[1] ^ B;
		V[2] = V[2] ^ C;
		V[3] = V[3] ^ D;
		V[4] = V[4] ^ E;
		V[5] = V[5] ^ F;
		V[6] = V[6] ^ G;
		V[7] = V[7] ^ H;
	}

	delete[] word;
	delete[] wordPrime;
}


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// Csm3MFCDlg 对话框



Csm3MFCDlg::Csm3MFCDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SM3MFC_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void Csm3MFCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(Csm3MFCDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &Csm3MFCDlg::OnBnClickedButton1)
	ON_EN_CHANGE(IDC_MFCEDITBROWSE1, &Csm3MFCDlg::OnEnChangeMfceditbrowse1)
	ON_BN_CLICKED(IDC_BUTTON2, &Csm3MFCDlg::OnBnClickedButton2)
	ON_EN_CHANGE(IDC_EDIT2, &Csm3MFCDlg::OnEnChangeEdit2)
	ON_EN_CHANGE(IDC_EDIT1, &Csm3MFCDlg::OnEnChangeEdit1)
	ON_BN_CLICKED(IDC_BUTTON3, &Csm3MFCDlg::OnBnClickedButton3)
END_MESSAGE_MAP()


// Csm3MFCDlg 消息处理程序

BOOL Csm3MFCDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void Csm3MFCDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void Csm3MFCDlg::OnPaint()
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
HCURSOR Csm3MFCDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void Csm3MFCDlg::OnEnChangeMfceditbrowse1()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	GetDlgItemText(IDC_MFCEDITBROWSE1, path);
}

void Csm3MFCDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码

	LARGE_INTEGER frequency, start, finish;	//计时
	QueryPerformanceFrequency(&frequency);	//获取时钟频率
	QueryPerformanceCounter(&start);

	char* buf;
	buf = new char[BUF_SIZE + 1];	//缓冲区64KB+1

	CFile file;
	CFileException e;
	if (!file.Open(path, CFile::modeRead | CFile::typeBinary, &e))
	{
		CString eError = _T("");
		eError.Format(_T("Can't open the file %s, error = %u\n"), e.m_strFileName, e.m_cause);
		AfxMessageBox(eError);
		return;
	}

	DWORD64 fileLength = file.GetLength();		//文件字节数
	DWORD64 countRead = fileLength / BUF_SIZE;	//需要读取次数
	for (int i = 0; i < countRead; i++)
	{
		memset(buf, 0, (BUF_SIZE + 1) * sizeof(char));
		file.Read((char*)buf, BUF_SIZE);
		SM3Full(buf);
	}
	if (fileLength % BUF_SIZE == 0)
	{
		char messageBlockCompletelyFill[64];
		messageBlockCompletelyFill[0] = { char(0x80) };
		for (int i = 1; i < 56; i++)
		{
			messageBlockCompletelyFill[i] = { 0x00 };
		}
		for (int i = 0; i < 8; i++)
		{
			messageBlockCompletelyFill[i + 56] = (fileLength * 8) >> (64 - 8 * (i + 1));
		}
		SM3CompletelyFill(messageBlockCompletelyFill);
	}
	else
	{
		memset(buf, 0, (BUF_SIZE + 1) * sizeof(char));
		DWORD fileRemainLength = fileLength % BUF_SIZE;
		file.Read(buf, fileRemainLength);
		SM3Remain(buf, fileLength, fileRemainLength);
	}

	file.Close();
	delete[] buf;


	CString str;
	for (int i = 0; i < 8; i++)
	{
		str.Format(_T("%x"), V[i]);
		VFinal += str;
	}

	SetDlgItemText(IDC_EDIT1, VFinal);

	QueryPerformanceCounter(&finish);
	CString time;
	time.Format(_T("%lfs"), (double)(finish.QuadPart - start.QuadPart) / frequency.QuadPart);
	SetDlgItemText(IDC_EDIT2, time);

	//恢复初始值
	V[0] = 0x7380166f;
	V[1] = 0x4914b2b9;
	V[2] = 0x172442d7;
	V[3] = 0xda8a0600;
	V[4] = 0xa96f30bc;
	V[5] = 0x163138aa;
	V[6] = 0xe38dee4d;
	V[7] = 0xb0fb0e4e;
	VFinal = _T("");		//将VFinal置空
}

void Csm3MFCDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	SetDlgItemText(IDC_EDIT1, _T(""));
	SetDlgItemText(IDC_EDIT2, _T(""));
	SetDlgItemText(IDC_EDIT3, _T(""));
	SetDlgItemText(IDC_MFCEDITBROWSE1, _T(""));
}


void Csm3MFCDlg::OnEnChangeEdit1()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}
void Csm3MFCDlg::OnEnChangeEdit2()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}


void Csm3MFCDlg::OnBnClickedButton3()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItemText(IDC_EDIT3, VFinalWeb);

	if (!VFinal.CompareNoCase(VFinalWeb))
	{
		AfxMessageBox(_T("杂凑值一致，文件未被修改"));
	}
	else
	{
		AfxMessageBox(_T("杂凑值不一致，文件已被修改"));
	}
}
