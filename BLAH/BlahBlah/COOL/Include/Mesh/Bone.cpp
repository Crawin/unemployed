#include "framework.h"
#include "Bone.h"
#include "Scene/ResourceManager.h"

void Bone::LoadBone(ComPtr<ID3D12GraphicsCommandList> commandList, std::ifstream& file, ResourceManager* manager)
{
	unsigned int size;
	file.read((char*)&size, sizeof(unsigned int));

	//XMFLOAT4X4* data = new XMFLOAT4X4[size];

	//m_Bones.resize(size);
	//file.read((char*)data, sizeof(XMFLOAT4X4) * size);
	//memcpy(&m_Bones[0], data, sizeof(XMFLOAT4X4) * size);
	////file.read((char*)(&m_Bones[0]), sizeof(XMFLOAT4X4) * size);

	//manager->CreateBufferFromData(commandList, (char*)data, sizeof(XMFLOAT4X4), size, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, std::format("bone_{}", m_Name), RESOURCE_TYPES::SHADER);
	////m_BoneDataIdx = manager->CreateBufferFromVector(commandList, m_Bones, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, std::format("bone_{}", m_Name), RESOURCE_TYPES::SHADER);
	//m_Lenght = m_Bones.size();

	//delete[] data;

	m_Bones.resize(size);
	file.read((char*)(&m_Bones[0]), sizeof(XMFLOAT4X4) * size);

	UINT bytes = static_cast<UINT>(size) * static_cast<UINT>(sizeof(XMFLOAT4X4));
	bytes = ((bytes + 255) & ~255);

	char* data = new char[bytes];
	memcpy(data, (char*)(&m_Bones[0]), sizeof(XMFLOAT4X4) * size);

	m_BoneDataIdx = manager->CreateBufferFromData(commandList, data, sizeof(XMFLOAT4X4), size,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, std::format("bone_{}", m_Name), RESOURCE_TYPES::SHADER);
	m_Lenght = m_Bones.size();

	delete[] data;
}
