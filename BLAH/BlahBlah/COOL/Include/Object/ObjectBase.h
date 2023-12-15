#pragma once

// 기본이 되는 오브젝트
// 씬에 배치되는 것들은 모두 얘 상속 받음
// 라이트도 오브젝트베이스

class ObjectBase
{
public:
	ObjectBase() {}
	virtual ~ObjectBase() {}

private:
	// 없으면 -1
	int m_MeshID = -1;
	// 없으면 -1
	int m_MaterialID = -1;


	// 업데이트 X 렌더 X
	bool m_Active = true;
	// 업데이트 O 렌더 X
	bool m_RenderOn = true;

public:
	void SetMesh(int mesh) { m_MeshID = mesh; }
	void SetMaterial(int material) { m_MaterialID = material; }

	bool GetActive() const { return m_Active; }
	bool GetRenderOn() const { return m_RenderOn; }

	void SetActive(bool active) { m_Active = active; }
	void SetRenderOn(bool active) { m_RenderOn = active; }



};

