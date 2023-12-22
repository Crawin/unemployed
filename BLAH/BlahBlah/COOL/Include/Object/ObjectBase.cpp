#include "../framework.h"
#include "ObjectBase.h"

void ObjectBase::SetChildWorlTransform(const ObjectBase* parent)
{
	// 1. ������ ���� ����� ����
	//XMMatrixRotationAxis();
	Animate();

	XMMATRIX scale = XMMatrixScaling(m_Scale.x, m_Scale.x, m_Scale.x);
	XMMATRIX rotate = XMMatrixRotationQuaternion(XMLoadFloat4(&m_Rotate));
	XMMATRIX translate = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);

	XMMATRIX worldMatrix = XMMatrixMultiply(translate, XMMatrixMultiply(rotate, scale));

	XMStoreFloat4x4(&m_LocalWorldMatrix, worldMatrix);

	if (parent) {
		// �θ� ����� �ִٸ� �θ�� ���� ���� world�� �ִ´�
		// todo
		// �θ��� ����� �ڽĿ��ٰ� ��ȯ����.
		const XMFLOAT4X4& parentMatrix = parent->m_WorldMatrix;
		XMMATRIX parent = XMLoadFloat4x4(&parentMatrix);

		XMMATRIX realWorld = XMMatrixMultiply(parent, worldMatrix);
		XMStoreFloat4x4(&m_WorldMatrix, realWorld);
	}
	else {
		// �θ� ���ٸ� ������ ���� ����� ��������� ��
		XMStoreFloat4x4(&m_WorldMatrix, worldMatrix);
	}

	for (auto& child : m_Children) {
		child->SetChildWorlTransform(this);
	}
}
