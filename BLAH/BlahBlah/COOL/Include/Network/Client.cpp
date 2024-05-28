#include "framework.h"
#include "VIVOX/vivoxheaders.h"
#include "Client.h"
#define SERVERPORT 9000

void print_error(const char* msg, int err_no)
{
	WCHAR* msg_buf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPWSTR>(&msg_buf), 0, NULL);
	std::cout << msg;
	std::wcout << L" : 에러 : " << msg_buf;
	//while (true);
	LocalFree(msg_buf);
}

Client::Client()
{
	//m_cpServerIP = (char*)"freerain.mooo.com";
	m_cpServerIP = (char*)"127.0.0.1";
	m_sServer = NULL;
}

Client::~Client()
{
	std::cout << "Client 소멸자 호출" << std::endl;
	// 소켓 닫기
	closesocket(m_sServer);

	// vivox 종료
	if (vivox_state != nullptr)
		vivox_state->game_state = false;

	// 윈속 종료
	WSACleanup();
}

void Client::Recv_Start()
{
	wsabuf[0].len = BUFSIZE - prev_packet_size;
	wsabuf[0].buf = buf + prev_packet_size;
	DWORD recv_flag = 0;

	ZeroMemory(&wsaover, sizeof(wsaover));
	int res = WSARecv(m_sServer, wsabuf, 1, nullptr, &recv_flag, &wsaover, recv_callback);
	if (0 != res)
	{
		int err_no = WSAGetLastError();
		if (WSA_IO_PENDING != err_no)
			print_error("WSARecv", WSAGetLastError());
	}
}

char* Client::Get_Buf()
{
	return buf;
}

void Client::Send_Pos(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& rot, const DirectX::XMFLOAT3& sp, float deltaTime)
{
	m_SendTimeElapsed += deltaTime;
	if (m_SendTimeElapsed >= m_SendFrame) {
		m_SendTimeElapsed = 0.0f;

		cs_packet_position temp(roomNum, pos, rot, sp);
		temp.sendTime = std::chrono::high_resolution_clock::now();
		EXP_OVER* send_over = new EXP_OVER;
		send_over->wsabuf->len = sizeof(cs_packet_position);
		memcpy(send_over->buf, &temp, sizeof(cs_packet_position));
		WSASend(m_sServer, send_over->wsabuf, 1, nullptr, 0, &send_over->over, send_callback);
	}
}

void Client::Send_Room(const PACKET_TYPE& type, const unsigned int& gameNum)
{
	EXP_OVER* send_over;
	if (gameNum == NULL)
	{
		cs_packet_make_room makeroom(type);
		send_over = new EXP_OVER;
		send_over->wsabuf->len = sizeof(cs_packet_make_room);
		memcpy(send_over->buf, &makeroom, sizeof(cs_packet_make_room));
	}
	else
	{
		cs_packet_enter_room enter(type, gameNum);
		send_over = new EXP_OVER;
		send_over->wsabuf->len = sizeof(cs_packet_enter_room);
		memcpy(send_over->buf, &enter, sizeof(cs_packet_enter_room));
	}
	ZeroMemory(&wsaover, sizeof(wsaover));
	WSASend(m_sServer, send_over->wsabuf, 1, nullptr, 0, &send_over->over, send_callback);
}

void Client::Connect_Server()
{
	if (playerSock[0] == NULL)
	{
		std::wcout.imbue(std::locale("korean"));

		// 원속 초기화
		WSADATA WSAData;
		WSAStartup(MAKEWORD(2, 2), &WSAData);

		// 소켓 생성
		m_sServer = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
		SOCKADDR_IN server_a;
		server_a.sin_family = AF_INET;
		server_a.sin_port = htons(SERVERPORT);
		if (strncmp(m_cpServerIP, "free", 4) == 0) {            // 아이피가 도메인이면
			struct hostent* ptr = gethostbyname(m_cpServerIP);
			memcpy(&server_a.sin_addr, ptr->h_addr, ptr->h_length);
		}
		else
		{
			inet_pton(AF_INET, m_cpServerIP, &server_a.sin_addr);
		}
		int res = WSAConnect(m_sServer, reinterpret_cast<sockaddr*>(&server_a), sizeof(server_a), nullptr, nullptr, NULL, NULL);
		if (0 != res)
		{
			print_error("WSAConnect", WSAGetLastError());
		}

		ZeroMemory(buf, BUFSIZE);
		Recv_Start();
	}
	else
	{
		std::cout << "이미 [" << playerSock[0] << "] 으로 로그인 되어있습니다." << std::endl;
	}
}

void Client::setPSock(const SOCKET& player)
{
	if (playerSock[0] == NULL)
		playerSock[0] = player;
	else
		playerSock[1] = player;
}

void Client::swapPSock()
{
	SOCKET temp = playerSock[0];
	playerSock[0] = playerSock[1];
	playerSock[1] = temp;
}

void Client::pull_packet(const int& current_size)
{
	memcpy(buf, buf + current_size, BUFSIZE - current_size);
}

void Client::logout_opponent()
{
	playerSock[1] = NULL;
	std::cout << "해당 클라이언트에서 상대방이 삭제되었습니다." << std::endl;
}

void CALLBACK recv_callback(DWORD err, DWORD recv_size, LPWSAOVERLAPPED pwsaover, DWORD send_flag)
{
	if (0 != err)
	{
		if (0 == recv_size)
			return;
		print_error("WSARecv", WSAGetLastError());
	}
	auto& client = Client::GetInstance();
	
	int current_size = 0;
	char* recv_buf = client.Get_Buf();
	while (current_size < recv_size)
	{
		packet_base* base = reinterpret_cast<packet_base*>(recv_buf + current_size);

		int size = base->getSize();
		if (size == 0)
			break;

		if (size + current_size > recv_size)
		{
			client.set_prev_packet_size(size);
			client.pull_packet(current_size);
			client.Recv_Start();
			return;
		}
		process_packet(base);
		current_size += size;
	}

	ZeroMemory(client.Get_Buf(), BUFSIZE);
	client.set_prev_packet_size(0);
	client.Recv_Start();
}

void CALLBACK send_callback(DWORD err, DWORD sent_size, LPWSAOVERLAPPED pwsaover, DWORD recv_flag)
{
	auto send_packet = reinterpret_cast<EXP_OVER*>(pwsaover);
	delete send_packet;
	if (0 != err)
	{
		print_error("WSASend", WSAGetLastError());
	}
}

void process_packet(packet_base*& base)
{
	auto& client = Client::GetInstance();
	switch (base->getType())		// PACKET_TYPE
	{
	case 0:									// POSITION
	{
		sc_packet_position* buf = reinterpret_cast<sc_packet_position*>(base);
		//if (buf->getPlayer() == client.getPSock()[0]) {
		//	client.characters[buf->getPlayer()].setPosRotSpeed(buf->getPos(), client.characters[buf->getPlayer()].getRot(), buf->getSpeed());	// 수정 필요
		//}
		//else
		//{
			client.characters[buf->getPlayer()].setPosRotSpeed(buf->getPos(), buf->getRot(), buf->getSpeed());
		//}
		break;
	}
	case 1:									// LOGIN
	{
		sc_packet_login* buf = reinterpret_cast<sc_packet_login*>(base);
		client.characters.try_emplace(buf->player);
		client.setPSock(buf->player);
		break;
	}
	case 2:									// make_room
	{
		sc_packet_make_room* buf = reinterpret_cast<sc_packet_make_room*>(base);
		std::cout << buf->getGameNum() << " 방 생성 완료" << std::endl;
		client.setRoomNum(buf->getGameNum());
		client.setCharType(1);
		client.characters.try_emplace(PoliceID);
		client.vivox_state = new VIVOX_STATE;
		client.vivox_state->game_state = true;
		std::thread vivox(Start_Vivox, client.getPSock()[0], buf->getGameNum(), client.vivox_state);
		vivox.detach();
		break;
	}
	case 3:									// enter_room
	{
		sc_packet_enter_room* buf = reinterpret_cast<sc_packet_enter_room*>(base);
		if (buf->getBool())
		{
			std::cout << buf->getGameNum() << "방 입장 완료" << std::endl;
			client.setRoomNum(buf->getGameNum());
			SOCKET playerSock = buf->getPlayer();
			//std::cout << "참가한 소켓은" << playerSock << "입니다." << std::endl;
			client.characters.try_emplace(playerSock);
			client.characters.try_emplace(PoliceID);
			client.setPSock(playerSock);
			client.setCharType(2);
			client.vivox_state = new VIVOX_STATE;
			client.vivox_state->game_state = true;
			std::thread vivox(Start_Vivox, client.getPSock()[0], buf->getGameNum(), client.vivox_state);
			vivox.detach();
		}
		else
		{
			std::cout << buf->getGameNum() << " 방이 존재하지 않습니다." << std::endl;
		}
		break;
	}
	case 4:									// pRoomPlayer
	{
		sc_packet_room_player* buf = reinterpret_cast<sc_packet_room_player*>(base);
		SOCKET playerSock = buf->getPlayerSock();
		client.characters.try_emplace(playerSock);
		client.setPSock(playerSock);
	}
		break;
	case 5:									// pLogout
	{
		sc_packet_logout* buf = reinterpret_cast<sc_packet_logout*>(base);
		client.characters.erase(buf->player);
		client.logout_opponent();
	}
		break;
	}
}