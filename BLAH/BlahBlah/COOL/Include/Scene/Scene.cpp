#include "Scene.h"
#include "Renderer/Renderer.h"
#include "framework.h"

Scene::Scene()
{
}

Scene::~Scene()
{
}



bool Scene::Init()
{
	m_SceneName = "Base";
	//pRenederer = Renderer::GetInstance().GetRendererPtr();
	std::cout << "��: " << m_SceneName << " ���� �Ϸ�" << std::endl;// << "������: " << pRenederer << std::endl;
	return true;
}


//void Scene::Render()
//{
//	if (pRenederer)
//	{
//		pRenederer->Render();
//	}
//}