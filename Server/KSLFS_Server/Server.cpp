#include "Common.h"
#include "Server.h"

void CServer::SetSTtype(const ServerType& type)
{
	stType = type;
}

void CServer::PrintInfo(const std::string word)
{
	Log_Mutex.lock();
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
	Log_Mutex.unlock();
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
	case 5:
		answer = "GAME_RUN";
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
	lRoomThreads.push_back(std::make_pair(std::make_pair(LISTEN, NULL), std::thread(&CRoomServer::ListenThread, this)));		// ListenThread ����, lRoomThreads�� ListenThread �߰�
}

void CRoomServer::ListenThread()
{
	Log_Mutex.lock();
	std::cout << "listenThread Run" << std::endl;
	Log_Mutex.unlock();

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
		Log_Mutex.lock();
		printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
			addr, ntohs(clientaddr.sin_port));
		Log_Mutex.unlock();

		lRoomThreads.push_back(std::make_pair(std::make_pair(ROOM_RECV, client_sock), std::thread(&CRoomServer::RecvThread, this, client_sock)));	// Accept �ɶ����� Recv�����带 ���� ���� �� mRoomThreads�� �߰� (Ÿ��, ������)	
	}

	// ���� ����
	WSACleanup();
	Log_Mutex.lock();
	std::cout << "ListenThread STOP" << std::endl;
	Log_Mutex.unlock();
}

void CRoomServer::RecvThread(const SOCKET& arg)
{
	Log_Mutex.lock();
	std::cout << "RecvThread Run" << std::endl;
	Log_Mutex.unlock();

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
		Log_Mutex.lock();
		printf("[ROOM_RECV] [TCP/%s:%d]: %s\n", addr, ntohs(clientaddr.sin_port), buf);
		Log_Mutex.unlock();

		if (strcmp(buf, "�����") == 0)
		{
			Log_Mutex.lock();
			std::cout << "���� �����մϴ�." << std::endl;
			Log_Mutex.unlock();
			bRoom = false;
			//lGameRecvThreads �� ���ο� �����带 �Ѱ� �����, �� ������� �����Ű��.
			const unsigned int GameNum = Make_GameNumber();
			//  GAME_RUN �����带 ����
			lGameRunThreads.push_back(std::make_pair(std::make_pair(GAME_RUN, GameNum), std::thread(&CRoomServer::GameRunThread, this, GameNum)));
			// mGameStorages�� key������ ���ȣ�� �ְ�, value�� ��Ŷ ������ ����Ʈ�� ����, �� ���� ����ҷ� �̿��Ѵ�.

			mGameStorages[GameNum][0].first = client_sock;				// p1 ���� �Ҵ�

			// GAME_RECV ������ ����
			lGameRecvThreads.push_back(std::make_pair(std::make_pair(GAME_RECV, std::make_pair(arg, GameNum)), std::thread(&CRoomServer::GameRecvThread, this, client_sock, GameNum)));
			std::thread t(&CRoomServer::DeleteThread, this, "lRoomThreads", client_sock, NULL);
			t.detach();
		}
		if (strncmp(buf, "������", 6) == 0)
		{
			unsigned int GameNum = 0;
			GameNum += (buf[7] - '0') * 10000;
			GameNum += (buf[8] - '0') * 1000;
			GameNum += (buf[9] - '0') * 100;
			GameNum += (buf[10] - '0') * 10;
			GameNum += (buf[11] - '0') * 1;
			std::cout << "GameNum: " << GameNum<<", "<<buf[7]<<buf[8]<<buf[9]<<buf[10]<<buf[11] << std::endl;

			mGameStorages[GameNum][1].first = client_sock;				// p2 ���� �Ҵ�

			Log_Mutex.lock();
			std::cout << "�濡 �����մϴ�." << std::endl;
			Log_Mutex.unlock();
			bRoom = false;
			lGameRecvThreads.push_back(std::make_pair(std::make_pair(GAME_RECV, std::make_pair(arg,GameNum)), std::thread(&CRoomServer::GameRecvThread, this, client_sock, GameNum)));
			std::thread t(&CRoomServer::DeleteThread, this, "lRoomThreads", client_sock, NULL);
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
		Log_Mutex.lock();
		printf("[ROOM_RECV] [TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
			addr, ntohs(clientaddr.sin_port));
		Log_Mutex.unlock();
		std::thread t(&CRoomServer::DeleteThread, this, "lRoomThreads", client_sock, NULL);
		t.detach();
	}
	delete[] buf;
}

void CRoomServer::Join()
{
	for (auto start = lRoomThreads.begin(); start != lRoomThreads.end(); ++start)
	{
		start->second.join();
	}
	Log_Mutex.lock();
	std::cout << "vRoomThread Join Complete" << std::endl;
	Log_Mutex.unlock();
}

void CRoomServer::CloseListen()
{
	closesocket(listen_sock);
}

void CRoomServer::PrintThreads()
{
	for (auto start = lRoomThreads.begin(); start != lRoomThreads.end(); ++start)
	{
		Log_Mutex.lock();
		std::cout << "\t" << GetServerType(start->first.first) << ", SOCKET: [" << start->first.second << "]" << std::endl;
		Log_Mutex.unlock();
	}
	for (auto start = lGameRecvThreads.begin(); start != lGameRecvThreads.end(); ++start)
	{
		Log_Mutex.lock();
		std::cout << "\t" << GetServerType(start->first.first) <<", SOCKET: ["<<start->first.second.first << "], GameNum: [" << start->first.second.second << "]" << std::endl;
		Log_Mutex.unlock();
	}
	for (auto start = lGameRunThreads.begin(); start != lGameRunThreads.end(); ++start)
	{
		Log_Mutex.lock();
		std::cout << "\t" << GetServerType(start->first.first) << ", GameNum: [" << start->first.second << "]" << std::endl;
		Log_Mutex.unlock();
	}
}

void CRoomServer::DeleteThread(const std::string& vThread, const SOCKET& arg, const unsigned int& num)
{
	if (vThread == "lRoomThreads")
	{
		for (auto a = lRoomThreads.begin(); a != lRoomThreads.end(); ++a)
		{
			if (a->first.second == arg)
			{
				a->second.join();
				lRoomThreads.erase(a);
				Log_Mutex.lock();
				std::cout <<"Socket: [" << arg << "] RoomThread ���� �Ϸ�" << std::endl;
				Log_Mutex.unlock();
				break;
			}
		}
	}

	if (vThread == "lGameRecvThreads")
	{
		for (auto a = lGameRecvThreads.begin(); a != lGameRecvThreads.end(); ++a)
		{
			if (a->first.second.first == arg && a->first.second.second == num)
			{
				a->second.join();
				lGameRecvThreads.erase(a);
				Log_Mutex.lock();
				std::cout << "Socket: [" << arg <<"], GameNumber: [" << num << "] GameRecvThread ���� �Ϸ�" << std::endl;
				Log_Mutex.unlock();
				break;
			}
		}
	}

	if (vThread == "lGameRunThreads")
	{
		for (auto a = lGameRunThreads.begin(); a != lGameRunThreads.end(); ++a)
		{
			if (a->first.second == num)
			{
				a->second.join();
				lGameRunThreads.erase(a);
				Log_Mutex.lock();
				std::cout << "GameNumber: [" << num << "] GameRunThread ���� �Ϸ�" << std::endl;
				Log_Mutex.unlock();
				break;
			}
		}
	}
}

unsigned int CRoomServer::Make_GameNumber()
{
	std::random_device rd;
	std::default_random_engine dre(rd());
	std::uniform_int_distribution <> uid(10000, 99999);			// ���ȣ 10000~99999����.
	unsigned int num;
	bool exist = true;
	while (exist)
	{
		exist = false;
		num = uid(dre);
		for (auto a = lGameRunThreads.begin(); a != lGameRunThreads.end(); ++a)	// ���� �������� �̹� �� ��ȣ�� �ִ��� Ȯ������
		{
			if (a->first.second == num)										// num �� �̹� �����ϴ� ��ȣ��� �ٽ� while���� ����.
			{
				exist = true;
				break;
			}
		}
	}

	return num;
}

void CRoomServer::GameRecvThread(const SOCKET& client_sock, const unsigned int& gameNum)
{
	Log_Mutex.lock();
	std::cout << "Socket: [" << client_sock << "] , RoomNumber: [" << gameNum << "] GameRecvThread Run " << std::endl;
	Log_Mutex.unlock();

	int retval;
	struct sockaddr_in clientaddr;
	char addr[INET_ADDRSTRLEN];
	int addrlen;
	u_short bufsize = getBufsize();
	char* buf = new char[bufsize + 1];

	// Ŭ���̾�Ʈ ���� ���
	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
	//std::string temp(bufsi)
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

		std::string save_buf(buf, retval);

		if (mGameStorages[gameNum][0].first == client_sock)		// p1�̸�
		{
			mGameStorages[gameNum][0].second.second.lock();
			mGameStorages[gameNum][0].second.first.emplace_back(save_buf);	// ���� �����͸� mGameStorages�� ����.
			mGameStorages[gameNum][0].second.second.unlock();
		}
		else if (mGameStorages[gameNum][1].first == client_sock)	// p2��
		{
			mGameStorages[gameNum][1].second.second.lock();
			mGameStorages[gameNum][1].second.first.emplace_back(save_buf);	// ���� �����͸� mGameStorages�� ����.
			mGameStorages[gameNum][1].second.second.unlock();
		}
		else
		{
			Log_Mutex.lock();
			std::cout << "SOCKET [" << client_sock << "] �� ���ӵ��� �ʾҽ��ϴ�." << std::endl;
			Log_Mutex.unlock();
			break;
		}

		//// ���� ������ ���
		//buf[retval] = '\0';
		//Log_Mutex.lock();
		//std::cout << "[GAME_RECV] [TCP " << addr << ":" << ntohs(clientaddr.sin_port) << "]: " << buf << std::endl;
		//Log_Mutex.unlock();

		// ������ ������
		retval = send(client_sock, &buf[0], retval, 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}
	}

	// ���� �ݱ�
	closesocket(client_sock);
	Log_Mutex.lock();
	printf("[GAME_RECV] [TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
		addr, ntohs(clientaddr.sin_port));
	Log_Mutex.unlock();
	std::thread t(&CRoomServer::DeleteThread, this, "lGameRecvThreads", client_sock, gameNum);
	t.detach();
	delete[] buf;
}

void CRoomServer::GameRunThread(const unsigned int& gameNum)
{
	Log_Mutex.lock();
	std::cout << "[" << gameNum << "] GameRunThread Run" << std::endl;
	Log_Mutex.unlock();

	std::array<Player, 2> p;
	while (1)
	{
		// ���� ����
		
		// ����
		for (int i = 0; i < 2; ++i)
		{
			if (p[i].getSocket() == NULL)		// P[i]�� ����Ǿ� ���� ���� ���¿���
			{
				// p[i] �� ������ �Ǿ��°�?
				if (mGameStorages[gameNum][i].first)
				{
					Log_Mutex.lock();
					std::cout << "P"<<i+1<<"[" << mGameStorages[gameNum][i].first << "]�� ����Ǿ����ϴ�." << std::endl;
					Log_Mutex.unlock();
					p[i].allocateSOCKET(mGameStorages[gameNum][i].first);
				}
			}
			else
			{
				p[i].printPlayerPos(i, true);
				p[i].syncPos();
			}

		}

		// ��� ó��
		for (int i = 0; i < 2; ++i)
		{
			if (mGameStorages[gameNum][i].second.first.size() > 0) {
				mGameStorages[gameNum][i].second.second.lock();

				for (auto start = mGameStorages[gameNum][i].second.first.begin(); start != mGameStorages[gameNum][i].second.first.end(); ++start)
				{
					switch ((*start)[0])
					{
					case 0:		// position
					{
						Socket_position temp;
						memcpy(&temp, &(*start)[0], sizeof(Socket_position));
						p[i].setPos(temp.pos);
						break;
					}
					}
				}
				mGameStorages[gameNum][i].second.first.clear();			// ��� ó�� �������� ��� ����� ����

				mGameStorages[gameNum][i].second.second.unlock();
			}
		}



		//�浹
		//�̺�Ʈ �߻�
	}
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
	Log_Mutex.lock();
	std::cout<<"CommandThread join complete" << std::endl;
	Log_Mutex.unlock();
	RoomServer->Join();
	delete RoomServer;
}

void CServerManager::CommandThread()
{
	bool CommandState = true;

	Log_Mutex.lock();
	std::cout << "CommandThread Run" << std::endl;
	Log_Mutex.unlock();

	std::string input;
	std::map<std::string, std::function<void()>> commands = {		// �̰��� �߰��ϰ� ���� ��ɾ� ���� { ��ɾ� , ���� }
		{"/STOP",[&CommandState, this]() {
			CommandState = false;
			RoomServer->CloseListen();
		}},
		{"/THREAD",[this]() {
			Log_Mutex.lock();
			std::cout << "-------------------------������ ���-------------------------------" << std::endl;
			std::cout << "aCommandThread" << std::endl << "\t" << GetServerType(aCommandThread.first) << std::endl;
			std::cout << "RoomServer Threads" << std::endl;
			Log_Mutex.unlock();
			RoomServer->PrintThreads();
			Log_Mutex.lock();
			std::cout << "-------------------------------------------------------------------" << std::endl;
			Log_Mutex.unlock();
		}},
		{"/HELP",[&commands]() {
			Log_Mutex.lock();
			std::cout << "��ɾ� ����: ";
			for (const auto& a : commands)
			{
				std::cout << a.first << ", ";
			}
			std::cout << std::endl;
			Log_Mutex.unlock();
		}}
	};

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
			Log_Mutex.lock();
			std::cout << "�������� �ʴ� ��ɾ� �Դϴ�." << std::endl;
			Log_Mutex.unlock();
		}
	}
}

const SOCKET Player::getSocket()
{
	return socket;
}

void Player::allocateSOCKET(const SOCKET& s)
{
	socket = s;
	m_xmf3Pos = DirectX::XMFLOAT3(0, 0, 0);
}

const DirectX::XMFLOAT3 Player::getPos()
{
	return m_xmf3Pos;
}

// (���ڿ�, ��ǥ ������ �����ÿ��� ���)
void Player::printPlayerPos(const int& i,const bool& d)
{
	Log_Mutex.lock();		// [p1: x, y, z]
	if (d)
	{
		if (m_xmf3Pos.x != m_xmf3Prev_Pos.x || m_xmf3Pos.y != m_xmf3Prev_Pos.y || m_xmf3Pos.z != m_xmf3Prev_Pos.z)		// ��ǥ�� �ٲ�� �ִٸ�
		{
			std::cout << "[P" << i+1 << ": " << m_xmf3Pos.x << ", " << m_xmf3Pos.y << ", " << m_xmf3Pos.z << "]" << std::endl;
		}
	}
	else 
	{
		std::cout << "[P" << i+1 << ": " << m_xmf3Pos.x << ", " << m_xmf3Pos.y << ", " << m_xmf3Pos.z << "]" << std::endl;
	}
	Log_Mutex.unlock();
}

void Player::syncPos()
{
	m_xmf3Prev_Pos = m_xmf3Pos;
}

void Player::setPos(const DirectX::XMFLOAT3& pos)
{
	m_xmf3Pos = pos;
}
