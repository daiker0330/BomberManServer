#pragma once
#include "stdafx.h"
#include <string>
using namespace std;

class CGameHost
{
	string msg[MAX_PLAYER+1];
	volatile bool available[MAX_PLAYER+1];

	volatile int init_times;
	int available_cnt;

	HANDLE monitor_thread;
	HANDLE ready, all_ready, read, all_read;
	

public:
	CGameHost(void);
	~CGameHost(void);

	void Init(int num);
	bool AllInit();
	void SetMessage(int num, char m[]);
	string GetAllMessage();
	void Leave(int num);

	static unsigned __stdcall Monitor(LPVOID p);
	void ReleaseReady();
	void WaitAllReady();
	void ReleaseRead();
	void WaitAllRead();
};

