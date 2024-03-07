#pragma once

enum ServerType {
	MANAGER,
	ROOM,
	LISTEN,
	ROOM_RECV,
	GAME_RECV,
	GAME_RUN
};

enum SendType
{
	POSITION,
	TEMP,
	EVENT
};

struct Socket_position {
	SendType type;		// POSITION일때
	DirectX::XMFLOAT3 pos;
};

struct Socket_event
{
	SendType type;		// EVENT일때
};

class CServer
{
private:
	ServerType stType;
	u_short usServerport = 9000;
	u_short usBufsize = 512;
public:
	void SetSTtype(const ServerType&);
	void PrintInfo(const std::string);
	std::string GetServerType(const ServerType&);
	u_short getPort();
	u_short getBufsize();
	virtual void Run();
};

class CRoomServer : CServer {
private:
	std::list<std::pair<std::pair<ServerType, SOCKET>, std::thread>> lRoomThreads;
	std::list<std::pair<std::pair<ServerType, std::pair<SOCKET, unsigned int>>, std::thread>> lGameRecvThreads;
	SOCKET listen_sock;
private:
	std::list<std::pair<std::pair<ServerType, unsigned int>, std::thread>> lGameRunThreads;
	std::map<unsigned int, std::array<std::pair<SOCKET, std::pair<std::list<std::string>, std::mutex>>, 2>> mGameStorages;

public:
	CRoomServer();
	~CRoomServer();

	void Run();
	void ListenThread();
	void RecvThread(const SOCKET&);
	void GameRecvThread(const SOCKET&, const unsigned int&);
	void GameRunThread(const unsigned int&);
	void Join();
	void CloseListen();
	void PrintThreads();
	void DeleteThread(const std::string&, const SOCKET&, const unsigned int&);
	unsigned int Make_GameNumber();
};

class CServerManager : CServer{
private:
	CRoomServer* RoomServer;
	std::pair<ServerType, std::thread> aCommandThread;
public:
	CServerManager();
	~CServerManager();

	void Run();
	void Join();
	void CommandThread();
};

extern std::mutex Log_Mutex;

class Player {
private:
	SOCKET socket = NULL;
private:
	DirectX::XMFLOAT3 m_xmf3Pos;
	DirectX::XMFLOAT3 m_xmf3Prev_Pos;
public:
	const SOCKET getSocket();
	void allocateSOCKET(const SOCKET&);
	const DirectX::XMFLOAT3 getPos();
	void printPlayerPos(const std::string&, const bool&);
	void syncPos();
	void setPos(const DirectX::XMFLOAT3&);
};