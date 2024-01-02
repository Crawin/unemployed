#pragma once

enum ServerType {
	MANAGER,
	ROOM,
	GAME
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
	std::vector<std::thread> vRoomThreads;
	std::vector<SOCKET> vAcceptSockets;
	//std::vector<sockaddr_in> vRunningSockets;

	SOCKET listen_sock;
public:
	CRoomServer();
	~CRoomServer();

	void Run();
	void ListenThread();
	void RecvThread(const SOCKET&);
	void Join();
	void CloseListen();
};

class CGameServer : CServer {
	int iRoomCode;
public:
	CGameServer();
	~CGameServer();

	void Run();
};

class CServerManager : CServer{
private:
	CRoomServer* RoomServer;
	std::vector<CGameServer*> vGameServers;
	std::array<std::thread, 1> vCommandThread;
public:
	CServerManager();
	~CServerManager();

	void Run();
	void Join();
	void CommandThread();
};