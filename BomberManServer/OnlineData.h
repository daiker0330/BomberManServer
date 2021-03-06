// The server data in the memory

#pragma once
#include <list>
#include <utility>

using namespace std;

class OnlineData
{
public:
	pair<int, string> romm_user[8][4];
	list<pair<int, pair<string,string>>> user_name_psd;
	list<pair<int, pair<string, int>>> user_ip_port;
	list<pair<int, int>> user_money;
	list<pair<int, bool>> user_vip;
	bool ready[8][4];
	pair<string, string> chat_message[100];
	int chat_num;
	int room_actor[8][4];
};