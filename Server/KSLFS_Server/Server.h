#pragma once

enum ServerType {
	MANAGER,
	ROOM,
	GAME_RECV,
	LISTEN,
	ROOM_RECV
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
	//std::map<SOCKET, std::thread> mRoomThreads;
	//std::vector<std::thread> vRoomThreads;
	//std::vector<std::thread> vGameThreads;
	std::vector<std::pair<ServerType, std::thread>> vRoomThreads;
	std::vector<std::pair<ServerType, std::thread>> vGameThreads;
	SOCKET listen_sock;
public:
	CRoomServer();
	~CRoomServer();

	void Run();
	void ListenThread();
	void RecvThread(const SOCKET&);
	void Join();
	void CloseListen();
	void PrintThreads();
};

class CServerManager : CServer{
private:
	CRoomServer* RoomServer;
	//std::array<std::thread, 1> aCommandThread;
	std::pair<ServerType, std::thread> aCommandThread;
public:
	CServerManager();
	~CServerManager();

	void Run();
	void Join();
	void CommandThread();
};