// BomberManServer.cpp : �������̨Ӧ�ó������ڵ㡣
//
#pragma once
#include "stdafx.h"

using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{
	
	CIOCPModel m_IOCP;
	char s;

	//SQLHENV  henv = SQL_NULL_HENV;//���廷�����
	//SQLHDBC  hdbc1 = SQL_NULL_HDBC;//�������ݿ����Ӿ��     
	//SQLHSTMT  hstmt1 = SQL_NULL_HSTMT;//���������
	//RETCODE retcode;

	//retcode = SQLAllocHandle(SQL_HANDLE_ENV, NULL, &henv);

	//if (retcode < 0)//������
	//{
	//	cout << "allocate ODBC Environment handle errors." << endl;
	//	return -1;
	//}
	//// Notify ODBC that this is an ODBC 3.0 application.
	//retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION,
	//	(SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER);
	//if (retcode < 0) //������
	//{
	//	cout << "the  ODBC is not version3.0 " << endl;
	//	return -1;
	//}

	//// Allocate an ODBC connection and connect.
	//retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc1);
	//if (retcode < 0) //������
	//{
	//	cout << "allocate ODBC connection handle errors." << endl;
	//	return -1;
	//}
	////Data Source Name must be of type User DNS or System DNS
	//wchar_t* szDSN = L"BombManServer";
	//wchar_t* szUID = L"daiker";//log name
	//wchar_t* szAuthStr = L"12345";//passward
	////connect to the Data Source
	//retcode = SQLConnect(hdbc1, (SQLWCHAR*)szDSN, (SWORD)wcslen(szDSN), (SQLWCHAR*)szUID, (SWORD)wcslen(szUID), (SQLWCHAR*)szAuthStr, (SWORD)wcslen(szAuthStr));
	//if (retcode < 0) //������
	//{
	//	cout << "connect to  ODBC datasource errors." << endl;
	//	return -1;
	//}
	//// Allocate a statement handle.
	//retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc1, &hstmt1);
	//if (retcode < 0) //������
	//{
	//	cout << "allocate ODBC statement handle errors." << endl;
	//	return -1;
	//}

	// Execute an SQL statement directly on the statement handle.ÿһ����涼����һ������������������ʱ���Ժܷ�����жϴ�������

	//retcode = SQLExecDirect(hstmt1, (SQLWCHAR*)"create table provider1(sno char(5) primary key,sname char(10) not null,status int,city char(10))", SQL_NTS);
	//if (retcode < 0)
	//{
	//	cout << "creat errors." << endl;
	//	return -1;
	//}

	//retcode = SQLExecDirect(hstmt1, (SQLWCHAR*)L"SELECT user_name,user_psd  FROM account", SQL_NTS);
	//if (retcode < 0)
	//{
	//	cout << "Executing statement  throught ODBC  errors." << endl;
	//	return -1;
	//}

	//// SQLBindCol variables
	//SQLCHAR      user_name[MaxNameLen + 1];
	//SQLCHAR   user_password[MaxNameLen + 1];
	//SQLINTEGER   columnLen = 0;//���ݿⶨ���и������еĳ���

	//while (1)
	//{
	//	retcode = SQLFetch(hstmt1);
	//	if (retcode == SQL_NO_DATA)
	//		break;

	//	retcode = SQLGetData(hstmt1, 1, SQL_C_CHAR, user_name, MaxNameLen, &columnLen);
	//	retcode = SQLGetData(hstmt1, 2, SQL_C_CHAR, user_password, MaxNameLen, &columnLen);
	//	if (columnLen > 0)
	//		printf("user_name = %s  user_password = %s\n", user_name, user_password);
	//	else
	//		printf("user_name = %s  user_password = NULL\n", user_name, user_password);
	//}

	if (false == m_IOCP.InitDB())
	{
		printf("load database error\n");
		exit(0);
	}

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

