#pragma once

#include <DirectXMath.h>

using namespace DirectX;

namespace Vector3
{
	inline XMFLOAT3 ScalarProduct(XMFLOAT3& vector, float scalar, bool normalize = true)
	{
		XMFLOAT3 result;
		if (normalize)
			XMStoreFloat3(&result, XMVector3Normalize(XMLoadFloat3(&vector)) * scalar);
		else
			XMStoreFloat3(&result, XMLoadFloat3(&vector) * scalar);
		return result;
	}

	inline XMFLOAT3 Add(const XMFLOAT3& vector1, const XMFLOAT3& vector2)
	{
		XMFLOAT3 result;
		XMStoreFloat3(&result, XMLoadFloat3(&vector1) + XMLoadFloat3(&vector2));
		return result;
	}

	inline XMFLOAT3 Add(XMFLOAT3& vector1, XMFLOAT3& vector2, float scalar)
	{
		XMFLOAT3 result;
		XMStoreFloat3(&result, XMLoadFloat3(&vector1) + (XMLoadFloat3(&vector2) * scalar));
		return result;
	}

	inline XMFLOAT3 Subtract(XMFLOAT3& vector1, XMFLOAT3& vector2)
	{
		XMFLOAT3 result;
		XMStoreFloat3(&result, XMLoadFloat3(&vector1) - XMLoadFloat3(&vector2));
		return result;
	}

	inline float DotProduct(XMFLOAT3& vector1, XMFLOAT3& vector2)
	{
		XMFLOAT3 result;
		XMStoreFloat3(&result, XMVector3Dot(XMLoadFloat3(&vector1), XMLoadFloat3(&vector2)));
		return result.x;
	}

	inline XMFLOAT3 CrossProduct(XMFLOAT3& vector1, XMFLOAT3& vector2, bool bNormalize = true)
	{
		XMFLOAT3 result;
		if (bNormalize)
			XMStoreFloat3(&result, XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&vector1), XMLoadFloat3(&vector2))));
		else
			XMStoreFloat3(&result, XMVector3Cross(XMLoadFloat3(&vector1), XMLoadFloat3(&vector2)));
		return result;
	}

	inline XMFLOAT3 Normalize(XMFLOAT3& vector)
	{
		XMFLOAT3 m_xmf3Normal;
		XMStoreFloat3(&m_xmf3Normal, XMVector3Normalize(XMLoadFloat3(&vector)));
		return m_xmf3Normal;
	}

	inline float Length(XMFLOAT3& vector)
	{
		XMFLOAT3 result;
		XMStoreFloat3(&result, XMVector3Length(XMLoadFloat3(&vector)));
		return result.x;
	}

	//inline float Angle(XMVECTOR& vector1, XMVECTOR& vector2)
	//{
	//	XMVECTOR xmvAngle = XMVector3AngleBetweenNormals(vector1, vector2);
	//	return XMConvertToDegrees(XMVectorGetX(xmvAngle));
	//}

	inline float Angle(XMFLOAT3& vector1, XMFLOAT3& vector2)
	{
		XMVECTOR angle = XMVector3AngleBetweenNormals(XMLoadFloat3(&vector1), XMLoadFloat3(&vector2));
		float temp = XMVectorGetX(angle);
		return XMConvertToDegrees(temp);
	}

	inline XMFLOAT3 TransformNormal(XMFLOAT3& vector, XMMATRIX& transformMatrix)
	{
		XMFLOAT3 result;
		XMStoreFloat3(&result, XMVector3TransformNormal(XMLoadFloat3(&vector), transformMatrix));
		return result;
	}

	inline XMFLOAT3 TransformCoord(XMFLOAT3& vector, XMMATRIX& transformMatrix)
	{
		XMFLOAT3 result;
		XMStoreFloat3(&result, XMVector3TransformCoord(XMLoadFloat3(&vector), transformMatrix));
		return result;
	}

	inline XMFLOAT3 TransformCoord(XMFLOAT3& vector, XMFLOAT4X4& mat)
	{
		XMMATRIX temp = XMLoadFloat4x4(&mat);
		return TransformCoord(vector, temp);
	}
}

namespace Vector4
{
	inline XMFLOAT4 Add(XMFLOAT4& xmf4Vector1, XMFLOAT4& xmf4Vector2)
	{
		XMFLOAT4 result;
		XMStoreFloat4(&result, XMLoadFloat4(&xmf4Vector1) + XMLoadFloat4(&xmf4Vector2));
		return result;
	}

	inline XMFLOAT4 Multiply(float fScalar, XMFLOAT4& xmf4Vector)
	{
		XMFLOAT4 result;
		XMStoreFloat4(&result, fScalar * XMLoadFloat4(&xmf4Vector));
		return result;
	}
}

namespace Matrix4x4
{
	inline XMFLOAT4X4 Identity()
	{
		XMFLOAT4X4 result;
		XMStoreFloat4x4(&result, XMMatrixIdentity());
		return result;
	}

	inline XMFLOAT4X4 Multiply(XMFLOAT4X4& mat1, XMFLOAT4X4& mat2)
	{
		XMFLOAT4X4 result;
		XMStoreFloat4x4(&result, XMLoadFloat4x4(&mat1) * XMLoadFloat4x4(&mat2));
		return result;
	}

	inline XMFLOAT4X4 Multiply(XMFLOAT4X4& mat1, XMMATRIX& mat2)
	{
		XMFLOAT4X4 result;
		XMStoreFloat4x4(&result, XMLoadFloat4x4(&mat1) * mat2);
		return result;
	}

	inline XMFLOAT4X4 Multiply(XMMATRIX& xmmtxMatrix1, XMFLOAT4X4& mat2)
	{
		XMFLOAT4X4 result;
		XMStoreFloat4x4(&result, xmmtxMatrix1 * XMLoadFloat4x4(&mat2));
		return result;
	}

	inline XMFLOAT4X4 Inverse(XMFLOAT4X4& mat)
	{
		XMFLOAT4X4 result;
		XMStoreFloat4x4(&result, XMMatrixInverse(NULL, XMLoadFloat4x4(&mat)));
		return result;
	}

	inline XMFLOAT4X4 Transpose(XMFLOAT4X4& mat)
	{
		XMFLOAT4X4 result;
		XMStoreFloat4x4(&result, XMMatrixTranspose(XMLoadFloat4x4(&mat)));
		return result;
	}

	inline XMFLOAT4X4 Orthographic(float ViewWidth, float ViewHeight, float nearZ, float farZ)
	{
		XMFLOAT4X4 result;
		XMStoreFloat4x4(&result, XMMatrixOrthographicLH(ViewWidth, ViewHeight, nearZ, farZ));
		return result;
	}

	inline XMFLOAT4X4 PerspectiveFovLH(float FovAngleY, float AspectRatio, float NearZ, float FarZ)
	{
		XMFLOAT4X4 result;
		XMStoreFloat4x4(&result, XMMatrixPerspectiveFovLH(FovAngleY, AspectRatio, NearZ, FarZ));
		return result;
	}

	inline XMFLOAT4X4 LookAtLH(XMFLOAT3& eye, XMFLOAT3& at, XMFLOAT3& up)
	{
		XMFLOAT4X4 result;
		XMStoreFloat4x4(&result, XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&at), XMLoadFloat3(&up)));
		return result;
	}
}
