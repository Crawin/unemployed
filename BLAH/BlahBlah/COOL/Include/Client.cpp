#include "Client.h"
#define SERVERPORT 9000
#define BUFSIZE    512

Client::Client()
{
	m_cpServerIP = (char*)"192.168.45.129";
	m_Sock = NULL;
}

Client::~Client()
{
	std::cout << "Client �Ҹ��� ȣ��" << std::endl;

	// ���� �ݱ�
	closesocket(m_Sock);

	// ���� ����
	WSACleanup();
	
}

int Client::Connect_Server()
{
	int retval;

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// ���� ����
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

//int main(int argc, char* argv[])
//{
//	while (1)
//	{
//		int retval;
//
//		// ����� �μ��� ������ IP �ּҷ� ���
//		if (argc > 1) SERVERIP = argv[1];
//
//		// ���� �ʱ�ȭ
//		WSADATA wsa;
//		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
//			return 1;
//
//		// ���� ����
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
//		// ������ ��ſ� ����� ����
//		char buf[BUFSIZE + 1];
//		int len;
//
//		SendPosition sp = { POSITION,0,0 };
//		// ������ ������ ���
//		while (1) {
//			// ������ �Է�
//			printf("\n[���� ������] ");
//			if (fgets(buf, BUFSIZE + 1, stdin) == NULL)
//				break;
//
//			// '\n' ���� ����
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
//				// ������ ������
//				retval = send(sock, buf, (int)strlen(buf), 0);
//				if (retval == SOCKET_ERROR) {
//					err_display("send()");
//					break;
//				}
//				break;
//			}
//
//			//// ������ ������
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
//			printf("[TCP Ŭ���̾�Ʈ] %d����Ʈ�� ���½��ϴ�.\n", retval);
//
//			// ������ �ޱ�
//			retval = recv(sock, buf, retval, MSG_WAITALL);
//			if (retval == SOCKET_ERROR) {
//				err_display("recv()");
//				break;
//			}
//			else if (retval == 0)
//				break;
//
//			// ���� ������ ���
//			buf[retval] = '\0';
//			printf("[TCP Ŭ���̾�Ʈ] %d����Ʈ�� �޾ҽ��ϴ�.\n", retval);
//			printf("[���� ������] %s\n", buf);
//		}
//
//		// ���� �ݱ�
//		closesocket(sock);
//
//		// ���� ����
//		WSACleanup();
//	}
//	return 0;
//}