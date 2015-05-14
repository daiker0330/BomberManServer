#pragma once
#include "stdafx.h"
#include <string>
using namespace std;

class CGameHost
{
	string msg[MAX_PLAYER+1];

	volatile int init_times;

	HANDLE monitor_thread;
	HANDLE ready, all_ready, read, all_read;
	

public:
	CGameHost(void);
	~CGameHost(void);

	void Init(int num);
	bool AllInit();
	void SetMessage(int num, char m[]);
	string GetAllMessage();

	static unsigned __stdcall Monitor(LPVOID p);
	void ReleaseReady();
	void WaitAllReady();
	void ReleaseRead();
	void WaitAllRead();
};

