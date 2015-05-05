#pragma once
#include <list>
#include <utility>

using namespace std;

class OnlineData
{
public:
	pair<int, string> romm_user[8][4];
	list<pair<int, pair<string,string>>> user_name_psd;
	list<pair<int, int>> user_money;
	list<pair<int, bool>> user_vip;
};