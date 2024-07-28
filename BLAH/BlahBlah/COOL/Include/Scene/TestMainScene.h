#pragma once
#include "Scene.h"

class Material;

namespace component {
    class Pawn;
};

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

protected:
    virtual bool LoadSceneExtra(ComPtr<ID3D12GraphicsCommandList> commandList);

    virtual void OnPreRender(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_CPU_DESCRIPTOR_HANDLE resultDsv);

    virtual void OnSelfHost();

    virtual void OnSelfGuest();

    virtual void OnGuestEnter();

public:
    virtual bool Enter(ComPtr<ID3D12GraphicsCommandList> commandList);

    virtual void Update(float deltaTime);

    virtual void Exit();

    virtual bool ProcessInput(UINT msg, WPARAM wParam, LPARAM lParam);

    virtual void OnServerConnected();

    virtual void OnGameStarted();

    void CutScenePawnMoving(component::Pawn* pawn, const std::function<void()>& endEvent, const XMFLOAT3& endPos, const XMFLOAT3& endRot, float endTime, bool eventOn = true);

    void AddEndEventAfterTime(const std::function<void()>& endEvent, float endTime);

    void ShowConversationUI(int talker, int conversation);
    
    void HideConversationUI();

    // 최종 결과를 resultRtv, resultDsv에 넘긴다
    //virtual void Render(std::vector<ComPtr<ID3D12GraphicsCommandList>>& commandLists, D3D12_CPU_DESCRIPTOR_HANDLE resultRtv, D3D12_CPU_DESCRIPTOR_HANDLE resultDsv);
};

