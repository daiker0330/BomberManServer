#pragma once
#include "stdafx.h"
#include <string>
using namespace std;

class CGameHost
{
	string msg[MAX_PLAYER+1];
	bool ready[MAX_PLAYER+1], used[MAX_PLAYER+1], available[MAX_PLAYER+1];

public:
	CGameHost(void);
	~CGameHost(void);

	void Init();
	void SetMessage(int num, char m[]);
	bool AllReady();
	void SetReady(int num, bool v){ready[num] = v;}
	bool AllUsed();
	void SetUsed(int num, bool v){used[num]=v;}
	string GetAllMessage();
	bool Ready(int i){return ready[i];}
	bool Used(int i){return used[i];}
	void SetAvailable(int i, bool v){available[i] = v;}
};

