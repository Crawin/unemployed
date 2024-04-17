#include "IOCP_Common.h"
#include "Mesh.h"
#include "IOCP.h"

void command_thread(bool*);

void IOCP_SERVER_MANAGER::start()
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
		//m_umMeshes.emplace(fileName.string(), temp);
		m_vMeshes.emplace_back(temp);
	}
	std::cout << "장애물 로드 완료" << std::endl;

	std::wcout.imbue(std::locale("korean"));

	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);

	HANDLE h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	SOCKET server_s = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server_a;
	server_a.sin_family = AF_INET;
	server_a.sin_port = htons(PORT);
	server_a.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	bind(server_s, reinterpret_cast<sockaddr*>(&server_a), sizeof(server_a));
	listen(server_s, SOMAXCONN);
	int addr_size = sizeof(server_a);
	unsigned int id = 1;

	SOCKET client_s = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
	EXP_OVER accept_over;
	ZeroMemory(&accept_over.over, sizeof(accept_over.over));
	accept_over.c_op = C_ACCEPT;
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(server_s), h_iocp, -1, 0);
	AcceptEx(server_s, client_s, accept_over.buf, 0, addr_size + 16, addr_size + 16, nullptr, &accept_over.over);
	
	bool server_on = true;

	std::thread command(command_thread, &server_on);

	std::cout << "서버 실행" << std::endl;
	//lobby = new Lobby;
	while (server_on)
	{
		DWORD rw_byte;
		ULONG_PTR key;
		WSAOVERLAPPED* over;
		BOOL ret = GetQueuedCompletionStatus(h_iocp, &rw_byte, &key, &over, INFINITE);
		if (FALSE == ret) {
			print_error("GQCS", WSAGetLastError());
			//exit(-1);
		}
		EXP_OVER* e_over = reinterpret_cast<EXP_OVER*>(over);
		switch (e_over->c_op)
		{
			case C_ACCEPT: 
			{
				CreateIoCompletionPort(reinterpret_cast<HANDLE>(client_s), h_iocp, id, 0);
				login_players.try_emplace(id, id, client_s, PS_LOBBY);
				login_players[id].do_recv();
				
				sc_packet_login login(client_s);
				login_players[id++].send_packet(reinterpret_cast<packet_base*>(&login));
				std::cout << "[" << id-1 << "," << client_s << "] Login" << std::endl;

				client_s = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
				ZeroMemory(&accept_over.over, sizeof(accept_over.over));
				AcceptEx(server_s, client_s, accept_over.buf, 0, addr_size + 16, addr_size + 16, nullptr, &accept_over.over);
			}
				break;
			case C_RECV: 
			{
				unsigned int my_id = static_cast<unsigned int>(key);
				if (0 == rw_byte) {
					SOCKET playerSock = login_players[my_id].getSock();
					std::cout << "[" << my_id << "," << playerSock << "] 이 연결을 종료했습니다." << std::endl;
					login_players.erase(my_id);
					continue;
				}

				process_packet(my_id, e_over);
				//login_players[my_id].print_message(rw_byte);
				//login_players[my_id].broadcast(rw_byte);
				login_players[my_id].do_recv();
			}
				break;
			case C_SEND: 
			{
				delete e_over;
			}
				break;
		}
	}
	//delete lobby;
	closesocket(server_s);
	WSACleanup();
	command.join();
}

void IOCP_SERVER_MANAGER::process_packet(const unsigned int& id, EXP_OVER*& over)
{
	packet_base* base = reinterpret_cast<packet_base*>(over->buf);
	switch (base->getType())
	{
		case 0:										//		pPOSITION,
		{
			cs_packet_position* position = reinterpret_cast<cs_packet_position*>(base);
			auto& gameRoom = Games[position->getNum()];
			if (!world_collision(position))
				gameRoom.setPlayerPR(id, position);
			else
			{
				std::cout << "[" << id << ", " << login_players[id].getSock() << "] 충돌" << std::endl;
				gameRoom.setPlayerRot(id, position);
			}

			// 충돌 이후 좌표 패킷을 만든 후
			sc_packet_position after_pos(login_players[id].getSock(), gameRoom.getPlayerPos(id), gameRoom.getPlayerRot(id));
			
			// 게임 방 내의 플레이어들 모두에게 패킷 전송
			Player* players = Games[position->getNum()].getPlayers();
			for (int i = 0; i < 2; ++i)
			{
				if (players[i].id)
				{
					login_players[players[i].id].send_packet(reinterpret_cast<packet_base*>(&after_pos));
					//std::cout << players[i].id << "에게 전송 완료" << std::endl;
				}
			}

			//Games[position->getNum()].world_collision(position);
			DirectX::XMFLOAT3 pos = position->getPosition();
			DirectX::XMFLOAT3 rot = position->getRotation();
			std::cout << "[" << id << ", " << login_players[id].getSock() << "] : ( " << pos.x << ", " << pos.y << ", " << pos.z << "), (" << rot.x << ", " << rot.y << ", " << rot.z << ")" << std::endl;
			
		}
			break;
		case 1:										//		pLOGIN,
			std::cout << "오류 발생 ["<<id<<" , "<<login_players[id].getSock()<<"] 로 부터 pLOGIN 형태의 패킷 수신" << std::endl;
			break;
		case 2:										//		pMAKEROOM
		{
			Games.try_emplace(currentRoom, currentRoom);
			Games[currentRoom].init(id, login_players[id].getSock());
			login_players[id].setState(PS_GAME);
			std::cout << "[" << id << ", " << login_players[id].getSock() << "] 이용자가 " << currentRoom << "방을 생성하였습니다." << std::endl;

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
			//std::cout << "충돌" << std::endl;
			return true;
		}
	}
	return false;
}

void Game::init(const unsigned int& i, const SOCKET& s)
{
	if (p[0].id == NULL)
	{
		p[0].id = i;
		p[0].sock = s;
	}
	else
	{
		p[1].id = i;
		p[1].sock = s;
	}
}

const DirectX::XMFLOAT3 Game::getPlayerPos(const unsigned int& id)
{
	for (auto& player : p)
	{
		if (player.id == id)
		{
			return player.position;
		}
	}
}

void Game::setPlayerPR(const unsigned int& id, cs_packet_position*& packet)
{
	for (auto& player : p)
	{
		if (player.id == id)
		{
			player.position = packet->getPosition();
			player.rotation = packet->getRotation();
		}
	}
}

void Game::setPlayerRot(const unsigned int& id, cs_packet_position*& packet)
{
	for (auto& player : p)
	{
		if (player.id == id)
		{
			player.rotation = packet->getRotation();
		}
	}
}

const DirectX::XMFLOAT3 Game::getPlayerRot(const unsigned int& id)
{
	for (auto& player : p)
	{
		if (player.id == id)
		{
			return player.rotation;
		}
	}
}

void command_thread(bool* state)
{
	std::string input;
	std::unordered_map<std::string, std::function<void()>> commands = {		// 이곳에 추가하고 싶은 명령어 기입 { 명령어 , 람다 }
		{"/STOP",[&state]() {
			*state = false;
		}},
	};

	while (*state)
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