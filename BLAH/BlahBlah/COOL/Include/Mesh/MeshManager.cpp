#include "framework.h"
#include "Mesh.h"
#include "MeshManager.h"

MeshManager::MeshManager()
{
}

MeshManager::~MeshManager()
{
	for (auto& pair : m_MeshMap) {
		delete pair.second;
	}
}

void MeshManager::BuildMesh(ComPtr<ID3D12GraphicsCommandList> commandList, std::ifstream& file, Mesh* mesh)
{
	mesh->BuildMesh(commandList, file);

	m_MeshMap[mesh->m_Name] = mesh;

	// 8. 서브메쉬 개수
	unsigned int childNum;
	file.read((char*)&childNum, sizeof(unsigned int));
	mesh->m_Childs.reserve(childNum);

	// 9. 서브메쉬(재귀)
	for (int i = 0; i < childNum; ++i) {
		Mesh* child = new Mesh;
		BuildMesh(commandList, file, child);
		mesh->m_Childs.push_back(child);
	}
}

bool MeshManager::LoadFile(ComPtr<ID3D12GraphicsCommandList> commandList, const std::string& fileName)
{
	std::ifstream meshFile(fileName, std::ios::binary);

	if (meshFile.fail()) {
		DebugPrint(std::format("Failed to open mesh file!! fileName: {}", fileName));
		return false;
	}

	Mesh* mesh = new Mesh;
	mesh->m_Name = ExtractFileName(fileName);
	BuildMesh(commandList, meshFile, mesh);


	DebugPrint(std::format("loaded file name: {}", mesh->m_Name));
	return true;
}

bool MeshManager::LoadFolder(ComPtr<ID3D12GraphicsCommandList> commandList, const std::string& pathName)
{
	if (std::filesystem::exists(pathName) == false) {
		DebugPrint(std::format("ERROR!! no such path!!, path: {}", pathName));
	}

	for (const auto& file : std::filesystem::directory_iterator(pathName)) {
		file.path().filename();

		std::string fileName = pathName + file.path().filename().string();
		if (LoadFile(commandList, fileName) == false) return false;
	}

	return true;
}

Mesh* MeshManager::GetMesh(const std::string& name)
{
    Mesh* mesh = m_MeshMap[name];

	if (mesh == nullptr) {
		DebugPrint(std::format("ERROR!! no such mesh, name: {}", name));
	}

    return mesh;
}
