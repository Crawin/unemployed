#pragma once

// �⺻�� �Ǵ� ������Ʈ
// ���� ��ġ�Ǵ� �͵��� ��� �� ��� ����
// ����Ʈ�� ������Ʈ���̽��� ���
// ī�޶� ������Ʈ���̽��� ���

class ObjectBase
{
public:
	ObjectBase() {}
	virtual ~ObjectBase() {}

private:
	// ������Ʈ X ���� X
	bool m_Active = true;
	// ������Ʈ O ���� X
	bool m_RenderOn = true;

protected:
	// local position
	XMFLOAT3 m_LocalScale = { 1.0f, 1.0f, 1.0f };
	XMFLOAT4 m_LocalRotate = { 0.0f, 0.0f, 0.0f, 0.0f };		// ���ʹϾ��̴�.
	XMFLOAT3 m_LocalPosition = { 0.0f, 0.0f, 0.0f };
	XMFLOAT4X4 m_LocalWorldMatrix = Matrix4x4::Identity();

//private:
	// world position, �θ��� ���� �޾ƿͼ� ���� ����� �����
	XMFLOAT3 m_Scale = { 1.0f, 1.0f, 1.0f };
	XMFLOAT4 m_Rotate = { 0.0f, 0.0f, 0.0f, 0.0f };				// ���ʹϾ��̴�.
	XMFLOAT3 m_Position = { 0.0f, 0.0f, 0.0f };
	XMFLOAT4X4 m_WorldMatrix = Matrix4x4::Identity();

	// �ڽĵ���
	std::vector<ObjectBase*> m_Children;

protected:
	// �ִϸ��̼��� �ִٸ� animate �Լ����� local s, r, t�� �����Ѵ�.
	virtual void Animate() {}

public:
	virtual void Update() {}
	
	// ��ǻ� �̸��� animate���� ���� ������ �ͱ� �ѵ� �� ��¼�ھ��
	void SetChildWorlTransform(const ObjectBase* parent);

public:
	bool GetActive() const { return m_Active; }
	bool GetRenderOn() const { return m_RenderOn; }
	XMFLOAT4 GetLocalScale() const { return m_LocalRotate; }
	XMFLOAT3 GetLocalRotate() const { return m_LocalScale; }
	XMFLOAT3 GetLocalPosition() const { return m_LocalPosition; }
	XMFLOAT4 GeWorldScale() const { return m_Rotate; }
	XMFLOAT3 GeWorldRotate() const { return m_Scale; }
	XMFLOAT3 GeWorldPosition() const { return m_Position; }

	void SetActive(bool active) { m_Active = active; }
	void SetRenderOn(bool active) { m_RenderOn = active; }
	void SetLocalScale(const XMFLOAT3& scale) { m_LocalScale = scale; }
	void SetLocalRotate(const XMFLOAT4& rotate) { m_LocalRotate = rotate; }
	void SetLocalPosition(const XMFLOAT3& position) { m_LocalPosition = position; }
	void SetWorldScale(const XMFLOAT3& scale) { m_Scale = scale; }
	void SetWorldRotate(const XMFLOAT4& rotate) { m_Rotate = rotate; }
	void SetWorldPosition(const XMFLOAT3& position) { m_Position = position; }
};

