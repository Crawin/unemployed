#include "framework.h"
#include "MaterialManager.h"
#include "Material.h"
#include "Renderer/Renderer.h"
#include "Shader/ShaderManager.h"


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
	Material* material = new Material;
	std::string shaderName;
	if (false == material->LoadFile(commandList, fileName, this, shaderName)) {
		DebugPrint(std::format("Failed to load material file!! fileName: {}", fileName));
		delete material;
		return false;
	}

	m_ShaderManager->AddMaterial(shaderName, material);
	material->m_Shader = m_ShaderManager->GetShader(shaderName);

	m_Materials.push_back(material);

	DebugPrint(std::format("loaded file name: {}", material->m_Name));
	return true;
}

bool MaterialManager::LoadFolder(ComPtr<ID3D12GraphicsCommandList> commandList, const std::string& pathName)
{
	m_CurrentPath = pathName;

	if (std::filesystem::exists(pathName) == false) {
		DebugPrint(std::format("ERROR!! no such path!!, path: {}", pathName));
	}

	DebugPrint(std::format("Material Directory ::::::: {}", pathName));

	for (const auto& file : std::filesystem::directory_iterator(pathName)) {
		if (file.is_directory()) continue;

		std::string fileName = pathName + file.path().filename().string();
		if (LoadFile(commandList, fileName) == false) return false;
	}

	DebugPrint("\n");

	return true;
}

Material* MaterialManager::GetMaterial(const std::string& name)
{
	for (auto& mat : m_Materials)
		if (mat->GetName() == name) return mat;

	DebugPrint(std::format("ERROR!! no such material, name: {}", name));
	return nullptr;
}

Material* MaterialManager::GetMaterial(int idx)
{
	if (0 <= idx && idx < m_Materials.size())
		return m_Materials[idx];

	DebugPrint(std::format("ERROR!! MaterialManager out of index!! mesh idx: {}", idx));
	return nullptr;
}

int MaterialManager::LoadOrGetTexture(ComPtr<ID3D12GraphicsCommandList> commandList, const std::string& name)
{
	// 없다면 새로 추가한다
	if (false == m_TextureIndexMap.contains(name)) {
		std::string dds = m_CurrentPath + "Texture\\" + name;
		std::wstring fileNamewstr(dds.begin(), dds.end());

		int idx = Renderer::GetInstance().CreateTextureFromDDSFile(commandList, fileNamewstr.c_str());

		m_TextureIndexMap[name] = idx;
	}

	return m_TextureIndexMap[name];
}
