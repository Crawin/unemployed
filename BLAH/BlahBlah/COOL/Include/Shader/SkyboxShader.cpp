#include "../framework.h"
#include "SkyboxShader.h"

SkyboxShader::SkyboxShader(int id, int queue, std::string_view shaderName, std::string_view skyboxName) :
	Shader(id, queue, shaderName)
{
}

SkyboxShader::~SkyboxShader()
{
}

bool SkyboxShader::InitShader(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, ComPtr<ID3D12RootSignature> rootSignature, ComPtr<ID3D12DescriptorHeap> resHeap)
{


	return CreateShader(device, commandList, rootSignature);
}

void SkyboxShader::Render(ComPtr<ID3D12GraphicsCommandList> commandList)
{

}
