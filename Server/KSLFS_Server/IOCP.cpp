#include "IOCP_Common.h"
#include "Mesh.h"
#include "IOCP.h"

void IOCP_SERVER_MANAGER::start()
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
		m_umMeshes.emplace(fileName.string(), temp);
	}
	std::cout << "��ֹ� �ε� �Ϸ�" << std::endl;

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
	
	std::cout << "���� ����" << std::endl;
	//lobby = new Lobby;
	while (true)
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
					std::cout << "[" << my_id << "," << playerSock << "] �� ������ �����߽��ϴ�." << std::endl;
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
}

void IOCP_SERVER_MANAGER::process_packet(const unsigned int& id, EXP_OVER*& over)
{
	packet_base* base = reinterpret_cast<packet_base*>(over->buf);
	switch (base->getType())
	{
		case 0:										//		pPOSITION,
		{
			cs_packet_position* position = reinterpret_cast<cs_packet_position*>(base);
			DirectX::XMFLOAT3 pos = position->getPosition();
			DirectX::XMFLOAT3 rot = position->getRotation();
			std::cout << "[" << id << ", " << login_players[id].getSock() << "] : ( " << pos.x << ", " << pos.y << ", " << pos.z << "), (" << rot.x << ", " << rot.y << ", " << rot.z << ")" << std::endl;

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
				Player* players = Games[n].getPlayer();
				// �� �Խ�Ʈ���� ȣ��Ʈ ���Ϲ�ȣ �����ֱ�
				sc_packet_enter_room sc_enter(n, true, players[0].sock);
				login_players[id].send_packet(reinterpret_cast<packet_base*>(&sc_enter));
				login_players[id].setState(PS_GAME);
				std::cout << "[" << id << ", " << login_players[id].getSock() << "] �̿��ڰ� " << n << "�濡 �����Ͽ����ϴ�." << std::endl;
				
				// �� ȣ��Ʈ���� �Խ�Ʈ ���Ϲ�ȣ �����ֱ�
				sc_packet_enter_room enter(n, true, players[1].sock);
				login_players[players[0].id].send_packet(reinterpret_cast<packet_base*>(&enter));
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
