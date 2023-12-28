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
	vRoomThreads.push_back(std::thread(&CRoomServer::ListenThread, this));
	std::cout << "listenThread Run" << std::endl;
	return false;
}

bool CRoomServer::ListenThread()
{
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// 소켓 생성
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
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
		vAcceptSockets.push_back(clientaddr);				// 현재 ACCEPT된 클라이언트 목록들

		// 접속한 클라이언트 정보 출력
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
			addr, ntohs(clientaddr.sin_port));
	}

	// 소켓 닫기
	closesocket(listen_sock);

	// 윈속 종료
	WSACleanup();
	std::cout << "ListenThread STOP" << std::endl;
	return false;
}

bool CRoomServer::Join()
{
	for (auto start = vRoomThreads.begin(); start != vRoomThreads.end(); ++start)
	{
		start->join();
	}
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

	std::thread CT(&CServerManager::CommandThread, this);		// 명령어 입력 쓰레드 실행
	CT.detach();

	RoomServer->Run();	// 룸 서버 실행
	return false;
}

bool CServerManager::Join()
{
	RoomServer->Join();
	return false;
}

bool CServerManager::CommandThread()
{
	bool CommandState = true;

	std::cout << "CommandThread Run" << std::endl;
	std::string input;
	std::map<std::string, std::function<void()>> commands = {
		{"stop",[&CommandState]() { CommandState = false; std::cout << "정지." << std::endl; }}
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
	}
	std::cout << "CommandThread Stop" << std::endl;
	return false;
}
