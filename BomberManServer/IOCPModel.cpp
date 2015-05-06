#pragma once
#include "StdAfx.h"
#include "IOCPModel.h"


#define WORKER_THREADS_PER_PROCESSOR 2
// 同时投递的Accept请求的数量
#define MAX_POST_ACCEPT              10
// 传递给Worker线程的退出信号
#define EXIT_CODE                    NULL


// 释放指针和句柄资源的宏

// 释放指针宏
#define RELEASE(x)                      {if(x != NULL ){delete x;x=NULL;}}
// 释放句柄宏
#define RELEASE_HANDLE(x)               {if(x != NULL && x!=INVALID_HANDLE_VALUE){ CloseHandle(x);x = NULL;}}
// 释放Socket宏
#define RELEASE_SOCKET(x)               {if(x !=INVALID_SOCKET) { closesocket(x);x=INVALID_SOCKET;}}

using namespace std;

CIOCPModel::CIOCPModel(void) :
m_nThreads(0),
m_hShutdownEvent(NULL),
m_hIOCompletionPort(NULL),
m_phWorkerThreads(NULL),
m_nPort(DEFAULT_PORT),
m_lpfnAcceptEx(NULL),
m_pListenContext(NULL)
{
	m_strIP = "127.0.0.1";
}

CIOCPModel::~CIOCPModel(void)
{
	// 确保资源彻底释放
	this->Stop();
}

///////////////////////////////////////////////////////////////////
// 工作者线程：  为IOCP请求服务的工作者线程
DWORD WINAPI CIOCPModel::_WorkerThread(LPVOID lpParam)
{
	THREADPARAMS_WORKER* pParam = (THREADPARAMS_WORKER*)lpParam;
	CIOCPModel* pIOCPModel = (CIOCPModel*)pParam->pIOCPModel;
	int nThreadNo = (int)pParam->nThreadNo;

	//printf("thread start up, ID: %d.", nThreadNo);

	OVERLAPPED           *pOverlapped = NULL;
	PER_SOCKET_CONTEXT   *pSocketContext = NULL;
	DWORD                dwBytesTransfered = 0;

	// 循环处理请求，直到接收到Shutdown信息为止
	while (WAIT_OBJECT_0 != WaitForSingleObject(pIOCPModel->m_hShutdownEvent, 0))
	{
		BOOL bReturn = GetQueuedCompletionStatus(
			pIOCPModel->m_hIOCompletionPort,
			&dwBytesTransfered,
			(PULONG_PTR)&pSocketContext,
			&pOverlapped,
			INFINITE);

		// 如果收到的是退出标志，则直接退出
		if (EXIT_CODE == (DWORD)pSocketContext)
		{
			break;
		}

		// 判断是否出现了错误
		if (!bReturn)
		{
			DWORD dwErr = GetLastError();

			// 显示一下提示信息
			if (!pIOCPModel->HandleError(pSocketContext, dwErr))
			{
				break;
			}

			continue;
		}
		else
		{
			// 读取传入的参数
			PER_IO_CONTEXT* pIoContext = CONTAINING_RECORD(pOverlapped, PER_IO_CONTEXT, m_Overlapped);

			// 判断是否有客户端断开了
			if ((0 == dwBytesTransfered) && (RECV_POSTED == pIoContext->m_OpType || SEND_POSTED == pIoContext->m_OpType))
			{
				printf("client %s:%d disconnect.\n", inet_ntoa(pSocketContext->m_ClientAddr.sin_addr), ntohs(pSocketContext->m_ClientAddr.sin_port));

				// 释放掉对应的资源
				pIOCPModel->_RemoveContext(pSocketContext);

				continue;
			}
			else
			{
				switch (pIoContext->m_OpType)
				{
					// Accept  
					case ACCEPT_POSTED:
					{
						// 处理连入请求
						pIOCPModel->_DoAccpet(pSocketContext, pIoContext);
					}
					break;

					// RECV
					case RECV_POSTED:
					{
						// 处理接收请求
						pIOCPModel->_DoRecv(pSocketContext, pIoContext);
					}
					break;

					// SEND
					case SEND_POSTED:
					{

					}
					break;
				default:
					printf("_WorkThread中的 pIoContext->m_OpType 参数异常.\n");
					break;
				} //switch
			}//if
		}//if

	}//while

	printf("工作者线程 %d 号退出.\n", nThreadNo);

	// 释放线程参数
	RELEASE(lpParam);

	return 0;
}



//====================================================================================
//
//				    系统初始化和终止
//
//====================================================================================




////////////////////////////////////////////////////////////////////
// 初始化WinSock 2.2
bool CIOCPModel::LoadSocketLib()
{
	WSADATA wsaData;
	int nResult;
	nResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	// 错误
	if (NO_ERROR != nResult)
	{
		printf("init WinSock 2.2 fail !\n");
		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////
//	启动服务器
bool CIOCPModel::Start()
{
	// 初始化线程互斥量
	InitializeCriticalSection(&m_csContextList);

	// 建立系统退出的事件通知
	m_hShutdownEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	// 初始化IOCP
	if (false == _InitializeIOCP())
	{
		printf("init IOCP fail !\n");
		return false;
	}
	else
	{
		printf("\nIOCP init finish.\n");
	}

	// 初始化Socket
	if (false == _InitializeListenSocket())
	{
		printf("Listen Socket init fail !\n");
		this->_DeInitialize();
		return false;
	}
	else
	{
		printf("Listen Socket init finish.\n");
	}

	printf("system ready, waiting....\n");

	return true;
}


////////////////////////////////////////////////////////////////////
//	开始发送系统退出消息，退出完成端口和线程资源
void CIOCPModel::Stop()
{
	if (m_pListenContext != NULL && m_pListenContext->m_Socket != INVALID_SOCKET)
	{
		// 激活关闭消息通知
		SetEvent(m_hShutdownEvent);

		for (int i = 0; i < m_nThreads; i++)
		{
			// 通知所有的完成端口操作退出
			PostQueuedCompletionStatus(m_hIOCompletionPort, 0, (DWORD)EXIT_CODE, NULL);
		}

		// 等待所有的客户端资源退出
		WaitForMultipleObjects(m_nThreads, m_phWorkerThreads, TRUE, INFINITE);

		// 清除客户端列表信息
		this->_ClearContextList();

		// 释放其他资源
		this->_DeInitialize();

		printf("stop listening\n");
	}
}


////////////////////////////////
// 初始化完成端口
bool CIOCPModel::_InitializeIOCP()
{
	// 建立第一个完成端口
	m_hIOCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	if (NULL == m_hIOCompletionPort)
	{
		printf("start complication port fail! error code: %d!\n", WSAGetLastError());
		return false;
	}

	// 根据本机中的处理器数量，建立对应的线程数
	m_nThreads = WORKER_THREADS_PER_PROCESSOR * _GetNoOfProcessors();

	// 为工作者线程初始化句柄
	m_phWorkerThreads = new HANDLE[m_nThreads];

	// 根据计算出来的数量建立工作者线程
	DWORD nThreadID;
	for (int i = 0; i < m_nThreads; i++)
	{
		THREADPARAMS_WORKER* pThreadParams = new THREADPARAMS_WORKER;
		pThreadParams->pIOCPModel = this;
		pThreadParams->nThreadNo = i + 1;
		m_phWorkerThreads[i] = ::CreateThread(0, 0, _WorkerThread, (void *)pThreadParams, 0, &nThreadID);
	}

	printf(" creat _WorkerThread %d.\n", m_nThreads);

	return true;
}


/////////////////////////////////////////////////////////////////
// 初始化Socket
bool CIOCPModel::_InitializeListenSocket()
{
	// AcceptEx 和 GetAcceptExSockaddrs 的GUID，用于导出函数指针
	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	GUID GuidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;

	// 服务器地址信息，用于绑定Socket
	struct sockaddr_in ServerAddress;

	// 生成用于监听的Socket的信息
	m_pListenContext = new PER_SOCKET_CONTEXT;

	// 需要使用重叠IO，必须得使用WSASocket来建立Socket，才可以支持重叠IO操作
	m_pListenContext->m_Socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == m_pListenContext->m_Socket)
	{
		printf("init Socket fail, error code: %d.\n", WSAGetLastError());
		return false;
	}
	else
	{
		printf("WSASocket() finish.\n");
	}

	// 将Listen Socket绑定至完成端口中
	if (NULL == CreateIoCompletionPort((HANDLE)m_pListenContext->m_Socket, m_hIOCompletionPort, (DWORD)m_pListenContext, 0))
	{
		printf("bind Listen Socket fail! error code: %d\n", WSAGetLastError());
		RELEASE_SOCKET(m_pListenContext->m_Socket);
		return false;
	}
	else
	{
		printf("Listen Socket bind success.\n");
	}

	// 填充地址信息
	ZeroMemory((char *)&ServerAddress, sizeof(ServerAddress));
	ServerAddress.sin_family = AF_INET;
	// 这里可以绑定任何可用的IP地址，或者绑定一个指定的IP地址 
	//ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	size_t len = m_strIP.size() + 1;
	//size_t converted = 0;
	//char *CStr;
	//CStr = (char*)malloc(len*sizeof(char));
	//wcstombs_s(&converted, CStr, len, m_strIP.c_str(), _TRUNCATE);
	ServerAddress.sin_addr.s_addr = inet_addr(m_strIP.c_str());
	ServerAddress.sin_port = htons(m_nPort);

	// 绑定地址和端口
	if (SOCKET_ERROR == bind(m_pListenContext->m_Socket, (struct sockaddr *) &ServerAddress, sizeof(ServerAddress)))
	{
		printf("bind() do error.\n");
		return false;
	}
	else
	{
		printf("bind() finish.\n");
	}

	// 开始进行监听
	if (SOCKET_ERROR == listen(m_pListenContext->m_Socket, SOMAXCONN))
	{
		printf("Listen() do error.\n");
		return false;
	}
	else
	{
		printf("Listen() finish.\n");
	}

	// 获取AcceptEx函数指针
	DWORD dwBytes = 0;
	if (SOCKET_ERROR == WSAIoctl(
		m_pListenContext->m_Socket,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidAcceptEx,
		sizeof(GuidAcceptEx),
		&m_lpfnAcceptEx,
		sizeof(m_lpfnAcceptEx),
		&dwBytes,
		NULL,
		NULL))
	{
		printf("WSAIoctl can not get AcceptEx pointer. error code: %d\n", WSAGetLastError());
		this->_DeInitialize();
		return false;
	}

	// 获取GetAcceptExSockAddrs函数指针
	if (SOCKET_ERROR == WSAIoctl(
		m_pListenContext->m_Socket,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidGetAcceptExSockAddrs,
		sizeof(GuidGetAcceptExSockAddrs),
		&m_lpfnGetAcceptExSockAddrs,
		sizeof(m_lpfnGetAcceptExSockAddrs),
		&dwBytes,
		NULL,
		NULL))
	{
		printf("WSAIoctl can not get GuidGetAcceptExSockAddrs pointer. error code %d\n", WSAGetLastError());
		this->_DeInitialize();
		return false;
	}


	// 为AcceptEx 准备参数，然后投递AcceptEx I/O请求
	for (int i = 0; i<MAX_POST_ACCEPT; i++)
	{
		// 新建一个IO_CONTEXT
		PER_IO_CONTEXT* pAcceptIoContext = m_pListenContext->GetNewIoContext();

		if (false == this->_PostAccept(pAcceptIoContext))
		{
			m_pListenContext->RemoveContext(pAcceptIoContext);
			return false;
		}
	}

	printf("post %d AcceptEx finish\n", MAX_POST_ACCEPT);

	return true;
}

////////////////////////////////////////////////////////////
//	最后释放掉所有资源
void CIOCPModel::_DeInitialize()
{
	// 删除客户端列表的互斥量
	DeleteCriticalSection(&m_csContextList);

	// 关闭系统退出事件句柄
	RELEASE_HANDLE(m_hShutdownEvent);

	// 释放工作者线程句柄指针
	for (int i = 0; i<m_nThreads; i++)
	{
		RELEASE_HANDLE(m_phWorkerThreads[i]);
	}

	RELEASE(m_phWorkerThreads);

	// 关闭IOCP句柄
	RELEASE_HANDLE(m_hIOCompletionPort);

	// 关闭监听Socket
	RELEASE(m_pListenContext);

	printf("release finish.\n");

	
}


//====================================================================================
//
//				    投递完成端口请求
//
//====================================================================================


//////////////////////////////////////////////////////////////////
// 投递Accept请求
bool CIOCPModel::_PostAccept(PER_IO_CONTEXT* pAcceptIoContext)
{
	//ASSERT(INVALID_SOCKET != m_pListenContext->m_Socket);

	// 准备参数
	DWORD dwBytes = 0;
	pAcceptIoContext->m_OpType = ACCEPT_POSTED;
	WSABUF *p_wbuf = &pAcceptIoContext->m_wsaBuf;
	OVERLAPPED *p_ol = &pAcceptIoContext->m_Overlapped;

	// 为以后新连入的客户端先准备好Socket
	pAcceptIoContext->m_sockAccept = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == pAcceptIoContext->m_sockAccept)
	{
		printf("creat use for Accept的Socket fail! error code: %d\n", WSAGetLastError());
		return false;
	}

	// 投递AcceptEx
	if (FALSE == m_lpfnAcceptEx(m_pListenContext->m_Socket, pAcceptIoContext->m_sockAccept, p_wbuf->buf, p_wbuf->len - ((sizeof(SOCKADDR_IN)+16) * 2),
		sizeof(SOCKADDR_IN)+16, sizeof(SOCKADDR_IN)+16, &dwBytes, p_ol))
	{
		if (WSA_IO_PENDING != WSAGetLastError())
		{
			printf("post AcceptEx fail, error code: %d\n", WSAGetLastError());
			return false;
		}
	}

	return true;
}

////////////////////////////////////////////////////////////
// 在有客户端连入的时候，进行处理
bool CIOCPModel::_DoAccpet(PER_SOCKET_CONTEXT* pSocketContext, PER_IO_CONTEXT* pIoContext)
{
	SOCKADDR_IN* ClientAddr = NULL;
	SOCKADDR_IN* LocalAddr = NULL;
	int remoteLen = sizeof(SOCKADDR_IN), localLen = sizeof(SOCKADDR_IN);

	///////////////////////////////////////////////////////////////////////////
	// 首先取得连入客户端的地址信息
	this->m_lpfnGetAcceptExSockAddrs(pIoContext->m_wsaBuf.buf, pIoContext->m_wsaBuf.len - ((sizeof(SOCKADDR_IN)+16) * 2),
		sizeof(SOCKADDR_IN)+16, sizeof(SOCKADDR_IN)+16, (LPSOCKADDR*)&LocalAddr, &localLen, (LPSOCKADDR*)&ClientAddr, &remoteLen);
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// 将ListenSocket上的Context复制出来一份为新连入的Socket新建一个SocketContext

	PER_SOCKET_CONTEXT* pNewSocketContext = new PER_SOCKET_CONTEXT;
	pNewSocketContext->m_Socket = pIoContext->m_sockAccept;
	memcpy(&(pNewSocketContext->m_ClientAddr), ClientAddr, sizeof(SOCKADDR_IN));

	// 参数设置完毕，将这个Socket和完成端口绑定(这也是一个关键步骤)
	if (false == this->_AssociateWithIOCP(pNewSocketContext))
	{
		RELEASE(pNewSocketContext);
		return false;
	}


	///////////////////////////////////////////////////////////////////////////////////////////////////
	// 建立其下的IoContext，用于在这个Socket上投递第一个Recv数据请求
	PER_IO_CONTEXT* pNewIoContext = pNewSocketContext->GetNewIoContext();
	pNewIoContext->m_OpType = RECV_POSTED;
	pNewIoContext->m_sockAccept = pNewSocketContext->m_Socket;

	// 如果Buffer需要保留
	//memcpy( pNewIoContext->m_szBuffer,pIoContext->m_szBuffer,MAX_BUFFER_LEN );

	if (false == this->_PostRecv(pNewIoContext))
	{
		pNewSocketContext->RemoveContext(pNewIoContext);
		return false;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////
	// 把这个有效的客户端信息，加入到ContextList中去
	this->_AddToContextList(pNewSocketContext);

	string msg = "0";
	send(pNewSocketContext->m_Socket, msg.c_str(), strlen(msg.c_str()), 0);

	////////////////////////////////////////////////////////////////////////////////////////////////
	// 把Listen Socket的那个IoContext重置，准备投递新的AcceptEx
	pIoContext->ResetBuffer();
	return this->_PostAccept(pIoContext);
}

////////////////////////////////////////////////////////////////////
// 投递接收数据请求
bool CIOCPModel::_PostRecv(PER_IO_CONTEXT* pIoContext)
{
	// 初始化变量
	DWORD dwFlags = 0;
	DWORD dwBytes = 0;
	WSABUF *p_wbuf = &pIoContext->m_wsaBuf;
	OVERLAPPED *p_ol = &pIoContext->m_Overlapped;

	pIoContext->ResetBuffer();
	pIoContext->m_OpType = RECV_POSTED;

	// 初始化完成后，，投递WSARecv请求
	int nBytesRecv = WSARecv(pIoContext->m_sockAccept, p_wbuf, 1, &dwBytes, &dwFlags, p_ol, NULL);

	// 重叠请求失败
	if ((SOCKET_ERROR == nBytesRecv) && (WSA_IO_PENDING != WSAGetLastError()))
	{
		printf("post WSARecv fail!\n");
		return false;
	}
	return true;
}

/////////////////////////////////////////////////////////////////
// 在有接收的数据到达的时候，进行处理
bool CIOCPModel::_DoRecv(PER_SOCKET_CONTEXT* pSocketContext, PER_IO_CONTEXT* pIoContext)
{
	// 先把上一次的数据显示出现，然后就重置状态，发出下一个Recv请求
	SOCKADDR_IN* ClientAddr = &pSocketContext->m_ClientAddr;
	
	/////////////////////////////////////////////////////////////////
	// 编写处理消息流程

	//printf("receive  %s:%d message:%s\n", inet_ntoa(ClientAddr->sin_addr), ntohs(ClientAddr->sin_port), pIoContext->m_wsaBuf.buf);
	
	CMessage msg,*recv_msg;
	string user = "daiker", psd = "12345", msg_str;

	//创建消息指针,获取消息内容
	recv_msg = (CMessage*)pIoContext->m_szBuffer;

	//处理消息
	switch (recv_msg->type1)
	{
		case MSG_SCENE:
		{
		
		}
		case MSG_LOGIN:
		{
			msg = dataProcess->Login(recv_msg);
			break;
		}
		case MSG_GAME:
		{
			msg = dataProcess->Game(recv_msg);
			break;
		}
		case MSG_ROOM:
		{
			msg = dataProcess->Room(recv_msg);
			break;
		}
		case MSG_LOBBY:
		{
			msg = dataProcess->Lobby(recv_msg);
			break;
		}
	}
	
	//回复消息
	send(pSocketContext->m_Socket, (char*)&msg, sizeof(CMessage), 0);

	//printf("type1:%d\n", recv_msg->type1);
	//printf("type2:%d\n", recv_msg->type2);
	//printf("str1:%s\n", recv_msg->str1);
	//printf("str2:%s\n", recv_msg->str2);
	//printf("msg:%s\n", recv_msg->msg);

	// 然后开始投递下一个WSARecv请求
	return _PostRecv(pIoContext);
}



/////////////////////////////////////////////////////
// 将句柄(Socket)绑定到完成端口中
bool CIOCPModel::_AssociateWithIOCP(PER_SOCKET_CONTEXT *pContext)
{
	// 将用于和客户端通信的SOCKET绑定到完成端口中
	HANDLE hTemp = CreateIoCompletionPort((HANDLE)pContext->m_Socket, m_hIOCompletionPort, (DWORD)pContext, 0);

	if (NULL == hTemp)
	{
		printf("run CreateIoCompletionPort() error. error code: %d\n", GetLastError());
		return false;
	}

	return true;
}




//====================================================================================
//
//				    ContextList 相关操作
//
//====================================================================================


//////////////////////////////////////////////////////////////
// 将客户端的相关信息存储到数组中
void CIOCPModel::_AddToContextList(PER_SOCKET_CONTEXT *pHandleData)
{
	EnterCriticalSection(&m_csContextList);

	m_arrayClientContext.push_back(pHandleData);

	LeaveCriticalSection(&m_csContextList);
}

////////////////////////////////////////////////////////////////
//	移除某个特定的Context
void CIOCPModel::_RemoveContext(PER_SOCKET_CONTEXT *pSocketContext)
{
	EnterCriticalSection(&m_csContextList);

	for (int i = 0; i<m_arrayClientContext.size(); i++)
	{
		if (pSocketContext == m_arrayClientContext.at(i))
		{
			RELEASE(pSocketContext);
			vector <PER_SOCKET_CONTEXT*>::iterator iter = m_arrayClientContext.begin();
			for (int j = 0; j < i; j++)
			{
				iter++;
			}
			m_arrayClientContext.erase(iter);
			//m_arrayClientContext.RemoveAt(i);
			break;
		}
	}

	LeaveCriticalSection(&m_csContextList);
}

////////////////////////////////////////////////////////////////
// 清空客户端信息
void CIOCPModel::_ClearContextList()
{
	EnterCriticalSection(&m_csContextList);

	for (int i = 0; i<m_arrayClientContext.size(); i++)
	{
		delete m_arrayClientContext.at(i);
	}

	m_arrayClientContext.clear();

	LeaveCriticalSection(&m_csContextList);
}



//====================================================================================
//
//				       其他辅助函数定义
//
//====================================================================================



////////////////////////////////////////////////////////////////////
// 获得本机的IP地址
string CIOCPModel::GetLocalIP()
{
	// 获得本机主机名
	char hostname[MAX_PATH] = { 0 };
	gethostname(hostname, MAX_PATH);
	struct hostent FAR* lpHostEnt = gethostbyname(hostname);
	if (lpHostEnt == NULL)
	{
		return DEFAULT_IP;
	}

	// 取得IP地址列表中的第一个为返回的IP(因为一台主机可能会绑定多个IP)
	LPSTR lpAddr = lpHostEnt->h_addr_list[0];

	// 将IP地址转化成字符串形式
	struct in_addr inAddr;
	memmove(&inAddr, lpAddr, 4);
	m_strIP = string(inet_ntoa(inAddr));

	return m_strIP;
}

///////////////////////////////////////////////////////////////////
// 获得本机中处理器的数量
int CIOCPModel::_GetNoOfProcessors()
{
	SYSTEM_INFO si;

	GetSystemInfo(&si);

	return si.dwNumberOfProcessors;
}

/////////////////////////////////////////////////////////////////////
// 判断客户端Socket是否已经断开，否则在一个无效的Socket上投递WSARecv操作会出现异常
// 使用的方法是尝试向这个socket发送数据，判断这个socket调用的返回值
// 因为如果客户端网络异常断开(例如客户端崩溃或者拔掉网线等)的时候，服务器端是无法收到客户端断开的通知的

bool CIOCPModel::_IsSocketAlive(SOCKET s)
{
	int nByteSent = send(s, "", 0, 0);
	if (-1 == nByteSent) return false;
	return true;
}

///////////////////////////////////////////////////////////////////
// 显示并处理完成端口上的错误
bool CIOCPModel::HandleError(PER_SOCKET_CONTEXT *pContext, const DWORD& dwErr)
{
	// 如果是超时了，就再继续等吧  
	if (WAIT_TIMEOUT == dwErr)
	{
		// 确认客户端是否还活着...
		if (!_IsSocketAlive(pContext->m_Socket))
		{
			printf("client error finish!\n");
			this->_RemoveContext(pContext);
			return true;
		}
		else
		{
			printf("network timeout! trying...\n");
			return true;
		}
	}

	// 可能是客户端异常退出了
	else if (ERROR_NETNAME_DELETED == dwErr)
	{
		printf("client error finish!!\n");
		this->_RemoveContext(pContext);
		return true;
	}

	else
	{
		printf("complication port error, thread killed. error code: %d\n", dwErr);
		return false;
	}
}




