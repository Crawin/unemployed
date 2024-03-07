#pragma once
#include "Scene.h"

class ObjectBase;
//class Camera;

class TestMainScene :
    public Scene
{
    //Camera* m_MainCamera = nullptr;

public:
    TestMainScene() {}
    virtual ~TestMainScene() {}

public:
    virtual bool Enter(ComPtr<ID3D12GraphicsCommandList> commandList);

    virtual void Update(float deltaTime);

    virtual void Exit();

    virtual bool ProcessInput(UINT msg, WPARAM wParam, LPARAM lParam);

};

