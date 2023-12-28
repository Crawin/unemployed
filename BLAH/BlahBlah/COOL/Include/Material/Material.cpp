#include "framework.h"
#include "Material.h"
#include "Renderer/Renderer.h"

bool Material::LoadTexture(ComPtr<ID3D12GraphicsCommandList> cmdList, const char* fileName)
{
	std::ifstream file(fileName);

	if (file.is_open() == false) 
	{
		DebugPrint(std::format("No Such File Name!!! file name: {}", fileName));
		return false;
	}

	// 파일 안에
	// "albedo" : skybox.dds
	// ""
	
	// 일단 임시로 걍 읽어보자



	//ParseJsonFileToMap();


	Renderer::GetInstance().CreateTextureFromDDSFile(cmdList, L"bg.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);



	return true;
}

void Material::SetDatas(ComPtr<ID3D12GraphicsCommandList> cmdList, UINT paramIdx)
{
	cmdList->SetGraphicsRoot32BitConstants(paramIdx, _countof(m_TextureIndex), m_TextureIndex, 0);
}
