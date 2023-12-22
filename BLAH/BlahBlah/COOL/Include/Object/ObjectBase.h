#pragma once

// 기본이 되는 오브젝트
// 씬에 배치되는 것들은 모두 얘 상속 받음
// 라이트도 오브젝트베이스를 상속
// 카메라도 오브젝트베이스를 상속

class ObjectBase
{
public:
	ObjectBase() {}
	virtual ~ObjectBase() {}

private:
	// 업데이트 X 렌더 X
	bool m_Active = true;
	// 업데이트 O 렌더 X
	bool m_RenderOn = true;

protected:
	// local position
	XMFLOAT3 m_LocalScale = { 1.0f, 1.0f, 1.0f };
	XMFLOAT4 m_LocalRotate = { 0.0f, 0.0f, 0.0f, 0.0f };		// 쿼터니언이다.
	XMFLOAT3 m_LocalPosition = { 0.0f, 0.0f, 0.0f };
	XMFLOAT4X4 m_LocalWorldMatrix = Matrix4x4::Identity();

//private:
	// world position, 부모의 것을 받아와서 실제 행렬을 만든다
	XMFLOAT3 m_Scale = { 1.0f, 1.0f, 1.0f };
	XMFLOAT4 m_Rotate = { 0.0f, 0.0f, 0.0f, 0.0f };				// 쿼터니언이다.
	XMFLOAT3 m_Position = { 0.0f, 0.0f, 0.0f };
	XMFLOAT4X4 m_WorldMatrix = Matrix4x4::Identity();

	// 자식들임
	std::vector<ObjectBase*> m_Children;

protected:
	// 애니메이션이 있다면 animate 함수에서 local s, r, t를 변경한다.
	virtual void Animate() {}

public:
	virtual void Update() {}
	
	// 사실상 이름이 animate여야 하지 않을까 싶긴 한데 뭐 어쩌겠어요
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

