#include "DealConnect.h"
#include <Winsock2.h>
#include <cstdio>
#include "opencv.hpp"
#pragma comment(lib,"ws2_32.lib")
using namespace std;
using namespace cv;
DealConnect::DealConnect(QWidget * parent)
{
	ui.setupUi(this);
	connect(ui.connect, &QPushButton::clicked, this, &DealConnect::StartConnect);
}

DealConnect::~DealConnect()
{
}

void DealConnect::closeEvent(QCloseEvent *)
{
	emit ExitWin();
}

void DealConnect::StartConnect()
{
	WSADATA wsaData;
	SOCKET sockClient;//�ͻ���Socket
	SOCKADDR_IN addrServer;//����˵�ַ
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	//�½��ͻ���socket
	sockClient = socket(AF_INET, SOCK_STREAM, 0);
	//����Ҫ���ӵķ���˵�ַ
	addrServer.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");//Ŀ��IP(10.106.20.74�ǻ��͵�ַ)
	addrServer.sin_family = AF_INET;
	addrServer.sin_port = htons(6000);//���Ӷ˿�
	//���ӵ������
	::connect(sockClient, (SOCKADDR*)&addrServer, sizeof(SOCKADDR));

	Mat image;
	VideoCapture capture(0);
	vector<uchar> data_encode;

	while (1)
	{
		if (!capture.read(image))
			break;
		imencode(".jpg", image, data_encode);
		int len_encode = data_encode.size();
		string len = to_string(len_encode);
		int length = len.length();
		for (int i = 0; i < 16 - length; i++)
		{
			len = len + " ";
		}
		//��������
		send(sockClient, len.c_str(), strlen(len.c_str()), 0);
		char send_char[1];
		for (int i = 0; i < len_encode; i++)
		{
			send_char[0] = data_encode[i];
			send(sockClient, send_char, 1, 0);
		}
		//���շ�����Ϣ
		char recvBuf[32];
		if (recv(sockClient, recvBuf, 32, 0))
			printf("%s\n", recvBuf);
		imshow("client", image);
		waitKey(5);
	}
	closesocket(sockClient);
	WSACleanup();
}
