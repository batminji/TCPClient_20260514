#include "stdafx.h"

int main()
{
	srand((unsigned int)time(nullptr));
	WSAData wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		return 1;
	}

	hostent* HostInfo = gethostbyname("login.calculate.edu");

	char ServerIP[1024] = { 0, };
	IN_ADDR Addr;
	Addr.s_addr = *(ULONG*)*HostInfo->h_addr_list;
	sprintf_s(ServerIP, "%s", inet_ntoa(Addr));
	printf("%s\n", ServerIP);

	SOCKET ServerSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ServerSocket == INVALID_SOCKET)
	{
		std::cout << "Listen Socket Error" << std::endl;
		exit(-1);
	}

	SOCKADDR_IN ServerSocketAddr;
	memset(&ServerSocketAddr, 0, sizeof(ServerSocketAddr));
	ServerSocketAddr.sin_family = AF_INET;
	if (!inet_pton(AF_INET, ServerIP, (PVOID)&ServerSocketAddr.sin_addr.s_addr))
	{
		printf("inet_pton Error\n");
		exit(-1);
	}
	ServerSocketAddr.sin_port = htons(SERVER_PORT);

	int retval;
	retval = connect(ServerSocket, (SOCKADDR*)&ServerSocketAddr, sizeof(ServerSocketAddr));
	if (retval == SOCKET_ERROR)
	{
		printf("Connect Error\n");
		exit(-1);
	}

	int WantSendBytes = 0;
	int SentBytes = 0;
	int TotalSentBytes = 0;

	int WantRecvBytes = 0;
	int RecvBytes = 0;
	int TotalRecvBytes = 0;

	unsigned long PlayerX = 0;
	unsigned long PlayerY = 0;

	while (true)
	{
		// Send
		PacketHeader SendHeader;
		MoveData SendData;

		SendData.Dir = ' ';
		if (_kbhit())
		{
			char key = _getch();
			if (key == 'w' || key == 'W')
			{
				SendData.Dir = 'W';
			}
			else if (key == 's' || key == 'S')
			{
				SendData.Dir = 'S';
			}
			else if (key == 'a' || key == 'A')
			{
				SendData.Dir = 'A';
			}
			else if (key == 'd' || key == 'D')
			{
				SendData.Dir = 'D';
			}

			SendHeader.Size = htons(sizeof(MoveData));
			SendHeader.Code = htons(static_cast<unsigned short>(PacketType::Move));

			// Send Header
			WantSendBytes = sizeof(SendHeader);
			SentBytes = 0;
			TotalSentBytes = 0;

			do
			{
				SentBytes = send(ServerSocket, (char*)&SendHeader + TotalSentBytes, WantSendBytes - TotalSentBytes, 0);
				if (SentBytes == 0)
				{
					printf("%d", WSAGetLastError());
					exit(-1);
				}
				else if (SentBytes < 0)
				{
					printf("Send Header Error\n");
					exit(-1);
				}
				TotalSentBytes += SentBytes;
			} while (TotalSentBytes < WantSendBytes);

			// Send Data
			WantSendBytes = sizeof(SendData);
			SentBytes = 0;
			TotalSentBytes = 0;

			do
			{
				SentBytes = send(ServerSocket, (char*)&SendData + TotalSentBytes, WantSendBytes - TotalSentBytes, 0);
				if (SentBytes <= 0)
				{
					printf("Send Data Error\n");
					exit(-1);
				}
				TotalSentBytes += SentBytes;
			} while (TotalSentBytes < WantSendBytes);

			// Recv
			PacketHeader RecvHeader;
			PositionData RecvData;

			// Recv Header
			WantRecvBytes = sizeof(RecvHeader);
			RecvBytes = 0;
			TotalRecvBytes = 0;

			RecvBytes = recv(ServerSocket, (char*)&RecvHeader + TotalRecvBytes, WantRecvBytes - TotalRecvBytes, MSG_WAITALL);
			if (RecvBytes <= 0)
			{
				printf("Recv Header Error\n");
				exit(-1);
			}

			RecvHeader.Size = ntohs(RecvHeader.Size);
			RecvHeader.Code = ntohs(RecvHeader.Code);

			switch (static_cast<PacketType>(RecvHeader.Code))
			{
			case PacketType::Position:
			{
				// Recv Data
				WantRecvBytes = RecvHeader.Size;
				RecvBytes = 0;
				TotalRecvBytes = 0;

				RecvBytes = recv(ServerSocket, (char*)&RecvData, WantRecvBytes, MSG_WAITALL);
				if (RecvBytes <= 0)
				{
					printf("Recv Data Error\n");
					exit(-1);
				}


				PlayerX = ntohl(RecvData.X);
				PlayerY = ntohl(RecvData.Y);

				printf("Player X : %d, Player Y : %d\n", PlayerX, PlayerY);
			}
			break;
			default:
				printf("InValid Packet Code\n");
				break;
			}
		}	
	}

	closesocket(ServerSocket);

	WSACleanup();
}