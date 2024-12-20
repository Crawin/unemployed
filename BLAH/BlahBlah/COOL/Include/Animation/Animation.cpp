﻿#include "framework.h"
#include "Animation.h"
#include "Scene/ResourceManager.h"

void Animation::LoadAnimation(ComPtr<ID3D12GraphicsCommandList> commandList, std::ifstream& file, ResourceManager* manager)
{
	// max frame
	file.read((char*)(&m_TotalPlayFrame), sizeof(m_TotalPlayFrame));

	// bone length
	file.read((char*)(&m_BoneLen), sizeof(m_BoneLen));

	// bone play frame == zero base
	m_AnimData.resize(m_BoneLen * (m_TotalPlayFrame + 1));
	//std::vector<XMFLOAT4X4> data(boneLen * m_TotalPlayFrame);

	// todo 
	// vector에다 넣은 데이터로 리소스를 생성하려 하면
	// 리소스 복사시 메모리 침범한다.
	// 나중에 무조건 고치자
	// 프로그램 간헐적으로 시작할때 꺼지는 원인

	file.read((char*)(&m_AnimData[0]), sizeof(XMFLOAT4X4) * m_AnimData.size());

	UINT bytes = static_cast<UINT>(m_AnimData.size()) * static_cast<UINT>(sizeof(XMFLOAT4X4));
	bytes = ((bytes + 255) & ~255);

	char* data = new char[bytes];
	memcpy(data, (char*)(&m_AnimData[0]), sizeof(XMFLOAT4X4) * m_AnimData.size());

	m_AnimationDataIdx = manager->CreateBufferFromData(commandList, data, sizeof(XMFLOAT4X4), m_AnimData.size(),
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, std::format("anim_{}", m_Name), RESOURCE_TYPES::SHADER);

	delete[] data;
}
