#include "Scene.h"
#include "Renderer/Renderer.h"
#include "framework.h"
#include "Mesh/MeshManager.h"
#include "Object/ObjectManager.h"
#include "Material/MaterialManager.h"

Scene::Scene()
{
	m_MeshManager = new MeshManager;
	m_ObjectManager = new ObjectManager;
	m_MaterialManager = new MaterialManager;
}

Scene::~Scene()
{
	if (m_MeshManager) delete m_MeshManager;
	if (m_ObjectManager) delete m_ObjectManager;
	if (m_MaterialManager) delete m_MaterialManager;

}

bool Scene::LoadScene(ComPtr<ID3D12GraphicsCommandList> commandList, const std::string& sceneName)
{
	std::string meshPath = sceneName + "\\Mesh\\";
	CHECK_CREATE_FAILED(m_MeshManager->LoadFolder(commandList, meshPath), "failed to load mesh");

	std::string objPath = sceneName + "\\Object\\";
	CHECK_CREATE_FAILED(m_ObjectManager->LoadFolder(objPath), "failed to load obj");

	std::string matPath = sceneName + "\\Material\\";
	CHECK_CREATE_FAILED(m_MaterialManager->LoadFolder(commandList, matPath), "failed to load Material");

	return true;
}

bool Scene::Enter(ComPtr<ID3D12GraphicsCommandList> commandList)
{
	if (LoadScene(commandList, m_SceneName) == false) 
	{
		ERROR_QUIT(std::format("ERROR!! Scene load error, {}", m_SceneName));
	}

	return false;
}

//bool Scene::Init()
//{
//	m_SceneName = "Base";
//	//pRenederer = Renderer::GetInstance().GetRendererPtr();
//	DebugPrint(std::format("씬: {} 생성", m_SceneName));
//	return true;
//}

//void Scene::Render()
//{
//	if (pRenederer)
//	{
//		pRenederer->Render();
//	}
//}