#include "Common.h"
#include "Server.h"

int main(int argc, char* argv[])
{
	CServerManager* ServerManager = new CServerManager;
	ServerManager->Run();
	ServerManager->Join();
	return 0;
}