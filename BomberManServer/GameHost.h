#pragma once
#include "stdafx.h"
#include <string>
using namespace std;

class CGameHost
{
	string msg[MAX_PLAYER+1];
	bool ready[MAX_PLAYER+1], used[MAX_PLAYER+1], available[MAX_PLAYER+1];

	int available_cnt;
	int init_times;
	int open_votes;

	bool open;
	
	float pos[5][9];

public:
	CGameHost(void);
	~CGameHost(void);

	void Init(int num);
	bool AllInit();
	void SetMessage(int num, char m[]);
	
	void SetPos(int num, float p[]);
	bool Verify();

	bool AllReady();
	void SetReady(int num, bool v){ready[num] = v;}
	void ClearReady();
	bool AllUsed();
	void SetUsed(int num, bool v){used[num]=v;}
	string GetAllMessage();
	bool Ready(int i){return ready[i];}
	bool Used(int i){return used[i];}
	void SetAvailable(int i, bool v){available[i] = v;}
	bool Available(int i){return available[i];}
	/*bool Open() const { return open; }
	void SetOpen(bool v) const { open=v; }
	void VoteOpen();*/
};

