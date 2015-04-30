#define MSG_NULL 0x00
#define MSG_SCENE 0x01
#define MSG_SCENE_LOGIN 0x01
#define MSG_SCENE_LOBBY 0x02
#define MSG_SCENE_ROOM 0x03
#define MSG_SCENE_GAME 0x04
#define MSG_MOVE 0x02
#define MSG_TOOL 0x03
#define MSG_BOMB 0x04
#define MSG_STATUS 0x05


class CMessage
{
public:
	unsigned short type1;
	unsigned short type2;
	unsigned short id;
	unsigned short x;
	unsigned short y;
	char str1[10];
	char str2[10];
};