#include "IOCP_Common.h"
#include "Mesh.h"
#include "IOCP.h"
#include "aStar.h"
std::vector<Mesh*> m_vMeshes;
std::unordered_map<int, NODE*> g_um_graph;
//concurrency::concurrent_unordered_map<int, std::atomic<std::shared_ptr>
std::unordered_map<unsigned int, SESSION> login_players;
std::mutex g_mutex_login_players;

void IOCP_SERVER_MANAGER::start()
{
	LoadResources();

	std::wcout.imbue(std::locale("korean"));
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);

	detail.m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	SOCKET server_s = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server_a;
	server_a.sin_family = AF_INET;
	server_a.sin_port = htons(PORT);
	server_a.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	bind(server_s, reinterpret_cast<sockaddr*>(&server_a), sizeof(server_a));
	listen(server_s, SOMAXCONN);
	int addr_size = sizeof(server_a);


	SOCKET client_s = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
	EXP_OVER accept_over;
	ZeroMemory(&accept_over.over, sizeof(accept_over.over));
	accept_over.c_op = C_ACCEPT;
	accept_over.sock = client_s;
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(server_s), detail.m_hIOCP, -1, 0);
	AcceptEx(server_s, client_s, accept_over.buf, 0, addr_size + 16, addr_size + 16, nullptr, &accept_over.over);

	std::thread command(&IOCP_SERVER_MANAGER::command_thread,this);

	detail.m_bServerState = true;
	std::cout << "서버 실행" << std::endl;
	//lobby = new Lobby;
	//worker(server_s);
	
	std::thread ai_thread{ &IOCP_SERVER_MANAGER::ai_thread,this };

	// 멀티쓰레드
	int num_threads = std::thread::hardware_concurrency();
	std::vector<std::thread> worker_threads;
	for (int i = 0; i < num_threads; ++i)
	{
		worker_threads.emplace_back(&IOCP_SERVER_MANAGER::worker, this, server_s);
	}

	for (auto& w : worker_threads)
		w.join();


	//// 싱글쓰레드 디버그용
	//worker(server_s);

	//delete lobby;
	closesocket(server_s);
	WSACleanup();
	command.join();
	ai_thread.join();

	for (auto& mesh : m_vMeshes)
		delete mesh;
	for (auto& navi : g_um_graph)
		delete navi.second;
}

void IOCP_SERVER_MANAGER::worker(SOCKET server_s)
{
	while (detail.m_bServerState)
	{
		DWORD rw_byte;
		ULONG_PTR key;
		WSAOVERLAPPED* over;
		BOOL ret = GetQueuedCompletionStatus(detail.m_hIOCP, &rw_byte, &key, &over, INFINITE);
		unsigned int my_id = static_cast<unsigned int>(key);

		if (FALSE == ret) {
			print_error("GQCS", WSAGetLastError());
			if (0 == rw_byte) {
				unsigned int gameNum = login_players[my_id].getGameNum();
				if (Games.find(gameNum) == Games.end())
				{
					std::cout << "Error!!" << gameNum << "방이 존재하지 않습니다." << std::endl;
					continue;
				}

				if (Games[gameNum].erasePlayer(my_id))
				{
					std::cout << gameNum << "방의 " << my_id << " 플레이어를 삭제하였습니다." << std::endl;
					auto players = Games[gameNum].getPlayers();
					if (players[0].id == NULL && players[1].id == NULL)
					{
						std::cout << gameNum << "에 플레이어가 존재하지 않습니다. 방을 삭제합니다." << std::endl;
						while (true)
						{
							bool before = true;
							bool after = false;
							if (Games[gameNum].CAS_state(before, after))
								break;
						}
					}
				}
				else
				{
					std::cout << "Error!!" << gameNum << "방의 " << my_id << " 플레이어가 존재하지 않아 삭제하지 못했습니다." << std::endl;
					continue;
				}
				g_mutex_login_players.lock();
				login_players.erase(my_id);
				g_mutex_login_players.unlock();
				std::cout << "login_players에서 " << my_id << "를 삭제하였습니다." << std::endl;
				continue;
			}
		}

		EXP_OVER* e_over = reinterpret_cast<EXP_OVER*>(over);
		switch (e_over->c_op)
		{
		case C_ACCEPT:
		{
			SOCKET client_s = e_over->sock;
			CreateIoCompletionPort(reinterpret_cast<HANDLE>(client_s), detail.m_hIOCP, detail.id, 0);
			g_mutex_login_players.lock();
			login_players.try_emplace(detail.id, detail.id, client_s, PS_LOBBY);
			g_mutex_login_players.unlock();
			login_players[detail.id].do_recv();

			sc_packet_login login(client_s);
			login_players[detail.id++].send_packet(reinterpret_cast<packet_base*>(&login));
			std::cout << "[" << detail.id - 1 << "," << client_s << "] Login" << std::endl;

			client_s = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
			e_over->sock = client_s;
			ZeroMemory(&e_over->over, sizeof(e_over->over));
			int addr_size = sizeof(SOCKADDR_IN);
			AcceptEx(server_s, client_s, e_over->buf, 0, addr_size + 16, addr_size + 16, nullptr, &e_over->over);
		}
		break;
		case C_RECV:
		{
			int current_size = 0;
			int total_recv_packet_size = rw_byte + e_over->prev_packet_size;
			while (current_size < total_recv_packet_size)
			{
				auto base = reinterpret_cast<packet_base*>(&e_over->buf[current_size]);
				char packet_size = base->getSize();
				if (packet_size + current_size > total_recv_packet_size)
				{
					login_players[my_id].set_prev_packet_size(total_recv_packet_size - current_size);
					login_players[my_id].pull_recv_buf(current_size);
					login_players[my_id].do_recv();
					continue;
				}
				else
				{
					process_packet(my_id, e_over);
					current_size += packet_size;
				}
			}
			login_players[my_id].set_prev_packet_size(0);
			login_players[my_id].do_recv();
		}
		break;
		case C_SEND:
		{
			delete e_over;
		}
		break;
		case C_SHUTDOWN:
			std::cout << "서버 종료 명령으로 인한 쓰레드 종료" << std::endl;
			break;
		case C_TIMER:
			//std::cout << "ai 실행" << std::endl;
			delete e_over;
			for (auto& game : Games)
			{
				bool b = true;
				if (game.second.CAS_state(b, b))
				{
					game.second.update(detail.m_bNPC);
				}
			}
			break;
		}
	}
}

void IOCP_SERVER_MANAGER::process_packet(const unsigned int& id, EXP_OVER*& over)
{
	packet_base* base = reinterpret_cast<packet_base*>(over->buf);
	switch (base->getType())
	{
		case pPOSITION:										//		pPOSITION,
		{
			cs_packet_position* position = reinterpret_cast<cs_packet_position*>(base);
			auto ping = std::chrono::high_resolution_clock::now() - position->sendTime;
			auto& gameRoom = Games[position->getNum()];
			
			// 충돌체크 하지 않고 위치 전달
			gameRoom.setPlayerPR(id, position);

			//short floor = floor_collision(position);
			short floor = floor_collision(id, gameRoom);
			if (floor >= 0)
			{
				if (detail.m_bLog)
					std::cout << id << "가 " << floor << "층으로 이동" << std::endl;
				gameRoom.setFloor(id, floor);
			}

			sc_packet_position after_pos(login_players[id].getSock(), position->getPosition(), position->getRotation(), position->getSpeed());

			// 게임 방 내의 플레이어들 모두에게 패킷 전송
			Player* players = Games[position->getNum()].getPlayers();
			for (int i = 0; i < 2; ++i)
			{
				if (players[i].id && players[i].id != id)
				{
					login_players[players[i].id].send_packet(reinterpret_cast<packet_base*>(&after_pos));
				}
			}

			DirectX::XMFLOAT3 pos = position->getPosition();
			DirectX::XMFLOAT3 rot = position->getRotation();

			if(detail.m_bLog)
				std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(ping)<<" [" << id << ", " << login_players[id].getSock() << "] : ( " << pos.x << ", " << pos.y << ", " << pos.z << "), (" << rot.x << ", " << rot.y << ", " << rot.z << ")" << std::endl;
			
		}
			break;
		case pLOGIN:										//		pLOGIN,
			std::cout << "오류 발생 ["<<id<<" , "<<login_players[id].getSock()<<"] 로 부터 pLOGIN 형태의 패킷 수신" << std::endl;
			break;
		case pMAKEROOM:										//		pMAKEROOM
		{
			Games.try_emplace(currentRoom, currentRoom);
			Games[currentRoom].init(id, login_players[id].getSock());
			login_players[id].setState(PS_GAME);
			login_players[id].setGameNum(currentRoom);
			std::cout << "[" << id << ", " << login_players[id].getSock() << "] 이용자가 " << currentRoom << "방을 생성하였습니다." << std::endl;

			sc_packet_make_room make(currentRoom);
			login_players[id].send_packet(reinterpret_cast<packet_base*>(&make));
			++currentRoom;
		}
			break;
		case pENTERROOM:										//		pENTERROOM
		{
			cs_packet_enter_room* cs_enter = reinterpret_cast<cs_packet_enter_room*>(base);
			const unsigned n = cs_enter->getRoomNum();
			auto f = Games.find(n);
			if (f != Games.end()&& f->second.hasEmpty())
			{
				Games[n].init(id, login_players[id].getSock());
				Player* players = Games[n].getPlayers();
				login_players[id].setState(PS_GAME);
				login_players[id].setGameNum(n);
				// 방 입장 알림 보내기 및  방 게스트에게 호스트 소켓번호 보내주기
				sc_packet_enter_room sc_enter(n, true, players[0].sock);
				login_players[id].send_packet(reinterpret_cast<packet_base*>(&sc_enter));

				std::cout << "[" << id << ", " << login_players[id].getSock() << "] 이용자가 " << n << "방에 입장하였습니다." << std::endl;
				
				// 방 호스트에게 게스트 소켓번호 보내주기
				sc_packet_room_player htog(players[1].sock);
				login_players[players[0].id].send_packet(reinterpret_cast<packet_base*>(&htog));
			}
			else
			{
				sc_packet_enter_room sc_enter(n, false, NULL);
				login_players[id].send_packet(reinterpret_cast<packet_base*>(&sc_enter));
			}
		}
			break;
		case pRoomPlayer:
			std::cout << "오류: 게임에 들어가있는 멤버들을 전송하는 패킷이 서버로 들어왔습니다." << std::endl;
			break;
		case pLogout:
			break;
		case pAttack:
			break;
		default:
			std::cout << "정의되지 않은 패킷 타입" << std::endl;
			break;
	}
}

void IOCP_SERVER_MANAGER::LoadResources()
{
	std::cout << "장애물 로드 시작" << std::endl;
	for (const auto& file : std::filesystem::directory_iterator("Resource/"))
	{
		const auto fileName = file.path();
		std::ifstream meshFile(fileName, std::ios::binary);
		if (meshFile.is_open() == false) {
			std::cout << "Failed to open mesh file!! fileName: " << fileName << std::endl;
		}
		Mesh* temp = new Mesh;
		temp->LoadMeshData(meshFile);
		m_vMeshes.emplace_back(temp);
	}
	std::cout << "장애물 로드 완료" << std::endl;
	//------------------------------------------------------------------------------------------
	std::cout << "A* NODE 로드 시작" << std::endl;
	MakeGraph();
	//for (auto& node : g_um_graph)
	//{
	//	std::cout << node.first << " 노드 생성 완료" << std::endl;
	//}
	std::cout << "A* NODE 로드 완료" << std::endl;
}

void IOCP_SERVER_MANAGER::command_thread()
{
	std::string input;
	std::unordered_map<std::string, std::function<void()>> commands = {		// 이곳에 추가하고 싶은 명령어 기입 { 명령어 , 람다 }
		{"/STOP",[this]() {
			detail.m_bServerState = false;
			EXP_OVER over;
			over.c_op = C_SHUTDOWN;
			int num_threads = std::thread::hardware_concurrency();
			for (int i = 0; i < num_threads; ++i)
				PostQueuedCompletionStatus(detail.m_hIOCP, 1, -1, reinterpret_cast<OVERLAPPED*>(&over));
		}}
		,
		{"/LOG",[this]() {
			detail.m_bLog = !detail.m_bLog;
			std::cout << "LOG 출력 : " << detail.m_bLog << std::endl;
		}}
		,
		{"/NPC",[this]() {
			detail.m_bNPC = !detail.m_bNPC;
			std::cout << "NPC 플레이어 추적 : " << detail.m_bNPC << std::endl;
		}}
		,
		{"/HELP",[this]() {
			std::cout << "-------------------------------------------------------------------" << std::endl;
			std::cout << "/STOP : 서버 종료\n/LOG: 로그 출력\n/NPC: NPC가 플레이어 추적" << std::endl;
			std::cout << "-------------------------------------------------------------------" << std::endl;
		}}
		,
		{"/STATE",[this]() {
			g_mutex_login_players.lock();
			for (auto& player : login_players)
			{
				std::cout << "[" << player.first << "]" << std::endl;
			}
			g_mutex_login_players.unlock();
		}}
	};

	while (detail.m_bServerState)
	{
		std::string input;
		std::cin >> input;

		if (commands.find(input) == commands.end())
		{
			std::cout << "해당 명령어가 존재하지 않습니다." << std::endl;
		}
		else commands[input]();
	}
}

void IOCP_SERVER_MANAGER::ai_thread()
{
	using namespace std::chrono;
	while (detail.m_bServerState)
	{
		auto start_t = system_clock::now();
		EXP_OVER* over = new EXP_OVER;
		over->c_op = C_TIMER;
		PostQueuedCompletionStatus(detail.m_hIOCP, 1, 0, &over->over);
		auto end_t = system_clock::now();
		auto hb_time = end_t - start_t;
		if (hb_time < 0.008s)
			std::this_thread::sleep_for(0.008s - hb_time);
		//if (hb_time < 3s)
		//	std::this_thread::sleep_for(3s - hb_time);
	}
}

unsigned short IOCP_SERVER_MANAGER::floor_collision(cs_packet_position*& packet)
{
	DirectX::XMFLOAT3 pivot = packet->getPosition();
	DirectX::XMFLOAT3 temp_extents = { 40,50,40 };
	DirectX::XMFLOAT4 temp_quarta = { 0,0,0,1 };
	DirectX::BoundingOrientedBox player(pivot, temp_extents, temp_quarta);
	
	DirectX::XMFLOAT3 trash;
	short floor;
	m_vMeshes[0]->m_Childs[0].m_Childs[0].floor_collision(player, &trash, &trash, &floor);
	return floor;
}

unsigned short IOCP_SERVER_MANAGER::floor_collision(const unsigned int& id, Game& gameRoom)
{
	DirectX::BoundingOrientedBox player_obb;
	gameRoom.getPlayerOBB(player_obb, id);
	
	DirectX::XMFLOAT3 trash;
	short floor;
	m_vMeshes[0]->m_Childs[0].m_Childs[0].floor_collision(player_obb, &trash, &trash, &floor);
	return floor;
}

void Game::init(const unsigned int& i, const SOCKET& s)
{
	if (player[0].id == NULL)
	{
		player[0].id = i;
		player[0].sock = s;
		player[0].make_obb();
	}
	else
	{
		player[1].id = i;
		player[1].sock = s;
		player[1].make_obb();
	}
}

const DirectX::XMFLOAT3 Game::getPlayerPos(const unsigned int& id)
{
	for (auto& p : player)
	{
		if (p.id == id)
		{
			return p.position;
		}
	}
}

void Game::setPlayerPR(const unsigned int& id, cs_packet_position*& packet)
{
	for (auto& p : player)
	{
		if (p.id == id)
		{
			p.position = packet->getPosition();
			p.rotation = packet->getRotation();
			p.speed = packet->getSpeed();
			break;
		}
	}
}

void Game::setPlayerPR_v2(const unsigned int& id, cs_packet_position*& packet, const DirectX::XMFLOAT3& newSpeed)
{
	for (auto& p : player)
	{
		if (p.id == id)
		{
			p.position = packet->getPosition();
			if (p.position.y < 0) p.position.y = 0;
			p.rotation = packet->getRotation();
			p.speed = newSpeed;
			break;
		}
	}
}

void Game::setPlayerPR_v3(const unsigned int& id, const DirectX::XMFLOAT3& newPosition, const DirectX::XMFLOAT3& newSpeed, const DirectX::XMFLOAT3& rot, const unsigned short& floor)
{
	for (auto& p : player)
	{
		if (p.id == id)
		{
			p.position = newPosition;
			p.rotation = rot;
			p.speed = newSpeed;
			p.m_floor = floor;
			break;
		}
	}
}

void Game::setPlayerRot(const unsigned int& id, cs_packet_position*& packet)
{
	for (auto& p : player)
	{
		if (p.id == id)
		{
			p.rotation = packet->getRotation();
			p.speed = DirectX::XMFLOAT3(0, 0, 0);
			break;
		}
	}
}

void Game::setPlayerRotSpeed(const unsigned int& id, cs_packet_position*& packet, DirectX::XMFLOAT3& newSpeed)
{
	for (auto& p : player)
	{
		if (p.id == id)
		{
			p.rotation = packet->getRotation();
			p.speed = newSpeed;
			DirectX::XMFLOAT3 pos = packet->getPosition();
			if (newSpeed.x == 0)
			{
				p.position.y = pos.y;
				p.position.z = pos.z;
			}
			else if (newSpeed.y == 0)
			{
				p.position.x = pos.x;
				p.position.z = pos.z;
			}
			else if (newSpeed.z == 0)
			{
				p.position.x = pos.x;
				p.position.y = pos.y;
			}
			if (p.position.y < 0) p.position.y = 0;
			break;
		}
	}
}

const DirectX::XMFLOAT3 Game::getPlayerRot(const unsigned int& id)
{
	for (auto& p : player)
	{
		if (p.id == id)
		{
			return p.rotation;
		}
	}
}

const DirectX::XMFLOAT3 Game::getPlayerSp(const unsigned int& id)
{
	for (auto& player : player)
	{
		if (player.id == id)
		{
			return player.speed;
		}
	}
}

void Game::update(const bool& npc_state)
{
	guard.guard_state_machine(player, npc_state);
	//std::cout << "npc 회전: " << guard.rotation.y << std::endl;
	for (auto& student : students)
	{
		student.student_state_machine(player);
	}
	sc_packet_position guard_position(guard.id, guard.position, guard.rotation, guard.speed);
	for (auto& p : player)
	{
		if (p.sock != NULL)
		{
			if (login_players.end() != login_players.find(p.id))
			{
				login_players[p.id].send_packet(reinterpret_cast<packet_base*>(&guard_position));

			}
			else
				p.sock = NULL;
		}
	}
}

bool Game::erasePlayer(const unsigned int& id)
{
	for (int i = 0; i < 2; ++i)
	{
		if (player[i].id == id)
		{
			sc_packet_logout logout(player[i].sock);

			login_players[player[1 - i].id].send_packet(reinterpret_cast<packet_base*>(&logout));
			player[i].reset();
			return true;
		}
	}
	return false;
}

void Game::setFloor(const unsigned int& id, const unsigned short& floor)
{
	for (auto& p : player)
	{
		if (p.id == id)
		{
			p.m_floor = floor;
			return;
		}
	}
}

bool Game::hasEmpty()
{
	for (int i = 0; i < 2; ++i)
	{
		if (player[i].id == NULL)
			return true;
	}
	return false;
}

bool Game::CAS_state(bool& before, bool& after)
{
	return std::atomic_compare_exchange_strong(&state, &before, after);
}

bool Game::getPlayerOBB(DirectX::BoundingOrientedBox& out, const unsigned int& id)
{
	for (auto& p : player)
	{
		if (p.id == id)
		{
			float pitch = DirectX::XMConvertToRadians(p.rotation.x); // x축을 기준으로 회전
			float yaw = DirectX::XMConvertToRadians(p.rotation.y);   // y축을 기준으로 회전
			float roll = DirectX::XMConvertToRadians(p.rotation.z);  // z축을 기준으로 회전
			DirectX::XMVECTOR player_rotation = DirectX::XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);
			DirectX::XMVECTOR player_location = DirectX::XMLoadFloat3(&p.position);
			p.obb.Transform(out, 1, player_rotation, player_location);
			return true;
		}
	}
	return false;
}

void NPC::guard_state_machine(Player* p,const bool& npc_state)
{
	for (auto& node : g_um_graph)
		node.second->init();
	//DirectX::BoundingOrientedBox obb(this->position, { 1,1,1 }, { 0,0,0,1 });
	//DirectX::XMFLOAT3 temp;

	//short now_floor = 0;
	//for (auto& mesh : m_vMeshes)
	//	mesh->m_Childs[0].m_Childs[0].floor_collision(obb, &temp, &temp, &now_floor);

	DirectX::BoundingOrientedBox now_obb;

	float pitch = DirectX::XMConvertToRadians(this->rotation.x); // x축을 기준으로 회전
	float yaw = DirectX::XMConvertToRadians(this->rotation.y);   // y축을 기준으로 회전
	float roll = DirectX::XMConvertToRadians(this->rotation.z);  // z축을 기준으로 회전
	DirectX::XMVECTOR guard_rotation = DirectX::XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);

	DirectX::XMVECTOR guard_location = DirectX::XMLoadFloat3(&this->position);
	this->obb.Transform(now_obb, 1, guard_rotation, guard_location);
	DirectX::XMFLOAT3 temp;

	short now_floor = 0;
	for (auto& mesh : m_vMeshes)
		mesh->m_Childs[0].m_Childs[0].floor_collision(now_obb, &temp, &temp, &now_floor);
	if (now_floor >= 0)
	{
		//std::cout << "npc가 " << now_floor << "로 이동" << std::endl;
		m_floor = now_floor;
	}

	if (this->state == 0)				// 두리번 두리번 상태
	{
		if (set_destination(p, npc_state))			// 두리번 거리다가 뭔갈 발견했으면, 목적지 조정하고 이동
		{
			this->state = 1;
		}
		else
		{	// 위치를 못찾았으면
			int duribun_duribun = 5;
			auto duration = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - arrive_time).count();
			if (duration > duribun_duribun)		// 5초 넘게 두리번 거렸는데 위치를 못찾았으면
			{
				int des = rand() % g_um_graph.size();
				std::cout << des << "로 목적지 설정" << std::endl;
				this->destination = g_um_graph[des]->pos; // 랜덤한 목적지 설정
				this->path = aStarSearch(position, destination);
				this->destination = path->pos;
				this->state = 1;
			}
		}
	}
	else if (this->state == 1)	// 목적지로 이동하는 중
	{
		set_destination(p, npc_state);		// 이동 하면서도 눈으로 보이는지, 소리가 들리는지 확인
	}

	if(state == 1)
		move();
}

void NPC::student_state_machine(Player* p)
{
	DirectX::BoundingOrientedBox now_obb;

	float pitch = DirectX::XMConvertToRadians(this->rotation.x); // x축을 기준으로 회전
	float yaw = DirectX::XMConvertToRadians(this->rotation.y);   // y축을 기준으로 회전
	float roll = DirectX::XMConvertToRadians(this->rotation.z);  // z축을 기준으로 회전
	DirectX::XMVECTOR student_rotation = DirectX::XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);

	DirectX::XMVECTOR student_location = DirectX::XMLoadFloat3(&this->position);
	this->obb.Transform(now_obb, 1, student_rotation, student_location);
	// 다른 캐릭터와 충돌했나?
	bool hit = false;
	DirectX::BoundingOrientedBox player_obb;
	float player_pitch;
	float player_yaw;
	float player_roll;
	DirectX::XMVECTOR player_rotation;
	DirectX::XMVECTOR player_location;
	for (int i = 0; i < 2; ++i)
	{
		if (p[i].sock == NULL)
			continue;
		player_pitch = DirectX::XMConvertToRadians(p[i].rotation.x);
		player_yaw = DirectX::XMConvertToRadians(p[i].rotation.y);
		player_roll = DirectX::XMConvertToRadians(p[i].rotation.z);
		player_rotation = DirectX::XMQuaternionRotationRollPitchYaw(player_pitch, player_yaw, player_roll);
		player_location = DirectX::XMLoadFloat3(&p[i].position);
		p[i].obb.Transform(player_obb, 1, player_rotation, player_location);
		hit = now_obb.Intersects(player_obb);
		if (hit)
		{
			std::cout << i<<" 와 충돌" << std::endl;
			break;
		}
	}

	//if (hit)
	//{
	//	using namespace std::chrono;
	//	attacked_time = steady_clock::now();

	//	sc_packet_npc_attack attack_student(this->id);
	//	for (int i = 0; i < 2; ++i)
	//	{
	//		if(p[i].sock != NULL)
	//			login_players[p[i].id].send_packet(reinterpret_cast<packet_base*>(&attack_student));
	//	}
	//}
	//else
	//{
	//	if (this->state == 0)			// 목적지 도착한 상태
	//	{
	//		// 2초동안 대기
	//		int stop = 2;
	//		auto duration = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - arrive_time).count();
	//		if (duration > 2)			// 2초 대기 이후
	//		{
	//			// 새로운 목표 위치 선정
	//			int des = rand() % g_um_graph.size();
	//			std::cout <<"student["<<this->id<<"] "<< des << "로 목적지 설정" << std::endl;
	//			this->state = 1;
	//		}
	//	}
	//	else if (this->state == 1)		// 목적지로 이동하는 중
	//	{
	//		move();		// 목표 위치로 이동
	//	}
	//}
}

bool NPC::can_see(Player& p)
{
	if (p.m_floor != m_floor)
		return false;

	DirectX::XMFLOAT3 playerPos = p.position;
	playerPos.y += 50;
	DirectX::XMFLOAT3 npcPos = position;
	npcPos.y += 50;
	for (auto& mesh : m_vMeshes)
	{
		if (mesh->can_see(playerPos, npcPos, m_floor))		// npc -> 플레이어로의 광선과 장애물들을 ray 충돌하여 npc가 플레이어를 볼 수 있는지 확인
		{
			//std::cout << "보인다 보여.." << std::endl;
			return true;
		}
	}
	return false;
}

bool NPC::can_hear(Player& p)
{
	const unsigned short can_hear_distance = 100;
	bool playerSound = p.sound.load();
	if (playerSound && distance(p) < can_hear_distance * can_hear_distance)		// 플레이어가 소리를 내고 있으며, 플레이어와의 거리가 들을 수 있는 거리 이내이면
		return true;
	return false;
}

float NPC::distance(Player& p)
{
	return (position.x - p.position.x) * (position.x - p.position.x) + (position.y - p.position.y) * (position.y - p.position.y) + (position.z - p.position.z) * (position.z - p.position.z);
}

bool NPC::compare_position(DirectX::XMFLOAT3& pos)
{
	float error_value = 5;
	if (this->position.x >= pos.x - error_value && this->position.x <= pos.x + error_value)
		if (this->position.y >= pos.y - error_value && this->position.y <= pos.y + error_value)
			if (this->position.z >= pos.z - error_value && this->position.z <= pos.z + error_value)
			{
				if (state == 1)
				{
					std::cout << "목적지 도착" << std::endl;
					return true;
				}
			}
	return false;
}

bool NPC::set_destination(Player*& p, const bool& npc_state)
{
	short n = find_near_player(p);
	if (npc_state) {
		for (int i = 0; i < 2; ++i)
		{
			if (p[n].sock == NULL)
			{
				n = 1 - n;
				continue;
			}

			if (can_see(p[n]))
			{
				std::cout << n << "P 발견" << std::endl;
				path = aStarSearch(position, destination);
				//this->destination = path->pos;
				this->destination = p[n].position;
				this->destination.y = position.y;
				if (nullptr != path)
					return true;
				else
					state = 0;
			}
			else if (can_hear(p[n]))
			{
				this->destination = p[n].position;
				this->destination.y = position.y;
				path = aStarSearch(position, destination);
				if (nullptr != path)
					return true;
				else
					state = 0;
			}
			n = 1 - n;
		}
	}

	// 플레이어를 찾지도, 듣지도 못했다면
	if (compare_position(this->destination))		// 목적지에 도달하였으면
	{
		if (this->path == nullptr || this->path->next == nullptr)			// 더 이상의 목적지가 없으면 두리번 상태로 변경
		{
			if (this->state != 0)
			{
				arrive_time = std::chrono::high_resolution_clock::now();	// 도달한 시각 기록
				this->state = 0;
				this->speed = DirectX::XMFLOAT3(0, 0, 0);
				std::cout << "두리번 상태로 변경" << std::endl;
			}
		}
		else
		{
			path = path->next;
			destination = path->pos;
		}

	}
	return false;
}

void NPC::move()
{
	using namespace DirectX;
	XMVECTOR ActorPos = XMLoadFloat3(&position);
	XMVECTOR DestPos = XMLoadFloat3(&destination);
	XMVECTOR direction = XMVector3Normalize(XMVectorSubtract(DestPos, ActorPos));
	XMVECTOR movement = XMVectorScale(direction, 5);
	XMStoreFloat3(&position, XMVectorAdd(ActorPos, movement));
	//std::cout << "경비 위치 " << position.x<< ", " << position.y<< ", " << position.z<< std::endl;
	
	XMFLOAT3 temp;
	XMStoreFloat3(&temp, direction);
	temp.y = 0;
	XMVECTOR direction_xz = XMVector3Normalize(XMLoadFloat3(&temp));
	XMFLOAT3 basic_head(0, 0, 1);
	XMVECTOR basic = XMLoadFloat3(&basic_head);
	XMMATRIX rotationMatrix = XMMatrixRotationY(XMConvertToRadians(rotation.y));
	XMVECTOR dir = XMVector3Transform(basic, rotationMatrix);

	//float costheta = XMVectorGetX(XMVector3Dot(basic, direction_xz));
	float costheta = XMVectorGetX(XMVector3Dot(dir, direction_xz));
	float radian = acos(costheta);
	float degree = XMConvertToDegrees(radian);

	//XMVECTOR cross = XMVector3Cross(basic, direction_xz);
	XMVECTOR cross = XMVector3Cross(dir, direction_xz);

	float wise = XMVectorGetY(cross);
	if (wise < 0)
	{
		degree = -degree;
	}
	
	//std::cout << degree << std::endl;
	//rotation.y = degree;
	if(degree*degree > 0.25) rotation.y += degree / 10;
	movement *= 100;
	DirectX::XMStoreFloat3(&speed, movement);
}

const short NPC::find_near_player(Player*& players)
{
	float distance[2] = { 1E+37,1E+37 };
	for (int i = 0; i < 2; ++i)
	{
		if (players[i].sock)
		{
			distance[i] = (this->position.x - players[i].position.x) * (this->position.x - players[i].position.x)
				+ (this->position.y - players[i].position.y) * (this->position.y - players[i].position.y)
				+ (this->position.z - players[i].position.z) * (this->position.z - players[i].position.z);
		}
	}

	if (distance[0] <= distance[1]) return 0;
	else return 1;

}
