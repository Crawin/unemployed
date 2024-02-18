#include "framework.h"
#include "SceneManager.h"
#include "Scene.h"
#include "Object/Component.h"

#include "TestMainScene.h"

void SceneManager::RegisterComponents()
{
	REGISTER_COMPONENT(component::Name, "Name");
	REGISTER_COMPONENT(component::Renderer, "Renderer");
	REGISTER_COMPONENT(component::Transform, "Transform");
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
