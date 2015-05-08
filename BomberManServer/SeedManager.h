#pragma once
class CSeedManager
{
	unsigned int seed[MAX_ROOMS+1];
	int cnt[MAX_ROOMS+1];

public:
	CSeedManager(void);
	~CSeedManager(void);

	unsigned int AskSeed(int num);
};

