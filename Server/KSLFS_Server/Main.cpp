#include "Common.h"
#include "Server.h"

std::mutex Chat_Mutex;
int main(int argc, char* argv[])
{
	CServerManager* ServerManager = new CServerManager;
	ServerManager->Run();
	ServerManager->Join();
	delete ServerManager;
	return 0;
}