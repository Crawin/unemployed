#include "../framework.h"
#include "TestShader.h"

TestShader::TestShader(int id, int queue, const std::string& name) :
	Shader(id, queue, name)
{
	m_PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
}

TestShader::~TestShader()
{
}

bool TestShader::InitShader(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, ComPtr<ID3D12RootSignature> rootSignature)
{
	// base path + shader name -> shader property
	// 
	// 

	return CreateShader(device, commandList, rootSignature);
}

void TestShader::Render(ComPtr<ID3D12GraphicsCommandList> commandList)
{
	Shader::Render(commandList);
	commandList->DrawInstanced(3, 1, 0, 0);
}

D3D12_SHADER_BYTECODE TestShader::CreateVertexShader()
{
	return CompileShaderCode("TestShader.hlsl", SHADER_TYPE::vs_5_1, m_VertxShaderBlob);
}

D3D12_SHADER_BYTECODE TestShader::CreatePixelShader()
{
	return CompileShaderCode("TestShader.hlsl", SHADER_TYPE::ps_5_1, m_PixelShaderBlob);
}

D3D12_INPUT_LAYOUT_DESC TestShader::GetInputLayout()
{
	D3D12_INPUT_LAYOUT_DESC temp = {};
	temp.pInputElementDescs = nullptr;
	return temp;
}
