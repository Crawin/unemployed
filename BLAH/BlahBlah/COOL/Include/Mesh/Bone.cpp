#include "framework.h"
#include "Bone.h"
#include "Scene/ResourceManager.h"

void Bone::LoadBone(ComPtr<ID3D12GraphicsCommandList> commandList, std::ifstream& file, ResourceManager* manager)
{
	unsigned int size;
	file.read((char*)&size, sizeof(unsigned int));

	m_Bones.resize(size);
	file.read((char*)(&m_Bones[0]), sizeof(XMFLOAT4X4) * size);

	m_BoneDataIdx = manager->CreateBufferFromVector(commandList, m_Bones, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, std::format("bone_{}", m_Name), true);
}
