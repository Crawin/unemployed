#include "IOCP_Common.h"
#include "IOCP.h"

void IOCP_SERVER_MANAGER::start()
{
	std::cout << "서버 실행" << std::endl;
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
	unsigned int id = 0;

	SOCKET client_s = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
	EXP_OVER accept_over;
	ZeroMemory(&accept_over.over, sizeof(accept_over.over));
	accept_over.c_op = C_ACCEPT;
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(server_s), h_iocp, -1, 0);
	AcceptEx(server_s, client_s, accept_over.buf, 0, addr_size + 16, addr_size + 16, nullptr, &accept_over.over);
	
	//lobby = new Lobby;
	while (true)
	{
		DWORD rw_byte;
		ULONG_PTR key;
		WSAOVERLAPPED* over;
		BOOL ret = GetQueuedCompletionStatus(h_iocp, &rw_byte, &key, &over, INFINITE);
		if (FALSE == ret) {
			print_error("GQCS", WSAGetLastError());
			exit(-1);
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
			break;
		case 1:										//		pLOGIN,
			std::cout << "오류 발생 ["<<id<<" , "<<login_players[id].getSock()<<"] 로 부터 pLOGIN 형태의 패킷 수신" << std::endl;
			break;
		case 2:										//		pMAKEROOM
		{
			Games.try_emplace(currentRoom, currentRoom);
			Games[currentRoom].init(id, login_players[id].getSock());
			currentRoom;
			std::cout << "[" << id << ", " << login_players[id].getSock() << "] 이용자가 " << currentRoom << "방을 생성하였습니다." << std::endl;

			sc_packet_make_room room(currentRoom);
			login_players[id].send_packet(reinterpret_cast<packet_base*>(&room));
			++currentRoom;
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
