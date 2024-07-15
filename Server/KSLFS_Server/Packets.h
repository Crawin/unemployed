#pragma once
#pragma pack(push,1)
/*
	PACKCET
	-----------------------------
	size
	type
	-----------------------------
	playernum;
	xmfloat3 pos
	xmfloat3 rot
*/

enum PACKET_TYPE
{
	pPOSITION,
	pLOGIN,
	pMAKEROOM,
	pENTERROOM,
	pRoomPlayer,
	pLogout,
	pAttack,
	pAnimation
};
enum class ANIMATION_STATE;

class packet_base
{
protected:
	char size;
	PACKET_TYPE type;
public:
	const unsigned char getSize() { return size; }
	const unsigned char getType() { return type; }
};

class sc_packet_position : public packet_base
{
	SOCKET player;
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 rotation;
	DirectX::XMFLOAT3 speed;
public:
	sc_packet_position(const SOCKET& s, const DirectX::XMFLOAT3& p, const DirectX::XMFLOAT3& r, const DirectX::XMFLOAT3& sp) : player(s), position(p), rotation(r), speed(sp) {
		size = sizeof(sc_packet_position);
		type = pPOSITION;
	}
	const SOCKET getPlayer() { return player; }
	const DirectX::XMFLOAT3 getPos() { return position; }
	const DirectX::XMFLOAT3 getRot() { return rotation; }
	const DirectX::XMFLOAT3 getSpeed() { return speed; }
};

class sc_packet_login : public packet_base
{
public:
	SOCKET player = NULL;
public:
	sc_packet_login(const SOCKET sc_socket)
	{
		player = sc_socket;
		size = sizeof(sc_packet_login);
		type = pLOGIN;
	}
};

class sc_packet_logout : public packet_base
{
public:
	SOCKET player = NULL;
	sc_packet_logout(const SOCKET sc_socket)
	{
		player = sc_socket;
		size = sizeof(sc_packet_logout);
		type = pLogout;
	}
};

class sc_packet_make_room : public packet_base
{
	unsigned int gameNum;
public:
	sc_packet_make_room(const unsigned int& n) : gameNum(n)
	{
		size = sizeof(sc_packet_make_room);
		type = pMAKEROOM;
	}
	const unsigned int getGameNum() { return gameNum; }
};

class sc_packet_enter_room : public packet_base
{
	unsigned int gameNum;
	bool enter;
	SOCKET player;
public:
	sc_packet_enter_room(const unsigned int& n, const bool& b, const SOCKET& p) : gameNum(n), enter(b), player(p)
	{
		size = sizeof(sc_packet_enter_room);
		type = pENTERROOM;
	}
	const bool getBool() { return enter; }
	const unsigned int getGameNum() { return gameNum; }
	const SOCKET getPlayer() { return player; }
};

class sc_packet_room_player : public packet_base
{
	SOCKET player = NULL;
public:
	sc_packet_room_player(const SOCKET playerSocket)
	{
		player = playerSocket;
		size = sizeof(sc_packet_room_player);
		type = pRoomPlayer;
	}
	const SOCKET getPlayerSock() { return player; }
};

class sc_packet_npc_attack : public packet_base
{
	unsigned int npc_id;
public:
	sc_packet_npc_attack(const unsigned int& id) : npc_id(id)
	{
		type = pAttack;
		size = sizeof(sc_packet_npc_attack);
	}
};

class sc_packet_anim_type : public packet_base
{
	SOCKET player = NULL;
	char anim_type;
public:
	sc_packet_anim_type(const SOCKET& playerSocket, const char anim_state) : player(playerSocket), anim_type(anim_state)
	{
		type = pAnimation;
		size = sizeof(sc_packet_anim_type);
	}
	const SOCKET getPlayerSock() { return player; }
};

//--------------------------------------------------------------------------------------------------------------------

class cs_packet_position : public packet_base
{
private:
	unsigned int roomNum;
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 rotation;
	DirectX::XMFLOAT3 speed;

public:
	cs_packet_position(const unsigned int& n, const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& rot, const DirectX::XMFLOAT3& sp)
		:roomNum(n), position(pos), rotation(rot), speed(sp)
	{
		size = sizeof(cs_packet_position);
		type = pPOSITION;
	};
	const unsigned int getNum() { return roomNum; }
	const DirectX::XMFLOAT3 getPosition() { return position; }
	const DirectX::XMFLOAT3 getRotation() { return rotation; }
	const DirectX::XMFLOAT3 getSpeed() { return speed; }
	std::chrono::steady_clock::time_point sendTime;
};

class cs_packet_make_room : public packet_base
{
public:
	cs_packet_make_room(const PACKET_TYPE& p)
	{
		size = sizeof(cs_packet_make_room);
		type = p;
	}
};

class cs_packet_enter_room : public packet_base
{
	unsigned int roomNum;
public:
	cs_packet_enter_room(const PACKET_TYPE& p, const unsigned int& n)
	{
		size = sizeof(cs_packet_enter_room);
		type = p;
		roomNum = n;
	}
	const unsigned int getRoomNum() { return roomNum; }
};

class cs_packet_anim_type : public packet_base
{
	char anim_type;
public:
	cs_packet_anim_type(ANIMATION_STATE anim_state)
	{
		size = sizeof(cs_packet_anim_type);
		type = pAnimation;
		anim_type = static_cast<char>(anim_state);
	}
	const char getAnimType() { return anim_type; }
};

class cs_packet_unlock_door : public packet_base
{
	char success;
	short doorNum;
};

class cs_packet_open_door : public packet_base
{
	float angle;

};

#pragma pack(pop)