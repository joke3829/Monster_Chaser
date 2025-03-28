constexpr int PORT_NUM = 3500;
constexpr int BUF_SIZE = 256; 
constexpr int NAME_SIZE = 20;

constexpr int W_WIDTH = 8;
constexpr int W_HEIGHT = 8;

// Packet ID


// ��Ŷ Ÿ�� ����
enum PACKET_TYPE {
	CS_LOGIN = 0,
	CS_MOVE = 1,   // Ŭ���̾�Ʈ �� ���� : �̵� ��û
	SC_UPDATE = 2  // ���� �� Ŭ���̾�Ʈ : ��ü �÷��̾� ��ġ ������Ʈ
};

constexpr char SC_LOGIN_INFO = 2;
constexpr char SC_ADD_PLAYER = 3;
constexpr char SC_MOVE_PLAYER = 4;





#pragma pack (push, 1)
struct CS_LOGIN_PACKET {
	unsigned char size;
	char	type;
	char	name[NAME_SIZE];
};
#pragma pack(pop)

// ���� �� Ŭ���̾�Ʈ : ��ü �÷��̾� ��ġ ��Ŷ
#pragma pack(push, 1)
struct SC_UPDATE_PACKET {
	short size;
	short type;  // SC_UPDATE
	short playerCount; // ���� ���� ���� �÷��̾� ��
	struct PlayerData {
		short id;
		int x;
		int y;
	} players[10];  // �ִ� 10����� ���� ���� ���� (�ʿ�� Ȯ��)
};
#pragma pack(pop)
struct CS_MOVE_PACKET { 
	short size;
	short type;  // CS_MOVE
	short id;    // Ŭ���̾�Ʈ ID
	char direction;  // 'W', 'A', 'S', 'D'
};
struct SC_LOGIN_INFO_PACKET {
	unsigned char size;
	char	type;
	short	id;
	short	x, y;
};

struct SC_ADD_PLAYER_PACKET {
	unsigned char size;
	char	type;
	short	id;
	short	x, y;
	char	name[NAME_SIZE];
};

struct SC_REMOVE_PLAYER_PACKET {		
	unsigned char size;
	char	type;
	short	id;
};

struct SC_MOVE_PLAYER_PACKET {
	unsigned char size;
	char	type;
	short	id;
	short	x, y;
};

#pragma pack (pop)