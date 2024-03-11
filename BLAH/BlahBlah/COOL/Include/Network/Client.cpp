#include "Client.h"
#define SERVERPORT 9000
#define BUFSIZE    512

Client::Client()
{
	//m_cpServerIP = (char*)"freerain.mooo.com";
	m_cpServerIP = (char*)"127.0.0.1";
	m_Sock = NULL;
}

Client::~Client()
{
	std::cout << "Client 소멸자 호출" << std::endl;
	// 소켓 닫기
	closesocket(m_Sock);
	// 윈속 종료
	WSACleanup();
	m_sRecv = 0;
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
	if (strncmp(m_cpServerIP, "127", 3)) {		// 아이피가 루프백 주소라면
		inet_pton(AF_INET, m_cpServerIP, &serveraddr.sin_addr);
	}
	else {
		struct hostent* ptr = gethostbyname(m_cpServerIP);
		struct in_addr addr;
		memcpy(&addr, ptr->h_addr, ptr->h_length);
		//char IP[INET_ADDRSTRLEN];
		//inet_ntop(AF_INET, &addr, IP, sizeof(IP));
		//inet_pton(AF_INET, IP, &serveraddr.sin_addr);
		serveraddr.sin_addr = addr;
	}
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(m_Sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");
	else
	{
		m_Recv_Thread = std::thread(&Client::Recv_Data, this);
		m_sRecv = 1;
	}
}

void Client::Send_Pos(const DirectX::XMFLOAT3& pos)
{
	SendPosition sp = { POSITION, pos };
	int retval = send(m_Sock, (char*)&sp, (int)sizeof(sp), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
	}
}

void Client::Send_Str(const std::string& str)
{
	int retval = send(m_Sock, &str[0], (int)sizeof(str), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
	}
}

void Client::Recv_Data()
{
	SendPosition sp;
	while (m_sRecv) {
		char buf[BUFSIZE + 1] = { 0, };
		int retval = recv(m_Sock, buf, BUFSIZE+1, 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
		}
		else {
			if (buf[0] == 0)
			{
				memcpy(&sp, buf, sizeof(sp));
				m_vRecv_Queue.push_back(sp);
			}
		}
	}
}

DirectX::XMFLOAT3 Client::Get_Recv_Queue()
{
	SendPosition temp = m_vRecv_Queue.back();
	m_vRecv_Queue.clear();
	return temp.pos;
}

int Client::Get_Recv_Size()
{
	return m_vRecv_Queue.size();
}

short Client::Get_RecvState()
{
	return m_sRecv;
}
