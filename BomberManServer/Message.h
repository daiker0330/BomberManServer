#pragma once

#define MSG_NULL				0x00

#define MSG_SCENE				0x01
#define MSG_SCENE_LOGIN			0x01
#define MSG_SCENE_LOBBY			0x02
#define MSG_SCENE_ROOM			0x03
#define MSG_SCENE_GAME			0x04

#define MSG_LOGIN				0x02
#define MSG_LOGIN_CKECK			0x01
#define MSG_LOGIN_CONFIRM		0x02
#define MSG_LOGIN_DENY			0x03
#define MSG_LOGIN_ONLINE		0x04

#define MSG_GAME				0x03
#define MSG_GAME_OPERATION		0x01
#define MSG_GAME_QUIT			0x02
#define MSG_GAME_START			0x03

#define MSG_ROOM				0x04
#define MSG_ROOM_TRY			0x01
#define MSG_ROOM_CONFIRM		0x02
#define MSG_ROOM_DENY			0x03
#define MSG_ROOM_NAME			0x04
#define MSG_ROOM_RETURN			0x05
#define MSG_ROOM_EMPTY			0x06
#define MSG_ROOM_EXIT			0x07
#define MSG_ROOM_READY			0x08
#define MSG_ROOM_GAME			0x09
#define MSG_ROOM_WAIT			0x10

#define MSG_LOBBY				0x05
#define MSG_LOBBY_ROOM			0x01
#define MSG_LOBBY_RETURN		0x02

#define MSG_CHAT				0x06
#define MSG_CHAT_SEND			0x01
#define MSG_CHAT_GET			0x02
#define MSG_CHAT_RETURN			0x03
#define MSG_CHAT_END			0x04
#define MSG_CHAT_DENY			0x05

#define MSG_DATA				0x07
#define MSG_DATA_GET_MONEY		0x01
#define MSG_DATA_SET_MONEY		0x02
#define MSG_DATA_GET_EXP		0x03
#define MSG_DATA_SET_EXP		0x04
#define MSG_DATA_SUCCESS		0x05
#define MSG_DATA_ERROR			0x06

class CMessage
{
public:
	unsigned short type1;
	unsigned short type2;
	char str1[20];
	char str2[20];
	int para1;
	int para2;
	char msg[100];
	float debug[9];

	CMessage()
	{
		memset(str1, 0, sizeof(str1));
		memset(str2, 0, sizeof(str2));
		memset(msg, 0, sizeof(msg));
	}
};