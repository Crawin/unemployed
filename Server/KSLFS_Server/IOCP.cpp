#include "IOCP_Common.h"
#include "IOCP.h"

void IOCP_SERVER_MANAGER::start()
{
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
	int id = 0;

	SOCKET client_s = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
	EXP_OVER accept_over;
	ZeroMemory(&accept_over.over, sizeof(accept_over.over));
	accept_over.c_op = C_ACCEPT;
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(server_s), h_iocp, -1, 0);
	AcceptEx(server_s, client_s, accept_over.buf, 0, addr_size + 16, addr_size + 16, nullptr, &accept_over.over);
	
	lobby = new Lobby;
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
		switch (e_over->c_op) {
		case C_RECV: {
			int my_id = static_cast<int>(key);
			if (0 == rw_byte) {
				login_players.erase(my_id);
				continue;
			}
			login_players[my_id].print_message(rw_byte);
			login_players[my_id].broadcast(rw_byte);
			login_players[my_id].do_recv();
		}
				   break;
		case C_SEND: {
			delete e_over;
		}
				   break;
		case C_ACCEPT: {
			CreateIoCompletionPort(reinterpret_cast<HANDLE>(client_s), h_iocp, id, 0);
			login_players.try_emplace(id, client_s, id, PS_LOBBY);
			login_players[id++].do_recv();

			client_s = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
			ZeroMemory(&accept_over.over, sizeof(accept_over.over));
			AcceptEx(server_s, client_s, accept_over.buf, 0, addr_size + 16, addr_size + 16, nullptr, &accept_over.over);
		}
					 break;
		}
	}
	delete lobby;
}