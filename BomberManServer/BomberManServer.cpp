// BomberManServer.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"

using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{
	
	CIOCPModel m_IOCP;
	char s;

	if (false == m_IOCP.LoadSocketLib())
	{
		printf("load Socket lib error\n");
		exit(0);
	}

	if (false == m_IOCP.Start())
	{
		printf("start Socket error\n");
		exit(0);
	}

	while (1)
	{
		cin >> s;
		if (s == 'q')
		{
			break;
		}
	}

	m_IOCP.Stop();

	//WORD wVersionRequested;
	//WSADATA wsaData;
	//int err;

	//wVersionRequested = MAKEWORD(1, 1);

	//err = WSAStartup(wVersionRequested, &wsaData);//�ú����Ĺ����Ǽ���һ��Winsocket��汾
	//if (err != 0) {
	//	return 0;
	//}


	//if (LOBYTE(wsaData.wVersion) != 1 ||
	//	HIBYTE(wsaData.wVersion) != 1) {
	//	WSACleanup();
	//	return 0;
	//}

	////����socket��̲���
	//SOCKET sockSrv = socket(AF_INET, SOCK_STREAM, 0);//�������ӵĿɿ��Է���SOCK_STRAM

	//SOCKADDR_IN addrSrv;//��ű��ص�ַ��Ϣ��
	//addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//htol�������ֽ���long��ת��Ϊ�����ֽ���
	//addrSrv.sin_family = AF_INET;
	//addrSrv.sin_port = htons(4161);//htos�������˿�ת�����ַ���1024���ϵ����ּ���

	//bind(sockSrv, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));//��socket�󶨵���Ӧ��ַ�Ͷ˿���

	//listen(sockSrv, 20);//�ȴ������е���󳤶�Ϊ5

	//SOCKADDR_IN addrClient;
	//int len = sizeof(SOCKADDR);

	//while (1)
	//{
	//	SOCKET sockConn = accept(sockSrv, (SOCKADDR*)&addrClient, &len);//����һ���µ��׽�������ͨ�ţ�����ǰ��ļ����׽���
	//	char sendBuf[100];
	//	sprintf_s(sendBuf, "Server IP is:%s",
	//	inet_ntoa(addrClient.sin_addr));//inet_nota�����ǽ��ַ�ת����ip��ַ
	//	send(sockConn, sendBuf, strlen(sendBuf) + 1, 0);//��������ͻ��˷�������

	//	char recvBuf[100];
	//	recv(sockConn, recvBuf, 100, 0);//�������ӿͻ��˽�������
	//	printf("%s\n", recvBuf);
	//	closesocket(sockConn);//�ر�socket
	//}

	/*int sockfd;
	int client_fd;
	struct sockaddr_in my_addr;
	struct sockaddr_in remote_addr;
	char * msg = "Hellow world!\n";
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		printf("scoket creat error!\n");
		exit(1);
	}
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(SERVPORT);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	memset(&(my_addr.sin_zero), 0, 8);
	if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
	{
		printf("bind error\n");
		exit(1);
	}
	if (listen(sockfd, BACKLOG) == -1)
	{
		printf("listen error\n");
		exit(1);
	}
	while (1)
	{
		int sin_size = sizeof(struct sockaddr_in);
		if ((client_fd = accept(sockfd, (struct sockaddr *)&remote_addr, &sin_size)) == -1)
		{
			printf("accept error\n");
			continue;
		}
		if (send(client_fd, msg, strlen(msg), 0) == -1)
		{
			printf("send error\n");
			shutdown(client_fd,2);
			exit(0);
		}
		shutdown(client_fd, 2);
	}*/

	return 0;
}

