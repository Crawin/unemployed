#pragma once
#include "../Object/ObjectBase.h"
struct CameraShaderData {
	XMFLOAT4X4 m_ViewMatrix;
	XMFLOAT4X4 m_ProjMatrix;
	XMFLOAT3 m_CameraPosition;
};

// 기본 카메라 클래스
// 나중에 1인칭 카메라나
// CCTV 카메라나 이런걸 상속시키자

class Camera : 
	public ObjectBase
{
	XMFLOAT4X4 m_ViewMatrix = Matrix4x4::Identity();
	XMFLOAT4X4 m_ProjMatrix = Matrix4x4::Identity();

	XMFLOAT3 m_Right =		{ 1.0f, 0.0f, 0.0f };
	XMFLOAT3 m_Up =			{ 0.0f, 1.0f, 0.0f };
	XMFLOAT3 m_Look =		{ 0.0f, 0.0f, 1.0f };

	float m_Fov = 90.0f;
	float m_Aspect = 1.777f;
	float m_Near = 0.1f;
	float m_Far = 1000.0f;

	// 컬링용
	BoundingFrustum m_BoundingFrustum{};

	// root signature
	CameraShaderData* m_ShaderData = nullptr;
	int m_MappedShaderData = -1;

public:
	Camera();
	Camera(float fov, float aspect, float nearPlane, float farPlane);
	virtual ~Camera();

	void Init();
	
	void SetLook(const XMFLOAT3& look) { m_Look = look; }
	void SetRight(const XMFLOAT3& right) { m_Right = right; }
	void SetUp(const XMFLOAT3& up) { m_Up = up; }

	XMFLOAT3 GetLook() const { return m_Look; }
	XMFLOAT3 GetRight() const { return m_Right; }
	XMFLOAT3 GetUp() const { return m_Up; }

	XMFLOAT4X4 GetViewMat() const { return m_ViewMatrix; }
	XMFLOAT4X4 GetProjMat() const { return m_ProjMatrix; }

	void Render(ComPtr<ID3D12GraphicsCommandList> commandList);

private:
	// 행렬 재생성
	void BuildViewMatrix();
	void BuildProjectionMatrix();

	void UpdateShaderData();


	//BoundingFrustum& GetBoundingFrustum() const { return m_BoundingFrustum; }

};

