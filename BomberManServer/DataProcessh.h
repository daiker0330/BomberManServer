#pragma once
#include "OnlineData.h"
#include "Message.h"
#include <sql.h>
#include <sqlext.h>
#include <odbcss.h>

#pragma comment(lib, "ODBC32.lib")

using namespace std;

class Dataprocess
{
public:
	CMessage Login(CMessage* recv_msg);
	CMessage Room(CMessage* recv_msg);
	CMessage Lobby(CMessage* recv_msg);
	string GetName(int id);
	bool InitDB();
	void DeInit();

private:
	OnlineData onlineData;

	SQLHENV  henv;//���廷�����
	SQLHDBC  hdbc1;//�������ݿ����Ӿ��     

	RETCODE retcode;
};