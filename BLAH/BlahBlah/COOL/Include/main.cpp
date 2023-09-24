//
//
#include "framework.h"
#include "Application.h"




int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	SIZE temp = { 1280, 720 };
	if (false == Application::Instance().Init(hInstance, temp)) {
		DebugPrint("Application Init failed!!\n");
		system("Pause");
		return 1;
	}



	Application::Instance().StartProgram();

}