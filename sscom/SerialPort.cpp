#include "stdafx.h"
#include "SerialPort.h"

#include <assert.h>


CSerialPort::CSerialPort()
{
	// init 
	m_hComm = NULL;
	m_ov.Offset = 0;
	m_ov.hEvent = NULL;
	m_ov.OffsetHigh = 0;
	m_EventWrite = NULL;
	m_EventClose = NULL;
	m_WriteBuf = NULL;
	m_ReadBuf = NULL;
	m_Thread = NULL;
	m_bThreadAlive = FALSE;

	// create events
	if (m_ov.hEvent != NULL)
		ResetEvent(m_ov.hEvent);
	else
		m_ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (m_EventWrite != NULL)
		ResetEvent(m_EventWrite);
	else
		m_EventWrite = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (m_EventClose != NULL)
		ResetEvent(m_EventClose);
	else
		m_EventClose = CreateEvent(NULL, TRUE, FALSE, NULL);

	// initialize the event objects
	m_hEventArray[0] = m_EventClose;	// highest priority
	m_hEventArray[1] = m_EventWrite;
	m_hEventArray[2] = m_ov.hEvent;
}

CSerialPort::~CSerialPort()
{
	// Thread Event  
	if(m_ov.hEvent != NULL)
		CloseHandle( m_ov.hEvent ); 
	if(m_EventWrite != NULL)
		CloseHandle( m_EventWrite ); 
	if(m_EventClose != NULL)
		CloseHandle( m_EventClose); 
}


BOOL CSerialPort::StartMonitoring()
{
	assert(m_Thread == NULL);
	if (!(m_Thread = AfxBeginThread(CommThread, this)))
		return FALSE;

	return TRUE;	
}


BOOL CSerialPort::StopMonitoring()
{
	SetEvent(m_EventClose);
	while(m_bThreadAlive)
		::Sleep(30);
	m_Thread = NULL;

	return TRUE;	
}

BOOL CSerialPort::OpenPort(CWnd* pPortOwner,	// the owner (CWnd)
	UINT  portnr,								// portnumber (1..4)
	UINT  baud,									// baudrate
	char  parity,								// parity 
	UINT  databits,								// databits 
	UINT  stopbits,								// stopbits 
	DWORD dwCommEvents)							// EV_RXCHAR, EV_CTS etc
{
	assert(pPortOwner != NULL);

	char szPort[50];
	char szBaud[50];
	char *ErrorInfo;

	// init port
	m_pOwner = pPortOwner;
	m_nPortNr = portnr;
	m_nbaud = baud;
	m_dwCommEvents = dwCommEvents;

	if(portnr < 10)
		sprintf(szPort, "COM%d", portnr);
	else
		sprintf(szPort, "\\\\.\\COM%d", portnr);

	sprintf(szBaud, "baud=%d parity=%c data=%d stop=%d", baud, parity, databits, stopbits);

	// open port
	m_hComm = CreateFile(szPort,		// communication port string (COMX)
		GENERIC_READ | GENERIC_WRITE,	// read/write types
		0,								// comm devices must be opened with exclusive access
		NULL,							// no security attributes
		OPEN_EXISTING,					// comm devices must use OPEN_EXISTING
		FILE_FLAG_OVERLAPPED,			// Async I/O
		0);

	if (m_hComm == INVALID_HANDLE_VALUE){
		ErrorInfo = "没有发现此串口或被占用";
		goto OPEN_FAIL;
	}

	// configure port
	m_CommTimeouts.ReadIntervalTimeout = 1000;
	m_CommTimeouts.ReadTotalTimeoutMultiplier = 1000;
	m_CommTimeouts.ReadTotalTimeoutConstant = 1000;
	m_CommTimeouts.WriteTotalTimeoutMultiplier = 1000;
	m_CommTimeouts.WriteTotalTimeoutConstant = 1000;


	// configure
	if (!SetCommTimeouts(m_hComm, &m_CommTimeouts)){
		ErrorInfo = "SetCommTimeouts";
		goto OPEN_FAIL;
	}
			   
	if (!SetCommMask(m_hComm, dwCommEvents)){
		ErrorInfo = "SetCommMask";
		goto OPEN_FAIL;
	}

	if (!GetCommState(m_hComm, &m_dcb)){
		ErrorInfo = "GetCommState";
		goto OPEN_FAIL;
	}

	if (!BuildCommDCB(szBaud, &m_dcb)){
		ErrorInfo = "BuildCommDCB";
		goto OPEN_FAIL;
	}

	if (!SetCommState(m_hComm, &m_dcb)){
		ErrorInfo = "SetCommState";
		goto OPEN_FAIL;
	}

	SetupComm(m_hComm, SERIAL_IN_SIZE, SERIAL_OUT_SIZE);									// 输入缓存1024， 输出缓存1024
	PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);		// flush the port

	return TRUE;


OPEN_FAIL:

	ProcessErrorMessage(ErrorInfo);
	if(m_hComm != INVALID_HANDLE_VALUE){
		CloseHandle(m_hComm);
		m_hComm = INVALID_HANDLE_VALUE; 
	}
	return FALSE;
}

void CSerialPort::ClosePort()
{
	if(m_hComm != INVALID_HANDLE_VALUE){
		CloseHandle(m_hComm);
		m_hComm = INVALID_HANDLE_VALUE; 
	}
}

void CSerialPort::WriteToPort(char* buff, int length)
{		
	assert(m_hComm != NULL);

	m_WriteBuf = buff;
	m_WriteSize = length;

	SetEvent(m_EventWrite);
}

void CSerialPort::ReadFromPort(char* buff)
{		
	assert(m_hComm != NULL);
	m_ReadBuf = buff;
}

void CSerialPort::QueryCom(CString *strSerialList, UINT *n)
{
	int i = 0; 
	CHAR Name[25]; 
	UCHAR szPortName[25]; 
	LONG Status; 
	DWORD dwIndex = 0; 
	DWORD dwName; 
	DWORD dwSizeofPortName; 
	DWORD Type;
	HKEY hKey; 

	LPCTSTR data_Set="HARDWARE\\DEVICEMAP\\SERIALCOMM\\";
	dwName = sizeof(Name); 
	dwSizeofPortName = sizeof(szPortName);
	long ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, data_Set, 0, KEY_READ, &hKey); //打开一个制定的注册表键,成功返回ERROR_SUCCESS即“0”值
	if(ret == ERROR_SUCCESS) 
	{
		do 
		{ 
			Status = RegEnumValue(hKey, dwIndex++, Name, &dwName, NULL, &Type, szPortName, &dwSizeofPortName);//读取键值 
			if((Status == ERROR_SUCCESS)||(Status == ERROR_MORE_DATA)) 
			{ 
				strSerialList[i] = CString(szPortName);       // 串口字符串保存 
				printf("serial:%s\n",strSerialList[i]);
				i++;	// 串口计数 
			} 

			dwName = sizeof(Name); 
			dwSizeofPortName = sizeof(szPortName); 
		} while((Status == ERROR_SUCCESS)||(Status == ERROR_MORE_DATA)); 

		RegCloseKey(hKey); 
	}

	*n = i;
}

UINT CSerialPort::CommThread(LPVOID pParam)
{
	CSerialPort *port = (CSerialPort*)pParam;

	port->m_bThreadAlive = TRUE;	

	DWORD Event = 0;
	DWORD CommEvent = 0;
	DWORD dwError = 0;
	COMSTAT comstat;
	BOOL  bResult = TRUE;
	
	memset(&comstat, 0, sizeof(COMSTAT));

	// Clear comm buffers at startup
	if (port->m_hComm)
		PurgeComm(port->m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

	for (;;) 
	{ 
		// Make a call to WaitCommEvent().  This call will return immediatly
		// because our port was created as an async port (FILE_FLAG_OVERLAPPED
		// and an m_OverlappedStructerlapped structure specified).  This call will cause the 
		// m_OverlappedStructerlapped element m_OverlappedStruct.hEvent, which is part of the m_hEventArray to 
		// be placed in a non-signeled state if there are no bytes available to be read,
		// or to a signeled state if there are bytes available.  If this event handle 
		// is set to the non-signeled state, it will be set to signeled when a 
		// character arrives at the port.
		bResult = WaitCommEvent(port->m_hComm, &Event, &port->m_ov);

		if (!bResult)  
		{ 
			// If WaitCommEvent() returns FALSE, process the last error to determin the reason
			switch (dwError = GetLastError()) 
			{ 
			case ERROR_IO_PENDING: 	
				{ 
					// This is a normal return value if there are no bytes
					// to read at the port.
					// Do nothing and continue
					break;
				}
			case 87:
				{
					// Under Windows NT, this value is returned for some reason.
					// I have not investigated why, but it is also a valid reply
					// Also do nothing and continue.
					break;
				}
			default:
				{
					// All other error codes indicate a serious error has
					// occured.  Process this error.
					port->ProcessErrorMessage("WaitCommEvent()");
					break;
				}
			}
		}
		else
		{
			// If WaitCommEvent() returns TRUE, check to be sure there are
			// actually bytes in the buffer to read.  
			//
			// If you are reading more than one byte at a time from the buffer 
			// (which this program does not do) you will have the situation occur 
			// where the first byte to arrive will cause the WaitForMultipleObjects() 
			// function to stop waiting.  The WaitForMultipleObjects() function 
			// resets the event handle in m_OverlappedStruct.hEvent to the non-signelead state
			// as it returns.  
			//
			// If in the time between the reset of this event and the call to 
			// ReadFile() more bytes arrive, the m_OverlappedStruct.hEvent handle will be set again
			// to the signeled state. When the call to ReadFile() occurs, it will 
			// read all of the bytes from the buffer, and the program will
			// loop back around to WaitCommEvent().
			// 
			// At this point you will be in the situation where m_OverlappedStruct.hEvent is set,
			// but there are no bytes available to read.  If you proceed and call
			// ReadFile(), it will return immediatly due to the async port setup, but
			// GetOverlappedResults() will not return until the next character arrives.
			//
			// It is not desirable for the GetOverlappedResults() function to be in 
			// this state.  The thread shutdown event (event 0) and the WriteFile()
			// event (Event2) will not work if the thread is blocked by GetOverlappedResults().
			//
			// The solution to this is to check the buffer with a call to ClearCommError().
			// This call will reset the event handle, and if there are no bytes to read
			// we can loop back through WaitCommEvent() again, then proceed.
			// If there are really bytes to read, do nothing and proceed.

			bResult = ClearCommError(port->m_hComm, &dwError, &comstat);

			if (comstat.cbInQue == 0)
				continue;
		}

		// Main wait function.  This function will normally block the thread
		// until one of three events occur that require action.
		Event = WaitForMultipleObjects(3, port->m_hEventArray, FALSE, INFINITE);

		switch (Event)
		{
			case 0: // close event
			{
					port->m_bThreadAlive = FALSE;
					AfxEndThread(100);

					break;
			}
			case 1:	// write event
			{
					SendPort(port);
					break;
			}  
			case 2: // read event
			{
				GetCommMask(port->m_hComm, &CommEvent);
				if (CommEvent & EV_RXCHAR)
					RecvPort(port, comstat);
				break;
			}
		} 
	} 

	return 0;
}

void CSerialPort::SendPort(CSerialPort* port)
{
	BOOL bWrite = FALSE;
	BOOL bResult = FALSE;
	DWORD BytesSent = 0;

	ResetEvent(port->m_EventWrite);
	PurgeComm(port->m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);	// Clear buffer

	port->m_ov.Offset = 0;
	port->m_ov.OffsetHigh = 0;

	bResult = WriteFile(port->m_hComm,		// Handle to COM Port
		port->m_WriteBuf,					// Pointer to message buffer in calling finction
		port->m_WriteSize,					// Length of message to send
		&BytesSent,							// Where to store the number of bytes sent
		&port->m_ov);						// Overlapped structure

	// deal with any error codes
	if (!bResult)  
	{
		DWORD dwError = GetLastError();
		switch(dwError)
		{
			case ERROR_IO_PENDING:
			{
				// asynchronous i/o is still in progress
				bWrite = TRUE;
				break;
			}
			default:
			{
				// all other error codes
				port->ProcessErrorMessage("WriteChar()");
			}
		}
	} 

	if(bWrite)
	{
		bResult = GetOverlappedResult(port->m_hComm,	// Handle to COM port 
			&port->m_ov,								// Overlapped structure
			&BytesSent,								// Stores number of bytes sent
			TRUE); 									// Wait flag

		if (!bResult)  
		{
			port->ProcessErrorMessage("GetOverlappedResults() is in Write");
		}
	}

	::SendMessage((port->m_pOwner)->m_hWnd, WM_COMM_TXEMPTY_DETECTED, (WPARAM)BytesSent, (LPARAM)0);
}

//
// Character received. Inform the owner
//
void CSerialPort::RecvPort(CSerialPort* port, COMSTAT comstat)
{
	BOOL  bRead = FALSE; 
	BOOL  bResult = FALSE;
	DWORD dwError = 0;
	DWORD BytesRecv = 0;

	for (;;) 
	{ 
		// ClearCommError() will update the COMSTAT structure and
		// clear any other errors.
		bResult = ClearCommError(port->m_hComm, &dwError, &comstat);

		// start forever loop.  I use this type of loop because I
		// do not know at runtime how many loops this will have to
		// run. My solution is to start a forever loop and to
		// break out of it when I have processed all of the
		// data available.  Be careful with this approach and
		// be sure your loop will exit.
		// My reasons for this are not as clear in this sample 
		// as it is in my production code, but I have found this 
		// solutiion to be the most efficient way to do this.
		if (comstat.cbInQue == 0)
		{
			// break out when all bytes have been read
			break;
		}

		bResult = ReadFile(port->m_hComm,		// Handle to COMM port 
			port->m_ReadBuf,						// RX Buffer Pointer
			comstat.cbInQue,					// Read all bytes
			&BytesRecv,							// Stores number of bytes read
			&port->m_ov);						// pointer to the m_ov structure
			
		if (!bResult)  
		{ 
			switch(dwError = GetLastError()) 
			{ 
				case ERROR_IO_PENDING: 	
				{ 
					// asynchronous i/o is still in progress 
					bRead = TRUE;
					break;
				}
				default:
				{
					// Another error has occured.  Process this error.
					port->ProcessErrorMessage("Recvport()");
				} 
			}
		}

		if (bRead)
		{
			bResult = GetOverlappedResult(port->m_hComm,	// Handle to COMM port 
				&port->m_ov,								// Overlapped structure
				&BytesRecv,								// Stores number of bytes read
				TRUE); 									// Wait flag

			if (!bResult)  
			{
				port->ProcessErrorMessage("GetOverlappedResults() in ReceiveChar()");
			}
		}

		port->m_ReadBuf += BytesRecv;

		::SendMessage((port->m_pOwner)->m_hWnd, WM_COMM_RXCHAR, (WPARAM)BytesRecv, (LPARAM)0);
	} 
}

void CSerialPort::ProcessErrorMessage(char* ErrorText)
{
	char *Temp = new char[200];

	LPVOID lpMsgBuf;

	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
		(LPTSTR) &lpMsgBuf,
		0,
		NULL 
		);

	sprintf(Temp, "WARNING:  %s Failed with the following error: \n%s\nPort: %d\n", (char*)ErrorText, lpMsgBuf, m_nPortNr); 
	AfxMessageBox(Temp);

	LocalFree(lpMsgBuf);
	delete [] Temp;
}
