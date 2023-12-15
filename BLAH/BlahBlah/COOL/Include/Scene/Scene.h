#pragma once

class Scene
{
public:
	Scene();
	virtual ~Scene();

private:
	std::string m_SceneName = "�̸��� ���� �ʾҴ�";

public:
	// ���Ŵ���
	//static Scene& GetInstance() {
	//	static Scene inst;
	//	return inst;
	//}

	virtual bool Init();

	// ���� ���Խ� �� �ൿ ex) �� �ε�, npc ���� ���
	virtual void Enter() = 0;
	// ���η��� �� �� �ൿ ex) npc�� �̵�, �ε����̸� �ε� ���α׷����� �ۼ�Ʈ �ø���
	virtual void Update() = 0;
	// �� ������ �� �ൿ ex) ��ü ����, �̷���?
	virtual void Exit() = 0;
	// �Է� ó��, ���� ���� �ʿ䰡 ������
	virtual void ProcessMouseInput(UINT msg, int x, int y) = 0;
	virtual void ProcessKeyboardInput(unsigned char key) = 0;

	//void Render();
};

