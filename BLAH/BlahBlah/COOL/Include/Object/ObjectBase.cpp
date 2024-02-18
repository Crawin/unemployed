#include "framework.h"
#include "ObjectBase.h"
#include <json/json.h>

ObjectBase::ObjectBase()
{
}

ObjectBase::ObjectBase(Json::Value& root, ResourceManager* manager)
{
	// is Root = On;
	// 해당 생성자로 생성시엔 true임
	m_IsRoot = true;

	// null이 아닐 시 default값을 사용함
	m_Active = (root["Active"].isNull()) ?		m_Active : root["Active"].asBool();
	m_RenderOn = (root["RenderOn"].isNull()) ?	m_RenderOn : root["RenderOn"].asBool();
	m_Name = (root["Name"].isNull()) ?			m_Name : root["Name"].asString();

	// transform, 없으면 큰일 난다
	m_Position.x = root["Position"][0].asFloat();
	m_Position.y = root["Position"][1].asFloat();
	m_Position.z = root["Position"][2].asFloat();

	m_Rotate.x = root["Rotate"][0].asFloat();
	m_Rotate.y = root["Rotate"][1].asFloat();
	m_Rotate.z = root["Rotate"][2].asFloat();
	m_Rotate.w = root["Rotate"][3].asFloat();

	m_Scale.x = root["Scale"][0].asFloat();
	m_Scale.y = root["Scale"][1].asFloat();
	m_Scale.z = root["Scale"][2].asFloat();
	

}

ObjectBase::~ObjectBase()
{
}

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
