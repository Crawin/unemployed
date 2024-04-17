#include "framework.h"
#include "Animation.h"
#include "Scene/ResourceManager.h"

void Animation::LoadAnimation(ComPtr<ID3D12GraphicsCommandList> commandList, std::ifstream& file, ResourceManager* manager)
{
	// max frame
	file.read((char*)(&m_TotalPlayFrame), sizeof(m_TotalPlayFrame));

	// bone length
	unsigned int boneLen;
	file.read((char*)(&boneLen), sizeof(boneLen));

	m_AnimData.resize(boneLen * m_TotalPlayFrame);
	//std::vector<XMFLOAT4X4> data(boneLen * m_TotalPlayFrame);
	file.read((char*)(&m_AnimData[0]), sizeof(XMFLOAT4X4) * m_AnimData.size());

	m_AnimationDataIdx = manager->CreateBufferFromVector(commandList, m_AnimData, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, std::format("anim_{}", m_Name), RESOURCE_TYPES::SHADER);

}
