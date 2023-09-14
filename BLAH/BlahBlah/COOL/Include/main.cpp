//
// 여기서는 실제로 게임을 돌리기 보단 테스트 용으로 돌림
// 그럼 여기서는 뭘 테스트 하냐
// 게임에서 기본 되는것들 오브젝트
// 게임 코드에선 실제 게임 스크립트나 그런거 쓰삼
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