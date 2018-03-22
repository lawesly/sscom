#pragma once

#include "FileMgr.h"

#define PROTOCOL_FLAG_HIGH  0x0D
#define PROTOCOL_FLAG_LOW   0xAE

#define PROTOCOL_NAME_LENGTH  128
#define PROTOCOL_PATH_LENGTH  512
#define PROTOCOL_SEND_LENGTH  (29 * 1024)

typedef enum{
	PROTOCOL_HAND_SHAKE = 0,
	PROTOCOL_FILE_SEND,
	PROTOCOL_FILE_FIN,
	PROTOCOL_HAND_ACK,
	PROTOCOL_SEND_ACK,
	PROTOCOL_FIN_ACK,
	PROTOCOL_FILE_RETRANS,
	PROTOCOL_INVALID,
	PROTOCOL_RETRY,
}CPROTOCOLCmd;

typedef struct{
	UINT16 flag;
	UINT16 cmd;
	UINT32 length;
}CPROTOCOLHead;

typedef struct{
	UINT m_filesize;
	char m_filename[PROTOCOL_NAME_LENGTH];
	char m_filepath[PROTOCOL_PATH_LENGTH];
}CPROTOCOLBody_FileHead;


typedef struct{
	UINT m_CRC;
	UINT length;
	char m_filebody[PROTOCOL_SEND_LENGTH];
}CPROTOCOLBody_FileSend;

class CProtocol
{
public:
	CProtocol(void);
	~CProtocol(void);

	BOOL Create(CFileMgr *_file, CPROTOCOLCmd _cmd);
	CPROTOCOLCmd   Parser(CFileMgr *_file);

private:
	BOOL CreateHandShake(CFileMgr *_file);
	BOOL CreateFileSend(CFileMgr *_file);
	BOOL CreateAck(CFileMgr *_file, CPROTOCOLCmd _cmd);
};
