#include "../framework.h"
#include "MaterialManager.h"
#include "Material.h"

MaterialManager::MaterialManager()
{
}

MaterialManager::~MaterialManager()
{
	for (int i = 0; i < m_Materials.size(); ++i) 
		if (m_Materials[i]) delete m_Materials[i];
}

bool MaterialManager::LoadFile(ComPtr<ID3D12GraphicsCommandList> commandList, const std::string& fileName)
{
	std::string file = m_CurrentPath + "\\" + fileName;

	std::ifstream matFile(file, std::ios::binary);

	if (matFile.fail()) {
		DebugPrint(std::format("Failed to open mesh file!! fileName: {}", fileName));
		return false;
	}
	
	// json 파일을 파싱 해야한다.



	//Mesh* mesh = new Mesh;
	//mesh->m_Name = ExtractFileName(fileName);
	//BuildMesh(commandList, meshFile, mesh);


	//DebugPrint(std::format("loaded file name: {}", mesh->m_Name));
	return true;
}

bool MaterialManager::LoadFolder(ComPtr<ID3D12GraphicsCommandList> commandList, const std::string& pathName)
{
	m_CurrentPath = pathName;

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

Material* MaterialManager::GetMaterial(const std::string& name)
{
	for (auto& mat : m_Materials)
		if (mat->GetName() == name) return mat;

	return nullptr;
}
