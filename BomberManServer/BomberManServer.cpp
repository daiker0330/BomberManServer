// BomberManServer.cpp : 定义控制台应用程序的入口点。
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

	//err = WSAStartup(wVersionRequested, &wsaData);//该函数的功能是加载一个Winsocket库版本
	//if (err != 0) {
	//	return 0;
	//}


	//if (LOBYTE(wsaData.wVersion) != 1 ||
	//	HIBYTE(wsaData.wVersion) != 1) {
	//	WSACleanup();
	//	return 0;
	//}

	////真正socket编程部分
	//SOCKET sockSrv = socket(AF_INET, SOCK_STREAM, 0);//面向连接的可靠性服务SOCK_STRAM

	//SOCKADDR_IN addrSrv;//存放本地地址信息的
	//addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//htol将主机字节序long型转换为网络字节序
	//addrSrv.sin_family = AF_INET;
	//addrSrv.sin_port = htons(4161);//htos用来将端口转换成字符，1024以上的数字即可

	//bind(sockSrv, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));//将socket绑定到相应地址和端口上

	//listen(sockSrv, 20);//等待队列中的最大长度为5

	//SOCKADDR_IN addrClient;
	//int len = sizeof(SOCKADDR);

	//while (1)
	//{
	//	SOCKET sockConn = accept(sockSrv, (SOCKADDR*)&addrClient, &len);//建立一个新的套接字用于通信，不是前面的监听套接字
	//	char sendBuf[100];
	//	sprintf_s(sendBuf, "Server IP is:%s",
	//	inet_ntoa(addrClient.sin_addr));//inet_nota函数是将字符转换成ip地址
	//	send(sockConn, sendBuf, strlen(sendBuf) + 1, 0);//服务器向客户端发送数据

	//	char recvBuf[100];
	//	recv(sockConn, recvBuf, 100, 0);//服务器从客户端接受数据
	//	printf("%s\n", recvBuf);
	//	closesocket(sockConn);//关闭socket
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

