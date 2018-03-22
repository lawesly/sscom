
// sscomDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "sscom.h"
#include "sscomDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CsscomDlg 对话框
CsscomDlg::CsscomDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CsscomDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_hIconOn  = AfxGetApp()->LoadIcon(IDR_OPENON);
	m_hIconOff	= AfxGetApp()->LoadIcon(IDR_OPENOFF);

	m_Port = NULL;
	m_ptl = NULL;
	m_file = NULL;
}

void CsscomDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CsscomDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_COMM_TXEMPTY_DETECTED, OnSendFinished)
	ON_MESSAGE(WM_COMM_RXCHAR, OnRecvFinished)
	ON_BN_CLICKED(IDC_BUTTON1, &CsscomDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CsscomDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON4, &CsscomDlg::OnBnClickedButton4)
	ON_MESSAGE(WM_SHOWTASK,OnShowTask)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CsscomDlg 消息处理程序

BOOL CsscomDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
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

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	UINT N;
	CString  SerialList[256];  //最多也就256个串口
	CSerialPort::QueryCom(SerialList, &N);

	for(UINT i=0; i<N; i++){
		((CComboBox *) GetDlgItem(IDC_COMBO1))->AddString(SerialList[i]);
	}

	((CComboBox *) GetDlgItem(IDC_COMBO1))->SetCurSel(0);
	((CComboBox *) GetDlgItem(IDC_COMBO2))->SetCurSel(11);
	((CComboBox *) GetDlgItem(IDC_COMBO3))->SetCurSel(0);
	((CComboBox *) GetDlgItem(IDC_COMBO4))->SetCurSel(0);
	((CComboBox *) GetDlgItem(IDC_COMBO5))->SetCurSel(0);

	((CProgressCtrl*)GetDlgItem(IDC_PROGRESS1))->SetRange(0, 100);
	((CProgressCtrl*)GetDlgItem(IDC_PROGRESS1))->SetPos(0);

	GetDlgItem(IDC_BUTTON4)->EnableWindow(FALSE);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CsscomDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else if (nID == SC_MINIMIZE)
	{
		ToTray(); 
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CsscomDlg::OnPaint()
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
HCURSOR CsscomDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

#define MAX_RECV_DELAY  300		// ms
#define MAX_SEND_UNIT   SERIAL_OUT_SIZE
#define REAL_SEND_UNIT ((m_file->m_sendlen-m_file->m_sendcurrent) > MAX_SEND_UNIT ? MAX_SEND_UNIT:(m_file->m_sendlen-m_file->m_sendcurrent))

BOOL CsscomDlg::InitCom()
{
	UINT Ret;
	UINT ComN;
	CString strCom;
	CString strBaud;
	CString strParity;
	CString strDataBit;
	CString StrStopbits;
	GetDlgItem(IDC_COMBO1)->GetWindowTextA(strCom);
	GetDlgItem(IDC_COMBO2)->GetWindowTextA(strBaud);
	GetDlgItem(IDC_COMBO3)->GetWindowTextA(strParity);
	GetDlgItem(IDC_COMBO4)->GetWindowTextA(strDataBit);
	GetDlgItem(IDC_COMBO5)->GetWindowTextA(StrStopbits);

	Ret = sscanf(strCom, "COM%d", &ComN);
	if(Ret == 1)
		return m_Port->OpenPort(this, ComN, _ttoi(strBaud), strParity[0], _ttoi(strDataBit), _ttoi(StrStopbits), EV_RXCHAR);
	else
		return FALSE;
}

void CsscomDlg::OnBnClickedButton1()
{
	CStatic* OpenIcon = (CStatic*) GetDlgItem(IDC_OPENONOFF);

	if(OpenIcon->GetIcon() == m_hIconOn){
		m_Port->StopMonitoring();
		m_Port->ClosePort();
		delete m_Port; m_Port = NULL;
		delete m_file; m_file = NULL;
		delete m_ptl;  m_ptl  = NULL;
		OpenIcon->SetIcon(m_hIconOff);
		GetDlgItem(IDC_COMBO1)->EnableWindow(TRUE);
		GetDlgItem(IDC_COMBO2)->EnableWindow(TRUE);
		GetDlgItem(IDC_COMBO3)->EnableWindow(TRUE);
		GetDlgItem(IDC_COMBO4)->EnableWindow(TRUE);
		GetDlgItem(IDC_COMBO5)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON1)->SetWindowText("打开串口");
		GetDlgItem(IDC_BUTTON4)->EnableWindow(FALSE);
	}
	else{
		m_ptl = new CProtocol();
		m_file = new CFileMgr(FILE_BUFF_SEND, FILE_BUFF_RECV);
		m_Port = new CSerialPort();
		if(InitCom()){
			m_Port->StartMonitoring();
			m_Port->ReadFromPort(m_file->m_recvbuf);
			OpenIcon->SetIcon(m_hIconOn);
			GetDlgItem(IDC_COMBO1)->EnableWindow(FALSE);
			GetDlgItem(IDC_COMBO2)->EnableWindow(FALSE);
			GetDlgItem(IDC_COMBO3)->EnableWindow(FALSE);
			GetDlgItem(IDC_COMBO4)->EnableWindow(FALSE);
			GetDlgItem(IDC_COMBO5)->EnableWindow(FALSE);
			GetDlgItem(IDC_BUTTON1)->SetWindowText("关闭串口");
			GetDlgItem(IDC_BUTTON4)->EnableWindow(TRUE);
		}
		else
		{
			delete m_Port; m_Port = NULL;
			delete m_file; m_file = NULL;
			delete m_ptl;  m_ptl  = NULL;
		}
	}
}


void CsscomDlg::OnBnClickedButton2()
{
	BOOL isOpen = TRUE;     //是否打开 
	CString defaultDir = "C:\\Users\\Administrator\\Desktop";   //默认打开的文件路径  
	CString fileName = "";         //默认打开的文件名  
	CString filter = "所有文件 (*.*)|*.*||";   //文件过虑的类型  
	CFileDialog openFileDlg(isOpen, defaultDir, fileName, OFN_HIDEREADONLY|OFN_READONLY, filter, NULL);  
	INT_PTR result = openFileDlg.DoModal();  
	if(result == IDOK) {  
		CString FileName = openFileDlg.GetPathName();
		GetDlgItem(IDC_EDIT1)->SetWindowText(FileName);
	}
}

void CsscomDlg::OnBnClickedButton4()
{
	CString strPathName;
	GetDlgItem(IDC_EDIT1)->GetWindowText(strPathName);
	m_file->SetFile(strPathName);

	m_ptl->Create(m_file, PROTOCOL_HAND_SHAKE);
	m_Port->WriteToPort(m_file->m_sendbuf, REAL_SEND_UNIT);

	char status[1024];
	sprintf(status, "文件名：%s", m_file->m_filename);
	GetDlgItem(ID_STATICTEXT1)->SetWindowText(status);
	GetDlgItem(ID_STATICTEXT3)->SetWindowText("");
	GetDlgItem(ID_STATICTEXT4)->SetWindowText("");
	((CProgressCtrl*)GetDlgItem(IDC_PROGRESS1))->SetPos(0);
}

LONG CsscomDlg::OnSendFinished(WPARAM bytes, LPARAM lp)
{
	UINT32 SendBytes = (UINT32)bytes;
	m_file->m_sendcurrent += SendBytes;
	if(m_file->m_sendcurrent < m_file->m_sendlen)
		m_Port->WriteToPort(m_file->m_sendbuf + m_file->m_sendcurrent, REAL_SEND_UNIT);

	return 0;
}

LONG CsscomDlg::OnRecvFinished(WPARAM bytes, LPARAM lp)
{
	UINT32 RecvBytes = (UINT32)bytes;
	m_file->m_recvlen += RecvBytes;

	KillTimer(1);
	SetTimer(1, MAX_RECV_DELAY, nullptr);

	return 0;
}

void CsscomDlg::OnTimer(UINT_PTR nIDEvent)
{
	char status[1024];

	if(nIDEvent == 1){

		if(m_ptl != NULL && m_file != NULL && m_Port != NULL){

			CPROTOCOLCmd cmd = m_ptl->Parser(m_file);
			
			if(cmd == PROTOCOL_HAND_SHAKE){

				// 初始化 UI 文件状态
				sprintf(status, "文件名：%s", m_file->m_filename);
				GetDlgItem(ID_STATICTEXT1)->SetWindowText(status);
				GetDlgItem(ID_STATICTEXT3)->SetWindowText("准备接收... ");
				GetDlgItem(ID_STATICTEXT4)->SetWindowText("");
				((CProgressCtrl*)GetDlgItem(IDC_PROGRESS1))->SetPos(0);

				m_file->m_recvlen = 0;
				m_Port->ReadFromPort(m_file->m_recvbuf);
				m_ptl->Create(m_file, PROTOCOL_HAND_ACK);
				m_Port->WriteToPort(m_file->m_sendbuf, REAL_SEND_UNIT);

			}else if(cmd == PROTOCOL_HAND_ACK){

				GetDlgItem(ID_STATICTEXT3)->SetWindowText("开始发送... ");
				
				m_file->m_recvlen = 0;
				m_Port->ReadFromPort(m_file->m_recvbuf);
				m_ptl->Create(m_file, PROTOCOL_FILE_SEND);
				m_Port->WriteToPort(m_file->m_sendbuf, REAL_SEND_UNIT);

			}else if(cmd == PROTOCOL_FILE_SEND){

				sprintf(status, "已接收：%d/%d", m_file->m_currentsize, m_file->m_filesize);
				GetDlgItem(ID_STATICTEXT3)->SetWindowText(status);
				((CProgressCtrl*)GetDlgItem(IDC_PROGRESS1))->SetPos(m_file->m_currentpercent);
				char *FinishedTime = m_file->GetFinishedTime(m_Port->m_nbaud, PROTOCOL_SEND_LENGTH, MAX_RECV_DELAY);
				GetDlgItem(ID_STATICTEXT2)->SetWindowText(FinishedTime);

				m_file->m_recvlen = 0;
				m_Port->ReadFromPort(m_file->m_recvbuf);
				m_ptl->Create(m_file, PROTOCOL_SEND_ACK);
				m_Port->WriteToPort(m_file->m_sendbuf, REAL_SEND_UNIT);

			}else if(cmd == PROTOCOL_SEND_ACK){
				
				sprintf(status, "已发送：%d/%d", m_file->m_currentsize, m_file->m_filesize);
				GetDlgItem(ID_STATICTEXT3)->SetWindowText(status);
				((CProgressCtrl*)GetDlgItem(IDC_PROGRESS1))->SetPos(m_file->m_currentpercent);
				char *FinishedTime = m_file->GetFinishedTime(m_Port->m_nbaud, PROTOCOL_SEND_LENGTH, MAX_RECV_DELAY);
				GetDlgItem(ID_STATICTEXT2)->SetWindowText(FinishedTime);

				m_file->m_recvlen = 0;
				m_Port->ReadFromPort(m_file->m_recvbuf);
				m_ptl->Create(m_file, PROTOCOL_FILE_SEND);
				m_Port->WriteToPort(m_file->m_sendbuf, REAL_SEND_UNIT);

			}else if(cmd == PROTOCOL_FILE_FIN){

				sprintf(status, "接收完成：%d/%d", m_file->m_currentsize, m_file->m_filesize);
				GetDlgItem(ID_STATICTEXT3)->SetWindowText(status);
				((CProgressCtrl*)GetDlgItem(IDC_PROGRESS1))->SetPos(m_file->m_currentpercent);
				GetDlgItem(ID_STATICTEXT2)->SetWindowText("");

				m_file->m_recvlen = 0;
				m_Port->ReadFromPort(m_file->m_recvbuf);
				m_ptl->Create(m_file, PROTOCOL_FIN_ACK);
				m_Port->WriteToPort(m_file->m_sendbuf, REAL_SEND_UNIT);

				m_file->ResetFile();		// 清除文件状态

			}else if(cmd == PROTOCOL_FIN_ACK){

				sprintf(status, "发送完成：%d/%d", m_file->m_currentsize, m_file->m_filesize);
				GetDlgItem(ID_STATICTEXT3)->SetWindowText(status);
				((CProgressCtrl*)GetDlgItem(IDC_PROGRESS1))->SetPos(m_file->m_currentpercent);
				GetDlgItem(ID_STATICTEXT2)->SetWindowText("");

				m_file->m_recvlen = 0;
				m_Port->ReadFromPort(m_file->m_recvbuf);

				m_file->ResetFile();		// 清除文件状态

			}else if(cmd == PROTOCOL_FILE_RETRANS){

				m_file->m_retransmission++;		
				sprintf(status, "重传：%d", m_file->m_retransmission);
				GetDlgItem(ID_STATICTEXT4)->SetWindowText(status);

				m_file->m_recvlen = 0;
				m_Port->ReadFromPort(m_file->m_recvbuf);
				m_file->m_sendcurrent = 0;
				m_Port->WriteToPort(m_file->m_sendbuf, REAL_SEND_UNIT);

			}else if(cmd == PROTOCOL_RETRY || cmd == PROTOCOL_INVALID){

				m_file->m_retransmission++;		
				sprintf(status, "重传：%d", m_file->m_retransmission);
				GetDlgItem(ID_STATICTEXT4)->SetWindowText(status);

				m_file->m_recvlen = 0;
				m_Port->ReadFromPort(m_file->m_recvbuf);
				m_ptl->Create(m_file, PROTOCOL_FILE_RETRANS);
				m_Port->WriteToPort(m_file->m_sendbuf, REAL_SEND_UNIT);

			}else{
				assert(FALSE);
			}
		}

		KillTimer(1);
	}
}

void CsscomDlg::ToTray()
{
	NOTIFYICONDATA nid;
	nid.cbSize = (DWORD)sizeof(NOTIFYICONDATA);
	nid.hWnd = this->m_hWnd; 
	nid.uID = IDR_MAINFRAME;
	nid.uFlags = NIF_ICON|NIF_MESSAGE|NIF_TIP ;
	nid.uCallbackMessage = WM_SHOWTASK;//自定义的消息名称
	nid.hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));
	strcpy(nid.szTip, "sscom 2.0"); //信息提示条
	Shell_NotifyIcon(NIM_ADD, &nid); //在托盘区添加图标
	ShowWindow(SW_HIDE); //隐藏主窗口
}

void CsscomDlg::DeleteTray()
{
	NOTIFYICONDATA nid;
	nid.cbSize = (DWORD)sizeof(NOTIFYICONDATA);
	nid.hWnd = this->m_hWnd;
	nid.uID = IDR_MAINFRAME;
	nid.uFlags = NIF_ICON|NIF_MESSAGE|NIF_TIP ;
	nid.uCallbackMessage = WM_SHOWTASK;//自定义的消息名称
	nid.hIcon = LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDR_MAINFRAME));
	strcpy(nid.szTip, "sscom 2.0"); //信息提示条为“计划任务提醒”
	Shell_NotifyIcon(NIM_DELETE,&nid); //在托盘区删除图标
}

LRESULT CsscomDlg::OnShowTask(WPARAM wParam,LPARAM lParam)
{
	if(wParam != IDR_MAINFRAME)
		return 1;

	switch(lParam)
	{
	case WM_RBUTTONUP:
		{  
			CPoint pt;  
			GetCursorPos(&pt); 
			CMenu menu;
			menu.CreatePopupMenu();
			menu.AppendMenuA(MF_STRING,WM_DESTROY,"关闭");
			SetForegroundWindow(); 
			menu.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this);
			PostMessage(WM_NULL,0,0);
		}
		break;
	case WM_MOUSEMOVE:
		{
			if(m_file != NULL){

				char *FinishedTime = m_file->GetFinishedTime(m_Port->m_nbaud, PROTOCOL_SEND_LENGTH, MAX_RECV_DELAY);

				NOTIFYICONDATA nid;
				nid.cbSize = (DWORD)sizeof(NOTIFYICONDATA);
				nid.hWnd = this->m_hWnd; 
				nid.uID = IDR_MAINFRAME;
				nid.uFlags = NIF_ICON|NIF_MESSAGE|NIF_TIP ;
				nid.uCallbackMessage = WM_SHOWTASK;//自定义的消息名称
				nid.hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));
				strcpy(nid.szTip, FinishedTime); //信息提示条
				Shell_NotifyIcon(NIM_MODIFY, &nid); //在托盘区添加图标
			}
		}
		break;
	case WM_LBUTTONDBLCLK:
		{	
			//双击左键的处理 
			this->ShowWindow(SW_SHOW);//显示主窗口
			DeleteTray();
		}	
		break;

	default: 
		break;
	}

	return 0;
}

