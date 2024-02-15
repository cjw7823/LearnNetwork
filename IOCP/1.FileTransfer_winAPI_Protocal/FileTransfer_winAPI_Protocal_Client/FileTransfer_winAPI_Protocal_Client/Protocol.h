#pragma once

/*
    �޼��� ����.
    Ȯ�强�� ���� ���� ���� ����.
*/
typedef enum CMDCODE
{
    CMD_SEND_LIST = 100,        // S->C ���� ����Ʈ ����
    CMD_FILE_BEGIN = 110,       // S->C ���� ���� ����.
    CMD_FILE_DATA = 120,        // S->C ���� ���� ��.
    CMD_FILE_END = 130,              // S->C ���� ���� ����.
    CMD_GET_LIST = 200,         // C->S ���� ����Ʈ ��û.
    CMD_GET_FILE = 210,         // C->S ���� ���� ��û.
    CMD_ERROR = 999,            // ����.
} CMDCODE;

/*
    �⺻ ���
*/
typedef struct HEADER
{
    CMDCODE cmdCode;   // ��� �ڵ�
    DWORD dwSize;   // ���� �� Ȯ�� ����� ����.
} HEADER;

/*
    Ȯ�� ���
*/

//Ȯ�����: S->C: ���� ����Ʈ ����
typedef struct SEND_FILELIST
{
    unsigned int nCount;	//������ ��������(GETFILE ����ü) ����
} SEND_FILELIST;

// CMD_GET_FILE Ȯ�����.
typedef struct GET_FILE
{
    unsigned int nIndex;	//���۹������� ������ �ε���
} GET_FILE;

// CMD_ERROR Ȯ�����.
typedef struct ERROR_CODE {
    int	nErrorCode;         // �����ڵ�: ���� Ȯ���� ����. -1 ����.
    char errorMessage[_MAX_FNAME]; // ���� �޽���, NULL�� ����Ǿ�� ��
} ERROR_CODE;

// ���� ����
typedef struct FILEINFO
{
    unsigned int nIndex;			// ������ �ε���
    WCHAR szFileName[MAX_PATH];	// ������ �ִ� ���� ��
    DWORD dwFileSize;				// ������ ����Ʈ ���� ũ��
} FILEINFO;