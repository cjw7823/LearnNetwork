#pragma once

/*
    메세지 종류.
    확장성을 위해 숫자 간격 설정.
*/
typedef enum CMDCODE
{
    CMD_SEND_LIST = 100,        // S->C 파일 리스트 전송
    CMD_FILE_BEGIN = 110,       // S->C 파일 전송 시작.
    CMD_FILE_DATA = 120,        // S->C 파일 전송 중.
    CMD_FILE_END = 130,              // S->C 파일 전송 종료.
    CMD_GET_LIST = 200,         // C->S 파일 리스트 요청.
    CMD_GET_FILE = 210,         // C->S 파일 전송 요청.
    CMD_ERROR = 999,            // 에러.
} CMDCODE;

/*
    기본 헤더
*/
typedef struct HEADER
{
    CMDCODE cmdCode;   // 명령 코드
    DWORD dwSize;   // 따라 올 확장 헤더의 길이.
} HEADER;

/*
    확장 헤더
*/

//확장헤더: S->C: 파일 리스트 전송
typedef struct SEND_FILELIST
{
    unsigned int nCount;	//전송할 파일정보(GETFILE 구조체) 개수
} SEND_FILELIST;

// CMD_GET_FILE 확장헤더.
typedef struct GET_FILE
{
    unsigned int nIndex;	//전송받으려는 파일의 인덱스
} GET_FILE;

// CMD_ERROR 확장헤더.
typedef struct ERROR_CODE {
    int	nErrorCode;         // 에러코드: 향후 확장을 위함. -1 고정.
    char errorMessage[_MAX_FNAME]; // 에러 메시지, NULL로 종료되어야 함
} ERROR_CODE;

// 파일 정보
typedef struct FILEINFO
{
    unsigned int nIndex;			// 파일의 인덱스
    WCHAR szFileName[MAX_PATH];	// 윈도우 최대 글자 수
    DWORD dwFileSize;				// 파일의 바이트 단위 크기
} FILEINFO;