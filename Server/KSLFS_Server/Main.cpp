#include "IOCP_Common.h"
#include "IOCP.h"

int main(int argc, char* argv[])
{
	IOCP_SERVER_MANAGER* ServerManager = new IOCP_SERVER_MANAGER;
	ServerManager->start();
	return 0;
}