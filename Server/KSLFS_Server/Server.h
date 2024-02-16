#pragma once

enum ServerType {
	MANAGER,
	ROOM,
	LISTEN,
	ROOM_RECV,
	GAME_RECV
};

enum SendType
{
	POSITION,
	TEMP
};

struct SendPosition {
	SendType type;
	float x;
	float y;
	float z;
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
	std::vector<std::pair<std::pair<ServerType, SOCKET>, std::thread>> vRoomThreads;
	std::vector<std::pair<std::pair<ServerType, std::pair<SOCKET, unsigned int>>, std::thread>> vGameThreads;
	SOCKET listen_sock;
public:
	CRoomServer();
	~CRoomServer();

	void Run();
	void ListenThread();
	void RecvThread(const SOCKET&);
	void GameThread(const SOCKET&, const unsigned int&);
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

extern std::mutex Chat_Mutex;