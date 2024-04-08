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
	DirectX::XMFLOAT3 rot;
};

struct Socket_event
{
	SendType type;		// EVENT일때
};

class Mesh;
class Player;

class CServer
{
private:
	ServerType stType;
	u_short usServerport = 9000;
	u_short usBufsize = 512;
public:
	inline void SetSTtype(const ServerType&);
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
	char m_cServerState = 0;
private:
	std::list<std::pair<std::pair<ServerType, unsigned int>, std::thread>> lGameRunThreads;
	std::map<unsigned int, std::array<std::pair<SOCKET, std::pair<std::list<std::string>, std::mutex>>, 2>> mGameStorages;
	std::unordered_map<std::string, Mesh> m_umMeshes;
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
	void WorldCollision(Player&);
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
	struct PlayerTransform {
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 rot;
	};
	std::array< PlayerTransform, 2> m_aTransform;			// [0]: 이전, [1]: 현재
	DirectX::BoundingOrientedBox m_boundingbox;
public:
	const SOCKET getSocket();
	void allocateSOCKET(const SOCKET&);
	const DirectX::XMFLOAT3 getPos();
	const DirectX::XMFLOAT3 getRot();
	void printPlayerPos(const int&, const bool&);
	void syncTransform();
	void setTransform(const Socket_position&);
	void undoPosition();
};