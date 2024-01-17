#include <WinSock2.h>
#include <iostream>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

int main(int argc, char* argv[])
{
	std::cout << "<Multiplexing Server>" << std::endl;

	WSADATA wsaData = { 0, };
	if (WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		std::cout << "WSAStartup() Failed" << std::endl;
		return 0;
	}

	SOCKET h_ServerSocket;
	SOCKET h_ClientSocket;
	h_ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (h_ServerSocket == INVALID_SOCKET)
	{
		std::cout << "socket() Failed" << std::endl;
		closesocket(h_ServerSocket);
		WSACleanup();
		return 0;
	}

	sockaddr_in addrServer;
	sockaddr_in addrClient;
	addrServer.sin_family = AF_INET;
	addrServer.sin_port = htons(8080);
	inet_pton(AF_INET, "127.0.0.1", &addrServer.sin_addr);

	if (bind(h_ServerSocket, (sockaddr*)&addrServer, sizeof(addrServer)))
	{
		std::cout << "bind() Failed" << std::endl;
		closesocket(h_ServerSocket); 
		WSACleanup();
		return 0;
	}

	if (listen(h_ServerSocket, SOMAXCONN))
	{
		std::cout << "listen() Failed" << std::endl;
		closesocket(h_ServerSocket);
		WSACleanup();
		return 0;
	}

	/*
		select �Լ��� Multiplexing
	*/
	fd_set Reads;
	fd_set Copys;

	FD_ZERO(&Reads);
	FD_SET(h_ServerSocket, &Reads);

	TIMEVAL timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 100;

	while (1)
	{
		Copys = Reads;//���� ���ÿ�/���� ������.

		int fd_num = select(0, &Copys, 0, 0, &timeout);//timeout ���� null �̸� ���ŷ.
		if (fd_num == SOCKET_ERROR)//select �Լ� ���� �߻�. ����ó�� ����.
			break;

		if (fd_num == 0)//select �Լ� �ð� ����.
			continue;

		for (int i = 0; i < Reads.fd_count; i++)
		{
			//�̺�Ʈ�� �Ͼ�� ó��
			if (FD_ISSET(Reads.fd_array[i], &Copys))
			{
				if (Reads.fd_array[i] == h_ServerSocket)
				{
					int ClientAddrLength = sizeof(addrClient);
					h_ClientSocket = accept(h_ServerSocket, (SOCKADDR*)&addrClient, &ClientAddrLength);
					FD_SET(h_ClientSocket, &Reads);
					printf("connection client : %d\n", h_ClientSocket);
				}
				else
				{
					//Ŭ���̾�Ʈ ������ ó��
					char Buffer[1024] = { 0, };
					int RecvLength = 0;
					RecvLength = recv(Reads.fd_array[i], Buffer, sizeof(Buffer), 0);
					if (RecvLength == 0)
					{
						//close connection
						FD_CLR(Reads.fd_array[i], &Reads);
						closesocket(Reads.fd_array[i]);
						printf("close connection : %llu\n", Reads.fd_array[i]);
					}
					else
					{
						for (int j = 0; j < Reads.fd_count; j++)
						{
							if (Reads.fd_array[j] != h_ServerSocket)
								send(Reads.fd_array[j], Buffer, RecvLength, 0);
						}
					}
				}
			}
		}
	}

	closesocket(h_ServerSocket);
	closesocket(h_ClientSocket);
	WSACleanup();

	return 0;
}