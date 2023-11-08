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
	std::cout << "��: " << sScene_name << " ���� �Ϸ�" << std::endl << "������: " << pRenederer << std::endl;
	return true;
}
