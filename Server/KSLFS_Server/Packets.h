#pragma once

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
	pMAKEROOM
};

class packet_base
{
protected:
	int size;
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
public:
	sc_packet_position(const sc_packet_position& t) : player(t.player), position(t.position), rotation(t.rotation) {
		size = t.size;
		type = t.type;
	}
	const SOCKET getPlayer() { return player; }
	const DirectX::XMFLOAT3 getPos() { return position; }
	const DirectX::XMFLOAT3 getRot() { return rotation; }
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

//--------------------------------------------------------------------------------------------------------------------

class cs_packet_position : public packet_base
{
private:
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 rotation;
public:
	cs_packet_position(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& rot) :position(pos), rotation(rot)
	{
		size = sizeof(cs_packet_position);
		type = pPOSITION;
	};
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