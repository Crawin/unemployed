#include "Client.h"
#include "VIVOX/vivoxheaders.h"
#define SERVERPORT 9000

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
	// 윈속 종료
	WSACleanup();
}

void Client::Recv_Start()
{
	ZeroMemory(buf, BUFSIZE);
	wsabuf[0].buf = buf;
	wsabuf[0].len = BUFSIZE;
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

void Client::Send_Pos(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& rot)
{
	cs_packet_position temp(pos, rot);
	wsabuf[0].buf = reinterpret_cast<char*>(&temp);
	wsabuf[0].len = sizeof(cs_packet_position);

	ZeroMemory(&wsaover, sizeof(wsaover));
	WSASend(m_sServer, wsabuf, 1, nullptr, 0, &wsaover, send_callback);
}

void Client::Send_Room(const PACKET_TYPE& type, const unsigned int& gameNum)
{
	if (gameNum == NULL)
	{
		cs_packet_make_room makeroom(type);
		wsabuf[0].buf = reinterpret_cast<char*>(&makeroom);
		wsabuf[0].len = sizeof(cs_packet_make_room);
	}
	else
	{
		cs_packet_enter_room enter(type, gameNum);
		wsabuf[0].buf = reinterpret_cast<char*>(&enter);
		wsabuf[0].len = sizeof(cs_packet_enter_room);
	}
	ZeroMemory(&wsaover, sizeof(wsaover));
	WSASend(m_sServer, wsabuf, 1, nullptr, 0, &wsaover, send_callback);
}

void Client::Connect_Server()
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

	Recv_Start();
}

void Client::setPSock(const SOCKET& player)
{
	playerSock = player;
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

		if (client.over_buf.size() > 0)
		{
			char temp_buf[50];
			ZeroMemory(temp_buf, 50);
			int i = 0;
			for (auto c : client.over_buf)		// 이전에 들어와서 짤려있던 패킷을 temp_buf로 이동
			{
				temp_buf[i++] = c;
			}

			int size = temp_buf[0];
			while (size > current_size + client.over_buf.size())	// 이전에 들어와있던 패킷에 추후 들어온 패킷 연결
			{
				temp_buf[i++] = recv_buf[current_size++];
			}
			packet_base* connected_packet = reinterpret_cast<packet_base*>(temp_buf);
			process_packet(connected_packet);
			continue;				// 잘린 패킷 처리 완료
		}

		int size = base->getSize();
		if (size == 0) break;									// 왜 서버에선 16바이트 보냈는데 32바이트를 받는거지?
		
		if (current_size + size > recv_size)				// 패킷이 짤려서 들어왔으면
		{
			while (current_size < recv_size)
				client.over_buf.emplace_back(reinterpret_cast<char*>(base)[current_size++]);
			break;
		}
		process_packet(base);
		current_size += size;
	}
	client.Recv_Start();
}

void CALLBACK send_callback(DWORD err, DWORD sent_size, LPWSAOVERLAPPED pwsaover, DWORD recv_flag)
{
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
		client.characters[buf->getPlayer()].setPosRot(buf->getPos(), buf->getRot());
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
		//std::thread vivox(Start_Vivox, client.getPSock(), buf->getGameNum());
		//vivox.detach();
		break;
	}
	case 3:									// enter_room
	{
		sc_packet_enter_room* buf = reinterpret_cast<sc_packet_enter_room*>(base);
		if (buf->getBool())
		{
			std::cout << buf->getGameNum() << "방 입장 완료" << std::endl;
			std::cout << "참가한 소켓은" << buf->getPlayer() << "입니다." << std::endl;
			client.characters.try_emplace(buf->getPlayer());
			//std::thread vivox(Start_Vivox, client.getPSock(), buf->getGameNum());
			//vivox.detach();
		}
		else
		{
			std::cout << buf->getGameNum() << " 방이 존재하지 않습니다." << std::endl;
		}
		break;
	}
	}
}