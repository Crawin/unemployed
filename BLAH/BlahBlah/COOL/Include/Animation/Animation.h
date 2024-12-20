﻿#pragma once

class ResourceManager;

// Animation 데이터 그 자체
class Animation
{
	friend class ResourceManager;
	friend class AnimationPlayer;

	// name
	std::string m_Name = "";

	int m_AnimationDataIdx = -1;
	unsigned int m_TotalPlayFrame = 500;
	unsigned int m_BoneLen = 500;

	std::vector<XMFLOAT4X4> m_AnimData;

	void LoadAnimation(ComPtr<ID3D12GraphicsCommandList> commandList, std::ifstream& file, ResourceManager* manager);
	
	bool m_Loop = true;
public:

	const std::string& GetName() const { return m_Name; }

	// todo 추후에 변경 가능성도 고려 해보자
	static const int m_Frame = 24;

	int GetFrame() { return m_Frame; }

	float GetEndTime() const { return static_cast<float>(m_TotalPlayFrame) / static_cast<float>(m_Frame); }
	int GetEndFrame() const { return m_TotalPlayFrame;  }
	int GetDataIdx() const { return m_AnimationDataIdx; }
	bool GetLoop() const { return m_Loop; }
	unsigned int GetBoneLen() const { return m_BoneLen; }

	const XMFLOAT4X4& GetBone(int idx) { return m_AnimData[idx]; }
};

