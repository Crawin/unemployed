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
	bool SetSTtype(const ServerType&);
	void PrintInfo(const std::string);
	u_short getPort();
	u_short getBufsize();
	virtual bool Run();
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

	bool Run();
	bool ListenThread();
	bool RecvThread(const SOCKET&);
	bool Join();
	bool CloseListen();
};

class CGameServer : CServer {
	int iRoomCode;
public:
	CGameServer();
	~CGameServer();

	//bool Run();
};

class CServerManager : CServer{
private:
	CRoomServer* RoomServer;
	std::vector<CGameServer*> vGameServers;
	std::array<std::thread, 1> vCommandThread;
public:
	CServerManager();
	~CServerManager();

	bool Run();
	bool Join();
	bool CommandThread();
};