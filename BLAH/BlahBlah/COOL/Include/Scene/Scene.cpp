#include "Scene.h"
//#include "Renderer/Renderer.h"
#include "framework.h"
#include "ECS/ECSManager.h"
#include "ResourceManager.h"
#include "ECS/Component.h"
#include "ECS/ECS_System.h"
#include "Shader/Shader.h"

#ifdef _DEBUG
#include "App/InputManager.h"
#endif
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
	m_ECSManager->InsertSystem(new ECSsystem::LocalToWorldTransform);
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

void Scene::Render(std::vector<ComPtr<ID3D12GraphicsCommandList>>& commandLists, D3D12_CPU_DESCRIPTOR_HANDLE resultRtv, D3D12_CPU_DESCRIPTOR_HANDLE resultDsv)
{
	// default deffered renderer
	m_ResourceManager->SetMRTStates(commandLists[0], D3D12_RESOURCE_STATE_RENDER_TARGET);

	// clear mrt
	float clearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_ResourceManager->ClearMRTS(commandLists[0], clearColor);

	// OM set
	auto rtMRT = m_ResourceManager->GetDefferedRenderTargetStart();
	commandLists[0]->OMSetRenderTargets(static_cast<int>(MULTIPLE_RENDER_TARGETS::MRT_END), &rtMRT, true, &resultDsv);
	//commandLists[0]->OMSetRenderTargets(1, &resultRtv, true, &resultDsv);

	// camera set
	auto& res = m_ResourceManager;
	//res->m_MainCamera->SetCameraData(commandLists[0]);
	
	// get light to make shadow map
	// todo 이걸 하자
	std::vector<component::Light*> lightVec;
	std::function<void(component::Light*)> getLight = [&commandLists, &lightVec](component::Light* light) { lightVec.push_back(light); };

	// light map
	// 1. light map을 만드는 모든 light들에 대해 프러스텀 컬링을 실시함 (renderer를 찾아 모아옴)
	// 2. lightmap 상태 변경(tex -> present)
	// 3. OM SET
	// 4. render
	// 4 - 1. shader에 light 데이터가 가야하는데 어케하지? light data를 
	// 4 - 2.
	// 5. present -> tex;


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

		res->m_Materials[material]->SetDatas(commandLists[0], static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT));

		//XMFLOAT4X4 t = Matrix4x4::Identity();
		res->m_Meshes[mesh]->Render(commandLists[0], renderComponent->GetWorldMatrix());
		};

	// execute function
	m_ECSManager->Execute<component::Renderer>(func);

	// mrt render end
	///////////////////////////////////////////////////////////////////////////////////////////

	// post processing
	commandLists[0]->ClearDepthStencilView(resultDsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	commandLists[0]->OMSetRenderTargets(1, &resultRtv, true, &resultDsv);
	m_ResourceManager->SetMRTStates(commandLists[0], D3D12_RESOURCE_STATE_COMMON);

	int postMat = m_ResourceManager->GetPostProcessingMaterial();

	m_ResourceManager->m_Materials[postMat]->GetShader()->Render(commandLists[0]);
	m_ResourceManager->m_Materials[postMat]->SetDatas(commandLists[0], static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT));

	commandLists[0]->DrawInstanced(6, 1, 0, 0);


#ifdef _DEBUG
	// test code
	// todo 적절한 위치로 옮기도록 하자
	if (InputManager::GetInstance().GetDebugMode()) {
		commandLists[0]->ClearDepthStencilView(resultDsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
		int debugMat = m_ResourceManager->GetDebuggingMaterial();

		m_ResourceManager->m_Materials[debugMat]->GetShader()->Render(commandLists[0]);
		int a[2] = { 0, 0 };
		commandLists[0]->SetGraphicsRoot32BitConstants(static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT), 1, a, 0);
		//m_ResourceManager->m_Materials[postMat]->SetDatas(commandLists[0], static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT));

		commandLists[0]->DrawInstanced(6, 1, 0, 0);
	}
#endif
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