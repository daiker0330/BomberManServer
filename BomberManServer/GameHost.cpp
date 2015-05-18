#include "stdafx.h"
#include "GameHost.h"
#include <process.h>

using namespace std;

CGameHost::CGameHost(void)
{
	init_times=0;
	
	/*for(int i=1; i<=MAX_PLAYER; i++)
	{
		available[i] = true;
		ready[i] = false;
		used[i] = false;
	}*/
}


CGameHost::~CGameHost(void)
{
	CloseHandle(monitor_thread);
	CloseHandle(ready);
	CloseHandle(read);
	CloseHandle(all_ready);
	CloseHandle(all_read);
}

void CGameHost::Init(int num)
{
	init_times = (init_times)%4+1;
	if(init_times!=1) // only the first one will actually do init
		return;

	available_cnt = MAX_PLAYER;
	for(int i=1; i<=MAX_PLAYER; i++)
		available[i] = true;

	ready = CreateSemaphore(NULL, 0, MAX_PLAYER, NULL);
	read = CreateSemaphore(NULL, 0, MAX_PLAYER, NULL);
	all_ready = CreateSemaphore(NULL, 0, MAX_PLAYER, NULL);
	all_read = CreateSemaphore(NULL, 0, MAX_PLAYER, NULL);
	come = CreateSemaphore(NULL, 0, MAX_PLAYER, NULL);
	all_come = CreateSemaphore(NULL, 0, MAX_PLAYER, NULL);
	monitor_thread = (HANDLE)_beginthreadex(NULL, 0, Monitor, (LPVOID)this, NULL, 0);
	return;
}



void CGameHost::SetMessage( int num, char m[] )
{
	string tmp(m);
	msg[num] = tmp;
	return;
}

string CGameHost::GetAllMessage()
{
	string ret="";
	int i;
	for(i=1;i<=MAX_PLAYER;i++)
	{
		if(!available[i])
			ret += "0 16 ";//default time:16 avoid wrapping forever
		else
			ret += msg[i] + " ";
	}

	return ret;
}


bool CGameHost::AllInit()
{
	return (init_times == 4);
}

unsigned __stdcall CGameHost::Monitor( LPVOID p )
{
	CGameHost *nowp = (CGameHost *)p;
	while(true)
	{
		int i;
		for(i=1; i<=nowp->available_cnt; i++)
		{
			WaitForSingleObject(nowp->come, INFINITE);
		}
		for(i=1; i<=nowp->available_cnt; i++)
		{
			ReleaseSemaphore(nowp->all_come, 1, NULL);
		}

		nowp->available_cnt=0;
		for(i=1; i<=MAX_PLAYER; i++)
		{
			if(nowp->available[i])
				nowp->available_cnt++;
		}

		for(i=1;i<=nowp->available_cnt;i++)
		{
			//cout<<"Waiting "<<nowp->ready<<endl;
			WaitForSingleObject(nowp->ready, INFINITE);
			//cout<<"Get Ready!"<<endl;
		}

		for(i=1;i<=nowp->available_cnt;i++)
		{
			ReleaseSemaphore(nowp->all_ready, 1, NULL);
		}

		for(i=1; i<=nowp->available_cnt; i++)
		{
			WaitForSingleObject(nowp->read, INFINITE);
		}

		for(i=1; i<=nowp->available_cnt;i++)
		{
			ReleaseSemaphore(nowp->all_read, 1, NULL);
		}

		if(nowp->available_cnt == 0)
			break;
	}
	return 0;
}

void CGameHost::ReleaseReady()
{
	//cout<<"Releasing "<<ready<<endl;
	ReleaseSemaphore(ready, 1, NULL);
}

void CGameHost::WaitAllReady()
{
	WaitForSingleObject(all_ready, INFINITE);
}

void CGameHost::ReleaseRead()
{
	ReleaseSemaphore(read, 1, NULL);
}

void CGameHost::WaitAllRead()
{
	WaitForSingleObject(all_read, INFINITE);
}

void CGameHost::Leave( int num )
{
	available[num] = false;
}

void CGameHost::ReleaseCome()
{
	ReleaseSemaphore(come, 1, NULL);
}

void CGameHost::WaitAllCome()
{
	WaitForSingleObject(all_come, INFINITE);
}
