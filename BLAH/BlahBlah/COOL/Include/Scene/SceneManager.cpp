#include "framework.h"
#include "SceneManager.h"
#include "Scene.h"
#include "Object/Component.h"
#include "Scene/ResourceManager.h"
#include "TestMainScene.h"
#include "Shader/Shader.h"
#include "ManagementComponents.h"

void SceneManager::RegisterComponents()
{
	// 사용 할 컴포넌트가 늘어날 경우
	// ManagementComponents로 들어가서 숫자를 바꿔라
	REGISTER_COMPONENT(component::Name, "Name");
	REGISTER_COMPONENT(component::Renderer, "Renderer");
	REGISTER_COMPONENT(component::Transform, "Transform");
	REGISTER_COMPONENT(component::Camera, "Camera");
}

SceneManager::SceneManager()
{
}


SceneManager::~SceneManager()
{
	if (m_PrevScene) delete m_PrevScene;
	if (m_CurrentScene) delete m_CurrentScene;
	if (m_NextScene) delete m_NextScene;
}

bool SceneManager::Init(ComPtr<ID3D12GraphicsCommandList> commandList, const char* firstScene)
{
	// todo 여기에다 둬도 될지 다시 생각해보자
	// Register Components
	RegisterComponents();


	// 임시
#ifdef _DEBUG
	std::string testscene = "Test";
	if (testscene == firstScene) 
	{
		m_CurrentScene = new TestMainScene;
		m_CurrentScene->m_SceneName = "Test";
		m_CurrentScene->Enter(commandList);
	}
#endif


	//m_CurrentScene = new Scene();

	return true;
}

void SceneManager::ChangeScene(Scene* newScene)
{
	if (newScene == nullptr) {
		ERROR_QUIT("ERROR!!!! NO SCENE")
		return;
	}

	m_CurrentScene->Exit();
	//newScene->Enter();

	if (m_PrevScene) delete m_PrevScene;
	m_PrevScene = m_CurrentScene;

	m_CurrentScene = newScene;

}

bool SceneManager::ProcessInput(UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (m_CurrentScene) 
	{
		return m_CurrentScene->ProcessInput(msg, wParam, lParam);
	}

	return false;
}

void SceneManager::Update(float deltaTime)
{
	if (m_CurrentScene) m_CurrentScene->Update(deltaTime);
}

void SceneManager::Render(std::vector<ComPtr<ID3D12GraphicsCommandList>>& commandLists)
{
	// 임시코드이다
	// todo 렌더가 된다는 것만 확인하고 바로 고치자

	// camera set
	auto& manager = m_CurrentScene->m_ResourceManager;
	manager->m_MainCamera->SetCameraData(commandLists[0]);

	// 리소스 set
	auto& heap = manager->m_ShaderResourceHeap;

	commandLists[0]->SetDescriptorHeaps(1, heap.GetAddressOf());
	commandLists[0]->SetGraphicsRootDescriptorTable(0, heap->GetGPUDescriptorHandleForHeapStart());

	for (int i = 0; i < manager->m_Components.size(); ++i) {
		component::Renderer* render = dynamic_cast<component::Renderer*>(manager->m_Components[i]);

		if (render) {
			int mat = render->GetMaterial();

			manager->m_Materials[mat]->GetShader()->Render(commandLists[0]);

			manager->m_Materials[mat]->SetDatas(commandLists[0], ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT);

			
			int mes = render->GetMesh();
			XMFLOAT4X4 t = Matrix4x4::Identity();
			manager->m_Meshes[mes]->Render(commandLists[0], t);
		}

	}


	//m_CurrentScene->Render(commandLists);
}
