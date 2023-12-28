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

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// ���� ����
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

	// ������ ��ſ� ����� ����
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
		vAcceptSockets.push_back(clientaddr);				// ���� ACCEPT�� Ŭ���̾�Ʈ ��ϵ�

		// ������ Ŭ���̾�Ʈ ���� ���
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
			addr, ntohs(clientaddr.sin_port));
	}

	// ���� �ݱ�
	closesocket(listen_sock);

	// ���� ����
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

	std::thread CT(&CServerManager::CommandThread, this);		// ��ɾ� �Է� ������ ����
	CT.detach();

	RoomServer->Run();	// �� ���� ����
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
		{"stop",[&CommandState]() { CommandState = false; std::cout << "����." << std::endl; }}
	};
	
	std::cout << "��ɾ� ����: ";
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
