#include "Common.h"
#include "Server.h"

bool CServer::SetSTtype(const ServerType& type)
{
	stType = type;
	return false;
}

void CServer::PrintInfo(const std::string word)
{
	std::cout << "Type: [ ";
	switch (stType)
	{
	case 0:
		std::cout << "MANAGER";
		break;
	case 1:
		std::cout << "ROOM";
		break;
	case 2:
		std::cout << "GAME";
		break;
	}
	std::cout<< " ] " << word << std::endl;
}

u_short CServer::getPort()
{
	return usServerport;
}

u_short CServer::getBufsize()
{
	return usBufsize;
}

bool CServer::Run()
{
	PrintInfo("Run");
	return false;
}

CRoomServer::CRoomServer()
{
	SetSTtype(ROOM);
}

CRoomServer::~CRoomServer()
{
}

bool CRoomServer::Run()
{
	PrintInfo("Run");
	vRoomThreads.push_back(std::thread(&CRoomServer::ListenThread, this));		// ListenThread 실행, vRoomThreads에 ListenThread 추가
	return false;
}

bool CRoomServer::ListenThread()
{
	std::cout << "listenThread Run" << std::endl;

	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// 소켓 생성
	//SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(getPort());
	retval = bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	// 데이터 통신에 사용할 변수
	SOCKET client_sock;
	struct sockaddr_in clientaddr;
	int addrlen;

	while (1) {
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}

		// 접속한 클라이언트 정보 출력
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
			addr, ntohs(clientaddr.sin_port));

		vRoomThreads.push_back(std::thread(&CRoomServer::RecvThread, this, client_sock));		// Accept 될때마다 Recv쓰레드를 한 개씩 생성 및 vRommThreads에 추가
	}

	// 소켓 닫기
	//closesocket(listen_sock);			이제 이거는 CommandThread 에서 stop 입력시 진행

	// 윈속 종료
	WSACleanup();
	std::cout << "ListenThread STOP" << std::endl;
	return false;
}

bool CRoomServer::RecvThread(const SOCKET& arg)
{
	std::cout << "RecvThread Run" << std::endl;
	
	int retval;
	SOCKET client_sock = (SOCKET)arg;
	struct sockaddr_in clientaddr;
	char addr[INET_ADDRSTRLEN];
	int addrlen;
	u_short bufsize = getBufsize();
	char* buf = new char[bufsize + 1];

	// 클라이언트 정보 얻기
	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

	while (1) {
		// 데이터 받기
		retval = recv(client_sock, buf, bufsize, 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retval == 0)
			break;

		// 받은 데이터 출력
		buf[retval] = '\0';
		printf("[TCP/%s:%d] %s\n", addr, ntohs(clientaddr.sin_port), buf);

		// 데이터 보내기
		retval = send(client_sock, buf, retval, 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}
	}

	// 소켓 닫기
	closesocket(client_sock);
	printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
		addr, ntohs(clientaddr.sin_port));

	delete[] buf;
	std::cout << "RecvThread STOP" << std::endl;
	return false;
}

bool CRoomServer::Join()
{
	for (auto start = vRoomThreads.begin(); start != vRoomThreads.end(); ++start)
	{
		start->join();
		std::cout <<"vRoomThread join complete" << std::endl;
	}
	return false;
}

bool CRoomServer::CloseListen()
{
	closesocket(listen_sock);
	return false;
}

CGameServer::CGameServer()
{
	SetSTtype(GAME);
	iRoomCode = 123;
}

CGameServer::~CGameServer()
{
}

//bool CGameServer::Run()
//{
//	PrintInfo("Run");
//	return false;
//}

CServerManager::CServerManager()
{
	SetSTtype(MANAGER);
	RoomServer = new CRoomServer;
}

CServerManager::~CServerManager()
{
}

bool CServerManager::Run()
{
	PrintInfo("Run");

	vCommandThread[0] = std::thread(&CServerManager::CommandThread, this); //명령어 입력 쓰레드 실행, 쓰레드를 vCommandThread 에 추가.

	RoomServer->Run();	// 룸 서버 실행
	return false;
}

bool CServerManager::Join()
{
	vCommandThread[0].join();
	std::cout<<"CommandThread join complete" << std::endl;
	RoomServer->Join();
	return false;
}

bool CServerManager::CommandThread()
{
	bool CommandState = true;

	std::cout << "CommandThread Run" << std::endl;
	std::string input;
	std::map<std::string, std::function<void()>> commands = {		// 이곳에 추가하고 싶은 명령어 기입 { 명령어 , 람다 }
		{"stop",[&CommandState, this]() {
			CommandState = false;
			std::cout << "정지." << std::endl;
			RoomServer->CloseListen();
		}}
	};
	
	std::cout << "명령어 모음: ";
	for (const auto& a : commands)
	{
		std::cout << a.first << ", ";
	}
	std::cout << std::endl;

	while (CommandState)
	{
		std::string input;
		std::cin >> input;

		auto result = commands.find(input);
		if (result != commands.end())
		{
			result->second();
		}
		else
		{
			std::cout << "존재하지 않는 명령어 입니다." << std::endl;
		}
	}
	std::cout << "CommandThread Stop" << std::endl;
	return false;
}
