#include "stdafx.h"
#include "GameHost.h"
using namespace std;

CGameHost::CGameHost(void)
{
	memset(ready, false, sizeof(ready));
	memset(used, false, sizeof(used));
}


CGameHost::~CGameHost(void)
{
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
		ret = ret & ready[i];
	return ret;
}

bool CGameHost::AllUsed()
{
	int i;
	bool ret = true;
	for(i=1;i<=MAX_PLAYER;i++)
		ret = ret & used[i];
	return ret;
}

string CGameHost::GetAllMessage()
{
	string ret="";
	int i;
	for(i=1;i<=MAX_PLAYER;i++)
	{
		ret += msg[i] + " ";
	}
	return ret;
}

