#include "StdAfx.h"
#include "Protocol.h"


CProtocol::CProtocol(void)
{
}

CProtocol::~CProtocol(void)
{
}

BOOL CProtocol::CreateHandShake(CFileMgr *_file)
{
	CPROTOCOLHead *CPHead = (CPROTOCOLHead *)_file->m_sendbuf;
	CPROTOCOLBody_FileHead *CPBody = (CPROTOCOLBody_FileHead*)(_file->m_sendbuf + sizeof(CPROTOCOLHead));

	CPHead->flag = ((PROTOCOL_FLAG_HIGH<<8) | PROTOCOL_FLAG_LOW);
	CPHead->cmd = PROTOCOL_HAND_SHAKE;
	CPHead->length = sizeof(CPROTOCOLBody_FileHead);

	CPBody->m_filesize = _file->m_filesize;
	memcpy(CPBody->m_filename, _file->m_filename, PROTOCOL_NAME_LENGTH);
	memcpy(CPBody->m_filepath, _file->m_filepath, PROTOCOL_PATH_LENGTH);

	_file->m_sendlen = sizeof(CPROTOCOLHead) + CPHead->length;
	_file->m_sendcurrent = 0;

	return TRUE;
}

BOOL CProtocol::CreateFileSend(CFileMgr *_file)
{
	CPROTOCOLHead *CPHead = (CPROTOCOLHead *)_file->m_sendbuf;
	CPROTOCOLBody_FileSend *CPBody = (CPROTOCOLBody_FileSend*)(_file->m_sendbuf + sizeof(CPROTOCOLHead));

	CPHead->flag = ((PROTOCOL_FLAG_HIGH<<8) | PROTOCOL_FLAG_LOW);

	CPBody->length = _file->ReadFile(CPBody->m_filebody, PROTOCOL_SEND_LENGTH);
	if(CPBody->length == PROTOCOL_SEND_LENGTH)
		CPHead->cmd = PROTOCOL_FILE_SEND;
	else
		CPHead->cmd = PROTOCOL_FILE_FIN;
	CPHead->length = sizeof(CPROTOCOLBody_FileSend);

	_file->m_sendlen = sizeof(CPROTOCOLHead) + CPHead->length;
	_file->m_sendcurrent = 0;

	return TRUE;
}

BOOL CProtocol::CreateAck(CFileMgr *_file, CPROTOCOLCmd _cmd)
{
	CPROTOCOLHead *CPHead = (CPROTOCOLHead *)_file->m_sendbuf;

	CPHead->flag = ((PROTOCOL_FLAG_HIGH<<8) | PROTOCOL_FLAG_LOW);
	CPHead->cmd = _cmd;
	CPHead->length = 0;

	_file->m_sendlen = sizeof(CPROTOCOLHead) + CPHead->length;
	_file->m_sendcurrent = 0;

	return TRUE;
}


BOOL CProtocol::Create(CFileMgr *_file, CPROTOCOLCmd _cmd)
{
	BOOL iRet = FALSE;

	switch (_cmd)
	{
	case PROTOCOL_HAND_SHAKE:
		iRet = CreateHandShake(_file);
		break;
		 
	case PROTOCOL_FILE_SEND:
	case PROTOCOL_FILE_FIN:
		iRet = CreateFileSend(_file);
		break;
							 
	case PROTOCOL_HAND_ACK:
	case PROTOCOL_SEND_ACK:
	case PROTOCOL_FIN_ACK:
	case PROTOCOL_FILE_RETRANS:
		iRet = CreateAck(_file, _cmd);
		break;	
	default:
		iRet = FALSE;
	}

	return iRet;
}


CPROTOCOLCmd CProtocol::Parser(CFileMgr *_file)
{
	if(_file->m_recvlen < sizeof(CPROTOCOLHead)){
		return PROTOCOL_RETRY;
	}

	CPROTOCOLHead *CPHead = (CPROTOCOLHead *)_file->m_recvbuf;
	if( CPHead->flag != ((PROTOCOL_FLAG_HIGH<<8) | PROTOCOL_FLAG_LOW))
	{
		return PROTOCOL_INVALID;
	}

	if((sizeof(CPROTOCOLHead) + CPHead->length) > _file->m_recvlen){
		return PROTOCOL_RETRY;
	}
	
	if(CPHead->cmd == PROTOCOL_HAND_SHAKE){
		CPROTOCOLBody_FileHead *CPBody = (CPROTOCOLBody_FileHead*)(_file->m_recvbuf + sizeof(CPROTOCOLHead));
		_file->SetFile(CPBody->m_filepath, CPBody->m_filename, CPBody->m_filesize);		
		return PROTOCOL_HAND_SHAKE;
	}else if (CPHead->cmd == PROTOCOL_HAND_ACK){
		return PROTOCOL_HAND_ACK;
	}else if(CPHead->cmd == PROTOCOL_FILE_SEND){
		CPROTOCOLBody_FileSend *CPBody = (CPROTOCOLBody_FileSend*)(_file->m_recvbuf + sizeof(CPROTOCOLHead));
		_file->WriteFile(CPBody->m_filebody, CPBody->length);
		return PROTOCOL_FILE_SEND;
	}else if(CPHead->cmd == PROTOCOL_SEND_ACK){
		return PROTOCOL_SEND_ACK;
	}else if(CPHead->cmd == PROTOCOL_FILE_FIN){
		CPROTOCOLBody_FileSend *CPBody = (CPROTOCOLBody_FileSend*)(_file->m_recvbuf + sizeof(CPROTOCOLHead));
		_file->WriteFile(CPBody->m_filebody, CPBody->length);
		_file->CloseFile();
		return PROTOCOL_FILE_FIN;
	}else if(CPHead->cmd == PROTOCOL_FIN_ACK){
		return PROTOCOL_FIN_ACK;
	}else if(CPHead->cmd == PROTOCOL_FILE_RETRANS){
		return PROTOCOL_FILE_RETRANS;
	}
	else{
		return PROTOCOL_INVALID;
	}
}
