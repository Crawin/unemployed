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
	while (m_bRecv) {
		char buf[BUFSIZE + 1] = { 0, };
		int retval = recv(m_Sock, buf, BUFSIZE+1, 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
		}
		else {
			SendPosition sp;
			memcpy(&sp, buf, retval);
			printf("[TCP 클라이언트] %d바이트를 받았습니다.\n", retval);
			std::cout << "Type: POSITION , X: " << sp.x << " , Y: " << sp.y << " , Z: " << sp.z << std::endl;
		}
	}
}

//int main(int argc, char* argv[])
//{
//	while (1)
//	{
//		int retval;
//
//		// 명령행 인수가 있으면 IP 주소로 사용
//		if (argc > 1) SERVERIP = argv[1];
//
//		// 윈속 초기화
//		WSADATA wsa;
//		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
//			return 1;
//
//		// 소켓 생성
//		SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
//		if (sock == INVALID_SOCKET) err_quit("socket()");
//
//		// connect()
//		struct sockaddr_in serveraddr;
//		memset(&serveraddr, 0, sizeof(serveraddr));
//		serveraddr.sin_family = AF_INET;
//		inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
//		serveraddr.sin_port = htons(SERVERPORT);
//		retval = connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
//		if (retval == SOCKET_ERROR) err_quit("connect()");
//
//		// 데이터 통신에 사용할 변수
//		char buf[BUFSIZE + 1];
//		int len;
//
//		SendPosition sp = { POSITION,0,0 };
//		// 서버와 데이터 통신
//		while (1) {
//			// 데이터 입력
//			printf("\n[보낼 데이터] ");
//			if (fgets(buf, BUFSIZE + 1, stdin) == NULL)
//				break;
//
//			// '\n' 문자 제거
//			len = (int)strlen(buf);
//			if (buf[len - 1] == '\n')
//				buf[len - 1] = '\0';
//			if (strlen(buf) == 0)
//				break;
//
//			switch (buf[0])
//			{
//			case 'w':
//				sp.y++;
//				break;
//			case 'a':
//				sp.x--;
//				break;
//			case 's':
//				sp.y--;
//				break;
//			case 'd':
//				sp.x++;
//				break;
//			default:
//				// 데이터 보내기
//				retval = send(sock, buf, (int)strlen(buf), 0);
//				if (retval == SOCKET_ERROR) {
//					err_display("send()");
//					break;
//				}
//				break;
//			}
//
//			//// 데이터 보내기
//			//retval = send(sock, buf, (int)strlen(buf), 0);
//			//if (retval == SOCKET_ERROR) {
//			//	err_display("send()");
//			//	break;
//			//}
//			if (strlen(buf) < 3)
//			{
//				retval = send(sock, (char*)&sp, (int)sizeof(sp), 0);
//				if (retval == SOCKET_ERROR) {
//					err_display("send()");
//					break;
//				}
//			}
//			printf("[TCP 클라이언트] %d바이트를 보냈습니다.\n", retval);
//
//			// 데이터 받기
//			retval = recv(sock, buf, retval, MSG_WAITALL);
//			if (retval == SOCKET_ERROR) {
//				err_display("recv()");
//				break;
//			}
//			else if (retval == 0)
//				break;
//
//			// 받은 데이터 출력
//			buf[retval] = '\0';
//			printf("[TCP 클라이언트] %d바이트를 받았습니다.\n", retval);
//			printf("[받은 데이터] %s\n", buf);
//		}
//
//		// 소켓 닫기
//		closesocket(sock);
//
//		// 윈속 종료
//		WSACleanup();
//	}
//	return 0;
//}