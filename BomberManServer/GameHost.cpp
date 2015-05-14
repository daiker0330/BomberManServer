#include "stdafx.h"
#include "GameHost.h"
#include <process.h>

using namespace std;

CGameHost::CGameHost(void)
{
	init_times=0;
	ready = CreateSemaphore(NULL, 0, 4, NULL);
	read = CreateSemaphore(NULL, 0, 4, NULL);
	all_ready = CreateSemaphore(NULL, 0, 4, NULL);
	all_read = CreateSemaphore(NULL, 0, 4, NULL);
	monitor_thread = (HANDLE)_beginthreadex(NULL, 0, Monitor, (LPVOID)this, NULL, 0);
	
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
	/*available[num] = true;
	ready[num] = false;
	used[num] = false;*/
	init_times = (init_times)%4+1;
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
		/*if(!available[i])
			ret += "0 0 ";
		else*/
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
		for(i=1;i<=MAX_PLAYER;i++)
		{
			cout<<"Waiting "<<nowp->ready<<endl;
			WaitForSingleObject(nowp->ready, INFINITE);
			cout<<"Get Ready!"<<endl;
		}

		for(i=1;i<=MAX_PLAYER;i++)
		{
			ReleaseSemaphore(nowp->all_ready, 1, NULL);
		}

		for(i=1; i<=MAX_PLAYER; i++)
		{
			WaitForSingleObject(nowp->read, INFINITE);
		}

		for(i=1; i<=MAX_PLAYER;i++)
		{
			ReleaseSemaphore(nowp->all_read, 1, NULL);
		}
	}
	return 0;
}

void CGameHost::ReleaseReady()
{
	cout<<"Releasing "<<ready<<endl;
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
