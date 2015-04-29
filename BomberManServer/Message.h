#define MSG_NULL 0x00
#define MSG_SCENE 0x01
#define MSG_SCENE_LOGIN 0x01
#define MSG_SCENE_LOBBY 0x02
#define MSG_SCENE_ROOM 0x03
#define MSG_SCENE_GAME 0x04
#define MSG_LOGIN 0x02
#define MSG_GAME 0x03

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
};