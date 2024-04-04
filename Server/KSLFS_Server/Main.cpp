#include "Common.h"
#include "Server.h"
#include "IOCP_Common.h"
#include "IOCP.h"

std::mutex Log_Mutex;
int main(int argc, char* argv[])
{
	//CServerManager* ServerManager = new CServerManager;
	//ServerManager->Run();
	//ServerManager->Join();
	//delete ServerManager;
	IOCP_SERVER_MANAGER* ServerManager = new IOCP_SERVER_MANAGER;
	ServerManager->start();
	return 0;
}