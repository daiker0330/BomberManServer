#include "stdafx.h"
#include "GameHost.h"
using namespace std;

CGameHost::CGameHost(void)
{
	init_times=0;
	memset(available, true, sizeof(available));
	memset(ready, false, sizeof(ready));
	memset(used, false, sizeof(used));
}


CGameHost::~CGameHost(void)
{
}

void CGameHost::Init(int num)
{
	available[num] = true;
	ready[num] = false;
	used[num] = false;
	init_times = (init_times)%4+1;
}



void CGameHost::SetMessage( int num, char m[] )
{
	string tmp(m);
	msg[num] = tmp;
	return;
}

bool CGameHost::AllReady()
{
	int i;
	bool ret = true;
	for(i=1;i<=MAX_PLAYER;i++)
	{
		if(!available[i])
			continue;
		ret = ret & ready[i];
	}
	return ret;
}

bool CGameHost::AllUsed()
{
	int i;
	bool ret = true;
	for(i=1;i<=MAX_PLAYER;i++)
	{
		if(!available[i])
			continue;
		ret = ret & used[i];
	}
	return ret;
}

string CGameHost::GetAllMessage()
{
	string ret="";
	int i;
	for(i=1;i<=MAX_PLAYER;i++)
	{
		if(!available[i])
			ret += "0 0 ";
		else
			ret += msg[i] + " ";
	}

	return ret;
}

void CGameHost::SetPos( int num, float p[] )
{
	memcpy(pos[num], p, sizeof(p));
}

bool CGameHost::Verify()
{
	int i,j;
	for(i=1;i<=8;i++)
	{
		for(j=1;j<=3;j++)
		{
			if(pos[j][i] != pos[j+1][i])
				return false;
		}
	}
	return true;
}

void CGameHost::ClearReady()
{
	if(AllReady())
	{
		memset(ready, false, sizeof(ready));
	}
}

bool CGameHost::AllInit()
{
	return (init_times == 4);
}