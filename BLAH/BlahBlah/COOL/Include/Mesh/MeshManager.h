#pragma once

// 모든 매니저 클래스는 싱글톤이 아니게 될 예정이다.

// todo
// mesh의 상속된 타입을 변경 가능하게 설정
// ex) Mesh, SkinedMesh
// 파일에 메쉬의 타입이 있음

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

