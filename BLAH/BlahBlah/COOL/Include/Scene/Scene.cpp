#include "Scene.h"
//#include "Renderer/Renderer.h"
#include "framework.h"
#include "Scene/ECSSystem.h"
#include "ResourceManager.h"
#include "Object/Component.h"
#include "Shader/Shader.h"
//#define SCENE_PATH "SceneData\\Scene\\"

Scene::Scene()
{
	m_ResourceManager = new ResourceManager;
}

Scene::~Scene()
{
	if (m_ResourceManager) delete m_ResourceManager;
}


bool Scene::LoadScene(ComPtr<ID3D12GraphicsCommandList> commandList, const std::string& sceneName)
{
	m_ECSManager = std::make_shared<ECSSystem<COMPONENT_COUNT>>();

	m_ResourceManager->SetECSManager(m_ECSManager);

	CHECK_CREATE_FAILED(m_ResourceManager->Init(commandList, sceneName), std::format("Can't Load Scene, name: {}", sceneName));

	return true;
}

bool Scene::Enter(ComPtr<ID3D12GraphicsCommandList> commandList)
{
	if (LoadScene(commandList, m_SceneName) == false) 
	{
		ERROR_QUIT(std::format("ERROR!! Scene load error, {}", m_SceneName));
	}

	return true;
}

void Scene::Render(std::vector<ComPtr<ID3D12GraphicsCommandList>>& commandLists)
{
	// 기본 render, forward render이다

	auto res = m_ResourceManager;

	std::function<void(component::Renderer*, component::Transform*)> func = [&commandLists, &res](component::Renderer* renderComponent, component::Transform* tr) {
		int material = renderComponent->GetMaterial();
		int mesh = renderComponent->GetMesh();

		res->m_Materials[material]->GetShader()->Render(commandLists[0]);

		res->m_Materials[material]->SetDatas(commandLists[0], ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT);

		XMFLOAT4X4 t = Matrix4x4::Identity();
		res->m_Meshes[mesh]->Render(commandLists[0], t);
		};


	m_ECSManager->Execute<component::Renderer, component::Transform>(func);

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