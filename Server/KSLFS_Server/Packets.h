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
	pAnimation,
	pOpenDoor,
	pUnlockDoor,
	pChangeDayOrNight,
	pGetItem,
	pKeyInput,
	pSound
};
enum class ANIMATION_STATE;
enum class GAME_INPUT;
enum class KEY_STATE;

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
	const SOCKET getPlayer() { return player; }
	const ANIMATION_STATE getAnimState() { return static_cast<ANIMATION_STATE>(anim_type); }
};

class sc_packet_change_day_or_night : public packet_base
{
	char time;
public:
	sc_packet_change_day_or_night(const char& time)
	{
		size = sizeof(sc_packet_change_day_or_night);
		type = pChangeDayOrNight;
		this->time = time;
	}
	const char getTime() { return time; }
};

class sc_packet_get_item : public packet_base
{
	SOCKET player;
	short item_id;
	char slot_id;
public:
	sc_packet_get_item(const SOCKET& player, const short& item_id, const char& slot_id)
	{
		size = sizeof(sc_packet_get_item);
		type = pGetItem;
		this->player = player;
		this->item_id = item_id;
		this->slot_id = slot_id;
	}
	const SOCKET getPlayer() { return player; }
	const int getItemID() { return static_cast<int>(item_id); }
	const char getSlotID() { return slot_id; }
};

class sc_packet_key_input : public packet_base
{
	SOCKET player;
	char key_state;
	char game_input;
public:
	sc_packet_key_input(const SOCKET& player, const KEY_STATE& key_state, const GAME_INPUT& game_input)
	{
		size = sizeof(sc_packet_key_input);
		type = pKeyInput;
		this->player = player;
		this->key_state = static_cast<char>(key_state);
		this->game_input = static_cast<char>(game_input);
	}
	const SOCKET getPlayer() { return player; }
	const KEY_STATE getKeyState() { return static_cast<KEY_STATE>(key_state); }
	const GAME_INPUT getGameInput() { return static_cast<GAME_INPUT>(game_input); }
};

//--------------------------------------------------------------------------------------------------------------------

class cs_packet_position : public packet_base
{
private:
	unsigned int id;
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 rotation;
	DirectX::XMFLOAT3 speed;

public:
	cs_packet_position(const unsigned int& n, const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& rot, const DirectX::XMFLOAT3& sp)
		:id(n), position(pos), rotation(rot), speed(sp)
	{
		size = sizeof(cs_packet_position);
		type = pPOSITION;
	};
	const unsigned int getID() { return id; }
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

class cs_packet_open_door : public packet_base
{
	short doorNum;
	char open;
public:
	cs_packet_open_door(const short& doorNum, const char& open)
	{
		size = sizeof(cs_packet_open_door);
		type = pOpenDoor;
		this->doorNum = doorNum;
		this->open = open;
	}
	const short getDoorNum() { return doorNum; }
	const char getOpen() { return open; }
};

class cs_packet_unlock_door : public packet_base
{
	short doorNum;
	char success;
public:
	cs_packet_unlock_door(const short& doorNum, const char& success)
	{
		size = sizeof(cs_packet_unlock_door);
		type = pUnlockDoor;
		this->doorNum = doorNum;
		this->success = success;
	}
	const short getDoorNum() { return doorNum; }
	const char getSuccess() { return success; }
};

class cs_packet_get_item : public packet_base
{
	short item_id;
	char slot_id;
public:
	cs_packet_get_item(const int& item_id, const char& slot_id)
	{
		size = sizeof(cs_packet_get_item);
		type = pGetItem;
		this->item_id = static_cast<short>(item_id);
		this->slot_id = slot_id;
	}
	const short getItemID() { return item_id; }
	const char getSlotID() { return slot_id; }
};

class cs_packet_key_input : public packet_base
{
	char key_state;
	char game_input;
public:
	cs_packet_key_input(const KEY_STATE& key_state, const GAME_INPUT& game_input)
	{
		size = sizeof(cs_packet_key_input);
		type = pKeyInput;
		this->key_state = static_cast<char>(key_state);
		this->game_input = static_cast<char>(game_input);
	}
	const KEY_STATE getKeyState() { return static_cast<KEY_STATE>(key_state); }
	const GAME_INPUT getGameInput() { return static_cast<GAME_INPUT>(game_input); }
};

enum SoundType
{
	voice,
	door
};

class cs_packet_sound_start : public packet_base
{
	DirectX::XMFLOAT3 position;
	char type;
public:
	cs_packet_sound_start(const DirectX::XMFLOAT3& pos, const SoundType& sound_type) {
		size = sizeof(cs_packet_sound_start);
		type = pSound;
		position = pos;
		type = static_cast<char>(sound_type);
	}
	const DirectX::XMFLOAT3 getPosition() { return position; }
	const SoundType getType() { return static_cast<SoundType>(type); }
};

#pragma pack(pop)