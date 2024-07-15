#pragma once
#include "Scene.h"

class LoadingScene : public Scene
{
	Scene* m_NextScene = nullptr;
	int m_LoadingStep = 0;
	//int m_MaxLoadingStep;
	SceneManager* m_Manager = nullptr;
	
public: 

	virtual void Update(float deltaTime);
	
	virtual void RenderSync(float deltaTime) {}

	void SetNextScene(Scene* scene) { m_LoadingStep = 0; m_NextScene = scene; }
	void SetSceneManager(SceneManager* manager) { m_Manager = manager; }

	virtual void Render(std::vector<ComPtr<ID3D12GraphicsCommandList>>& commandLists, D3D12_CPU_DESCRIPTOR_HANDLE resultRtv, D3D12_CPU_DESCRIPTOR_HANDLE resultDsv);
};

