#include "IOCP_Common.h"
#include "Mesh.h"
#include "aStar.h"
#include "IOCP.h"
std::vector<Mesh*> m_vMeshes;
std::unordered_map<int, NODE*> g_um_graph;
//concurrency::concurrent_unordered_map<int, std::atomic<std::shared_ptr>
std::unordered_map<unsigned int, SESSION> login_players;
std::priority_queue<npc_info> g_npc_timer;
std::mutex g_mutex_npc_timer;
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
	std::cout << "���� ����" << std::endl;
	//lobby = new Lobby;
	//worker(server_s);
	
	std::thread ai_thread{ &IOCP_SERVER_MANAGER::ai_timer,this };

	// ��Ƽ������
	int num_threads = std::thread::hardware_concurrency();
	std::vector<std::thread> worker_threads;
	for (int i = 0; i < num_threads; ++i)
	{
		worker_threads.emplace_back(&IOCP_SERVER_MANAGER::worker, this, server_s);
	}

	for (auto& w : worker_threads)
		w.join();


	//// �̱۾����� ����׿�
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
					std::cout << "Error!!" << gameNum << "���� �������� �ʽ��ϴ�." << std::endl;
					continue;
				}

				if (Games[gameNum].erasePlayer(my_id))
				{
					std::cout << gameNum << "���� " << my_id << " �÷��̾ �����Ͽ����ϴ�." << std::endl;
					auto players = Games[gameNum].getPlayers();
					if (players[0].id == NULL && players[1].id == NULL)
					{
						std::cout << gameNum << "�� �÷��̾ �������� �ʽ��ϴ�. ���� �����մϴ�." << std::endl;
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
					std::cout << "Error!!" << gameNum << "���� " << my_id << " �÷��̾ �������� �ʾ� �������� ���߽��ϴ�." << std::endl;
					continue;
				}
				g_mutex_login_players.lock();
				login_players.erase(my_id);
				g_mutex_login_players.unlock();
				std::cout << "login_players���� " << my_id << "�� �����Ͽ����ϴ�." << std::endl;
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
			std::cout << "���� ���� ������� ���� ������ ����" << std::endl;
			break;
		case C_TIMER:
			//std::cout << "ai ����" << std::endl;
			delete e_over;
			//for (auto& game : Games)
			//{
			//	bool b = true;
			//	if (game.second.CAS_state(b, b))
			//	{
			//		game.second.update(detail.m_bNPC);
			//	}
			//}
			Games[rw_byte].update(detail.m_bNPC, my_id);
			bool b = true;
			if (Games[rw_byte].CAS_state(b, b))
			{
				using namespace std::chrono;
				g_mutex_npc_timer.lock();
				g_npc_timer.emplace(rw_byte, my_id, milliseconds(17));
				g_mutex_npc_timer.unlock();
				//std::cout << "NPC[" << my_id << "] 0.008�� �� ������Ʈ �߰�" << std::endl;
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
			
			// �浹üũ ���� �ʰ� ��ġ ����
			gameRoom.setPlayerPR(id, position);

			//short floor = floor_collision(position);
			float floor = floor_collision(id, gameRoom);
			if (floor < 6)
			{
				if (detail.m_bLog)
					std::cout << id << "�� " << floor << "������ �̵�" << std::endl;
				gameRoom.setFloor(id, floor);
			}

			sc_packet_position after_pos(login_players[id].getSock(), position->getPosition(), position->getRotation(), position->getSpeed());

			// ���� �� ���� �÷��̾�� ��ο��� ��Ŷ ����
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
			std::cout << "���� �߻� ["<<id<<" , "<<login_players[id].getSock()<<"] �� ���� pLOGIN ������ ��Ŷ ����" << std::endl;
			break;
		case pMAKEROOM:										//		pMAKEROOM
		{
			Games.try_emplace(currentRoom, currentRoom);
			Games[currentRoom].init(id, login_players[id].getSock());
			login_players[id].setState(PS_GAME);
			login_players[id].setGameNum(currentRoom);
			std::cout << "[" << id << ", " << login_players[id].getSock() << "] �̿��ڰ� " << currentRoom << "���� �����Ͽ����ϴ�." << std::endl;

			sc_packet_make_room make(currentRoom);
			login_players[id].send_packet(reinterpret_cast<packet_base*>(&make));
			++currentRoom;
			
			g_mutex_npc_timer.lock();
			for (int npc_id = 1; npc_id < STUDENT_SIZE + 2; ++npc_id)
			{
				g_npc_timer.emplace(currentRoom - 1, npc_id, std::chrono::milliseconds(0));
			}
			g_mutex_npc_timer.unlock();

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
				// �� ���� �˸� ������ ��  �� �Խ�Ʈ���� ȣ��Ʈ ���Ϲ�ȣ �����ֱ�
				sc_packet_enter_room sc_enter(n, true, players[0].sock);
				login_players[id].send_packet(reinterpret_cast<packet_base*>(&sc_enter));

				std::cout << "[" << id << ", " << login_players[id].getSock() << "] �̿��ڰ� " << n << "�濡 �����Ͽ����ϴ�." << std::endl;
				
				// �� ȣ��Ʈ���� �Խ�Ʈ ���Ϲ�ȣ �����ֱ�
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
			std::cout << "����: ���ӿ� ���ִ� ������� �����ϴ� ��Ŷ�� ������ ���Խ��ϴ�." << std::endl;
			break;
		case pLogout:
			break;
		case pAttack:
			break;
		default:
			std::cout << "���ǵ��� ���� ��Ŷ Ÿ��" << std::endl;
			break;
	}
}

void IOCP_SERVER_MANAGER::LoadResources()
{
	std::cout << "��ֹ� �ε� ����" << std::endl;
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
		std::cout << fileName << "�ε� �Ϸ�" << std::endl;
	}
	std::cout << "��ֹ� �ε� �Ϸ�" << std::endl;
	//------------------------------------------------------------------------------------------
	// astar ��� npc���� �����ϱ� ���� ��带 ��������� �Ҵ�
	//std::cout << "A* NODE �ε� ����" << std::endl;
	//MakeGraph();
	////for (auto& node : g_um_graph)
	////{
	////	std::cout << node.first << " ��� ���� �Ϸ�" << std::endl;
	////}
	//std::cout << "A* NODE �ε� �Ϸ�" << std::endl;
}

void IOCP_SERVER_MANAGER::command_thread()
{
	std::string input;
	std::unordered_map<std::string, std::function<void()>> commands = {		// �̰��� �߰��ϰ� ���� ��ɾ� ���� { ��ɾ� , ���� }
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
			std::cout << "LOG ��� : " << detail.m_bLog << std::endl;
		}}
		,
		{"/NPC",[this]() {
			detail.m_bNPC = !detail.m_bNPC;
			std::cout << "NPC �÷��̾� ���� : " << detail.m_bNPC << std::endl;
		}}
		,
		{"/HELP",[this]() {
			std::cout << "-------------------------------------------------------------------" << std::endl;
			std::cout << "/STOP : ���� ����\n/LOG: �α� ���\n/NPC: NPC�� �÷��̾� ����" << std::endl;
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
			std::cout << "�ش� ��ɾ �������� �ʽ��ϴ�." << std::endl;
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

void IOCP_SERVER_MANAGER::ai_timer()
{
	using namespace std::chrono;
	while (detail.m_bServerState)
	{
		if (g_npc_timer.empty())
		{
			//std::this_thread::sleep_for(0.008s);
			continue;
		}
		else
		{
			g_mutex_npc_timer.lock();
			auto q = g_npc_timer.top();
			if (q.start_time < steady_clock::now())
			{
				g_npc_timer.pop();
				g_mutex_npc_timer.unlock();
				EXP_OVER* over = new EXP_OVER;
				over->c_op = C_TIMER;
				PostQueuedCompletionStatus(detail.m_hIOCP, q.gameNum, q.id, &over->over);
				/*g_npc_timer.emplace(q.gameNum, q.id, steady_clock::now() + 5s);*/				// GQCS�۾��� �Ϸ�Ǹ� Ÿ�̸Ӹ� �߰�
			}
			else
			{
				g_mutex_npc_timer.unlock();
			}
		}
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

float IOCP_SERVER_MANAGER::floor_collision(const unsigned int& id, Game& gameRoom)
{
	DirectX::BoundingOrientedBox player_obb;
	gameRoom.getPlayerOBB(player_obb, id);
	
	DirectX::XMFLOAT3 trash;
	float now_floor = 0;
	auto& map_floors = m_vMeshes[0]->m_Childs[0].m_Childs[0].m_Childs;
	for (auto& floor : map_floors)
	{
		if (floor.floor_collision(player_obb, now_floor))
			break;
	}
	return now_floor;
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
{	// �ƹ��� �����ص� npc ���� Ÿ�̸Ӹ� �Ἥ update�ϴ°� �� ���ƺ��δ� ������
	bool f = false;
	if (std::atomic_compare_exchange_strong(&guard.updating, &f, true))
	{
		guard.guard_state_machine(player, npc_state);
	}
	//std::cout << "npc ȸ��: " << guard.rotation.y << std::endl;
	for (auto& student : students)
	{
		if(std::atomic_compare_exchange_strong(&student.updating,&f,true))	// ������Ʈ ���ε� �ٸ� �����忡�� update�Լ��� �ҷ��� 2�� ������Ʈ�� ���� ���ϰ� ����
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
				for (auto& student : students)
				{
					sc_packet_position student_position(student.id, student.position, student.rotation, student.speed);
					login_players[p.id].send_packet(reinterpret_cast<packet_base*>(&student_position));
				}
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
			float pitch = DirectX::XMConvertToRadians(p.rotation.x); // x���� �������� ȸ��
			float yaw = DirectX::XMConvertToRadians(p.rotation.y);   // y���� �������� ȸ��
			float roll = DirectX::XMConvertToRadians(p.rotation.z);  // z���� �������� ȸ��
			DirectX::XMVECTOR player_rotation = DirectX::XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);
			DirectX::XMVECTOR player_location = DirectX::XMLoadFloat3(&p.position);
			p.obb.Transform(out, 1, player_rotation, player_location);
			return true;
		}
	}
	return false;
}

void Game::update(const bool& npc_state, const unsigned int& npc_id)
{
	if (npc_id == 1)		// guard
	{
		guard.guard_state_machine(player, npc_state);
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
	else if (npc_id < STUDENT_SIZE + 2)	// student
	{
		auto& student = students[npc_id - 2];
		student.student_state_machine(player);

		sc_packet_position student_position(student.id, student.position, student.rotation, student.speed);
		for (auto& p : player)
		{
			if (p.sock != NULL)
			{
				if (login_players.end() != login_players.find(p.id))
				{
					login_players[p.id].send_packet(reinterpret_cast<packet_base*>(&student_position));
				}
				else
					p.sock = NULL;
			}
		}
	}
	else
	{
		std::cout << "�߸��� NPC���̵� �Դϴ�." << std::endl;
	}
}

NPC::NPC()
{
	DirectX::XMFLOAT3 basic_obb_position = { 0,85,0 };
	DirectX::XMFLOAT3 basic_obb_extents = basic_extents;
	DirectX::XMFLOAT4 basic_obb_orients = { 0,0,0,1 };
	obb = DirectX::BoundingOrientedBox(basic_obb_position, basic_obb_extents, basic_obb_orients);
	movement_speed = 5;
	MakeGraph(astar_graph);
}

void NPC::guard_state_machine(Player* p,const bool& npc_state)
{
	DirectX::BoundingOrientedBox now_obb;

	float pitch = DirectX::XMConvertToRadians(this->rotation.x); // x���� �������� ȸ��
	float yaw = DirectX::XMConvertToRadians(this->rotation.y);   // y���� �������� ȸ��
	float roll = DirectX::XMConvertToRadians(this->rotation.z);  // z���� �������� ȸ��
	DirectX::XMVECTOR guard_rotation = DirectX::XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);

	DirectX::XMVECTOR guard_location = DirectX::XMLoadFloat3(&this->position);
	this->obb.Transform(now_obb, 1, guard_rotation, guard_location);
	DirectX::XMFLOAT3 temp;

	float now_floor = 0;
	auto& map_floors = m_vMeshes[0]->m_Childs[0].m_Childs[0].m_Childs;
	
	bool b_stair = false;
	auto map_floors_cnt = map_floors.size();

	auto& stairs = map_floors[map_floors_cnt - 1].m_Childs;

	for (int i = 0; i < 4; ++i)
	{	// ����� ���� +Z, -Z, +X, -X
		if (stairs[i].stair_collision(now_obb, this->position.y, i))
		{
			b_stair = true;
			break;
		}

	}

	if (b_stair == false)		// ��ܰ� �浹���� ���ߴٸ� �ٴڰ� �浹������ Ȯ��
	{
		for (int i = 0; i < map_floors_cnt - 1; ++i)
		{
			auto& floor = map_floors[i];
			if (floor.floor_collision(now_obb, now_floor, this->position.y))
				break;
		}
		if (now_floor <= 5.5)
		{
			if (m_floor != now_floor)
				std::cout << "npc�� " << m_floor << " -> " << now_floor << "�� �̵�" << std::endl;
			m_floor = now_floor;
		}
	}

	if (this->state == 0)				// �θ��� �θ��� ����
	{
		if (set_destination(p, npc_state))			// �θ��� �Ÿ��ٰ� ���� �߰�������, ������ �����ϰ� �̵�
		{
			this->state = 1;
		}
		else
		{	// ��ġ�� ��ã������
			int duribun_duribun = 5;
			auto duration = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - arrive_time).count();
			if (duration > duribun_duribun)		// 5�� �Ѱ� �θ��� �ŷȴµ� ��ġ�� ��ã������
			{
				this->state = 1;
				// ���尡 ����� astar �׷��� ����
				//std::unordered_map<int, NODE*> guard_graph;
				//MakeGraph(guard_graph);
				while (true)
				{
					reset_graph();
					int des = rand() % astar_graph.size();
					std::cout << des << "�� ������ ����" << std::endl;
					this->destination = astar_graph[des]->pos; // ������ ������ ����
					this->path = aStarSearch(position, destination, astar_graph);
					if (this->path != nullptr)
						break;
				}
				this->destination = path->pos;
			}
		}
	}
	else if (this->state == 1)	// �������� �̵��ϴ� ��
	{
		set_destination(p, npc_state);		// �̵� �ϸ鼭�� ������ ���̴���, �Ҹ��� �鸮���� Ȯ��
	}

	if(state == 1)
		move();

	bool t = true;
	while (true)
	{
		if (std::atomic_compare_exchange_strong(&this->updating, &t, false))
			break;
	}
}

void NPC::student_state_machine(Player* p)
{
	DirectX::BoundingOrientedBox now_obb;

	float pitch = DirectX::XMConvertToRadians(this->rotation.x); // x���� �������� ȸ��
	float yaw = DirectX::XMConvertToRadians(this->rotation.y);   // y���� �������� ȸ��
	float roll = DirectX::XMConvertToRadians(this->rotation.z);  // z���� �������� ȸ��
	DirectX::XMVECTOR student_rotation = DirectX::XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);

	DirectX::XMVECTOR student_location = DirectX::XMLoadFloat3(&this->position);
	this->obb.Transform(now_obb, 1, student_rotation, student_location);
	// �ٸ� ĳ���Ϳ� �浹�߳�?
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
		if (hit&& state!=2)
		{
			std::cout << i<<" �� �浹" << std::endl;
			break;
		}
	}

	if (hit)
	{
		if (state != 2)				// �浹 ������ �Ͼ����, ������ �浹�̸�
		{
			state = 2;
			std::cout << "���� �浹�� �߻��ߴ�@" << std::endl;
			using namespace std::chrono;
			attacked_time = steady_clock::now();				// �浹�� �ð��� ����
			arrive_time = steady_clock::now();
			sc_packet_npc_attack attack_student(this->id);
			for (int i = 0; i < 2; ++i)
			{
				if (p[i].sock != NULL)
				{
					login_players[p[i].id].send_packet(reinterpret_cast<packet_base*>(&attack_student));
					std::cout << "�浹 ��Ŷ ���� �Ϸ�" << std::endl;
				}
			}
		}
		else
		{		//  �浹 �Ǿ��ִ� ���¿��� 3�ʰ� �����ٸ�
			using namespace std::chrono;
			if (attacked_time + 3s < steady_clock::now())		// �浹 ���� 3�ʰ� ��������
			{
				std::cout << "�浹 �ִϸ��̼��� ������� 3�ʰ� ���� state�� ������ ���� ���·� �����ߴ�!" << std::endl;
				state = 0;
			}
		}
	}
	else
	{
		if (this->state == 0)			// ������ ������ ����
		{
			// 2�ʵ��� ���
			int stop = 2;
			auto duration = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - arrive_time).count();
			if (duration > stop)			// 2�� ��� ����
			{
				this->state = 1;
				// ���ο� ��ǥ ��ġ ����
				// �л��� ����� astar �׷��� ���� ����
				while (true)
				{
					reset_graph();
					int des = rand() % astar_graph.size();
					std::cout << "student[" << this->id << "] " << des << "�� ������ ����" << std::endl;
					this->destination = astar_graph[des]->pos; // ������ ������ ����
					this->path = aStarSearch(position, destination, astar_graph);
					if (this->path != nullptr)
						break;
				}
				this->destination = path->pos;
			}
		}
		else if (this->state == 1)		// �������� �̵��ϴ� ��
		{
			if (compare_position(this->destination))		// �������� �����Ͽ�����
			{
				if (this->path == nullptr || this->path->next == nullptr)			// �� �̻��� �������� ������ �θ��� ���·� ����
				{
					if (this->state != 0)
					{
						arrive_time = std::chrono::high_resolution_clock::now();	// ������ �ð� ���
						this->state = 0;
						this->speed = DirectX::XMFLOAT3(0, 0, 0);
						std::cout << "NPC[" << id << "] �θ��� ���·� ����" << std::endl;
					}
				}
				else
				{
					path = path->next;
					destination = path->pos;
				}
			}
			else
			{
				move();		// ��ǥ ��ġ�� �̵�
			}
		}
		else if (this->state == 2)		// �浹 �ִϸ��̼��� ���� ��
		{
			using namespace std::chrono;
			if (attacked_time + 3s < steady_clock::now())		// �浹 ���� 3�ʰ� ��������
			{
				std::cout << "�浹 �ִϸ��̼��� ������� 3�ʰ� ���� state�� ������ ���� ���·� �����ߴ�!" << std::endl;
				state = 0;
			}
		}
	}

	bool t = true;
	while (true)
	{
		if (std::atomic_compare_exchange_strong(&this->updating, &t, false))
			break;
	}
}

bool NPC::can_see(Player& p)
{
	if (p.m_floor != m_floor)
		return false;

	DirectX::XMFLOAT3 playerPos = p.position;
	playerPos.y += 127;
	DirectX::XMFLOAT3 npcPos = position;
	npcPos.y += 127;
	for (auto& mesh : m_vMeshes[0]->m_Childs[0].m_Childs[m_floor + 2].m_Childs)
	{
		if (mesh.sight_block(playerPos, npcPos))
		{
			// �ϳ��� ���̶� �����ִٸ� �þ߿� ������ �ʴ� ��.
			return false;
		}
		//if (mesh->can_see(playerPos, npcPos, m_floor))		// npc -> �÷��̾���� ������ ��ֹ����� ray �浹�Ͽ� npc�� �÷��̾ �� �� �ִ��� Ȯ��
		//{
		//	std::cout << "���δ� ����.." << std::endl;
		//	return true;
		//}
	}
	// ��� ���� �� ���ƺôµ� ������ ������ �þ߿� ���̴� ��.
	return true;
}

bool NPC::can_hear(Player& p)
{
	const unsigned short can_hear_distance = 100;
	bool playerSound = p.sound.load();
	if (playerSound && distance(p) < can_hear_distance * can_hear_distance)		// �÷��̾ �Ҹ��� ���� ������, �÷��̾���� �Ÿ��� ���� �� �ִ� �Ÿ� �̳��̸�
		return true;
	return false;
}

float NPC::distance(Player& p)
{
	return (position.x - p.position.x) * (position.x - p.position.x) + (position.y - p.position.y) * (position.y - p.position.y) + (position.z - p.position.z) * (position.z - p.position.z);
}

bool NPC::compare_position(DirectX::XMFLOAT3& pos)
{
	float error_value = 10;
	if (this->position.x >= pos.x - error_value && this->position.x <= pos.x + error_value)
		if (this->position.y >= pos.y - error_value && this->position.y <= pos.y + error_value)
			if (this->position.z >= pos.z - error_value && this->position.z <= pos.z + error_value)
			{
				if (state == 1)
				{
					std::cout <<"NPC["<<this->id<<"] ������ ����" << std::endl;
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
				std::cout << n << "P �߰�" << std::endl;
				//this->destination = p[n].position;
				reset_graph();
				reset_path();
				this->path = aStarSearch(position, p[n].position, astar_graph);
				compare_length_next_path(path, this->position, p[n].position);
				if (path == nullptr)				// ����� ��庸�� �÷��̾ �� ������
				{
					this->destination = p[n].position;
				}
				else
				{
					this->destination = path->pos;
				}
				this->destination.y = this->position.y;

				return true;
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

	// �÷��̾ ã����, ������ ���ߴٸ�
	if (compare_position(this->destination))		// �������� �����Ͽ�����
	{
		if (this->path == nullptr || this->path->next == nullptr)			// �� �̻��� �������� ������ �θ��� ���·� ����
		{
			if (this->state != 0)
			{
				arrive_time = std::chrono::high_resolution_clock::now();	// ������ �ð� ���
				this->state = 0;
				this->speed = DirectX::XMFLOAT3(0, 0, 0);
				std::cout << "�θ��� ���·� ����" << std::endl;
			}
		}
		else
		{
			PATH* next = path->next;
			delete path;
			path = next;
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
	XMVECTOR movement = XMVectorScale(direction, movement_speed);
	XMStoreFloat3(&position, XMVectorAdd(ActorPos, movement));
	//std::cout << "��� ��ġ " << position.x<< ", " << position.y<< ", " << position.z<< std::endl;
	
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

void NPC::reset_graph()
{
	for (auto& graph : astar_graph)
	{
		graph.second->init();
	}
}

void NPC::reset_path()
{
	PATH* next = this->path;
	while (this->path != nullptr)
	{
		next = next->next;
		delete this->path;
		this->path = next;
	}
}
