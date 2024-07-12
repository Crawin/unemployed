#include "framework.h"
#include "SceneManager.h"
#include "Scene.h"
#include "ECS/Component/Component.h"
#include "Scene/ResourceManager.h"
#include "TestMainScene.h"
#include "Shader/Shader.h"

void SceneManager::RegisterComponents()
{
	// 사용 할 컴포넌트가 늘어날 경우
	// framework.h로 가서 COMPONENT_COUNT의 숫자를 바꿔라
	REGISTER_COMPONENT(component::Name, "Name");
	REGISTER_COMPONENT(component::Renderer, "Renderer");
	REGISTER_COMPONENT(component::AnimationController, "AnimationController");
	REGISTER_COMPONENT(component::AnimationExecutor, "AnimationExecutor");
	REGISTER_COMPONENT(component::Attach, "Attach");
	//REGISTER_COMPONENT(component::Root, "Root");
	//REGISTER_COMPONENT(component::Children, "_MANUAL_00_CHILDREN");			// 자동으로 되어선 안된다
	REGISTER_COMPONENT(component::SelfEntity, "_MANUAL_01_SELFENTITY");			// 자동으로 되어선 안된다
	REGISTER_COMPONENT(component::Transform, "Transform");
	REGISTER_COMPONENT(component::Camera, "Camera");
	REGISTER_COMPONENT(component::Light, "Light");
	REGISTER_COMPONENT(component::Physics, "Physics");
	REGISTER_COMPONENT(component::DayLight, "DayLight");
	REGISTER_COMPONENT(component::Server, "Server");
	REGISTER_COMPONENT(component::Collider, "Collider");
	REGISTER_COMPONENT(component::DynamicCollider, "DynamicCollider");
	REGISTER_COMPONENT(component::AttachInput, "AttachInput");
	REGISTER_COMPONENT(component::Interaction, "Interaction");
	REGISTER_COMPONENT(component::Player, "Player");
	REGISTER_COMPONENT(component::PlayerController, "PlayerController");
	REGISTER_COMPONENT(component::Pawn, "Pawn");

	// temp
	REGISTER_COMPONENT(component::TestInput, "TestInput");

	// ui
	REGISTER_COMPONENT(component::UICanvas, "UICanvas");
	REGISTER_COMPONENT(component::UITransform, "UITransform");
	REGISTER_COMPONENT(component::Button, "Button");
	REGISTER_COMPONENT(component::UIRenderer, "UIRenderer");


	// content
	REGISTER_COMPONENT(component::DiaAnimationControl, "DiaAnimationControl");
	REGISTER_COMPONENT(component::PlayerAnimControll, "PlayerAnimControll");
	REGISTER_COMPONENT(component::DoorControl, "Door");
	REGISTER_COMPONENT(component::Inventory, "Inventory");
	REGISTER_COMPONENT(component::Holdable, "Holdable");
	REGISTER_COMPONENT(component::Attackable, "Attackable");
	REGISTER_COMPONENT(component::Throwable, "Throwable");
	REGISTER_COMPONENT(component::Screen, "Screen");
	REGISTER_COMPONENT(component::Sittable, "Sittable");


	// ui content
	REGISTER_COMPONENT(component::UIKeypad, "UIKeypad");

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
	// Register Components
	RegisterComponents();


	// 임시
	std::string testscene = "Test";
	if (testscene == firstScene) 
	{
		m_CurrentScene = new TestMainScene;
		m_CurrentScene->m_SceneName = "Test";
		m_CurrentScene->Enter(commandList);
	}
#ifdef _DEBUG
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

void SceneManager::SyncWithRender(float deltaTime)
{
	if (m_CurrentScene) m_CurrentScene->RenderSync(deltaTime);
}

void SceneManager::Render(std::vector<ComPtr<ID3D12GraphicsCommandList>>& commandLists, D3D12_CPU_DESCRIPTOR_HANDLE resultRtv, D3D12_CPU_DESCRIPTOR_HANDLE resultDsv)
{
	m_CurrentScene->Render(commandLists, resultRtv, resultDsv);

}
