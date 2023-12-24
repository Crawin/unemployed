#pragma once

// ��� �Ŵ��� Ŭ������ �̱����� �ƴϰ� �� �����̴�.

class Mesh;

class MeshManager
{
	std::map<std::string, Mesh*> m_MeshMap;

public:
	MeshManager();
	~MeshManager();

private:
	// build mesh from file, workd in recursion
	void BuildMesh(ComPtr<ID3D12GraphicsCommandList> commandList, std::ifstream& file, Mesh* mesh);

	// load file
	bool LoadFile(ComPtr<ID3D12GraphicsCommandList> commandList, const std::string& fileName);
public:

	// load files in folder path
	bool LoadFolder(ComPtr<ID3D12GraphicsCommandList> commandList, const std::string& pathName);

	Mesh* GetMesh(const std::string& name);
};

