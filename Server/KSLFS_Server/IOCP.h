#pragma once
constexpr short PORT = 9000;
constexpr int BUFSIZE = 256;

enum C_OP { C_RECV, C_SEND, C_ACCEPT };

class EXP_OVER
{
public:
	WSAOVERLAPPED over;
	WSABUF wsabuf[1];
	char buf[BUFSIZE];
	C_OP	c_op;

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
};

enum PlayerState { PS_LOBBY, PS_GAME };

class SESSION {
	PlayerState state;
	EXP_OVER recv_over;
	SOCKET client_s;
	char c_id;
public:
	SESSION(SOCKET s, char my_id, PlayerState ps) : client_s(s), c_id(my_id), state(ps) {
		recv_over.c_op = C_RECV;
	}
	SESSION() {
		std::cout << "ERROR";
		exit(-1);
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
			print_error("WSARecv", WSAGetLastError());
		}
	}

	void print_message(DWORD recv_size)
	{
		std::cout << "Client[" << c_id << "] Sent : ";
		for (DWORD i = 0; i < recv_size; ++i)
			std::cout << recv_over.buf[i];
		std::cout << std::endl;
	}

	//void broadcast(int m_size)
	//{
	//	for (auto& p : g_players)
	//		p.second.do_send(static_cast<int>(p.first), recv_over.buf, m_size);
	//}
};

class Lobby
{
};

class Game
{
};

class IOCP_SERVER_MANAGER
{
private:
	std::unordered_map<int, SESSION> login_players;
	Lobby* lobby;
public:
	void start();
};