#include "framework.h"
#include "Mesh.h"
#include "MeshManager.h"

MeshManager::MeshManager()
{
}

MeshManager::~MeshManager()
{
	for (auto& mesh : m_Meshes) {
		delete mesh;
	}
}

void MeshManager::BuildMesh(ComPtr<ID3D12GraphicsCommandList> commandList, std::ifstream& file, Mesh* mesh)
{
	//mesh->BuildMesh(commandList, file);

	m_Meshes.push_back(mesh);
	//m_MeshMap[mesh->m_Name] = mesh;


	// 8. 서브메쉬 개수
	unsigned int childNum;
	file.read((char*)&childNum, sizeof(unsigned int));
	//mesh->m_Childs.reserve(childNum);

	// 9. 서브메쉬(재귀)
	for (unsigned int i = 0; i < childNum; ++i) {
		Mesh* child = new Mesh;
		BuildMesh(commandList, file, child);
		//mesh->m_Childs.push_back(child);
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
	//mesh->m_Name = ExtractFileName(fileName);
	BuildMesh(commandList, meshFile, mesh);


	//DebugPrint(std::format("loaded file name: {}", mesh->m_Name));
	return true;
}

bool MeshManager::LoadFolder(ComPtr<ID3D12GraphicsCommandList> commandList, const std::string& pathName)
{
	if (std::filesystem::exists(pathName) == false) {
		DebugPrint(std::format("ERROR!! no such path!!, path: {}", pathName));
	}

	DebugPrint(std::format("Mesh Directory ::::::: {}", pathName));

	for (const auto& file : std::filesystem::directory_iterator(pathName)) {
		if (file.is_directory()) continue;

		std::string fileName = pathName + file.path().filename().string();
		if (LoadFile(commandList, fileName) == false) return false;
	}

	DebugPrint("\n");

	return true;
}

Mesh* MeshManager::GetMesh(const std::string& name)
{
	//for (const auto& mesh : m_Meshes) 
		//if (mesh->m_Name == name) return mesh;

	DebugPrint(std::format("ERROR!! no such mesh, name: {}", name));
    return nullptr;
}

Mesh* MeshManager::GetMesh(int idx)
{
	if (0 <= idx && idx < m_Meshes.size()) 
		return m_Meshes[idx];

	DebugPrint(std::format("ERROR!! MeshManager out of index!! mesh idx: {}", idx));
	return nullptr;
}
