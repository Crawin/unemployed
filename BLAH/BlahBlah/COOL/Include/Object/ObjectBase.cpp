#include "framework.h"
#include "ObjectBase.h"

void ObjectBase::SetChildWorlTransform(const ObjectBase* parent)
{
	// 1. 본인의 로컬 행렬을 생성
	//XMMatrixRotationAxis();
	Animate();

	XMMATRIX scale = XMMatrixScaling(m_Scale.x, m_Scale.x, m_Scale.x);
	XMMATRIX rotate = XMMatrixRotationQuaternion(XMLoadFloat4(&m_Rotate));
	XMMATRIX translate = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);

	XMMATRIX worldMatrix = XMMatrixMultiply(translate, XMMatrixMultiply(rotate, scale));

	XMStoreFloat4x4(&m_LocalWorldMatrix, worldMatrix);

	if (parent) {
		// 부모 행렬이 있다면 부모와 곱한 값을 world에 넣는다
		// todo
		// 부모의 행렬을 자식에다가 변환하자.
		const XMFLOAT4X4& parentMatrix = parent->m_WorldMatrix;
		XMMATRIX parent = XMLoadFloat4x4(&parentMatrix);

		XMMATRIX realWorld = XMMatrixMultiply(parent, worldMatrix);
		XMStoreFloat4x4(&m_WorldMatrix, realWorld);
	}
	else {
		// 부모가 없다면 본인의 로컬 행렬이 월드행렬이 됨
		XMStoreFloat4x4(&m_WorldMatrix, worldMatrix);
	}

	for (auto& child : m_Children) {
		child->SetChildWorlTransform(this);
	}
}
