#pragma once

// �⺻�� �Ǵ� ������Ʈ
// ���� ��ġ�Ǵ� �͵��� ��� �� ��� ����
// ����Ʈ�� ������Ʈ���̽�

class ObjectBase
{
public:
	ObjectBase() {}
	virtual ~ObjectBase() {}

private:
	// ������ -1
	int m_MeshID = -1;
	// ������ -1
	int m_MaterialID = -1;


	// ������Ʈ X ���� X
	bool m_Active = true;
	// ������Ʈ O ���� X
	bool m_RenderOn = true;

public:
	void SetMesh(int mesh) { m_MeshID = mesh; }
	void SetMaterial(int material) { m_MaterialID = material; }

	bool GetActive() const { return m_Active; }
	bool GetRenderOn() const { return m_RenderOn; }

	void SetActive(bool active) { m_Active = active; }
	void SetRenderOn(bool active) { m_RenderOn = active; }



};

