#include "IOCP_Common.h"
#include "Mesh.h"
#include "IOCP.h"
#include "aStar.h"
std::vector<Mesh*> m_vMeshes;
std::unordered_map<int, NODE*> g_um_graph;
//concurrency::concurrent_unordered_map<int, std::atomic<std::shared_ptr>
std::unordered_map<unsigned int, SESSION> login_players;

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
	
	std::thread ai_thread{ &IOCP_SERVER_MANAGER::ai_thread,this };

	// ��Ƽ������
	//int num_threads = std::thread::hardware_concurrency();
	//std::vector<std::thread> worker_threads;
	//for (int i = 0; i < num_threads; ++i)
	//{
	//	worker_threads.emplace_back(&IOCP_SERVER_MANAGER::worker, this, server_s);
	//}

	//for (auto& w : worker_threads)
	//	w.join();


	// �̱۾����� ����׿�
	worker(server_s);

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
		if (FALSE == ret) {
			print_error("GQCS", WSAGetLastError());
			//exit(-1);
		}
		EXP_OVER* e_over = reinterpret_cast<EXP_OVER*>(over);
		switch (e_over->c_op)
		{
		case C_ACCEPT:
		{
			SOCKET client_s = e_over->sock;
			CreateIoCompletionPort(reinterpret_cast<HANDLE>(client_s), detail.m_hIOCP, detail.id, 0);
			login_players.try_emplace(detail.id, detail.id, client_s, PS_LOBBY);
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
			unsigned int my_id = static_cast<unsigned int>(key);
			if (0 == rw_byte) {
				unsigned int gameNum = login_players[my_id].getGameNum();
				if (Games.find(gameNum) == Games.end())
				{
					std::cout<< "Error!!" << gameNum << "���� �������� �ʽ��ϴ�." << std::endl;
					continue;
				}
				if (!Games[gameNum].erasePlayer(my_id))
				{
					std::cout << "Error!!" << gameNum << "���� " << my_id << " �÷��̾ �������� �ʾ� �������� ���߽��ϴ�." << std::endl;
					continue;
				}
				login_players.erase(my_id);
				continue;
			}

			process_packet(my_id, e_over);
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
			for (auto& game : Games)
			{
				game.second.update(detail.m_bNPC);
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
		case 0:										//		pPOSITION,
		{
			cs_packet_position* position = reinterpret_cast<cs_packet_position*>(base);
			auto ping = std::chrono::high_resolution_clock::now() - position->sendTime;
			auto& gameRoom = Games[position->getNum()];
			//if (!world_collision(position))
			//	gameRoom.setPlayerPR(id, position);
			//else
			//{
			//	std::cout << "[" << id << ", " << login_players[id].getSock() << "] �浹" << std::endl;
			//	gameRoom.setPlayerRot(id, position);
			//}

			//DirectX::XMFLOAT3 newSpeed = { 0,0,0 };
			//if (!world_collision_v2(position, &newSpeed))
			//	gameRoom.setPlayerPR_v2(id, position, newSpeed);
			//else
			//{
			//	std::cout << "[" << id << ", " << login_players[id].getSock() << "] �浹" << std::endl;
			//	std::cout << "("<<position->getSpeed().x <<","<<position->getSpeed().y<<","<<position->getSpeed().z<<") -> (" << newSpeed.x << "," << newSpeed.y << "," << newSpeed.z << ")" << std::endl;
			//	gameRoom.setPlayerRotSpeed(id, position, newSpeed);
			//}

			//DirectX::XMFLOAT3 newPosition = gameRoom.getPlayerPos(id);
			//DirectX::XMFLOAT3 newSpeed = position->getSpeed();
			//DirectX::XMFLOAT3 rot = position->getRotation();

			//unsigned short floor;
			//world_collision_v3(position, &newPosition, &newSpeed, &floor, ping);
			//gameRoom.setPlayerPR_v3(id, newPosition, newSpeed, rot, floor);


			//// �浹 ���� ��ǥ ��Ŷ�� ���� ��
			//sc_packet_position after_pos(login_players[id].getSock(), gameRoom.getPlayerPos(id), gameRoom.getPlayerRot(id),gameRoom.getPlayerSp(id));
		
			
			// �浹üũ ���� �ʰ� ��ġ ����
			short floor = floor_collision(position);
			if (floor >= 0)
			{
				if(detail.m_bLog)
					std::cout << id << "�� " << floor << "������ �̵�" << std::endl;
				gameRoom.setFloor(id, floor);
			}
			gameRoom.setPlayerPR(id, position);
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
		case 1:										//		pLOGIN,
			std::cout << "���� �߻� ["<<id<<" , "<<login_players[id].getSock()<<"] �� ���� pLOGIN ������ ��Ŷ ����" << std::endl;
			break;
		case 2:										//		pMAKEROOM
		{
			Games.try_emplace(currentRoom, currentRoom);
			Games[currentRoom].init(id, login_players[id].getSock());
			login_players[id].setState(PS_GAME);
			login_players[id].setGameNum(currentRoom);
			std::cout << "[" << id << ", " << login_players[id].getSock() << "] �̿��ڰ� " << currentRoom << "���� �����Ͽ����ϴ�." << std::endl;

			sc_packet_make_room make(currentRoom);
			login_players[id].send_packet(reinterpret_cast<packet_base*>(&make));
			++currentRoom;
		}
			break;
		case 3:										//		pENTERROOM
		{
			cs_packet_enter_room* cs_enter = reinterpret_cast<cs_packet_enter_room*>(base);
			const unsigned n = cs_enter->getRoomNum();
			auto f = Games.find(n);
			if (f != Games.end())
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
		case 4:
			std::cout << "����: ���ӿ� ���ִ� ������� �����ϴ� ��Ŷ�� ������ ���Խ��ϴ�." << std::endl;
			break;
	}
}

bool IOCP_SERVER_MANAGER::world_collision(cs_packet_position*& player)
{
	DirectX::XMFLOAT3 pos = player->getPosition();
	DirectX::XMFLOAT3 rot = player->getRotation();

	float randianX = DirectX::XMConvertToRadians(rot.x);
	float randianY = DirectX::XMConvertToRadians(rot.y);
	DirectX::XMVECTOR quaternionX = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(1, 0, 0, 0), randianX);
	DirectX::XMVECTOR quaternionY = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(0, 1, 0, 0), randianY);
	//DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(0, 0, 1, 0), rot.z);

	DirectX::XMVECTOR quaternion = DirectX::XMQuaternionMultiply(quaternionX, quaternionY);

	DirectX::XMFLOAT4 quaternionValues;
	DirectX::XMStoreFloat4(&quaternionValues, quaternion);

	DirectX::XMFLOAT3 temp_extents = { 1,1,1 };
	DirectX::BoundingOrientedBox pOBB(pos, temp_extents, quaternionValues);
	//std::cout << "Player : (" << pOBB.Center.x << "," << pOBB.Center.y << "," << pOBB.Center.z << "), Extents: ("
	//	<< pOBB.Extents.x << "," << pOBB.Extents.y << "," << pOBB.Extents.z << ")" << std::endl;
	for (auto& world : m_vMeshes)
	{
		if (world->collision(pOBB))
		{
			//std::cout << "�浹" << std::endl;
			return true;
		}
	}
	return false;
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
	}
	std::cout << "��ֹ� �ε� �Ϸ�" << std::endl;
	//------------------------------------------------------------------------------------------
	std::cout << "A* NODE �ε� ����" << std::endl;
	MakeGraph();
	//for (auto& node : g_um_graph)
	//{
	//	std::cout << node.first << " ��� ���� �Ϸ�" << std::endl;
	//}
	std::cout << "A* NODE �ε� �Ϸ�" << std::endl;
}

bool IOCP_SERVER_MANAGER::world_collision_v2(cs_packet_position*& player, DirectX::XMFLOAT3* newSpeed)
{
	DirectX::XMFLOAT3 pos = player->getPosition();
	DirectX::XMFLOAT3 rot = player->getRotation();
	DirectX::XMFLOAT3 spd = player->getSpeed();
	*newSpeed = spd;

	float randianX = DirectX::XMConvertToRadians(rot.x);
	float randianY = DirectX::XMConvertToRadians(rot.y);
	DirectX::XMVECTOR quaternionX = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(1, 0, 0, 0), randianX);
	DirectX::XMVECTOR quaternionY = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(0, 1, 0, 0), randianY);

	DirectX::XMVECTOR quaternion = DirectX::XMQuaternionMultiply(quaternionX, quaternionY);

	DirectX::XMFLOAT4 quaternionValues;
	DirectX::XMStoreFloat4(&quaternionValues, quaternion);

	DirectX::XMFLOAT3 temp_extents = { 1,1,1 };
	DirectX::BoundingOrientedBox pOBB(pos, temp_extents, quaternionValues);



	for (auto& world : m_vMeshes)
	{
		if (world->collision_v2(pOBB, spd, newSpeed))
		{
			//std::cout << "�浹" << std::endl;
			//�ٴ� �浹
			if (pos.y <= 0) newSpeed->y = 0;
			return true;
		}
	}
	//�ٴ� �浹
	if (pos.y <= 0) newSpeed->y = 0;
	return false;
}

bool IOCP_SERVER_MANAGER::world_collision_v3(cs_packet_position*& player, DirectX::XMFLOAT3* newPosition, DirectX::XMFLOAT3* newSpeed,unsigned short* ptrFloor, std::chrono::nanoseconds& ping)
{
	DirectX::XMFLOAT3 pos = player->getPosition();
	pos.y += 50;
	DirectX::XMFLOAT3 rot = player->getRotation();
	DirectX::XMFLOAT3 spd = player->getSpeed();
	auto sendTime = player->sendTime;

	float randianX = DirectX::XMConvertToRadians(rot.x);
	float randianY = DirectX::XMConvertToRadians(rot.y);
	DirectX::XMVECTOR quaternionX = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(1, 0, 0, 0), randianX);
	DirectX::XMVECTOR quaternionY = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(0, 1, 0, 0), randianY);

	DirectX::XMVECTOR quaternion = DirectX::XMQuaternionMultiply(quaternionX, quaternionY);

	DirectX::XMFLOAT4 quaternionValues;
	DirectX::XMStoreFloat4(&quaternionValues, quaternion);

	DirectX::XMFLOAT3 temp_extents = { 40,50,40 };
	DirectX::BoundingOrientedBox pOBB(pos, temp_extents, quaternionValues);

	for (auto& world : m_vMeshes)
	{
		if (world->collision_v3(pOBB, newPosition, spd, newSpeed, ptrFloor, sendTime, ping))
		{
			return true;
		}
	}
	//�浹���� �ʾҴٸ� ��ġ�� ��Ŷ���� ���� ��ġ��
	newPosition->x = player->getPosition().x;
	newPosition->z = player->getPosition().z;
	return false;
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
		}}
		,
		{"/NPC",[this]() {
			detail.m_bNPC = !detail.m_bNPC;
		}}
		,
		{"/HELP",[this]() {
			std::cout << "-------------------------------------------------------------------" << std::endl;
			std::cout << "/STOP : ���� ����\n/LOG: �α� ���\n/NPC: NPC�� �÷��̾� ����" << std::endl;
			std::cout << "-------------------------------------------------------------------" << std::endl;
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
		if (hb_time < 0.016s)
			std::this_thread::sleep_for(0.016s - hb_time);
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

void Game::init(const unsigned int& i, const SOCKET& s)
{
	if (player[0].id == NULL)
	{
		player[0].id = i;
		player[0].sock = s;
	}
	else
	{
		player[1].id = i;
		player[1].sock = s;
	}
	if (guard.id == NULL)
		guard.id = 1;
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
	guard.state_machine(player,npc_state);
	//std::cout << "npc ȸ��: " << guard.rotation.y << std::endl;
	sc_packet_position npc(guard.id, guard.position, guard.rotation, guard.speed);
	for (auto& p : player)
	{
		if (p.sock != NULL)
		{
			if (login_players.end() != login_players.find(p.id))
				login_players[p.id].send_packet(reinterpret_cast<packet_base*>(&npc));
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

void NPC::state_machine(Player* p,const bool& npc_state)
{
	for (auto& node : g_um_graph)
		node.second->init();
	DirectX::BoundingOrientedBox obb(this->position, { 1,1,1 }, { 0,0,0,1 });
	obb.Center.y *= 100;
	DirectX::XMFLOAT3 temp;
	short now_floor = 0;
	for (auto& mesh : m_vMeshes)
		mesh->m_Childs[0].m_Childs[0].floor_collision(obb, &temp, &temp, &now_floor);
	if (now_floor >= 0)
	{
		//std::cout << "npc�� " << now_floor << "�� �̵�" << std::endl;
		m_floor = now_floor;
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
				int des = rand() % g_um_graph.size();
				std::cout << des << "�� ������ ����" << std::endl;
				this->destination = g_um_graph[des]->pos; // ������ ������ ����
				this->path = aStarSearch(position, destination);
				this->destination = path->pos;
				this->state = 1;
			}
		}
	}
	else if (this->state == 1)	// �������� �̵��ϴ� ��
	{
		set_destination(p, npc_state);		// �̵� �ϸ鼭�� ������ ���̴���, �Ҹ��� �鸮���� Ȯ��
	}

	if(state == 1)
		move();
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
		if (mesh->can_see(playerPos, npcPos, m_floor))		// npc -> �÷��̾���� ������ ��ֹ����� ray �浹�Ͽ� npc�� �÷��̾ �� �� �ִ��� Ȯ��
		{
			//std::cout << "���δ� ����.." << std::endl;
			return true;
		}
	}
	return false;
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
	float error_value = 5;
	if (this->position.x >= pos.x - error_value && this->position.x <= pos.x + error_value)
		if (this->position.y >= pos.y - error_value && this->position.y <= pos.y + error_value)
			if (this->position.z >= pos.z - error_value && this->position.z <= pos.z + error_value)
			{
				if (state == 1)
				{
					std::cout << "������ ����" << std::endl;
					return true;
				}
			}
	return false;
}

bool NPC::set_destination(Player*& p, const bool& npc_state)
{
	//short n = find_near_player(p);
	if (npc_state) {
		for (int i = 0; i < 2; ++i)
		{
			if (p[i].sock == NULL)
				continue;

			if (can_see(p[i]))
			{
				std::cout << i << "P �߰�" << std::endl;
				path = aStarSearch(position, destination);
				//this->destination = path->pos;
				this->destination = p[i].position;
				if (nullptr != path)
					return true;
				else
					state = 0;
			}
			else if (can_hear(p[i]))
			{
				this->destination = p[i].position;
				path = aStarSearch(position, destination);
				if (nullptr != path)
					return true;
				else
					state = 0;
			}
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
	XMVECTOR movement = XMVectorScale(direction, 10);
	XMStoreFloat3(&position, XMVectorAdd(ActorPos, movement));
	//std::cout << "��� ��ġ " << position.x<< ", " << position.y<< ", " << position.z<< std::endl;
	
	XMFLOAT3 temp;
	XMStoreFloat3(&temp, direction);
	temp.y = 0;
	XMVECTOR direction_xz = XMVector3Normalize(XMLoadFloat3(&temp));
	XMFLOAT3 basic_head(0, 0, 1);
	XMVECTOR basic = XMLoadFloat3(&basic_head);

	float costheta = XMVectorGetX(XMVector3Dot(basic, direction_xz));
	float radian = acos(costheta);
	float theta = XMConvertToDegrees(radian);

	XMVECTOR cross = XMVector3Cross(basic, direction_xz);
	float wise = XMVectorGetY(cross);
	if (wise < 0)
	{
		theta = -theta;
	}
	
	rotation.y = theta;
	movement *= 100;
	DirectX::XMStoreFloat3(&speed, movement);
}

const short NPC::find_near_player(Player*& players)
{
	float distance[2] = { 1E+37 ,1E+37 };
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
