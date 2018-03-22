
// sscomDlg.h : 头文件
//

#pragma once
#include "afxwin.h"

#include "FileMgr.h"
#include "Protocol.h"
#include "SerialPort.h"


// CsscomDlg 对话框
class CsscomDlg : public CDialogEx
{
// 构造
public:
	CsscomDlg(CWnd* pParent = NULL);	// 标准构造函数

	// 对话框数据
	enum { IDD = IDD_SSCOM_DIALOG };

	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

protected:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton4();
	afx_msg LONG OnSendFinished(WPARAM bytes, LPARAM lp);
	afx_msg LONG OnRecvFinished(WPARAM bytes, LPARAM lp);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg LRESULT OnShowTask(WPARAM wParam,LPARAM lParam) ;

private:
	HICON m_hIconOn;     // 串口打开时的红灯图标句柄
	HICON m_hIconOff;    // 串口关闭时的指示图标句柄

	CSerialPort *m_Port;  // 串口句柄
	CFileMgr	*m_file;
	CProtocol	*m_ptl;

	BOOL InitCom();
	void ToTray();
	void DeleteTray();
};
