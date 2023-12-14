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
	std::cout << "¾À: " << m_SceneName << " »ý¼º ¿Ï·á" << std::endl;// << "·»´õ·¯: " << pRenederer << std::endl;
	return true;
}


//void Scene::Render()
//{
//	if (pRenederer)
//	{
//		pRenederer->Render();
//	}
//}