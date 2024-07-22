#pragma once

class ResourceManager;

class Bone
{
	friend class ResourceManager;

	// name
	std::string m_Name = "";

	// 일단 matrix를 cpu에도 보관하자
	std::vector<XMFLOAT4X4> m_Bones;

	int m_BoneDataIdx = -1;
	int m_Lenght = -1;

	void LoadBone(ComPtr<ID3D12GraphicsCommandList> commandList, std::ifstream& file, ResourceManager* manager);

public:
	const std::string& GetName() const { return m_Name; }

	int GetLength() const { return m_Lenght; }
	int GetBoneDataIdx() const { return m_BoneDataIdx; }
};

