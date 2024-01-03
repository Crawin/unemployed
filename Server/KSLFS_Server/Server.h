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
	u_short getPort();
	u_short getBufsize();
	virtual void Run();
};

class CRoomServer : CServer {
private:
	//std::map<SOCKET, std::thread> mRoomThreads;
	std::vector<std::thread> vRoomThreads;
	std::vector<std::thread> vGameThreads;
	SOCKET listen_sock;
public:
	CRoomServer();
	~CRoomServer();

	void Run();
	void ListenThread();
	void RecvThread(const SOCKET&);
	void Join();
	void CloseListen();
	//SOCKET GetRoomThreads();
};

class CServerManager : CServer{
private:
	CRoomServer* RoomServer;
	std::array<std::thread, 1> aCommandThread;
public:
	CServerManager();
	~CServerManager();

	void Run();
	void Join();
	void CommandThread();
};