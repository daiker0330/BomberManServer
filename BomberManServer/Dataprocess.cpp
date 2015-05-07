#pragma once
#include "StdAfx.h"
#include "DataProcessh.h"

using namespace std;

bool Dataprocess::InitDB()
{
	henv = SQL_NULL_HENV;//定义环境句柄
	hdbc1 = SQL_NULL_HDBC;//定义数据库连接句柄     


	retcode = SQLAllocHandle(SQL_HANDLE_ENV, NULL, &henv);

	if (retcode < 0)//错误处理
	{
		cout << "allocate ODBC Environment handle errors." << endl;
		return false;
	}
	// Notify ODBC that this is an ODBC 3.0 application.
	retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION,
		(SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER);
	if (retcode < 0) //错误处理
	{
		cout << "the  ODBC is not version3.0 " << endl;
		return false;
	}

	// Allocate an ODBC connection and connect.
	retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc1);
	if (retcode < 0) //错误处理
	{
		cout << "allocate ODBC connection handle errors." << endl;
		return false;
	}
	//Data Source Name must be of type User DNS or System DNS
	wchar_t* szDSN = L"BombManServer";
	wchar_t* szUID = L"daiker";//log name
	wchar_t* szAuthStr = L"12345";//passward
	//connect to the Data Source
	retcode = SQLConnect(hdbc1, (SQLWCHAR*)szDSN, (SWORD)wcslen(szDSN), (SQLWCHAR*)szUID, (SWORD)wcslen(szUID), (SQLWCHAR*)szAuthStr, (SWORD)wcslen(szAuthStr));
	if (retcode < 0) //错误处理
	{
		cout << "connect to  ODBC datasource errors." << endl;
		return false;
	}

	SQLHSTMT  hstmt1 = SQL_NULL_HSTMT;//定义语句句柄
	// Allocate a statement handle.
	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc1, &hstmt1);
	if (retcode < 0) //错误处理
	{
		cout << "allocate ODBC statement handle errors." << endl;
		return false;
	}

	retcode = SQLExecDirect(hstmt1, (SQLWCHAR*)L"SELECT id,user_name,user_psd,money,VIP  FROM account", SQL_NTS);
	if (retcode < 0)
	{
		cout << "Executing statement  throught ODBC  errors." << endl;
		return -1;
	}

	// SQLBindCol variables
	SQLINTEGER		id;
	SQLCHAR			user_name[MaxNameLen + 1];
	SQLCHAR			user_password[MaxNameLen + 1];
	SQLINTEGER		money;
	SQLINTEGER		VIP;
	SQLINTEGER		columnLen = 0;//数据库定义中该属性列的长度

	while (1)
	{
		retcode = SQLFetch(hstmt1);
		if (retcode == SQL_NO_DATA)
			break;

		retcode = SQLGetData(hstmt1, 1, SQL_C_LONG, &id, 0, &columnLen);
		retcode = SQLGetData(hstmt1, 2, SQL_C_CHAR, user_name, MaxNameLen, &columnLen);
		retcode = SQLGetData(hstmt1, 3, SQL_C_CHAR, user_password, MaxNameLen, &columnLen);
		retcode = SQLGetData(hstmt1, 4, SQL_C_LONG, &money, 0, &columnLen);
		retcode = SQLGetData(hstmt1, 5, SQL_C_LONG, &VIP, 0, &columnLen);

		pair<int, pair<string, string>> tmp_name;
		tmp_name.first = id;
		tmp_name.second.first.append((char *)user_name);
		tmp_name.second.second.append((char *)user_password);
		onlineData.user_name_psd.push_back(tmp_name);

		pair<int, int> tmp_money;
		tmp_money.first = id;
		tmp_money.second = money;
		onlineData.user_money.push_back(tmp_money);

		pair<int, bool> tmp_vip;
		tmp_vip.first = id;
		if (VIP == 1)
		{
			tmp_vip.second = true;
		}
		else
		{
			tmp_vip.second = false;
		}
		onlineData.user_vip.push_back(tmp_vip);
	}
	SQLFreeHandle(SQL_HANDLE_STMT, hstmt1);

	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			onlineData.romm_user[i][j].first = 0;
			onlineData.romm_user[i][j].second = "";
			onlineData.ready[i][j] = false;
		}
	}

	return true;
}

CMessage Dataprocess::Login(CMessage* recv_msg)
{
	CMessage msg;
	msg.type1 = MSG_LOGIN;
	msg.type2 = MSG_NULL;
	if (recv_msg->type2 == MSG_LOGIN_CKECK)
	{
		msg.type2 = MSG_LOGIN_DENY;
		list<pair<int, pair<string, string>>>::iterator p;
		for (p = onlineData.user_name_psd.begin(); p != onlineData.user_name_psd.end(); p++)
		{
			if (strcmp(recv_msg->str1, p->second.first.c_str()) == 0 && strcmp(recv_msg->str2, p->second.second.c_str()) == 0)
			{
				msg.type2 = MSG_LOGIN_CONFIRM;
				msg.para1 = p->first;

				list<pair<int, int>>::iterator p2;
				for (p2 = onlineData.user_money.begin(); p2 != onlineData.user_money.end(); p2++)
				{
					if (p2->first == p->first)
					{
						msg.para2 = p2->second;
					}
				}

				strcpy_s(msg.str1, 20, "NOT");
				list<pair<int, bool>>::iterator p3;
				for (p3 = onlineData.user_vip.begin(); p3 != onlineData.user_vip.end(); p3++)
				{
					if (p3->first == p->first && p3->second == true)
					{
						strcpy_s(msg.str1, 20, "VIP");
					}
				}
			}
		}
	}
	return msg;
}

CMessage Dataprocess::Room(CMessage* recv_msg)
{
	CMessage msg;
	msg.type1 = MSG_ROOM;
	msg.type2 = MSG_NULL;
	if (recv_msg->type2 == MSG_ROOM_TRY)
	{
		int i;
		for (i = 0; i < 4; i++)
		{
			if (onlineData.romm_user[recv_msg->para1][i].first == 0)
			{
				onlineData.romm_user[recv_msg->para1][i].first = recv_msg->para2;
				onlineData.romm_user[recv_msg->para1][i].second = GetName(recv_msg->para2);
				msg.type2 = MSG_ROOM_CONFIRM;
				msg.para1 = i;
				break;
			}
		}
		if (i == 4)
		{
			msg.type2 = MSG_ROOM_DENY;
		}
	}
	else if (recv_msg->type2 == MSG_ROOM_NAME)
	{
		if (onlineData.romm_user[recv_msg->para1][recv_msg->para2].first != 0)
		{
			msg.type2 = MSG_ROOM_RETURN;
			msg.para1 = onlineData.romm_user[recv_msg->para1][recv_msg->para2].first;
			strcpy_s(msg.str1, 20, onlineData.romm_user[recv_msg->para1][recv_msg->para2].second.c_str());
		}
		else
		{
			msg.type2 = MSG_ROOM_EMPTY;
		}
	}
	else if (recv_msg->type2 == MSG_ROOM_EXIT)
	{
		onlineData.romm_user[recv_msg->para1][recv_msg->para2].first = 0;
	}
	else if (recv_msg->type2 == MSG_ROOM_READY)
	{
		onlineData.ready[recv_msg->para1][recv_msg->para2] = true;
		int i;
		for (i = 0; i < 4; i++)
		{
			if (onlineData.ready[recv_msg->para1][i] == false)
			{
				break;
			}
		}
		if (i == 4)
		{
			game_host[recv_msg->para1].Init();
			msg.type2 = MSG_ROOM_GAME;
		}
	}
	return msg;
}

string Dataprocess::GetName(int id)
{
	list<pair<int, pair<string, string>>>::iterator p;
	for (p = onlineData.user_name_psd.begin(); p != onlineData.user_name_psd.end(); p++)
	{
		if (p->first == id)
		{
			return p->second.first;
		}
	}
	return string("");
}

void Dataprocess::DeInit()
{
	/* Clean up.*/
	SQLDisconnect(hdbc1);
	SQLFreeHandle(SQL_HANDLE_DBC, hdbc1);
	SQLFreeHandle(SQL_HANDLE_ENV, henv);
}

CMessage Dataprocess::Lobby(CMessage* recv_msg)
{
	CMessage msg;
	msg.type1 = MSG_LOBBY;
	msg.type2 = MSG_NULL;
	if (recv_msg->type2 == MSG_LOBBY_ROOM)
	{
		int i,num=0;
		msg.type2 = MSG_LOBBY_RETURN;
		for (i = 0; i < 4; i++)
		{
			if (onlineData.romm_user[recv_msg->para1][i].first != 0)
			{
				num++;
			}
		}
		msg.para1 = num;
	}
	return msg;
}

CMessage Dataprocess::Game( CMessage* recv_msg )
{
	CMessage ret;
	int now_roomnum = recv_msg->para1;
	int now_playernum = recv_msg->para2;

	if(recv_msg->type2 == MSG_GAME_OPERATION)
	{
		ret.type1 = MSG_GAME;
		ret.type2 = MSG_GAME_OPERATION;

		while(game_host[now_roomnum].Ready(now_playernum))
		{
			Sleep(1);
		}
		
		game_host[now_roomnum].SetMessage(now_playernum, recv_msg->msg);
		game_host[now_roomnum].SetReady(now_playernum, true);
		game_host[now_roomnum].SetUsed(now_playernum, false);
		while(!game_host[now_roomnum].AllReady())
		{
			Sleep(1);
		}

		string all_msg = game_host[now_roomnum].GetAllMessage();
		game_host[now_roomnum].SetUsed(now_playernum, true);
		while(!game_host[now_roomnum].AllUsed())
		{
			Sleep(1);
		}
		game_host[now_roomnum].SetReady(now_playernum, false);

		strcpy_s(ret.msg, all_msg.c_str());

		/*stringstream sio(ret.msg);
		sio<<recv_msg->msg<<" 0 0 0";
		string tmp = sio.str();
		strcpy_s(ret.msg, tmp.c_str());*/
		
	}
	else if(recv_msg->type2 == MSG_GAME_QUIT)
	{
		game_host[now_roomnum].SetAvailable(now_playernum, false);
	}
	return ret;
}
