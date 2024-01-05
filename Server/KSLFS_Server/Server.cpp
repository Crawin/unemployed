#include "Common.h"
#include "Server.h"

void CServer::SetSTtype(const ServerType& type)
{
	stType = type;
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
	}
	std::cout<< " ] " << word << std::endl;
}

std::string CServer::GetServerType(const ServerType& s)
{
	std::string answer;
	switch (s)
	{
	case 0:
		answer = "MANAGER";
		break;
	case 1:
		answer = "ROOM";
		break;
	case 2:
		answer = "LISTEN";
		break;
	case 3:
		answer = "ROOM_RECV";
		break;
	case 4:
		answer = "GAME_RECV";
		break;
	default:
		answer = "ERROR";
	}
	return answer;
}

u_short CServer::getPort()
{
	return usServerport;
}

u_short CServer::getBufsize()
{
	return usBufsize;
}

void CServer::Run()
{
	PrintInfo("Run");
}

CRoomServer::CRoomServer()
{
	SetSTtype(ROOM);
}

CRoomServer::~CRoomServer()
{
	PrintInfo("STOP");
}

void CRoomServer::Run()
{
	PrintInfo("Run");
	vRoomThreads.push_back(std::make_pair(std::make_pair(LISTEN, NULL), std::thread(&CRoomServer::ListenThread, this)));		// ListenThread ����, vRoomThreads�� ListenThread �߰�
}

void CRoomServer::ListenThread()
{
	std::cout << "listenThread Run" << std::endl;

	int retval;

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return;

	// ���� ����
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

		// ������ Ŭ���̾�Ʈ ���� ���
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
			addr, ntohs(clientaddr.sin_port));

		vRoomThreads.push_back(std::make_pair(std::make_pair(ROOM_RECV, client_sock), std::thread(&CRoomServer::RecvThread, this, client_sock)));	// Accept �ɶ����� Recv�����带 ���� ���� �� mRoomThreads�� �߰� (Ÿ��, ������)	
	}

	// ���� ����
	WSACleanup();
	std::cout << "ListenThread STOP" << std::endl;
}

void CRoomServer::RecvThread(const SOCKET& arg)
{
	std::cout << "RecvThread Run" << std::endl;
	
	int retval;
	SOCKET client_sock = (SOCKET)arg;
	struct sockaddr_in clientaddr;
	char addr[INET_ADDRSTRLEN];
	int addrlen;
	u_short bufsize = getBufsize();
	char* buf = new char[bufsize + 1];

	// Ŭ���̾�Ʈ ���� ���
	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

	bool bRoom = true;
	while (bRoom) {
		// ������ �ޱ�
		retval = recv(client_sock, buf, bufsize, 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retval == 0)
			break;

		// ���� ������ ���
		buf[retval] = '\0';
		printf("[ROOM_RECV] [TCP/%s:%d]: %s\n", addr, ntohs(clientaddr.sin_port), buf);

		if (strcmp(buf, "�����") == 0)
		{
			std::cout << "���� �����մϴ�." << std::endl;
			bRoom = false;
			//vGameThreads �� ���ο� �����带 �Ѱ� �����, �� ������� �����Ű��.
			vGameThreads.push_back(std::make_pair(std::make_pair(GAME_RECV, 100), std::thread(&CRoomServer::GameThread, this, client_sock)));			// ���⼭ 100�� �� ��ȣ��, �ٲ�����Ѵ�.
			std::thread t(&CRoomServer::DeleteThread, this, "vRoomThreads", client_sock);
			t.detach();
		}


		// ������ ������
		retval = send(client_sock, buf, retval, 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}
	}

	// ���� �ݱ�
	if (bRoom)
	{
		closesocket(client_sock);
		printf("[ROOM_RECV] [TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
			addr, ntohs(clientaddr.sin_port));
		std::thread t(&CRoomServer::DeleteThread, this, "vRoomThreads", client_sock);
		t.detach();
	}
	delete[] buf;
}

void CRoomServer::Join()
{
	for (auto start = vRoomThreads.begin(); start != vRoomThreads.end(); ++start)
	{
		start->second.join();
		std::cout << "vRoomThread Join Complete" << std::endl;
	}
}

void CRoomServer::CloseListen()
{
	closesocket(listen_sock);
}

void CRoomServer::PrintThreads()
{
	for (auto start = vRoomThreads.begin(); start != vRoomThreads.end(); ++start)
	{
		std::cout << "\t" << GetServerType(start->first.first) << std::endl;
	}
	for (auto start = vGameThreads.begin(); start != vGameThreads.end(); ++start)
	{
		std::cout << "\t" << GetServerType(start->first.first) << std::endl;
	}
}

void CRoomServer::DeleteThread(const std::string& vThread, const SOCKET& arg)
{
	if (vThread == "vRoomThreads")
	{
		for (auto a = vRoomThreads.begin(); a != vRoomThreads.end(); ++a)
		{
			if (a->first.second == arg)
			{
				a->second.join();
				vRoomThreads.erase(a);
				std::cout << arg << " RoomThread ���� �Ϸ�" << std::endl;
				break;
			}
		}
	}

	if (vThread == "vGameThreads")
	{
		for (auto a = vGameThreads.begin(); a != vGameThreads.end(); ++a)
		{
			if (a->first.second == arg)
			{
				a->second.join();
				vGameThreads.erase(a);
				std::cout << arg << " GameThread ���� �Ϸ�" << std::endl;
				break;
			}
		}
	}
}

void CRoomServer::GameThread(const SOCKET& arg)
{
	std::cout << arg << " GameThread Run" << std::endl;
	
	int retval;
	SOCKET client_sock = (SOCKET)arg;
	struct sockaddr_in clientaddr;
	char addr[INET_ADDRSTRLEN];
	int addrlen;
	u_short bufsize = getBufsize();
	char* buf = new char[bufsize + 1];

	// Ŭ���̾�Ʈ ���� ���
	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

	bool bRoom = true;
	while (bRoom) {
		// ������ �ޱ�
		retval = recv(client_sock, buf, bufsize, 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retval == 0)
			break;

		// ���� ������ ���
		buf[retval] = '\0';
		printf("[GAME_RECV] [TCP/%s:%d]: %s\n", addr, ntohs(clientaddr.sin_port), buf);

		// ������ ������
		retval = send(client_sock, buf, retval, 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}
	}

	// ���� �ݱ�
	closesocket(client_sock);
	printf("[GAME_RECV] [TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
		addr, ntohs(clientaddr.sin_port));

	delete[] buf;
}

CServerManager::CServerManager()
{
	SetSTtype(MANAGER);
	RoomServer = new CRoomServer;
}

CServerManager::~CServerManager()
{
	PrintInfo("STOP");
}

void CServerManager::Run()
{
	PrintInfo("Run");

	aCommandThread = std::make_pair(MANAGER, std::thread(&CServerManager::CommandThread, this));	//��ɾ� �Է� ������ ����, �����带 vCommandThread �� �߰�.

	RoomServer->Run();	// �� ���� ����
}

void CServerManager::Join()
{
	aCommandThread.second.join();
	std::cout<<"CommandThread join complete" << std::endl;
	RoomServer->Join();
	delete RoomServer;
}

void CServerManager::CommandThread()
{
	bool CommandState = true;

	std::cout << "CommandThread Run" << std::endl;
	std::string input;
	std::map<std::string, std::function<void()>> commands = {		// �̰��� �߰��ϰ� ���� ��ɾ� ���� { ��ɾ� , ���� }
		{"stop",[&CommandState, this]() {
			CommandState = false;
			RoomServer->CloseListen();
		}},
		{"������",[this]() {
			std::cout << "-------------------------������ ���-------------------------------" << std::endl;
			std::cout << "aCommandThread" << std::endl << "\t" << GetServerType(aCommandThread.first) << std::endl;
			std::cout << "RoomServer Threads" << std::endl;
			RoomServer->PrintThreads();
			std::cout << "-------------------------------------------------------------------" << std::endl;
		}}
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
		else
		{
			std::cout << "�������� �ʴ� ��ɾ� �Դϴ�." << std::endl;
		}
	}
}
