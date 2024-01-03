#include "Scene.h"
#include "Renderer/Renderer.h"
#include "framework.h"
#include "Mesh/MeshManager.h"
#include "Object/ObjectManager.h"
#include "Material/MaterialManager.h"
#include "Shader/ShaderManager.h"

Scene::Scene()
{
	m_MeshManager = new MeshManager;
	m_ObjectManager = new ObjectManager;
	m_MaterialManager = new MaterialManager;
	m_ShaderManager = new ShaderManager;

	m_MaterialManager->RegisterShaderManager(m_ShaderManager);
}

Scene::~Scene()
{
	if (m_MeshManager) delete m_MeshManager;
	if (m_ObjectManager) delete m_ObjectManager;
	if (m_MaterialManager) delete m_MaterialManager;
	if (m_ShaderManager) delete m_ShaderManager;
}

#define SHADER_PATH "SceneData\\Shader\\"
#define MESH_PATH "SceneData\\Mesh\\"
#define MATERIAL_PATH "SceneData\\Material\\"
#define SCENE_PATH "SceneData\\Scene\\"

bool Scene::LoadScene(ComPtr<ID3D12GraphicsCommandList> commandList, const std::string& sceneName)
{
	// load shader first
	std::string shaderPath = SHADER_PATH;
	CHECK_CREATE_FAILED(m_ShaderManager->LoadFolder(commandList, shaderPath), "failed to load shader");

	std::string meshPath = MESH_PATH;
	CHECK_CREATE_FAILED(m_MeshManager->LoadFolder(commandList, meshPath), "failed to load mesh");

	std::string matPath = MATERIAL_PATH;
	CHECK_CREATE_FAILED(m_MaterialManager->LoadFolder(commandList, matPath), "failed to load Material");

	std::string objPath(SCENE_PATH);
	objPath += sceneName + "\\Object\\";				// SceneData/Scene/ scenename / Object
	CHECK_CREATE_FAILED(m_ObjectManager->LoadFolder(objPath), "failed to load obj");

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