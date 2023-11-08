#include "Scene.h"
#include "Renderer/Renderer.h"
#include "framework.h"

CScene::CScene()
{
}

CScene::~CScene()
{
}

void CScene::Render()
{
	if (pRenederer)
	{
		pRenederer->Render();
	}
}

bool CScene::Init()
{
	sScene_name = "Base";
	pRenederer = Renderer::Instance().GetRendererPtr();
	std::cout << "¾À: " << sScene_name << " »ý¼º ¿Ï·á" << std::endl << "·»´õ·¯: " << pRenederer << std::endl;
	return true;
}
