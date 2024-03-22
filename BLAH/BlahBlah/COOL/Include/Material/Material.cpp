#include "framework.h"
#include "Material.h"
//#include "Renderer/Renderer.h"
#include "Scene/ResourceManager.h"
#include <json/json.h>

Material::Material()
{
	memset(m_TextureIndex, -1, _countof(m_TextureIndex));
}

Material::Material(std::string_view name)
	: m_Name{ name }
{
	memset(m_TextureIndex, -1, _countof(m_TextureIndex));
}

Material::~Material()
{
}

bool Material::LoadFile(ComPtr<ID3D12GraphicsCommandList> cmdList, const std::string& fileName, ResourceManager* manager, std::string& shaderName)
{
	// load file
	std::ifstream file(fileName);

	if (file.is_open() == false) 
	{
		DebugPrint(std::format("No Such File Name!!! file name: {}", fileName));
		return false;
	}


	// json 파일을 파싱 해야한다.
	Json::Value root;
	Json::Reader reader;

	if (false == reader.parse(file, root)) {
		DebugPrint(std::format("Failed to open material file!! fileName: {}", fileName));
		return false;
	};

	if (root["name"].isNull()) {
		DebugPrint("ERROR!! has no name!!");
		return false;
	}

	m_Name = root["name"].asString();
	//material->m_Name = root["name"].asString();

	// todo 유지보수 에러!!!!!!!!!!!!!!!!!!
	// MATERIAL_TYPES
	const char* materialTypes[] = { "BaseColor", "Roughness", "Metalic", "Specular", "Normal" };

	//for (int i = 0; i < static_cast<int>(MATERIAL_TYPES::MATERIAL_END); ++i)
	for (int i = 0; i < _countof(materialTypes); ++i)
	{
		if (root[materialTypes[i]].isNull()) continue;

		// texture의 이름
		std::string name = root[materialTypes[i]].asString();

		int idx = manager->GetTexture(cmdList, name);

		m_TextureIndex[i] = idx;
	}

	if (root["Shader"].isNull())
	{
		DebugPrint("ERROR!! has no shader");
		return false;
	}

	shaderName = root["Shader"].asString();
	m_Shader = nullptr;

	//Renderer::GetInstance().CreateTextureFromDDSFile(cmdList, L"bg.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	return true;
}

void Material::SetDatas(ComPtr<ID3D12GraphicsCommandList> cmdList, UINT paramIdx)
{
	cmdList->SetGraphicsRoot32BitConstants(paramIdx, 16, m_TextureIndex, 0);
}


