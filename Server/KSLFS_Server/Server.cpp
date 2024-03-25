#include "Common.h"
#include "Server.h"
#include "Mesh.h"

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
	m_cServerState = 1;


	for (const auto& file : std::filesystem::directory_iterator("Resource/"))
	{
		const auto fileName = file.path();
		std::ifstream meshFile(fileName, std::ios::binary);
		if (meshFile.is_open() == false) {
			std::cout << "Failed to open mesh file!! fileName: " << fileName << std::endl;
		}
		Mesh temp;
		temp.LoadMeshData(meshFile);
		m_umMeshes.emplace(fileName.string(), temp);
	}

	lRoomThreads.emplace_back(std::make_pair(LISTEN, NULL), std::thread(&CRoomServer::ListenThread, this));		// ListenThread 실행, lRoomThreads에 ListenThread 추가
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
	//listen_sock = socket(AF_INET, SOCK_STREAM, 0);		//표준 Socket API
	listen_sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
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
		//client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);		//표준 Socket API
		client_sock = WSAAccept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen, NULL, 0);
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

		lRoomThreads.emplace_back(std::make_pair(ROOM_RECV, client_sock), std::thread(&CRoomServer::RecvThread, this, client_sock));	// Accept 될때마다 Recv쓰레드를 각각 생성 및 mRoomThreads에 추가 (타입, 쓰레드)	
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

	DWORD retval;
	SOCKET client_sock = (SOCKET)arg;
	struct sockaddr_in clientaddr;
	char addr[INET_ADDRSTRLEN];
	int addrlen;
	u_short bufsize = getBufsize();
	char* buf = new char[bufsize + 1];
	WSABUF recv_buf;
	recv_buf.buf = buf;
	DWORD recv_flag = 0;

	// 클라이언트 정보 얻기
	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

	bool bRoom = true;
	while (bRoom) {
		// 데이터 받기
		//retval = recv(client_sock, buf, bufsize, 0);		//표준 Socket API
		//if (retval == SOCKET_ERROR) {
		//	err_display("recv()");
		//	break;
		//}
		//else if (retval == 0)
		//	break;
		recv_buf.len = bufsize + 1;
		if (WSARecv(client_sock, &recv_buf, 1, &retval, &recv_flag, 0, 0) == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}

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
			lGameRunThreads.emplace_back(std::make_pair(GAME_RUN, GameNum), std::thread(&CRoomServer::GameRunThread, this, GameNum));
			// mGameStorages에 key값으로 방번호를 넣고, value로 패킷 구조의 리스트를 만들어서, 얘 전용 저장소로 이용한다.

			mGameStorages[GameNum][0].first = client_sock;				// p1 소켓 할당

			// GAME_RECV 쓰레드 생성
			lGameRecvThreads.emplace_back(std::make_pair(GAME_RECV, std::make_pair(arg, GameNum)), std::thread(&CRoomServer::GameRecvThread, this, client_sock, GameNum));
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
			std::cout << "GameNum: " << GameNum<<", "<<buf[7]<<buf[8]<<buf[9]<<buf[10]<<buf[11] << std::endl;

			mGameStorages[GameNum][1].first = client_sock;				// p2 소켓 할당

			Log_Mutex.lock();
			std::cout << "방에 입장합니다." << std::endl;
			Log_Mutex.unlock();
			bRoom = false;
			lGameRecvThreads.emplace_back(std::make_pair(GAME_RECV, std::make_pair(arg,GameNum)), std::thread(&CRoomServer::GameRecvThread, this, client_sock, GameNum));
			std::thread t(&CRoomServer::DeleteThread, this, "lRoomThreads", client_sock, NULL);
			t.detach();
		}


		// 데이터 보내기
		//retval = send(client_sock, buf, retval, 0);		//표준 Socket API
		//if (retval == SOCKET_ERROR) {
		//	err_display("send()");
		//	break;
		//}
		recv_buf.len = retval;
		if (WSASend(client_sock, &recv_buf, 1, &retval, 0, 0, 0) == SOCKET_ERROR) {
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
	m_cServerState = 0;
	for (auto start = lRoomThreads.begin(); start != lRoomThreads.end(); ++start)
	{
		start->second.join();
	}
	Log_Mutex.lock();
	std::cout << "lRoomThread Join Complete" << std::endl;
	Log_Mutex.unlock();

	//for (auto start = lGameRecvThreads.begin(); start != lGameRecvThreads.end(); ++start)
	//{
	//	start->second.join();
	//}
	//Log_Mutex.lock();
	//std::cout << "lGameRecvThreads Join Complete" << std::endl;
	//Log_Mutex.unlock();

	for (auto start = lGameRunThreads.begin(); start != lGameRunThreads.end(); ++start)
	{
		start->second.join();
	}
	Log_Mutex.lock();
	std::cout << "lGameRunThreads Join Complete" << std::endl;
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

void CRoomServer::WorldCollision(Player& p)
{
	DirectX::XMFLOAT3 temp_extents = { 0,0,0 };
	DirectX::BoundingBox pBound(p.getPos(), temp_extents);
	for (auto start = m_umMeshes.begin(); start != m_umMeshes.end(); ++start)
	{
		DirectX::BoundingBox parent(start->second.GetCenter(), start->second.GetExtents());
		if (pBound.Intersects(parent))
		{
			p.undoPosition();
			return;
		}

		if (start->second.m_Childs.size() > 0)
		{
			for (auto child = start->second.m_Childs.begin(); child != start->second.m_Childs.end(); ++child)
			{
				DirectX::BoundingBox box(child->GetCenter(), child->GetExtents());
				if (pBound.Intersects(box))
				{
					p.undoPosition();
					return;
				}
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

	DWORD retval;
	struct sockaddr_in clientaddr;
	char addr[INET_ADDRSTRLEN];
	int addrlen;
	u_short bufsize = getBufsize();
	char* buf = new char[bufsize + 1];
	WSABUF recv_buf;
	recv_buf.buf = buf;
	DWORD recv_flag = 0;
	// 클라이언트 정보 얻기
	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

	bool bRoom = true;
	while (bRoom) {
		// 데이터 받기
		//retval = recv(client_sock, buf, bufsize, 0);		// 표준 Socket API
		//if (retval == SOCKET_ERROR) {
		//	err_display("recv()");
		//	break;
		//}
		recv_buf.len = bufsize + 1;
		if (WSARecv(client_sock, &recv_buf, 1, &retval, &recv_flag, 0, 0) == SOCKET_ERROR) {
			err_display("recv()");
			break;
		};

		std::string save_buf(buf, retval);

		if (mGameStorages[gameNum][0].first == client_sock)		// p1이면
		{
			mGameStorages[gameNum][0].second.second.lock();
			mGameStorages[gameNum][0].second.first.emplace_back(save_buf);	// 받은 데이터를 mGameStorages에 저장.
			mGameStorages[gameNum][0].second.second.unlock();
		}
		else if (mGameStorages[gameNum][1].first == client_sock)	// p2면
		{
			mGameStorages[gameNum][1].second.second.lock();
			mGameStorages[gameNum][1].second.first.emplace_back(save_buf);	// 받은 데이터를 mGameStorages에 저장.
			mGameStorages[gameNum][1].second.second.unlock();
		}
		else
		{
			Log_Mutex.lock();
			std::cout << "SOCKET [" << client_sock << "] 이 접속되지 않았습니다." << std::endl;
			Log_Mutex.unlock();
			break;
		}

		// 받은 데이터 출력
		//buf[retval] = '\0';
		//Log_Mutex.lock();
		//std::cout << "[GAME_RECV] [TCP " << addr << ":" << ntohs(clientaddr.sin_port) << "]: " << buf << std::endl;
		//Log_Mutex.unlock();

		// 데이터 보내기
		//retval = send(client_sock, &buf[0], retval, 0);		// 표준 Socket API
		//if (retval == SOCKET_ERROR) {
		//	err_display("send()");
		//	break;
		//}
		
		//recv_buf.len = retval;
		//if (WSASend(client_sock, &recv_buf, 1, &retval, 0, 0, 0) == SOCKET_ERROR) {
		//	err_display("send()");
		//	break;
		//}
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

	WSABUF wsabuf;


	std::array<Player, 2> p;
	short GameOver = 1;
	while (GameOver)
	{
		if (!m_cServerState)	break;		// 서버가 꺼졌으면 while문 탈출
		// 게임 로직
		
		// 접속
		for (int i = 0; i < 2; ++i)
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
				}
			}
			else
			{
				p[i].printPlayerPos(i, true);
				p[i].syncTransform();
			}

		}

		// 명령 처리
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
						p[i].setTransform(temp);
						break;
					}
					}
				}
				mGameStorages[gameNum][i].second.first.clear();			// 명령 처리 끝났으니 명령 저장소 비우기

				mGameStorages[gameNum][i].second.second.unlock();
			}
		}

		//충돌
		WorldCollision(p[0]);
		//WorldCollision(p[1]);

		//이벤트 발생

		//Socket_position sp;
		//sp.pos = p[0].getPos();
		//sp.rot = p[0].getRot();
		//sp.type = POSITION;
		//wsabuf.buf = (char*)&sp;
		//wsabuf.len = sizeof(sp);
		//if (WSASend(mGameStorages[gameNum][0].first, &wsabuf, 1, nullptr, 0, 0, 0) == SOCKET_ERROR)
		//{
		//	err_display("send()");
		//	break;
		//}
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
	std::unordered_map<std::string, std::function<void()>> commands = {		// 이곳에 추가하고 싶은 명령어 기입 { 명령어 , 람다 }
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

		if (commands.find(input) == commands.end())
		{
			Log_Mutex.lock();
			std::cout << "해당 명령어가 존재하지 않습니다." << std::endl;
			Log_Mutex.unlock();
		}
		else commands[input]();
	}
}

const SOCKET Player::getSocket()
{
	return socket;
}

void Player::allocateSOCKET(const SOCKET& s)
{
	socket = s;
	m_aTransform[1].pos = DirectX::XMFLOAT3(0, 0, 0);
	m_aTransform[1].rot = DirectX::XMFLOAT3(0, 0, 0);
}

const DirectX::XMFLOAT3 Player::getPos()
{
	return m_aTransform[1].pos;
}

const DirectX::XMFLOAT3 Player::getRot()
{
	return m_aTransform[1].rot;
}

// (문자열, 좌표 변경이 있을시에만 출력)
void Player::printPlayerPos(const int& i,const bool& d)
{
	Log_Mutex.lock();		// [p1: x, y, z]
	if (d)
	{
		if (m_aTransform[0].pos.x != m_aTransform[1].pos.x || m_aTransform[0].pos.y != m_aTransform[1].pos.y || m_aTransform[0].pos.z != m_aTransform[1].pos.z ||
			m_aTransform[0].rot.x!= m_aTransform[1].rot.x || m_aTransform[0].rot.y != m_aTransform[0].rot.y|| m_aTransform[0].rot.z!= m_aTransform[0].rot.z)		// 좌표나 회전이 바뀐게 있으면
		{
			std::cout << "[P" << i + 1 << ": " << m_aTransform[1].pos.x << ", " << m_aTransform[1].pos.y << ", " << m_aTransform[1].pos.z << " ||| " << m_aTransform[1].rot.x << ", " << m_aTransform[1].rot.y << ", " << m_aTransform[1].rot.z << "]" << std::endl;
		}
	}
	else 
	{
		std::cout << "[P" << i+1 << ": " << m_aTransform[1].pos.x << ", " << m_aTransform[1].pos.y << ", " << m_aTransform[1].pos.z << "]" << std::endl;
	}
	Log_Mutex.unlock();
}

void Player::syncTransform()
{
	m_aTransform[0] = m_aTransform[1];
}

void Player::setTransform(const Socket_position& sp)
{
	m_aTransform[1].pos = sp.pos;
	m_aTransform[1].rot = sp.rot;
}

void Player::undoPosition()
{
	m_aTransform[1].pos = m_aTransform[0].pos;
}
