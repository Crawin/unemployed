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
	// �ʿ��� �ؽ��ĵ� �ε��� �ٵ� ������ ���� �ұ�
	// 4�ôϱ� ���� ����

	Material temp{};
	m_Materials.push_back(temp);

	return CreateShader(device, commandList, rootSignature);
}

void SkyboxShader::Render(ComPtr<ID3D12GraphicsCommandList> commandList)
{
	Shader::Render(commandList);
	commandList->DrawInstanced(6, 1, 0, 0);				// draw rect
}
