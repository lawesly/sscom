#ifndef __SERIALPORT_H__
#define __SERIALPORT_H__

#define WM_COMM_BREAK_DETECTED		WM_USER+1	// A break was detected on input.
#define WM_COMM_CTS_DETECTED		WM_USER+2	// The CTS (clear-to-send) signal changed state. 
#define WM_COMM_DSR_DETECTED		WM_USER+3	// The DSR (data-set-ready) signal changed state. 
#define WM_COMM_ERR_DETECTED		WM_USER+4	// A line-status error occurred. Line-status errors are CE_FRAME, CE_OVERRUN, and CE_RXPARITY. 
#define WM_COMM_RING_DETECTED		WM_USER+5	// A ring indicator was detected. 
#define WM_COMM_RLSD_DETECTED		WM_USER+6	// The RLSD (receive-line-signal-detect) signal changed state. 
#define WM_COMM_RXCHAR				WM_USER+7	// A character was received and placed in the input buffer. 
#define WM_COMM_RXFLAG_DETECTED		WM_USER+8	// The event character was received and placed in the input buffer.  
#define WM_COMM_TXEMPTY_DETECTED	WM_USER+9	// The last character in the output buffer was sent.  
#define WM_SHOWTASK					WM_USER+10

#define SERIAL_IN_SIZE				(10*1024)
#define SERIAL_OUT_SIZE				(1024)

class CSerialPort
{														 
public:
	CSerialPort();
	virtual		~CSerialPort();

	UINT    m_nbaud;
										
	BOOL	OpenPort(CWnd* pPortOwner, UINT portnr, UINT baud, char parity, UINT databits, UINT stopbits, DWORD dwCommEvents);
	void	ClosePort();

	BOOL	StartMonitoring();
	BOOL	StopMonitoring();

	void	WriteToPort(char* buff,int length);
	void    ReadFromPort(char* buff);
	static void	QueryCom(CString *strSerialList, UINT *n);

protected:
	static UINT	CommThread(LPVOID pParam);
	static void	RecvPort(CSerialPort* port, COMSTAT comstat);
	static void	SendPort(CSerialPort* port);
	void		ProcessErrorMessage(char* ErrorText);

	// thread
	CWinThread*			m_Thread;
	BOOL				m_bThreadAlive;

	// handles
	HANDLE				m_EventWrite;
	HANDLE				m_EventClose;
	OVERLAPPED			m_ov;

	// Event array. 
	// One element is used for each event. There are two event handles for each port.
	// A Write event and a receive character event which is located in the overlapped structure (m_ov.hEvent).
	// There is a general shutdown when the port is closed. 
	HANDLE				m_hEventArray[3];

	// config
	COMMTIMEOUTS		m_CommTimeouts;

	HANDLE				m_hComm;
	CWnd*				m_pOwner;
	DCB					m_dcb;
	UINT				m_nPortNr;
	char*				m_WriteBuf;
	DWORD				m_WriteSize; 
	char*				m_ReadBuf;
	DWORD				m_dwCommEvents;
};

#endif