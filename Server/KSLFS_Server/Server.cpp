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
	lRoomThreads.push_back(std::make_pair(std::make_pair(LISTEN, NULL), std::thread(&CRoomServer::ListenThread, this)));		// ListenThread 실행, lRoomThreads에 ListenThread 추가
}

void CRoomServer::ListenThread()
{
	Log_Mutex.lock();
	std::cout << "listenThread Run" << std::endl;
	Log_Mutex.unlock();

	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return;

	// 소켓 생성
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
		Log_Mutex.lock();
		printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
			addr, ntohs(clientaddr.sin_port));
		Log_Mutex.unlock();

		lRoomThreads.push_back(std::make_pair(std::make_pair(ROOM_RECV, client_sock), std::thread(&CRoomServer::RecvThread, this, client_sock)));	// Accept 될때마다 Recv쓰레드를 각각 생성 및 mRoomThreads에 추가 (타입, 쓰레드)	
	}

	// 윈속 종료
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

	// 클라이언트 정보 얻기
	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

	bool bRoom = true;
	while (bRoom) {
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
		Log_Mutex.lock();
		printf("[ROOM_RECV] [TCP/%s:%d]: %s\n", addr, ntohs(clientaddr.sin_port), buf);
		Log_Mutex.unlock();

		if (strcmp(buf, "방생성") == 0)
		{
			Log_Mutex.lock();
			std::cout << "방을 생성합니다." << std::endl;
			Log_Mutex.unlock();
			bRoom = false;
			//lGameRecvThreads 에 새로운 쓰레드를 한개 만들고, 이 쓰레드는 종료시키자.
			const unsigned int GameNum = Make_GameNumber();
			//  GAME_RUN 쓰레드를 생성
			lGameRunThreads.push_back(std::make_pair(std::make_pair(GAME_RUN, GameNum), std::thread(&CRoomServer::GameRunThread, this, GameNum)));
			// mGameStorages에 key값으로 방번호를 넣고, value로 패킷 구조의 리스트를 만들어서, 얘 전용 저장소로 이용한다.
			SendPosition temp;
			mGameStorages[GameNum][0].first = client_sock;				// p1 소켓 할당
			mGameStorages[GameNum][0].second.emplace_back(temp);		// p1에 접속했다는 의미로 임시 데이터 할당

			// GAME_RECV 쓰레드 생성
			lGameRecvThreads.push_back(std::make_pair(std::make_pair(GAME_RECV, std::make_pair(arg, GameNum)), std::thread(&CRoomServer::GameRecvThread, this, client_sock, GameNum)));
			std::thread t(&CRoomServer::DeleteThread, this, "lRoomThreads", client_sock, NULL);
			t.detach();
		}
		if (strncmp(buf, "방입장", 6) == 0)
		{
			unsigned int GameNum = 0;
			GameNum += (buf[7] - '0') * 10000;
			GameNum += (buf[8] - '0') * 1000;
			GameNum += (buf[9] - '0') * 100;
			GameNum += (buf[10] - '0') * 10;
			GameNum += (buf[11] - '0') * 1;
			std::cout << "GameNum: " << GameNum << std::endl;

			SendPosition temp;
			mGameStorages[GameNum][1].first = client_sock;				// p2 소켓 할당
			mGameStorages[GameNum][1].second.emplace_back(temp);		// p2에 접속했다는 의미로 임시 데이터 할당

			Log_Mutex.lock();
			std::cout << "방에 입장합니다." << std::endl;
			Log_Mutex.unlock();
			bRoom = false;
			lGameRecvThreads.push_back(std::make_pair(std::make_pair(GAME_RECV, std::make_pair(arg,GameNum)), std::thread(&CRoomServer::GameRecvThread, this, client_sock, GameNum)));
			std::thread t(&CRoomServer::DeleteThread, this, "lRoomThreads", client_sock, NULL);
			t.detach();
		}


		// 데이터 보내기
		retval = send(client_sock, buf, retval, 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}
	}

	// 소켓 닫기
	if (bRoom)
	{
		closesocket(client_sock);
		Log_Mutex.lock();
		printf("[ROOM_RECV] [TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
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
				std::cout <<"Socket: [" << arg << "] RoomThread 삭제 완료" << std::endl;
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
				std::cout << "Socket: [" << arg <<"], GameNumber: [" << num << "] GameRecvThread 삭제 완료" << std::endl;
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
				std::cout << "GameNumber: [" << num << "] GameRunThread 삭제 완료" << std::endl;
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
	std::uniform_int_distribution <> uid(10000, 99999);			// 방번호 10000~99999까지.
	unsigned int num;
	bool exist = true;
	while (exist)
	{
		exist = false;
		num = uid(dre);
		for (auto a = lGameRunThreads.begin(); a != lGameRunThreads.end(); ++a)	// 랜덤 돌렸을때 이미 방 번호가 있는지 확인하자
		{
			if (a->first.second == num)										// num 이 이미 존재하는 번호라면 다시 while문을 돌자.
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

	// 클라이언트 정보 얻기
	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

	bool bRoom = true;
	while (bRoom) {
		// 데이터 받기
		retval = recv(client_sock, buf, bufsize, 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retval == 0)
			break;

		switch (buf[0])
		{
		case 0:				// POSITION
		{
			SendPosition sp;
			memcpy(&sp, buf, sizeof(sp));
			if (mGameStorages[gameNum][0].first == client_sock)		// p1이면
			{
				mGameStorages[gameNum][0].second.emplace_back(sp);	// 받은 데이터를 mGameStorages에 저장.
			}
			else if (mGameStorages[gameNum][1].first == client_sock)	// p2면
			{
				mGameStorages[gameNum][1].second.emplace_back(sp);	// 받은 데이터를 mGameStorages에 저장.
			}
			else
			{
				Log_Mutex.lock();
				std::cout << "SOCKET [" << client_sock << "] 이 접속되지 않았습니다." << std::endl;
				Log_Mutex.unlock();
				break;
			}

			Log_Mutex.lock();
			std::cout << "Type: POSITION , X: " << sp.pos.x << " , Y: " << sp.pos.y << " , Z: " << sp.pos.z << std::endl;
			Log_Mutex.unlock();
			break;
		}
		default:
			// 받은 데이터 출력
			buf[retval] = '\0';
			Log_Mutex.lock();
			printf("[GAME_RECV] [TCP/%s:%d]: %s\n", addr, ntohs(clientaddr.sin_port), buf);
			Log_Mutex.unlock();
			break;
		}

		// 데이터 보내기
		retval = send(client_sock, buf, retval, 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}
	}

	// 소켓 닫기
	closesocket(client_sock);
	Log_Mutex.lock();
	printf("[GAME_RECV] [TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
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
		// 게임 로직
		
		// 접속
		for (int i = 0; i < 1; ++i)
		{
			if (p[i].getSocket() == NULL)		// P[i]이 연결되어 있지 않은 상태에서
			{
				// p[i] 가 연결이 되었는가?
				if (mGameStorages[gameNum][i].first)
				{
					Log_Mutex.lock();
					std::cout << "P"<<i+1<<"[" << mGameStorages[gameNum][i].first << "]이 연결되었습니다." << std::endl;
					Log_Mutex.unlock();
					p[i].allocateSOCKET(mGameStorages[gameNum][i].first);
					mGameStorages[gameNum][i].second.clear();
				}
			}
			else
			{
				p[i].printPlayerPos("p1", true);
				p[i].syncPos();
			}

		}

		// 이동
		for (int i = 0; i < 1; ++i)
		{
			if (mGameStorages[gameNum][i].second.size() > 0)
			{
				SendPosition temp;
				memcpy(&temp, &mGameStorages[gameNum][i].second.front(), sizeof(SendPosition));
				p[i].setPos(temp.pos);
				mGameStorages[gameNum][i].second.erase(mGameStorages[gameNum][i].second.begin());
			}
		}



		//충돌
		//이벤트 발생
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

	aCommandThread = std::make_pair(MANAGER, std::thread(&CServerManager::CommandThread, this));	//명령어 입력 쓰레드 실행, 쓰레드를 vCommandThread 에 추가.

	RoomServer->Run();	// 룸 서버 실행
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
	std::map<std::string, std::function<void()>> commands = {		// 이곳에 추가하고 싶은 명령어 기입 { 명령어 , 람다 }
		{"/STOP",[&CommandState, this]() {
			CommandState = false;
			RoomServer->CloseListen();
		}},
		{"/THREAD",[this]() {
			Log_Mutex.lock();
			std::cout << "-------------------------쓰레드 목록-------------------------------" << std::endl;
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
			std::cout << "명령어 모음: ";
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
			std::cout << "존재하지 않는 명령어 입니다." << std::endl;
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

// (문자열, 좌표 변경이 있을시에만 출력)
void Player::printPlayerPos(const std::string& s,const bool& d)
{
	Log_Mutex.lock();		// [p1: x, y, z]
	if (d)
	{
		if (m_xmf3Pos.x != m_xmf3Prev_Pos.x || m_xmf3Pos.y != m_xmf3Prev_Pos.y || m_xmf3Pos.z != m_xmf3Prev_Pos.z)		// 좌표가 바뀐게 있다면
		{
			std::cout << "[" << s << ": " << m_xmf3Pos.x << ", " << m_xmf3Pos.y << ", " << m_xmf3Pos.z << "]" << std::endl;
		}
	}
	else 
	{
		std::cout << "[" << s << ": " << m_xmf3Pos.x << ", " << m_xmf3Pos.y << ", " << m_xmf3Pos.z << "]" << std::endl;
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
