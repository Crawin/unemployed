#include "framework.h"
#include "MaterialManager.h"
#include "Material.h"
#include "Renderer/Renderer.h"
#include <json/json.h>


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
	std::ifstream matFile(fileName, std::ios::binary);

	if (matFile.fail()) {
		DebugPrint(std::format("Failed to open material file!! fileName: {}", fileName));
		return false;
	}
	
	// json 파일을 파싱 해야한다.
	Json::Value root;
	Json::Reader reader;
	if (false == reader.parse(matFile, root)) {
		DebugPrint(std::format("Failed to open material file!! fileName: {}", fileName));
		return false;
	};

	Material* material = new Material;

	material->m_Name = root["name"].asString();
	//material->m_Name = root["name"].asString();

	// todo 유지보수 에러!!!!!!!!!!!!!!!!!!
	// MATERIAL_TYPES
	const char* materialTypes[] = { "BaseColor", "Roughness", "Metalic", "Specular", "Normal" };

	for (int i = 0; i < MATERIAL_TYPES::MATERIAL_END; ++i) {
		if (root[materialTypes[i]].isNull()) continue;

		// texture의 이름
		std::string name = root[materialTypes[i]].asString();

		// 없다면 새로 추가한다
		if (false == m_TextureIndexMap.contains(name)) {
			std::string dds = m_CurrentPath + "Texture\\" + name;
			std::wstring fileNamewstr(dds.begin(), dds.end());

			int idx = Renderer::GetInstance().CreateTextureFromDDSFile(commandList, fileNamewstr.c_str());

			m_TextureIndexMap[name] = idx;
		}

		material->m_TextureIndex[i] = m_TextureIndexMap[name];
	}

	m_Materials.push_back(material);

	return true;
}

bool MaterialManager::LoadFolder(ComPtr<ID3D12GraphicsCommandList> commandList, const std::string& pathName)
{
	m_CurrentPath = pathName;

	if (std::filesystem::exists(pathName) == false) {
		DebugPrint(std::format("ERROR!! no such path!!, path: {}", pathName));
	}

	for (const auto& file : std::filesystem::directory_iterator(pathName)) {
		if (file.is_directory()) continue;

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
