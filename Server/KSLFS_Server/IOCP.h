#pragma once
constexpr short PORT = 9000;
constexpr int BUFSIZE = 256;

class packet_base;
class sc_packet_position;
class sc_packet_login;
class Mesh;

extern std::vector<Mesh*> m_vMeshes;

enum C_OP { C_RECV, C_SEND, C_ACCEPT, C_SHUTDOWN, C_TIMER };

class EXP_OVER
{
public:
	WSAOVERLAPPED over;
	WSABUF wsabuf[1];
	char buf[BUFSIZE];
	C_OP	c_op;
	SOCKET sock;

	EXP_OVER()
	{
		ZeroMemory(&over, sizeof(over));
		wsabuf[0].buf = buf;
		wsabuf[0].len = BUFSIZE;
	}

	EXP_OVER(int s_id, char* mess, int m_size)
	{
		ZeroMemory(&over, sizeof(over));
		wsabuf[0].buf = buf;
		wsabuf[0].len = m_size + 2;

		buf[0] = m_size + 2;
		buf[1] = s_id;
		memcpy(buf + 2, mess, m_size);
	}

	template<typename packet>
	EXP_OVER(packet* p)
	{
		ZeroMemory(&over, sizeof(over));
		wsabuf[0].buf = buf;
		wsabuf[0].len = sizeof(packet);
		memcpy(buf, p, sizeof(packet));
	}
};

enum PlayerState { PS_LOBBY, PS_GAME };

class SESSION {
	EXP_OVER recv_over;
	SOCKET client_s;
	PlayerState state;
	int mi_id;
	unsigned int gameNum = NULL;
public:
	SESSION(int id, SOCKET s, PlayerState ps) :mi_id(id), client_s(s), state(ps) {
		recv_over.c_op = C_RECV;
	}
	SESSION() {
		//std::cout << "ERROR";
		//exit(-1);
	}
	~SESSION() { closesocket(client_s); }
	void do_recv()
	{
		DWORD recv_flag = 0;
		ZeroMemory(&recv_over.over, sizeof(recv_over.over));
		int res = WSARecv(client_s, recv_over.wsabuf, 1, nullptr, &recv_flag, &recv_over.over, nullptr);
		if (0 != res) {
			int err_no = WSAGetLastError();
			if (WSA_IO_PENDING != err_no)
				print_error("WSARecv", WSAGetLastError());
		}
	}

	void do_send(int s_id, char* mess, int recv_size)
	{
		auto b = new EXP_OVER(s_id, mess, recv_size);
		b->c_op = C_SEND;
		int res = WSASend(client_s, b->wsabuf, 1, nullptr, 0, &b->over, nullptr);
		if (0 != res) {
			print_error("WSASend", WSAGetLastError());
		}
	}


	void send_packet(packet_base* base)
	{
		switch (base->getType())
		{
			case 0:				// POSITION
			{
				auto position = reinterpret_cast<sc_packet_position*>(base);
				auto sendOver = new EXP_OVER(position);
				sendOver->c_op = C_SEND;
				int res = WSASend(client_s, sendOver->wsabuf, 1, nullptr, 0, &sendOver->over, nullptr);
				if (0 != res) {
					print_error("WSASend", WSAGetLastError());
				}
			}
				break;
			case 1:				// LOGIN
			{
				auto login = reinterpret_cast<sc_packet_login*>(base);
				auto sendOver = new EXP_OVER(login);
				sendOver->c_op = C_SEND;
				int res = WSASend(client_s, sendOver->wsabuf, 1, nullptr, 0, &sendOver->over, nullptr);
				if (0 != res) {
					print_error("WSASend", WSAGetLastError());
				}
			}
				break;
			case 2:				//	pMAKEROOM
			{
				auto room = reinterpret_cast<sc_packet_make_room*>(base);
				auto sendOver = new EXP_OVER(room);
				sendOver->c_op = C_SEND;
				int res = WSASend(client_s, sendOver->wsabuf, 1, nullptr, 0, &sendOver->over, nullptr);
				if (0 != res) {
					print_error("WSASend", WSAGetLastError());
				}
			}
				break;
			case 3:				// pENTERROOM
			{
				auto enter = reinterpret_cast<sc_packet_enter_room*>(base);
				auto sendOver = new EXP_OVER(enter);
				sendOver->c_op = C_SEND;
				int res = WSASend(client_s, sendOver->wsabuf, 1, nullptr, 0, &sendOver->over, nullptr);
				if (0 != res) {
					print_error("WSASend", WSAGetLastError());
				}
			}
				break;
			case 4:				//	pRoomPlayer
			{
				auto player = reinterpret_cast<sc_packet_room_player*>(base);
				auto sendOver = new EXP_OVER(player);
				sendOver->c_op = C_SEND;
				int res = WSASend(client_s, sendOver->wsabuf, 1, nullptr, 0, &sendOver->over, nullptr);
				if (0 != res) {
					print_error("WSASend", WSAGetLastError());
				}
			}
				break;
		}
	}

	void print_message(DWORD recv_size)
	{
		std::cout << "Client[" << mi_id << "] Sent : ";
		for (DWORD i = 0; i < recv_size; ++i)
			std::cout << recv_over.buf[i];
		std::cout << std::endl;
	}

	const SOCKET getSock() { return client_s; }

	void setState(const PlayerState& ps) { state = ps; }

	const unsigned int getGameNum() { return gameNum; }

	void setGameNum(const unsigned int& gameNum) { this->gameNum = gameNum; }
};

extern std::unordered_map<unsigned int, SESSION> login_players;

class Lobby
{
};

class Player
{
public:
	unsigned int id = NULL;
	SOCKET sock = NULL;
	unsigned short m_floor = 0;
	std::atomic_bool sound;
	DirectX::XMFLOAT3 position = { 0,0,0 };
	DirectX::XMFLOAT3 rotation = { 0,0,0 };
	DirectX::XMFLOAT3 speed = { 0,0,0 };

	void reset()
	{
		id = NULL;
		sock = NULL;
		m_floor = 0;
		sound = false;
		position = { 0,0,0 };
		rotation = { 0,0,0 };
		speed = { 0,0,0 };
	}
};

class PATH;

class NPC
{
public:
	short state = 0;				// 0: idle, 1: ¿Ãµø¡ﬂ
	unsigned int id = NULL;
	unsigned short m_floor = 0;
	DirectX::XMFLOAT3 position = { 31.18,0,-8.53 };
	DirectX::XMFLOAT3 rotation = { 0,0,0 };
	DirectX::XMFLOAT3 speed = { 0,0,0 };
	DirectX::XMFLOAT3 destination = { 0,0,0 };
	std::chrono::steady_clock::time_point arrive_time;
	PATH* path = nullptr;
	void state_machine(Player*);
	bool can_see(Player&);
	bool can_hear(Player&);
	float distance(Player&);
	bool compare_position(DirectX::XMFLOAT3&);
	bool set_destination(Player*&);
	void move();
};

class Game
{
	unsigned int GameNum;
	Player p[2];
	NPC guard;
public:
	Game() { std::cout << "Game initialize error" << std::endl; }
	Game(const unsigned int& n) : GameNum(n) {}
	void init(const unsigned int& i, const SOCKET& s);
	Player* getPlayers() { return p; };
	void setPlayerPR(const unsigned int&, cs_packet_position*&);
	void setPlayerPR_v2(const unsigned int&, cs_packet_position*&, const DirectX::XMFLOAT3&);
	void setPlayerPR_v3(const unsigned int& id, const DirectX::XMFLOAT3& newPosition, const DirectX::XMFLOAT3& newSpeed, const DirectX::XMFLOAT3& rot, const unsigned short& floor);
	void setPlayerRot(const unsigned int&, cs_packet_position*&);
	void setPlayerRotSpeed(const unsigned int&, cs_packet_position*&, DirectX::XMFLOAT3&);
	const DirectX::XMFLOAT3 getPlayerPos(const unsigned int&);
	const DirectX::XMFLOAT3 getPlayerRot(const unsigned int&);
	const DirectX::XMFLOAT3 getPlayerSp(const unsigned int&);
	bool erasePlayer(const unsigned int& id);
	void update();
};

class ServerDetails
{
public:
	bool m_bServerState = false;
	HANDLE m_hIOCP;
	std::atomic_int id = 1;
};

class IOCP_SERVER_MANAGER
{
private:
	std::atomic_int currentRoom = 10000;
	std::unordered_map<unsigned int, Game> Games;

	ServerDetails detail;
public:
	IOCP_SERVER_MANAGER() {}
	void start();
	void LoadResources();
	void worker(SOCKET server_s);
	void process_packet(const unsigned int&, EXP_OVER*&);
	bool world_collision(cs_packet_position*&);
	bool world_collision_v2(cs_packet_position*&, DirectX::XMFLOAT3*);
	bool world_collision_v3(cs_packet_position*& player, DirectX::XMFLOAT3* newPosition, DirectX::XMFLOAT3* newSpeed,unsigned short* floor, std::chrono::nanoseconds& ping);
	void command_thread();
	void ai_thread();
};