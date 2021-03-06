#pragma once
#include "StdAfx.h"
#include "DataProcessh.h"

using namespace std;

bool Dataprocess::Init()
{
	InitializeCriticalSection(&cs);
	return InitDB();
}

bool Dataprocess::InitDB()
{
	henv = SQL_NULL_HENV;		// Define the environment handle
	hdbc1 = SQL_NULL_HDBC;		// Define the database connection handle


	retcode = SQLAllocHandle(SQL_HANDLE_ENV, NULL, &henv);

	if (retcode < 0)			// Error handle
	{
		cout << "allocate ODBC Environment handle errors." << endl;
		return false;
	}
	
	// Notify ODBC that this is an ODBC 3.0 application.
	retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION,
		(SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER);
	if (retcode < 0) 			// Error handle
	{
		cout << "the  ODBC is not version 3.0." << endl;
		return false;
	}

	// Allocate an ODBC connection and connect.
	retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc1);
	if (retcode < 0) 			// Error handle
	{
		cout << "allocate ODBC connection handle errors." << endl;
		return false;
	}
	
	// Data Source Name must be of type User DNS or System DNS
	wchar_t* szDSN = L"BombManServer";
	wchar_t* szUID = L"daiker";			// login name
	wchar_t* szAuthStr = L"12345";		// passward
	
	//connect to the Data Source
	retcode = SQLConnect(hdbc1, (SQLWCHAR*)szDSN, (SWORD)wcslen(szDSN), (SQLWCHAR*)szUID, (SWORD)wcslen(szUID), (SQLWCHAR*)szAuthStr, (SWORD)wcslen(szAuthStr));
	if (retcode < 0) 			// Error handle
	{
		cout << "connect to  ODBC datasource errors." << endl;
		return false;
	}

	SQLHSTMT  hstmt1 = SQL_NULL_HSTMT;	// Define the SQL handle
	
	// Allocate a statement handle.
	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc1, &hstmt1);
	if (retcode < 0) 			// Error handle
	{
		cout << "allocate ODBC statement handle errors." << endl;
		return false;
	}

	// Execute the SQL statement
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
	SQLINTEGER		columnLen = 0;

	while (1)
	{
		retcode = SQLFetch(hstmt1);
		if (retcode == SQL_NO_DATA)
			break;

		// Get the data from one row
		retcode = SQLGetData(hstmt1, 1, SQL_C_LONG, &id, 0, &columnLen);
		retcode = SQLGetData(hstmt1, 2, SQL_C_CHAR, user_name, MaxNameLen, &columnLen);
		retcode = SQLGetData(hstmt1, 3, SQL_C_CHAR, user_password, MaxNameLen, &columnLen);
		retcode = SQLGetData(hstmt1, 4, SQL_C_LONG, &money, 0, &columnLen);
		retcode = SQLGetData(hstmt1, 5, SQL_C_LONG, &VIP, 0, &columnLen);

		// Cache the data into the memory
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

	// Init room data
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			onlineData.romm_user[i][j].first = 0;
			onlineData.romm_user[i][j].second = "";
			onlineData.ready[i][j] = false;
		}
	}

	// Init chat data
	onlineData.chat_num = 0;

	return true;
}

// Handle the Login request
CMessage Dataprocess::Login(CMessage* recv_msg, SOCKADDR_IN* socket)
{
	CMessage msg;
	msg.type1 = MSG_LOGIN;
	msg.type2 = MSG_NULL;
	
	// Check the message type
	if (recv_msg->type2 == MSG_LOGIN_CKECK)
	{
		msg.type2 = MSG_LOGIN_DENY;
		
		// p: iterator to the all the users in the database
		list<pair<int, pair<string, string>>>::iterator p;
		for (p = onlineData.user_name_psd.begin(); p != onlineData.user_name_psd.end(); p++)
		{
			// Compare the username and password
			if (strcmp(recv_msg->str1, p->second.first.c_str()) == 0 && strcmp(recv_msg->str2, p->second.second.c_str()) == 0)
			{
				// Accept the login
				msg.type2 = MSG_LOGIN_CONFIRM;
				
				// Set the user id
				msg.para1 = p->first;

				// Get the money attribute of the user
				list<pair<int, int>>::iterator p2;
				for (p2 = onlineData.user_money.begin(); p2 != onlineData.user_money.end(); p2++)
				{
					if (p2->first == p->first)
					{
						msg.para2 = p2->second;
					}
				}

				// Get the VIP attribute of the user
				strcpy_s(msg.str1, 20, "NOT");
				list<pair<int, bool>>::iterator p3;
				for (p3 = onlineData.user_vip.begin(); p3 != onlineData.user_vip.end(); p3++)
				{
					if (p3->first == p->first && p3->second == true)
					{
						strcpy_s(msg.str1, 20, "VIP");
					}
				}

				// Store the connection info
				pair<int, pair<string, int>> ip_port;
				ip_port.first = p->first;
				ip_port.second.first = inet_ntoa(socket->sin_addr);
				ip_port.second.second = socket->sin_port;

				onlineData.user_ip_port.push_back(ip_port);
			}
		}
	}
	return msg;
}

// Handle the room related request
CMessage Dataprocess::Room(CMessage* recv_msg)
{
	CMessage msg;
	msg.type1 = MSG_ROOM;
	msg.type2 = MSG_NULL;
	// Enter room request
	if (recv_msg->type2 == MSG_ROOM_TRY)
	{
		int i;
		// Check available room
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
		
		// No available romm
		if (i == 4)
		{
			msg.type2 = MSG_ROOM_DENY;
		}
	}
	// Request the other users' name in the same room
	else if (recv_msg->type2 == MSG_ROOM_NAME)
	{
		// There is a player in the seat
		if (onlineData.romm_user[recv_msg->para1][recv_msg->para2].first != 0)
		{
			msg.type2 = MSG_ROOM_RETURN;
			// Set the user id
			msg.para1 = onlineData.romm_user[recv_msg->para1][recv_msg->para2].first;
			// Set the user name
			strcpy_s(msg.str1, 20, onlineData.romm_user[recv_msg->para1][recv_msg->para2].second.c_str());
		}
		// No player in the seat
		else
		{
			msg.type2 = MSG_ROOM_EMPTY;
		}
	}
	// Exit room request
	else if (recv_msg->type2 == MSG_ROOM_EXIT)
	{
		onlineData.romm_user[recv_msg->para1][recv_msg->para2].first = 0;
	}
	// Get ready for the game
	else if (recv_msg->type2 == MSG_ROOM_READY)
	{
		// Set ready flag
		onlineData.ready[recv_msg->para1][recv_msg->para2] = true;
		int i;
		// Check other players' ready states
		for (i = 0; i < 4; i++)
		{
			if (onlineData.ready[recv_msg->para1][i] == false)
			{
				break;
			}
		}
		// Everyone is ready, game starts!
		if (i == 4)
		{
			// Make sure the game init process is atomic
			EnterCriticalSection(&cs);
			game_host[recv_msg->para1].Init(recv_msg->para2+1);
			LeaveCriticalSection(&cs);

			// Wait until the init process done
			while(!game_host[recv_msg->para1].AllInit())
			{
				;
			}
			
			// Game Begin!
			msg.type2 = MSG_ROOM_GAME;
		}
	}
	// Request change the actor
	else if (recv_msg->type2 == MSG_ROOM_SET_ACTOR)
	{
		onlineData.room_actor[recv_msg->para1][recv_msg->para2] = (int)recv_msg->msg[0];
	}
	// Request the actor of other players
	else if (recv_msg->type2 == MSG_ROOM_GET_ACTOR)
	{
		msg.type2 = MSG_ROOM_RETURN_ACTOR;
		msg.para1 = recv_msg->para2;
		msg.para2 = onlineData.room_actor[recv_msg->para1][recv_msg->para2];
	}
	// Cancel the ready state
	else if (recv_msg->type2 == MSG_ROOM_NOT_READY)
	{
		onlineData.ready[recv_msg->para1][recv_msg->para2] = false;
	}
	return msg;
}

// Get the user name
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
	// Clean up
	SQLDisconnect(hdbc1);
	SQLFreeHandle(SQL_HANDLE_DBC, hdbc1);
	SQLFreeHandle(SQL_HANDLE_ENV, henv);
}

// Handle the lobby related request
CMessage Dataprocess::Lobby(CMessage* recv_msg)
{
	CMessage msg;
	msg.type1 = MSG_LOBBY;
	msg.type2 = MSG_NULL;
	
	// Request the state of the room
	if (recv_msg->type2 == MSG_LOBBY_ROOM)
	{
		int i,num=0;
		msg.type2 = MSG_LOBBY_RETURN;
		
		// Count the players in the room
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

// Handle the game related request
CMessage Dataprocess::Game( CMessage* recv_msg )
{
	CMessage ret;
	int now_roomnum = recv_msg->para1;
	int now_playernum = recv_msg->para2;
	/*if(true)
	{
		cout<<"Received Game Message: "<<endl;
		cout<<"type2 = "<<recv_msg->type2;
		cout<<" player_num = "<<now_playernum<<" msg= "<<recv_msg->msg<<endl;
	}*/
	
	// Game init request
	if(recv_msg->type2 == MSG_GAME_START)
	{
		// Assign the random seed
		unsigned int seed = seed_manager.AskSeed(now_roomnum);
		ret.type1 = MSG_GAME;
		ret.type2 = MSG_GAME_START;
		ret.para1 = seed;
		return ret;
	}

	// Exit game request
	if(recv_msg->type2 == MSG_GAME_QUIT)
	{
		game_host[now_roomnum].Leave(now_playernum);
		onlineData.ready[now_roomnum][now_playernum-1] = false;
	}

	game_host[now_roomnum].ReleaseCome();
	game_host[now_roomnum].WaitAllCome();

	// Update the operation of the player
	if(recv_msg->type2 == MSG_GAME_OPERATION)
	{
		ret.type1 = MSG_GAME;
		ret.type2 = MSG_GAME_OPERATION;

		//cout<<"Entered from "<<now_playernum<<endl;

		// Set the operation
		game_host[now_roomnum].SetMessage(now_playernum, recv_msg->msg);
		game_host[now_roomnum].ReleaseReady();

		//cout<<"Released Ready from "<<now_playernum<<endl;

		game_host[now_roomnum].WaitAllReady();

		//cout<<"AllReady from"<<now_playernum<<endl;
		       
		string all_msg = game_host[now_roomnum].GetAllMessage();
		game_host[now_roomnum].ReleaseRead();

		//cout<<"Released Read from "<<now_playernum<<endl;

		game_host[now_roomnum].WaitAllRead();

		//cout<<"AllUsed from "<<now_playernum<<endl;

		strcpy_s(ret.msg, all_msg.c_str());

		//cout<<"Return msg "<<now_playernum<<": "<<ret.msg<<endl;

		/*stringstream sio(ret.msg);
		sio<<recv_msg->msg<<" 0 0 0";
		string tmp = sio.str();
		strcpy_s(ret.msg, tmp.c_str());*/
		
	}
	
	return ret;
}

// Handle the chat related request
CMessage Dataprocess::Chat(CMessage* recv_msg)
{
	CMessage msg;
	msg.type1 = MSG_CHAT;
	msg.type2 = MSG_NULL;
	// Send chat message
	if (recv_msg->type2 == MSG_CHAT_SEND)
	{
		onlineData.chat_message[onlineData.chat_num].first.append(recv_msg->str1);
		onlineData.chat_message[onlineData.chat_num].second.append(recv_msg->str2);
		onlineData.chat_num++;
	}
	// Get chat message
	else if (recv_msg->type2 == MSG_CHAT_GET)
	{
		if (onlineData.chat_num == recv_msg->para1 || onlineData.chat_num == 0)
		{
			msg.type2 = MSG_CHAT_DENY;
		}
		else if (recv_msg->para1 < onlineData.chat_num)
		{
			strcpy_s(msg.str1, 20, onlineData.chat_message[recv_msg->para1].first.c_str());
			strcpy_s(msg.str2, 20, onlineData.chat_message[recv_msg->para1].second.c_str());
			msg.type2 = MSG_CHAT_RETURN;
		}
	}
	return msg;
}

// Handle the data related request
CMessage Dataprocess::Data(CMessage* recv_msg)
{
	CMessage msg;
	msg.type1 = MSG_DATA;
	msg.type2 = MSG_NULL;
	
	// Request the money attribute of the user
	if (recv_msg->type2 == MSG_DATA_GET_MONEY)
	{
		SQLHSTMT  hstmt1 = SQL_NULL_HSTMT;

		// Allocate a statement handle.
		retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc1, &hstmt1);
		if (retcode < 0)
		{
			msg.type2 = MSG_DATA_ERROR;
		}

		// Set the SQL statement
		wchar_t sql_str[100];
		swprintf_s(sql_str, L"SELECT [money] FROM account WHERE (ID = %d)", recv_msg->para1);

		// Execute the SQL statement
		retcode = SQLExecDirect(hstmt1, (SQLWCHAR*)sql_str, SQL_NTS);
		if (retcode < 0)
		{
			msg.type2 = MSG_DATA_ERROR;
		}
		else
		{
			msg.type2 = MSG_DATA_SUCCESS;
			
			// SQLBindCol variables
			SQLINTEGER		money;
			SQLINTEGER		columnLen = 0;

			while (1)
			{
				retcode = SQLFetch(hstmt1);
				if (retcode == SQL_NO_DATA)
					break;

				retcode = SQLGetData(hstmt1, 1, SQL_C_LONG, &money, 0, &columnLen);

				msg.para1 = money;
			}
			
			SQLFreeHandle(SQL_HANDLE_STMT, hstmt1);
		}
	}
	// Set the money attribute of the user
	else if (recv_msg->type2 == MSG_DATA_SET_MONEY)
	{
		SQLHSTMT  hstmt1 = SQL_NULL_HSTMT;

		// Allocate a statement handle.
		retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc1, &hstmt1);
		if (retcode < 0)
		{
			msg.type2 = MSG_DATA_ERROR;
		}

		// Set the SQL statement
		wchar_t sql_str[100];
		swprintf_s(sql_str, L"UPDATE account SET [money] = %d WHERE [ID] = %d", recv_msg->para2, recv_msg->para1);

		// Execute the SQL statement
		retcode = SQLExecDirect(hstmt1, (SQLWCHAR*)sql_str, SQL_NTS);
		if (retcode < 0)
		{
			msg.type2 = MSG_DATA_ERROR;
		}
		else
		{
			msg.type2 = MSG_DATA_SUCCESS;
			SQLFreeHandle(SQL_HANDLE_STMT, hstmt1);
		}
	}
	// Request the EXP attribute of the user
	else if (recv_msg->type2 == MSG_DATA_GET_EXP)
	{
		SQLHSTMT  hstmt1 = SQL_NULL_HSTMT;

		// Allocate a statement handle.
		retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc1, &hstmt1);
		if (retcode < 0)
		{
			msg.type2 = MSG_DATA_ERROR;
		}

		// Set the SQL statement
		wchar_t sql_str[100];
		swprintf_s(sql_str, L"SELECT [exp] FROM account WHERE (ID = %d)", recv_msg->para1);

		// Execute the SQL statement
		retcode = SQLExecDirect(hstmt1, (SQLWCHAR*)sql_str, SQL_NTS);
		if (retcode < 0)
		{
			msg.type2 = MSG_DATA_ERROR;
		}
		else
		{
			msg.type2 = MSG_DATA_SUCCESS;

			// SQLBindCol variables
			SQLINTEGER		exp;
			SQLINTEGER		columnLen = 0;

			while (1)
			{
				retcode = SQLFetch(hstmt1);
				if (retcode == SQL_NO_DATA)
					break;

				retcode = SQLGetData(hstmt1, 1, SQL_C_LONG, &exp, 0, &columnLen);

				msg.para1 = exp;
			}
			
			SQLFreeHandle(SQL_HANDLE_STMT, hstmt1);
		}
	}
	// Set the EXP attribute of the user
	else if (recv_msg->type2 == MSG_DATA_SET_EXP)
	{
		SQLHSTMT  hstmt1 = SQL_NULL_HSTMT;

		// Allocate a statement handle.
		retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc1, &hstmt1);
		if (retcode < 0)
		{
			msg.type2 = MSG_DATA_ERROR;
		}

		// Set the SQL statement
		wchar_t sql_str[100];
		swprintf_s(sql_str, L"UPDATE account SET [exp] = %d WHERE [ID] = %d", recv_msg->para2, recv_msg->para1);

		// Execute the SQL statement
		retcode = SQLExecDirect(hstmt1, (SQLWCHAR*)sql_str, SQL_NTS);
		if (retcode < 0)
		{
			msg.type2 = MSG_DATA_ERROR;
		}
		else
		{
			msg.type2 = MSG_DATA_SUCCESS;
			SQLFreeHandle(SQL_HANDLE_STMT, hstmt1);
		}
	}
	// Request the ready state of the other player
	else if (recv_msg->type2 == MSG_DATA_GET_READY)
	{
		msg.type2 = MSG_DATA_SUCCESS;
		if (onlineData.ready[recv_msg->para1][recv_msg->para2])
		{
			msg.para1 = 1;
		}
		else
		{
			msg.para1 = 0;
		}
	}
	// Request the VIP attribute of the user
	else if (recv_msg->type2 == MSG_DATA_GET_VIP)
	{
		msg.type2 = MSG_DATA_SUCCESS;
		list<pair<int, bool>>::iterator p3;
		for (p3 = onlineData.user_vip.begin(); p3 != onlineData.user_vip.end(); p3++)
		{
			if (p3->first == recv_msg->para1)
			{
				msg.para1=p3->second;
			}
		}
	}
	return msg;
}

// Clean up
void Dataprocess::Disconnect(string ip, int port)
{
	int id;
	list<pair<int, pair<string, int>>>::iterator p;
	
	// Clean connection info
	for (p = onlineData.user_ip_port.begin(); p != onlineData.user_ip_port.end(); p++)
	{
		if (ip == p->second.first &&port == p->second.second)
		{
			id = p->first;
			onlineData.user_ip_port.erase(p);
			break;
		}
	}
	
	// Clean user info
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (onlineData.romm_user[i][j].first == id)
			{
				onlineData.romm_user[i][j].first = 0;
			}
		}
	}
}