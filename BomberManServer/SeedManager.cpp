#include "stdafx.h"
#include "SeedManager.h"
#include <stdlib.h>
#include <time.h>

CSeedManager::CSeedManager(void)
{
	memset(cnt,0,sizeof(cnt));
}


CSeedManager::~CSeedManager(void)
{
}

unsigned int CSeedManager::AskSeed( int num )
{
	if(cnt[num] == 0)
	{
		cnt[num] = 4;
		seed[num] = unsigned (time(0));
	}
	cnt[num]--;
	return seed[num];
}
