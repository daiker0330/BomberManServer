#pragma once
#include "OnlineData.h"
#include "Message.h"
#include <sql.h>
#include <sqlext.h>
#include <odbcss.h>
#include <sstream>
#include "GameHost.h"
#include "StdAfx.h"

#pragma comment(lib, "ODBC32.lib")

using namespace std;

class Dataprocess
{
public:
	CMessage Login(CMessage* recv_msg, SOCKADDR_IN* socket);
	CMessage Room(CMessage* recv_msg);
	CMessage Lobby(CMessage* recv_msg);
	CMessage Game(CMessage* recv_msg);
	CMessage Chat(CMessage* recv_msg);
	CMessage Data(CMessage* recv_msg);
	void Disconnect(string ip,int port);
	string GetName(int id);
	bool InitDB();
	void DeInit();

private:
	OnlineData onlineData;

	SQLHENV  henv;//定义环境句柄
	SQLHDBC  hdbc1;//定义数据库连接句柄     

	RETCODE retcode;

	CGameHost game_host[MAX_ROOMS+1];
};

