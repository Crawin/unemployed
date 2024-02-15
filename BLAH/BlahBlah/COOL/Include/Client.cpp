#include "Client.h"
#define SERVERPORT 9000
#define BUFSIZE    512

Client::Client()
{
	m_cpServerIP = (char*)"192.168.45.129";
	m_Sock = NULL;
	m_bRecv = TRUE;
}

Client::~Client()
{
	std::cout << "Client 소멸자 호출" << std::endl;
	// 소켓 닫기
	closesocket(m_Sock);
	// 윈속 종료
	WSACleanup();
	m_bRecv = FALSE;
	m_Recv_Thread.join();
}

int Client::Connect_Server()
{
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// 소켓 생성
	m_Sock = socket(AF_INET, SOCK_STREAM, 0);
	if (m_Sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	inet_pton(AF_INET, m_cpServerIP, &serveraddr.sin_addr);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(m_Sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");
	else m_Recv_Thread = std::thread(&Client::Recv_Data, this);
}

void Client::Send_Pos(const SendPosition& sp)
{
	int retval = send(m_Sock, (char*)&sp, (int)sizeof(sp), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
	}
}

void Client::Send_Str(const char* str)
{
	int retval = send(m_Sock, str, (int)sizeof(str), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
	}
}

void Client::Recv_Data()
{
	SendPosition sp;
	while (m_bRecv) {
		char buf[BUFSIZE + 1] = { 0, };
		int retval = recv(m_Sock, buf, BUFSIZE+1, 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
		}
		else {
			if (buf[0] == 0)
			{
				memcpy(&sp, buf, sizeof(sp));
				//printf("[TCP 클라이언트] %d바이트를 받았습니다.\n", retval);
				std::cout << "Type: POSITION , X: " << sp.x << " , Y: " << sp.y << " , Z: " << sp.z << std::endl;
				m_vRecv_Queue.push_back(sp);
			}
		}
	}
}

DirectX::XMFLOAT3 Client::Get_Recv_Queue()
{
	SendPosition temp = m_vRecv_Queue.back();
	DirectX::XMFLOAT3 pos = { temp.x,temp.y,temp.z };
	m_vRecv_Queue.clear();
	return pos;
}

int Client::Get_Recv_Size()
{
	return m_vRecv_Queue.size();
}
