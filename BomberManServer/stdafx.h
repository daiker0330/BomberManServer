// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <Winsock2.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "Message.h"
#include <iostream>
#include <list>
#include <utility>
#include <sql.h>
#include <sqlext.h>
#include <odbcss.h>
#include <MSWSock.h>
#include <vector>
#include <atlconv.h>
#include <windowsx.h>



#define ASSERT(x) {if(!(x)) _asm{int 0x03}}

#define MAXBUFLEN   255 
#define MaxNameLen  20
#define SERVPORT 4160
#define BACKLOG 20

const int MAX_PLAYER = 4;
const int MAX_ROOMS = 8;


// TODO:  �ڴ˴����ó�����Ҫ������ͷ�ļ�
