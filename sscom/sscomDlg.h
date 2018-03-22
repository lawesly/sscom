
// sscomDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"

#include "FileMgr.h"
#include "Protocol.h"
#include "SerialPort.h"


// CsscomDlg �Ի���
class CsscomDlg : public CDialogEx
{
// ����
public:
	CsscomDlg(CWnd* pParent = NULL);	// ��׼���캯��

	// �Ի�������
	enum { IDD = IDD_SSCOM_DIALOG };

	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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
	HICON m_hIconOn;     // ���ڴ�ʱ�ĺ��ͼ����
	HICON m_hIconOff;    // ���ڹر�ʱ��ָʾͼ����

	CSerialPort *m_Port;  // ���ھ��
	CFileMgr	*m_file;
	CProtocol	*m_ptl;

	BOOL InitCom();
	void ToTray();
	void DeleteTray();
};
