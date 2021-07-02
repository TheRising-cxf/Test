#include "Cserver.h"
#include <Winsock2.h>
#include <cstdio>
#include "opencv.hpp"
//using namespace std;
using namespace cv;
#pragma comment(lib,"ws2_32.lib")
int  main()
{
	WSADATA wsaData;
	SOCKET sockServer;
	SOCKADDR_IN addrServer;
	SOCKET sockClient;
	SOCKADDR_IN addrClient;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	sockServer = socket(AF_INET, SOCK_STREAM, 0);
	addrServer.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//INADDR_ANY��ʾ�κ�IP
	addrServer.sin_family = AF_INET;
	addrServer.sin_port = htons(6000);//�󶨶˿�6000
	bind(sockServer, (SOCKADDR*)&addrServer, sizeof(SOCKADDR));

	//Listen������
	listen(sockServer, 5);//5Ϊ�ȴ�������Ŀ
	printf("������������:\n������...\n");
	int len = sizeof(SOCKADDR);
	//���������̣�ֱ���пͻ�����������Ϊֹ
	sockClient = accept(sockServer, (SOCKADDR*)&addrClient, &len);
	char recvBuf[16];
	char recvBuf_1[1];
	Mat img_decode;
	std::vector<uchar> data;

	while (1)
	{
		if (recv(sockClient, recvBuf, 16, 0))
		{
			for (int i = 0; i < 16; i++)
			{
				if (recvBuf[i]<'0' || recvBuf[i]>'9') recvBuf[i] = ' ';
			}
			data.resize(atoi(recvBuf));
			for (int i = 0; i < atoi(recvBuf); i++)
			{
				recv(sockClient, recvBuf_1, 1, 0);
				data[i] = recvBuf_1[0];
			}
			printf("Image recieved successfully!\n");
			send(sockClient, "Server has recieved messages!", 29, 0);
			img_decode = imdecode(data, CV_LOAD_IMAGE_COLOR);
			imshow("server", img_decode);
			if (waitKey(5) == 27) break;
		}
	}
	closesocket(sockClient);
	WSACleanup();

}