#include "StdAfx.h"
#include "FileMgr.h"


CFileMgr::CFileMgr(void)
{
}

CFileMgr::CFileMgr(UINT sendsize, UINT recvsize)
{
	m_sendbuf = new char[sendsize];
	m_sendsize = sendsize;
	m_sendlen = 0;
	m_sendcurrent = 0;

	m_recvbuf = new char[recvsize];
	m_recvsize = recvsize;
	m_recvlen = 0;
	m_recvcurrent = 0;

	assert(m_sendbuf != NULL);
	assert(m_recvbuf != NULL);

	TRACE("m_sendbuf:0x%x\r\n", m_sendbuf);
	TRACE("m_recvbuf:0x%x\r\n", m_recvbuf);

	ResetFile();
}

CFileMgr::~CFileMgr(void)
{
	if(m_sendbuf != NULL){
		delete m_sendbuf;
		m_sendbuf = NULL;
	}

	if(m_recvbuf != NULL){
		delete m_recvbuf;
		m_recvbuf = NULL;
	}
}

void CFileMgr::SetFile(CString path)
{
	m_mode = FILE_MODE_SEND;
	CString filepath = path;
	CString filename = filepath.Mid(filepath.ReverseFind('\\')+1);
	strncpy(m_filename, filename, filename.GetLength()+1);
	filepath.Replace(' ', '\ ');
	strncpy(m_filepath, filepath, filepath.GetLength()+1);

	if(!(m_hfile.Open(m_filepath, CFile::modeRead | CFile::typeBinary))) 
	{
		assert("Open File Failed!");
		return;
	}

	m_hfile.SeekToEnd();
	m_filesize = m_hfile.GetLength();
	m_hfile.SeekToBegin();
	m_hfile.Close();
}

void CFileMgr::SetFile(char *filepath, char *filename, UINT size)
{
	m_mode = FILE_MODE_RECV;
	m_filesize = size;
	memcpy(m_filename, filename, FILE_NAME_LENGTH);
	memcpy(m_filepath, filepath, FILE_PATH_LENGTH);
}

void CFileMgr::ResetFile()
{
	m_mode = FILE_MODE_NONE;
	m_filesize = 0;
	memset(m_filename, 0, FILE_NAME_LENGTH);
	memset(m_filepath, 0, FILE_PATH_LENGTH);

	m_currentsize = 0;
	m_currentpercent = 0;
	m_retransmission = 0;
}

UINT32 CFileMgr::ReadFile(char *buf, UINT len)
{
	UINT32 ReadBytes;
	if(m_hfile.m_hFile == CFile::hFileNull)
	{
		if(!(m_hfile.Open(m_filepath, CFile::modeRead | CFile::typeBinary))) 
		{
			AfxMessageBox("Open File Failed!");
			return 0;
		}
	}

	ReadBytes = m_hfile.Read(buf, len);
	m_currentsize += ReadBytes;
	m_currentpercent = (((float)m_currentsize)/((float)m_filesize)) * 100;
	return ReadBytes;
}

void CFileMgr::WriteFile(char *buf, UINT len)
{
	UINT32 WriteBytes;
	if(m_hfile.m_hFile == CFile::hFileNull)
	{
		//获取当前程序路径
		CString  strPathName;
		GetModuleFileName(NULL, strPathName.GetBuffer(256), 256);
		strPathName.ReleaseBuffer(256);
		int nPos  = strPathName.ReverseFind('\\');
		strPathName = strPathName.Left(nPos + 1);
		if(!(m_hfile.Open(strPathName + m_filename, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary))) 
		{
			AfxMessageBox("Open File Failed!");
			return;
		}
	}

	WriteBytes = len;
	m_currentsize += WriteBytes;
	m_currentpercent = (((float)m_currentsize)/((float)m_filesize)) * 100;
	m_hfile.Write(buf, len);

	return ;
}

void CFileMgr::CloseFile()
{
	assert(m_hfile.m_hFile != CFile::hFileNull);

	if(m_mode == FILE_MODE_RECV){
		m_hfile.Flush();
	}

	m_hfile.Close();
}

char * CFileMgr::GetFinishedTime(int baud, UINT unit, UINT delay)
{
	UINT TransTime = (m_filesize - m_currentsize)/(baud/10);
	UINT DelayTime = ((m_filesize - m_currentsize)/unit)*delay/1000;
	UINT FinishedTime = TransTime + DelayTime;
	UINT hours = FinishedTime/3600;
	UINT mins = (FinishedTime%3600)/60;
	UINT secs = FinishedTime%60;

	memset(m_FinishedTime, 0, sizeof(m_FinishedTime));

	if(hours != 0)
		sprintf(m_FinishedTime + strlen(m_FinishedTime), "%d小时", hours);
	if(mins != 0)
		sprintf(m_FinishedTime + strlen(m_FinishedTime), "%d分", mins);
	if(secs != 0)
		sprintf(m_FinishedTime + strlen(m_FinishedTime), "%d秒", secs);

	return m_FinishedTime;
}
