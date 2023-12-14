#include "../framework.h"
#include "SceneManager.h"
#include "Scene.h"

SceneManager::SceneManager()
{
}


SceneManager::~SceneManager()
{
}

void SceneManager::ChangeScene(Scene* newScene)
{
	if (newScene == nullptr) {
		DebugPrint("ERROR!!!! NO SCENE");
		exit(1);
		return;
	}

	m_CurrentScene->Exit();
	newScene->Enter();

	if (m_PrevScene) delete m_PrevScene;
	m_PrevScene = m_CurrentScene;

	m_CurrentScene = newScene;

}

void SceneManager::ProcessInput(UINT msg, WPARAM wParam, LPARAM lParam)
{
	// todo/ �̿ϼ�
	// Ű����, ���콺 ������ ������ �й�
}
