#pragma once
constexpr short PORT = 9000;
constexpr int BUFSIZE = 1024;
constexpr int STUDENT_SIZE = 1;

class packet_base;
class sc_packet_position;
class sc_packet_login;
class Mesh;
class NODE;

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
	short prev_packet_size = 0;

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

	void set_prev_packet_size(const short& size)
	{
		recv_over.prev_packet_size = size;
		if (size == 0)
			ZeroMemory(recv_over.buf, BUFSIZE);
		recv_over.wsabuf->buf = recv_over.buf + size;
		recv_over.wsabuf->len = BUFSIZE - size;
	}

	void pull_recv_buf(const int& start)
	{
		memcpy(recv_over.buf, recv_over.buf + start, BUFSIZE - start);
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
				break;
			}
			case pLOGIN:				// LOGIN
			{
				auto login = reinterpret_cast<sc_packet_login*>(base);
				auto sendOver = new EXP_OVER(login);
				sendOver->c_op = C_SEND;
				int res = WSASend(client_s, sendOver->wsabuf, 1, nullptr, 0, &sendOver->over, nullptr);
				if (0 != res) {
					print_error("WSASend", WSAGetLastError());
				}
				break;
			}
			case pMAKEROOM:				//	pMAKEROOM
			{
				auto room = reinterpret_cast<sc_packet_make_room*>(base);
				auto sendOver = new EXP_OVER(room);
				sendOver->c_op = C_SEND;
				int res = WSASend(client_s, sendOver->wsabuf, 1, nullptr, 0, &sendOver->over, nullptr);
				if (0 != res) {
					print_error("WSASend", WSAGetLastError());
				}
				break;
			}
			case pENTERROOM:				// pENTERROOM
			{
				auto enter = reinterpret_cast<sc_packet_enter_room*>(base);
				auto sendOver = new EXP_OVER(enter);
				sendOver->c_op = C_SEND;
				int res = WSASend(client_s, sendOver->wsabuf, 1, nullptr, 0, &sendOver->over, nullptr);
				if (0 != res) {
					print_error("WSASend", WSAGetLastError());
				}
				break;
			}
			case pRoomPlayer:				//	pRoomPlayer
			{
				auto player = reinterpret_cast<sc_packet_room_player*>(base);
				auto sendOver = new EXP_OVER(player);
				sendOver->c_op = C_SEND;
				int res = WSASend(client_s, sendOver->wsabuf, 1, nullptr, 0, &sendOver->over, nullptr);
				if (0 != res) {
					print_error("WSASend", WSAGetLastError());
				}
				break;
			}
			case pLogout:				// logout
			{
				auto logout = reinterpret_cast<sc_packet_logout*>(base);
				auto sendOver = new EXP_OVER(logout);
				sendOver->c_op = C_SEND;
				int res = WSASend(client_s, sendOver->wsabuf, 1, nullptr, 0, &sendOver->over, nullptr);
				if (0 != res) {
					print_error("WSASend", WSAGetLastError());
				}
				break;
			}
			case pAttack:
			{
				//std::cout << "attack send" << std::endl;
				auto attack = reinterpret_cast<sc_packet_npc_attack*>(base);
				auto sendOver = new EXP_OVER(attack);
				sendOver->c_op = C_SEND;
				int res = WSASend(client_s, sendOver->wsabuf, 1, nullptr, 0, &sendOver->over, nullptr);
				if (0 != res) {
					print_error("WSASend", WSAGetLastError());
				}
				break;
			}
			case pAnimation:
			{
				auto anim = reinterpret_cast<sc_packet_anim_type*>(base);
				auto sendOver = new EXP_OVER(anim);
				sendOver->c_op = C_SEND;
				int res = WSASend(client_s, sendOver->wsabuf, 1, nullptr, 0, &sendOver->over, nullptr);
				if (0 != res) {
					print_error("WSASend", WSAGetLastError());
				}
				break;
			}
			case pOpenDoor:
			{
				auto open = reinterpret_cast<cs_packet_open_door*>(base);
				auto sendOver = new EXP_OVER(open);
				sendOver->c_op = C_SEND;
				int res = WSASend(client_s, sendOver->wsabuf, 1, nullptr, 0, &sendOver->over, nullptr);
				if (0 != res) {
					print_error("WSASend", WSAGetLastError());
				}
				std::cout << "오픈 패킷 전송 완료" << std::endl;
			}
				break;
			case pUnlockDoor:
			{
				auto unlock = reinterpret_cast<cs_packet_unlock_door*>(base);
				auto sendOver = new EXP_OVER(unlock);
				sendOver->c_op = C_SEND;
				int res = WSASend(client_s, sendOver->wsabuf, 1, nullptr, 0, &sendOver->over, nullptr);
				if (0 != res) {
					print_error("WSASend", WSAGetLastError());
				}
				std::cout << "언락 전송 완료" << std::endl;
			}
				break;
			case pChangeDayOrNight:
			{
				auto changeDayOrNight = reinterpret_cast<sc_packet_change_day_or_night*>(base);
				auto sendOver = new EXP_OVER(changeDayOrNight);
				sendOver->c_op = C_SEND;
				int res = WSASend(client_s, sendOver->wsabuf, 1, nullptr, 0, &sendOver->over, nullptr);
				if (0 != res) {
					print_error("WSASend", WSAGetLastError());
				}
			}
				break;
			case pGetItem:
			{
				auto getItem = reinterpret_cast<sc_packet_get_item*>(base);
				auto sendOver = new EXP_OVER(getItem);
				sendOver->c_op = C_SEND;
				int res = WSASend(client_s, sendOver->wsabuf, 1, nullptr, 0, &sendOver->over, nullptr);
				if (0 != res) {
					print_error("WSASend", WSAGetLastError());
				}
			}
				break;
			case pKeyInput:
			{
				auto keyInput = reinterpret_cast<sc_packet_key_input*>(base);
				auto sendOver = new EXP_OVER(keyInput);
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

constexpr DirectX::XMFLOAT3 basic_extents = { 26,20,26 };		// 클라상에서 이미 충돌을 하고 서버에 전송하기에 클라의 충돌 범위인 25,85,25 보다 크게 해야 서버에서 충돌이 발생

class Player
{
public:
	unsigned int id = NULL;
	SOCKET sock = NULL;
	float m_floor = 0;
	std::atomic_bool sound;
	DirectX::XMFLOAT3 position = { 0,0,0 };
	DirectX::XMFLOAT3 rotation = { 0,0,0 };
	DirectX::XMFLOAT3 speed = { 0,0,0 };
	DirectX::BoundingOrientedBox obb;
	void reset()
	{
		id = NULL;
		sock = NULL;
		m_floor = 0;
		sound = false;
		position = { 0,0,0 };
		rotation = { 0,0,0 };
		speed = { 0,0,0 };
		obb = DirectX::BoundingOrientedBox();
	}
	void make_obb()
	{
		DirectX::XMFLOAT3 temp_extents = basic_extents;
		DirectX::XMFLOAT4 temp_quarta = { 0,0,0,1 };
		obb = DirectX::BoundingOrientedBox(DirectX::XMFLOAT3(0, 0, 0), temp_extents, temp_quarta);		// 0, 85, 0
	}
};

class PATH;

class npc_info
{
public:
	unsigned int gameNum;
	unsigned int id;
	std::chrono::steady_clock::time_point start_time;
	npc_info(const unsigned int& npc_gamenum, const unsigned int& npc_id, const std::chrono::milliseconds& wake_time) :gameNum(npc_gamenum), id(npc_id) { start_time = std::chrono::steady_clock::now() + wake_time; }

	bool operator<(const npc_info& n)const
	{
		return start_time > n.start_time;
	}
};

class NPC
{
public:
	short state = 0;				// 0: idle, 1: 이동중 , 2: 충돌 애니메이션 진행중
	std::atomic_bool updating = false;
	unsigned int id = NULL;
	float m_floor = 1;
	DirectX::XMFLOAT3 position = { 0,0,0 };
	DirectX::XMFLOAT3 rotation = { 0,0,0 };
	DirectX::XMFLOAT3 speed = { 0,0,0 };
	DirectX::XMFLOAT3 destination = { 0,0,0 };
	std::chrono::steady_clock::time_point arrive_time;
	std::chrono::steady_clock::time_point attacked_time;
	PATH* path = nullptr;
	DirectX::BoundingOrientedBox obb;
	float movement_speed;
	NODE* goalNode = nullptr;

	std::unordered_map<int, NODE*> astar_graph;
public:
	NPC();
	void guard_state_machine(Player*, const bool& npc_state);
	void student_state_machine(Player*);
	bool can_see(Player&,bool& floor_gap);
	bool can_hear(Player&);
	float distance(Player&);
	bool compare_position(DirectX::XMFLOAT3&);
	bool set_destination(Player*&, const bool& npc_state);
	void move();
	const short find_near_player(Player*&);
	void reset_graph();
	void reset_path();
};

//struct GameDetails
//{
//	bool m_bLOG = false;
//	bool m_bNPC = true;
//};

class Game
{
	std::atomic_bool state;				// 0 = 삭제된애, 1 = 돌아가고있는애
	unsigned int GameNum;
	Player player[2];
	NPC guard;
	NPC students[STUDENT_SIZE];
	
public:
	Game() { std::cout << "Game initialize error" << std::endl; }
	Game(const unsigned int& n) : GameNum(n), state(true) {
		// 가드 초기위치 설정
		guard.position = DirectX::XMFLOAT3(3160, 0, -400);
		guard.id = 1;
		
		// npc 초기위치를 어떻게 설정할깝쇼
		// 노가다로 설정해둘까
		for (int i = 0; i < STUDENT_SIZE; ++i)
		{
			students[i].id = i+2;
			students[i].position = DirectX::XMFLOAT3(500.0, 0.0, 1000.0);
			students[i].movement_speed = 1;
		}
	}
	void init(const unsigned int& i, const SOCKET& s);
	Player* getPlayers() { return player; };
	void setPlayerPR(const unsigned int&, cs_packet_position*&);
	void setPlayerPR_v2(const unsigned int&, cs_packet_position*&, const DirectX::XMFLOAT3&);
	void setPlayerPR_v3(const unsigned int& id, const DirectX::XMFLOAT3& newPosition, const DirectX::XMFLOAT3& newSpeed, const DirectX::XMFLOAT3& rot, const unsigned short& floor);
	void setPlayerRot(const unsigned int&, cs_packet_position*&);
	void setPlayerRotSpeed(const unsigned int&, cs_packet_position*&, DirectX::XMFLOAT3&);
	const DirectX::XMFLOAT3 getPlayerPos(const unsigned int&);
	const DirectX::XMFLOAT3 getPlayerRot(const unsigned int&);
	const DirectX::XMFLOAT3 getPlayerSp(const unsigned int&);
	bool erasePlayer(const unsigned int& id);
	void setFloor(const unsigned int& id, const float& floor);
	void update(const bool& npc_state);
	bool hasEmpty();
	bool CAS_state(bool& before, bool& after);
	bool getPlayerOBB(DirectX::BoundingOrientedBox& out, const unsigned int& id);
	void update(const bool& npc_state, const unsigned int& npc_id);
	
};

struct ServerDetails
{
	bool m_bServerState = false;
	HANDLE m_hIOCP;
	std::atomic_int id = 1;
	bool m_bLog = false;
	bool m_bNPC = true;
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
	unsigned short floor_collision(cs_packet_position*& packet);
	float floor_collision(const unsigned int& id, Game& gameRoom);
	void command_thread();
	void ai_thread();
	void ai_timer();
};