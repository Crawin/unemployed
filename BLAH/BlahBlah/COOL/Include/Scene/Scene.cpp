#include "Scene.h"
//#include "Renderer/Renderer.h"
#include "framework.h"
#include "ECS/ECSManager.h"
#include "ResourceManager.h"
#include "ECS/Component.h"
#include "ECS/ECS_System.h"
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
	m_ECSManager = std::make_shared<ECSManager>();

	m_ResourceManager->SetECSManager(m_ECSManager);

	// default systems
	// 넣는 순서에 따라 system이 돌아가는게 달라짐
	m_ECSManager->InsertSystem(new ECSsystem::SyncWithTransform);
	m_ECSManager->InsertSystem(new ECSsystem::MoveByInput);

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

void Scene::Update(float deltaTime)
{
	m_ECSManager->UpdateSystem(deltaTime);
}

void Scene::Render(std::vector<ComPtr<ID3D12GraphicsCommandList>>& commandLists)
{
	// 기본 render, forward render이다

	// camera set
	auto& res = m_ResourceManager;
	//res->m_MainCamera->SetCameraData(commandLists[0]);
	
	// get Camera
	std::vector<component::Camera*> camVec;
	std::function<void(component::Camera*)> getCam = [&commandLists, &camVec](component::Camera* cam) {	camVec.push_back(cam); };
	m_ECSManager->Execute(getCam);

	camVec[0]->SetCameraData(commandLists[0]);

	// heap set
	auto& heap = res->m_ShaderResourceHeap;
	commandLists[0]->SetDescriptorHeaps(1, heap.GetAddressOf());
	commandLists[0]->SetGraphicsRootDescriptorTable(0, heap->GetGPUDescriptorHandleForHeapStart());

	// make function
	std::function<void(component::Renderer*)> func = [&commandLists, &res](component::Renderer* renderComponent) {
		int material = renderComponent->GetMaterial();
		int mesh = renderComponent->GetMesh();

		res->m_Materials[material]->GetShader()->Render(commandLists[0]);

		res->m_Materials[material]->SetDatas(commandLists[0], ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT);

		XMFLOAT4X4 t = Matrix4x4::Identity();
		res->m_Meshes[mesh]->Render(commandLists[0], t);
		};

	// execute function
	m_ECSManager->Execute<component::Renderer>(func);

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