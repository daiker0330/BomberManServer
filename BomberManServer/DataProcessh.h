#pragma once
#include "OnlineData.h"
#include "Message.h"
#include <sql.h>
#include <sqlext.h>
#include <odbcss.h>
#include <sstream>

#pragma comment(lib, "ODBC32.lib")

using namespace std;

class Dataprocess
{
public:
	CMessage Login(CMessage* recv_msg);
	CMessage Room(CMessage* recv_msg);
	CMessage Lobby(CMessage* recv_msg);
	CMessage Game(CMessage* recv_msg);
	string GetName(int id);
	bool InitDB();
	void DeInit();

private:
	OnlineData onlineData;

	SQLHENV  henv;//定义环境句柄
	SQLHDBC  hdbc1;//定义数据库连接句柄     

	RETCODE retcode;
};

