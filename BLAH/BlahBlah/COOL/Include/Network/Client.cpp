#include "Client.h"
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
	while (current_size < recv_size)
	{
		char* recv_buf = client.Get_Buf();

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

			switch (recv_buf[current_size + 1])		// PACKET_TYPE
			{
				case 0:									// POSITION
				{
					sc_packet_position temp;
					memcpy(&temp, recv_buf + current_size, sizeof(sc_packet_position));
					Client::GetInstance().m_vPosition_Queue.emplace_back(temp);
					break;
				}
				case 1:
				{
					break;
				}
				case 2:
				{
					break;
				}
			}
			client.over_buf.clear();
			continue;				// 잘린 패킷 처리 완료
		}

		int size = recv_buf[current_size];
		
		if (current_size + size > recv_size)				// 패킷이 짤려서 들어왔으면
		{
			while (current_size < recv_size)
				client.over_buf.emplace_back(recv_buf[current_size++]);
		}

		switch (recv_buf[current_size + 1])		// PACKET_TYPE
		{
			case 0:									// POSITION
			{
				sc_packet_position temp;
				memcpy(&temp, recv_buf + current_size, sizeof(sc_packet_position));
				Client::GetInstance().m_vPosition_Queue.emplace_back(temp);
				break;
			}
			case 1:
			{
				break;
			}
			case 2:
			{
				break;
			}
		}
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
