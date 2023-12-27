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
public:
	bool SetSTtype(ServerType);
	void PrintInfo(const std::string);
	u_short getPort();
	virtual bool Run();
};

class CRoomServer : CServer {
private:
	std::vector<std::thread> vRoomThreads;
public:
	CRoomServer();
	~CRoomServer();

	bool Run();
	bool ListenThread();
	bool Join();
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
	std::vector<CGameServer*> GameServers;
public:
	CServerManager();
	~CServerManager();

	bool Run();
	bool Join();
};