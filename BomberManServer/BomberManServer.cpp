// BomberManServer.cpp : 定义控制台应用程序的入口点。
//
#pragma once
#include "stdafx.h"
#include "DataProcessh.h"
#include "IOCPModel.h"

using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{
	
	CIOCPModel m_IOCP;
	Dataprocess m_Datapricess;
	char s;

	if (false == m_Datapricess.InitDB())
	{
		printf("load database error\n");
		exit(0);
	}

	m_IOCP.SetDataProcess(&m_Datapricess);

	if (false == m_IOCP.LoadSocketLib())
	{
		printf("load Socket lib error\n");
		exit(0);
	}

	if (false == m_IOCP.Start())
	{
		printf("start Socket error\n");
		exit(0);
	}

	while (1)
	{
		cin >> s;
		if (s == 'q')
		{
			break;
		}
	}

	m_IOCP.Stop();

	

	return 0;
}

