#pragma once
#include "Scene.h"

class Material;

class TestMainScene :
    public Scene
{
    //Camera* m_MainCamera = nullptr;

public:
    TestMainScene() {}
    virtual ~TestMainScene() {}

private:
    int m_SkyMaterialIdx = -1;
    int m_SphereSkyMeshIndex = -1;

    void ChangeDayToNight();

protected:
    virtual bool LoadSceneExtra(ComPtr<ID3D12GraphicsCommandList> commandList);

    virtual void OnPreRender(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_CPU_DESCRIPTOR_HANDLE resultDsv);

public:
    virtual bool Enter(ComPtr<ID3D12GraphicsCommandList> commandList);

    virtual void Update(float deltaTime);

    virtual void Exit();

    virtual bool ProcessInput(UINT msg, WPARAM wParam, LPARAM lParam);

    // 최종 결과를 resultRtv, resultDsv에 넘긴다
    //virtual void Render(std::vector<ComPtr<ID3D12GraphicsCommandList>>& commandLists, D3D12_CPU_DESCRIPTOR_HANDLE resultRtv, D3D12_CPU_DESCRIPTOR_HANDLE resultDsv);
};

