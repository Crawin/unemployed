#pragma once
#include "Scene.h"

class ObjectBase;
class Camera;

class TestMainScene :
    public Scene
{
    TestMainScene() {}
    virtual ~TestMainScene() {}

    std::vector<ObjectBase> m_SceneObjects;
    std::vector<Camera> m_Cameras;

    Camera* m_MainCamera;

public:


};

