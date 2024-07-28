#include "framework.h"
#include "SceneManager.h"
#include "Scene.h"
#include "ECS/Component/Component.h"
#include "Scene/ResourceManager.h"
#include "TestMainScene.h"
#include "LoadingScene.h"
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
	REGISTER_COMPONENT(component::DayLightManager, "DayLightManager");
	REGISTER_COMPONENT(component::Server, "Server");
	REGISTER_COMPONENT(component::Collider, "Collider");
	REGISTER_COMPONENT(component::DynamicCollider, "DynamicCollider");
	REGISTER_COMPONENT(component::AttachInput, "AttachInput");
	REGISTER_COMPONENT(component::Interaction, "Interaction");
	REGISTER_COMPONENT(component::Player, "Player");
	REGISTER_COMPONENT(component::Pawn, "Pawn");
	REGISTER_COMPONENT(component::PlayerController, "PlayerController");
	REGISTER_COMPONENT(component::AI, "AI");
	REGISTER_COMPONENT(component::Particle, "Particle");
	REGISTER_COMPONENT(component::ParticleEmitter, "ParticleEmitter");

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
	REGISTER_COMPONENT(component::CCTVController, "CCTVController");
	REGISTER_COMPONENT(component::RCController, "RCController");
	REGISTER_COMPONENT(component::VandingMachine, "VandingMachine");
	REGISTER_COMPONENT(component::Drink, "Drink");
	REGISTER_COMPONENT(component::CreditCard, "CreditCard");
	REGISTER_COMPONENT(component::KeyTool, "KeyTool");
	REGISTER_COMPONENT(component::Key, "Key");			// 굳이 필요 한것인가?
	REGISTER_COMPONENT(component::CreditCard, "CreditCard");
	REGISTER_COMPONENT(component::Consumable, "Consumable");
	REGISTER_COMPONENT(component::StartInteraction, "StartInteraction");


	
	// ui content
	REGISTER_COMPONENT(component::UIKeypad, "UIKeypad");
	REGISTER_COMPONENT(component::UIDoorKey, "UIDoorKey");
	REGISTER_COMPONENT(component::UICutLine, "UICutLine");
	REGISTER_COMPONENT(component::UITreasureChest, "UITreasureChest");
	REGISTER_COMPONENT(component::UIVandingMachine, "UIVandingMachine");
	REGISTER_COMPONENT(component::UIEnding, "UIEnding");
	REGISTER_COMPONENT(component::UIConversation, "UIConversation");
	REGISTER_COMPONENT(component::UILogin, "UILogin");

}

SceneManager::SceneManager()
{
}


SceneManager::~SceneManager()
{
	if (m_PrevScene) delete m_PrevScene;
	if (m_CurrentScene) delete m_CurrentScene;
	if (m_NextScene) delete m_NextScene;

	if (m_LoadingScene) delete m_LoadingScene;
}

bool SceneManager::Init(ComPtr<ID3D12GraphicsCommandList> commandList, const char* firstScene)
{
	// Register Components
	RegisterComponents();

	TestMainScene* mainScene = new TestMainScene;
	mainScene->m_SceneName = firstScene;
	mainScene->SetSceneManager(this);

	LoadingScene* loadScene = new LoadingScene;
	loadScene->m_SceneName = "Loading";
	loadScene->Enter(commandList);
	loadScene->SetSceneManager(this);
	loadScene->SetNextScene(mainScene);
	m_LoadingScene = loadScene;
	m_CurrentScene = m_LoadingScene;

	return true;
}

void SceneManager::SetToChangeScene(const std::string& nextSceneName, int sceneType)
{
	m_ToChangeScene = true;
	m_NextSceneName = nextSceneName;
	m_NextSceneType = sceneType;
}

void SceneManager::ChangeScene(Scene* newScene, bool isFromLoading)
{
	if (newScene == nullptr) {
		ERROR_QUIT("ERROR!!!! NO SCENE")
		return;
	}

	m_CurrentScene->Exit();
	//newScene->Enter();

	if (m_PrevScene != nullptr) {
		delete m_PrevScene;
		m_PrevScene = nullptr;
	}

	if (isFromLoading == false) 
		m_PrevScene = m_CurrentScene;


	m_CurrentScene = newScene;
	m_CurrentScene->Enter();
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
	// check scene change
	if (m_ToChangeScene) {
		Scene* nextScene = nullptr;
		switch (m_NextSceneType) {
		case 0:
			nextScene = new TestMainScene;
			nextScene->m_SceneName = m_NextSceneName;
			nextScene->SetSceneManager(this);
			break;
			
		case 1:
			//nextScene = new EndScene;
			//nextScene->m_SceneName = m_NextSceneName;
			//nextScene->SetSceneManager(this);
			break;

		}

		m_LoadingScene->SetNextScene(nextScene);
		
		m_CurrentScene->Exit();
		delete m_CurrentScene;
		m_CurrentScene = m_LoadingScene;

		m_ToChangeScene = false;
	}

	if (m_CurrentScene) m_CurrentScene->RenderSync(deltaTime);
}

void SceneManager::Render(std::vector<ComPtr<ID3D12GraphicsCommandList>>& commandLists, D3D12_CPU_DESCRIPTOR_HANDLE resultRtv, D3D12_CPU_DESCRIPTOR_HANDLE resultDsv)
{
	m_CurrentScene->Render(commandLists, resultRtv, resultDsv);

}

void SceneManager::ProcessPacket(packet_base* packet)
{
	if (m_CurrentScene) m_CurrentScene->ProcessPacket(packet);
}

void SceneManager::PossessPlayer(bool isHost)
{
	m_CurrentScene->PossessPlayer(isHost);
}

#ifdef _DEBUG

void SceneManager::StartGame()
{
	m_CurrentScene->OnGameStarted();
}

#endif
