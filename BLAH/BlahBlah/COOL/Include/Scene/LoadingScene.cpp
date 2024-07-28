#include "framework.h"
#include "LoadingScene.h"
#include "Renderer/Renderer.h"
#include "ResourceManager.h"
#include "ECS/ECSManager.h"
#include "SceneManager.h"
#include "App/Application.h"
#include "Network/Client.h"

void LoadingScene::Exit()
{
	//Application::GetInstance().ChangeBorderlessMode(true);
}

void LoadingScene::Update(float deltaTime)
{
	//m_LoadingStep 
	// 0. init
	// 1. late init
	// 2. onstart
	// final.. scenemanager->ChangeScene;
	
	if (m_NextScene) {
		switch (m_LoadingStep) {
		case 0:
			m_NextScene->SetManagers();
			m_NextScene->AddSystem();
			break;

		case 1:
		{
			// init
			auto cmdAloc = Renderer::GetInstance().GetCommandAllocator(Renderer::CMDID::LOADING);
			auto cmdlist = Renderer::GetInstance().GetCommandList(Renderer::CMDID::LOADING);

			cmdAloc->Reset();
			cmdlist->Reset(cmdAloc.Get(), nullptr);
			bool res = m_NextScene->m_ResourceManager->Init(cmdlist, m_NextScene->m_SceneName);

			if (res == false) {
				ERROR_QUIT("LOADING ERROR!!, resource failed!");
			}

			Renderer::GetInstance().ExecuteAndEraseUploadHeap(cmdlist);
			break;
		}
		case 2:
		{
			// late init
			auto cmdAloc = Renderer::GetInstance().GetCommandAllocator(Renderer::CMDID::LOADING);
			auto cmdlist = Renderer::GetInstance().GetCommandList(Renderer::CMDID::LOADING);

			cmdAloc->Reset();
			cmdlist->Reset(cmdAloc.Get(), nullptr);

			m_NextScene->LoadSceneExtra(cmdlist);

			Renderer::GetInstance().ExecuteAndEraseUploadHeap(cmdlist);
			cmdAloc->Reset();
			cmdlist->Reset(cmdAloc.Get(), nullptr);
			break;
		}
		case 3:
			m_NextScene->m_ECSManager->InitSystem();
			break;

		}


		++m_LoadingStep;
	}

	// load end!
	// change scene
	if (m_LoadingStep > 3) {
		m_SceneManager->ChangeScene(m_NextScene, true);
		m_NextScene = nullptr;
		//Client::GetInstance().Connect_Server();
	}

}

void LoadingScene::Render(std::vector<ComPtr<ID3D12GraphicsCommandList>>& commandLists, D3D12_CPU_DESCRIPTOR_HANDLE resultRtv, D3D12_CPU_DESCRIPTOR_HANDLE resultDsv)
{
	// heap set
	SetResourceHeap(commandLists[0]);

	DrawUI(commandLists[0], resultRtv, resultDsv);
}
