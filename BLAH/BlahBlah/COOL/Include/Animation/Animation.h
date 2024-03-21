#pragma once

class ResourceManager;

class Animation
{
	friend class ResourceManager;
	friend class AnimationPlayer;

	// name
	std::string m_Name = "";

	int m_AnimationDataIdx = -1;
	unsigned int m_TotalPlayFrame = 500;

	void LoadAnimation(ComPtr<ID3D12GraphicsCommandList> commandList, std::ifstream& file, ResourceManager* manager);
	
public:

	const std::string& GetName() const { return m_Name; }

	// todo 추후에 변경 가능성도 고려 해보자
	static const int m_Frame = 24;

	float GetEndTime() const { return static_cast<float>(m_TotalPlayFrame) / static_cast<float>(m_Frame); }
	int GetEndFrame() const { return m_TotalPlayFrame;  }
	int GetDataIdx() const { return m_AnimationDataIdx; }
};

