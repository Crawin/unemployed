#pragma once

#define SCENE_PATH "SceneData\\"

class Scene;
// ��� ����
// 1. ������ �ϴٰ�(�ΰ��Ӿ�) �ٸ� ������ �̵��ؾ��Ѵ�(���� ��������, ���θ޴�, ���ӿ�����)
//		�ΰ��Ӿ����� ���ӿ����� �Ǵ� ��Ȳ�� ChangeScene(���ӿ�����)
//		1. �ΰ��Ӿ�.Exit() ȣ��
//		2. ���ӿ����� Enter() ȣ��
//		3. �ΰ��Ӿ�.Update()�� return;
// ���� �̱��濡�� Application�� ����� �ٲ�.

class SceneManager
{
	std::string m_BaseScenePath = SCENE_PATH;

	Scene* m_CurrentScene = nullptr;
	Scene* m_PrevScene = nullptr;
	Scene* m_NextScene = nullptr;

public:
	SceneManager();
	~SceneManager();

//public:
//	static SceneManager& GetInstance() {
//		static SceneManager inst;
//		return inst;
//	}

	bool Init(ComPtr<ID3D12GraphicsCommandList> commandList, const char* firstSceneName);

	// �� ��ȯ�� �ε��� �ִٸ� ��¼��? �ε� ���� ������ �־���� ���� ������?
	// �̰� �����غ���
	void ChangeScene(Scene* newScene);

	bool ProcessInput(UINT msg, WPARAM wParam, LPARAM lParam);

	void Update(float deltaTime);

};

