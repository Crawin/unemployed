#include "../framework.h"
#include "SkyboxShader.h"
#include "../Material.h"

SkyboxShader::SkyboxShader(int queue, std::string_view shaderName) :
	Shader(queue, shaderName)
{
}

SkyboxShader::~SkyboxShader()
{
}

bool SkyboxShader::InitShader(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, ComPtr<ID3D12RootSignature> rootSignature, ComPtr<ID3D12DescriptorHeap> resHeap)
{
	// 필요한 텍스쳐들 로드함 근데 귀찮다 내일 할까
	// 4시니까 내일 하자

	Material temp{};
	m_Materials.push_back(temp);

	return CreateShader(device, commandList, rootSignature);
}

void SkyboxShader::Render(ComPtr<ID3D12GraphicsCommandList> commandList)
{
	Shader::Render(commandList);
	commandList->DrawInstanced(6, 1, 0, 0);				// draw rect
}

D3D12_SHADER_BYTECODE SkyboxShader::CreateVertexShader()
{
	return CompileShaderCode("TestRootSignature.hlsl", SHADER_TYPE::vs_5_1, m_VertxShaderBlob);
}

D3D12_SHADER_BYTECODE SkyboxShader::CreatePixelShader()
{
	return CompileShaderCode("TestRootSignature.hlsl", SHADER_TYPE::ps_5_1, m_PixelShaderBlob);
}

