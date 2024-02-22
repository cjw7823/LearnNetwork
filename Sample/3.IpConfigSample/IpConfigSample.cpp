// IpConfigSample.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"


#include <winsock2.h>
//IP Helper API 사용을 위한 헤더 및 라이브러리 설정
#include <iphlpapi.h>
#pragma comment(lib, "Iphlpapi.lib")

#include <stdio.h>
#include <stdlib.h>

int _tmain(int argc, _TCHAR* argv[])
{
	//한 NIC에 대한 설정이 저장될 수 있는 메모리 확보.
	ULONG uBufferLength = sizeof (IP_ADAPTER_INFO);
	PIP_ADAPTER_INFO pNicInfo = (IP_ADAPTER_INFO*)malloc(uBufferLength);

	//설정 정보를 확인한다.
	DWORD dwResult = ::GetAdaptersInfo(pNicInfo, &uBufferLength);
	if (dwResult == ERROR_BUFFER_OVERFLOW)
	{
		//NIC이 2개 이상이라 모든 정보를 담을 수 없는 경우이므로
		//메모리를 다시 할당 한다.
		free(pNicInfo);
		pNicInfo = (IP_ADAPTER_INFO *)malloc(uBufferLength);

		//버퍼 크기 조정 후 다시 설정 정보를 확인 한다.
		if (::GetAdaptersInfo(pNicInfo, &uBufferLength) != NO_ERROR)
		{
			free(pNicInfo);
			puts("ERROR: 어댑터 정보를 알 수 없습니다.");
			return 0;
		}
	}

	//연결 리스트 형태로 설정 정보에 접근한다.
	PIP_ADAPTER_INFO pNicInfoTmp = pNicInfo;
	while (pNicInfoTmp != NULL)
	{
		printf("설명: %s\n", pNicInfoTmp->Description);
		printf("MAC 주소: %02X-%02X-%02X-%02X-%02X-%02X\n",
			pNicInfoTmp->Address[0], pNicInfoTmp->Address[1],
			pNicInfoTmp->Address[2], pNicInfoTmp->Address[3],
			pNicInfoTmp->Address[4], pNicInfoTmp->Address[5]);
		printf("IP 주소: %s\n\n",
			pNicInfo->IpAddressList.IpAddress.String);

		pNicInfoTmp = pNicInfoTmp->Next;
	}

	free(pNicInfo);
	return 0;
}

