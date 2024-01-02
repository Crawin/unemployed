﻿#include "framework.h"
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
	
	Material* material = new Material;
	if (false == material->LoadFile(commandList, fileName, this)) {
		DebugPrint(std::format("Failed to load material file!! fileName: {}", fileName));
		delete material;
		return false;
	}

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
