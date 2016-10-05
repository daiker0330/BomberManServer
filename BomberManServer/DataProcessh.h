#pragma once
#include "OnlineData.h"
#include "Message.h"
#include <sql.h>
#include <sqlext.h>
#include <odbcss.h>
#include <sstream>
#include "GameHost.h"
#include "StdAfx.h"
#include "SeedManager.h"

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
	
	bool Init();
	bool InitDB();
	void DeInit();

private:
	OnlineData onlineData;

	SQLHENV  henv;		// Define the environment handle
	SQLHDBC  hdbc1;		// Define the database connection handle  

	RETCODE retcode;

	CGameHost game_host[MAX_ROOMS+1];
	CSeedManager seed_manager;
	CRITICAL_SECTION cs;
};



