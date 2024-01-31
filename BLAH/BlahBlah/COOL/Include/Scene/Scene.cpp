#include "Scene.h"
#include "Renderer/Renderer.h"
#include "framework.h"
#include "ResourceManager.h"
#include "Object/ObjectManager.h"

//#define SCENE_PATH "SceneData\\Scene\\"

Scene::Scene()
{
	m_ResourceManager = new ResourceManager;
	m_ObjectManager = new ObjectManager;
}

Scene::~Scene()
{
	if (m_ObjectManager) delete m_ObjectManager;
}


bool Scene::LoadScene(ComPtr<ID3D12GraphicsCommandList> commandList, const std::string& sceneName)
{
	// todo
	// 고친다

	//// load shader first
	//std::string shaderPath = SHADER_PATH;
	////CHECK_CREATE_FAILED(m_ShaderManager->LoadFolder(commandList, shaderPath), "failed to load shader");

	//std::string meshPath = MESH_PATH;
	//CHECK_CREATE_FAILED(m_MeshManager->LoadFolder(commandList, meshPath), "failed to load mesh");

	//std::string matPath = MATERIAL_PATH;
	//CHECK_CREATE_FAILED(m_MaterialManager->LoadFolder(commandList, matPath), "failed to load Material");

	//std::string objPath(SCENE_PATH);
	//objPath += sceneName + "\\Object\\";				// SceneData/Scene/ scenename / Object
	//CHECK_CREATE_FAILED(m_ObjectManager->LoadFolder(objPath), "failed to load obj");

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