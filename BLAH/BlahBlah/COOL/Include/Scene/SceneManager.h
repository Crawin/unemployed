#pragma once

class Scene;
// ��� ����
// 1. ������ �ϴٰ�(�ΰ��Ӿ�) �ٸ� ������ �̵��ؾ��Ѵ�(���� ��������, ���θ޴�, ���ӿ�����)
//		�ΰ��Ӿ����� ���ӿ����� �Ǵ� ��Ȳ�� ChangeScene(���ӿ�����)
//		1. �ΰ��Ӿ�.Exit() ȣ��
//		2. ���ӿ����� Enter() ȣ��
//		3. �ΰ��Ӿ�.Update()�� return;

class SceneManager
{
	SceneManager();
	~SceneManager();

	Scene* m_CurrentScene = nullptr;
	Scene* m_PrevScene = nullptr;
	Scene* m_NextScene = nullptr;

public:
	static SceneManager& GetInstance() {
		static SceneManager inst;
		return inst;
	}

	// �� ��ȯ�� �ε��� �ִٸ� ��¼��? �ε� ���� ������ �־���� ���� ������?
	// �̰� �����غ���
	void ChangeScene(Scene* newScene);

	void ProcessInput(UINT msg, WPARAM wParam, LPARAM lParam);

};

