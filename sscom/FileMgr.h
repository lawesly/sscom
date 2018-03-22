#pragma once

#define FILE_MODE_NONE 0
#define FILE_MODE_SEND 1
#define FILE_MODE_RECV 2

#define FILE_NAME_LENGTH  128
#define FILE_PATH_LENGTH  512

#define FILE_BUFF_SEND	  (30 * 1024)
#define FILE_BUFF_RECV	  (30 * 1024)


class CFileMgr
{
public:
	CFileMgr(void);
	~CFileMgr(void);
	CFileMgr(UINT sendsize, UINT recvsize);

	void   SetFile(CString path);
	void   SetFile(char *filepath, char *filename, UINT size);
	void   ResetFile();
	UINT32 ReadFile(char *buf, UINT len);
	void   WriteFile(char *buf, UINT len);
	void   CloseFile();
	char  *GetFinishedTime(int baud, UINT unit, UINT delay);

	// 文件句柄
	CFile m_hfile;
	char *m_sendbuf;
	UINT  m_sendsize;
	UINT  m_sendlen;
	UINT  m_sendcurrent;
	char *m_recvbuf;
	UINT  m_recvsize;
	UINT  m_recvlen;
	UINT  m_recvcurrent;

	// 文件信息
	UINT m_mode;
	UINT m_filesize;
	char m_filename[FILE_NAME_LENGTH];
	char m_filepath[FILE_PATH_LENGTH];

	// 文件状态
	UINT m_currentsize;
	UINT m_currentpercent;
	UINT m_retransmission;
	char m_FinishedTime[256];
};
