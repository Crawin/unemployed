#pragma once

// �� Enum
// �̰ſ� ���� �ε� �� ���� ������ �޶���
enum SCENE_TYPE {
	LOGO = 0,
	LOADING,			// loading
	MAIN,				// Main Scene
	SCENE_TYPE_MAX
};

// https://docs.unity3d.com/Packages/com.unity.render-pipelines.universal@17.0/manual/index.html
// urp ����
// ���̸�.json�� ���ԵǾ�� �� ����
// 0. �� �̸��� Ÿ��
// 1. next scene�� json ���� ��
// 2. �ε����� �ʿ�����?
// 3. ī�޶�(�̸�, ��ġ, IsMain, ť), isMain �̶�� scene���� ���� ����� ���� ������ ���
// 3. ���̴� ����Ʈ (�̸�, ť, ī�޶�� ���� �Ǿ�����, unity urp�� pass�� ����� �����̴�)
// 4. ���͸��� ����Ʈ (���̴��� ���� �Ǿ�����)
// 5. ������Ʈ ����Ʈ (Ÿ��(ī�޶�,�÷��̾�ĳ����,npc,�ǹ�)/�޽�/���͸���)

// �ǹ���
// �ٸ� ī�޶� ���۵ǰ� �Ϸ��� � ������ �ؾ��ұ�
// �ٸ� ī�޶󿡼� ���� ���ϴ� ���͸���� �����ϰ� ����?

// ��� �ε�� ������ �Ѵ�.

class MeshManager;
class ObjectManager;
class ObjectBase;
class Camera;

class Scene
{
	friend class SceneManager;
public:
	Scene();
	virtual ~Scene();

private:
	std::string m_SceneName = "�̸��� ���� �ʾҴ�";

protected:
	MeshManager* m_MeshManager = nullptr;
	ObjectManager* m_ObjectManager = nullptr;


	//virtual bool Init();
private:
	bool LoadScene(ComPtr<ID3D12GraphicsCommandList> commandList, const std::string& sceneName);

public:

	// ���� ���Խ� �� �ൿ ex) �� �ε�, npc ���� ���
	// true: �ε� �� �ʿ� / �ε��ϴ� �����带 �����Ѵ�. �ε������� �˷���
	// false: �ε� �� ���ʿ�
	virtual bool Enter(ComPtr<ID3D12GraphicsCommandList> commandList);
	// ���η��� �� �� �ൿ ex) npc�� �̵�, �ε����̸� �ε� ���α׷����� �ۼ�Ʈ �ø���
	virtual void Update(float deltaTime) = 0;
	// �� ������ �� �ൿ ex) ��ü ����, �̷���?
	virtual void Exit() = 0;
	// �Է� ó��. �ش� ���� Ȱ��ȭ�Ǿ� ���� �� �� �Է� ó��
	virtual bool ProcessInput(UINT msg, WPARAM wParam, LPARAM lParam) = 0;

	//void Render();
};

