#include "../framework.h"
#include "MeshShader.h"

// 임시
#include "../Mesh/Mesh.h"

bool MeshShader::InitShader(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, ComPtr<ID3D12RootSignature> rootSignature, ComPtr<ID3D12DescriptorHeap> resHeap)
{
	// 임시
	//tempMesh.LoadFile(commandList, "satodiatemptemp.bin");

	return CreateShader(device, commandList, rootSignature);
}

void MeshShader::Render(ComPtr<ID3D12GraphicsCommandList> commandList)
{
	Shader::Render(commandList);

	// 임시
	XMFLOAT4X4 t = Matrix4x4::Identity();
	//tempMesh.Render(commandList, t);
}

D3D12_SHADER_BYTECODE MeshShader::CreateVertexShader()
{
	return CompileShaderCode("TestMeshShader.hlsl", SHADER_TYPE::vs_5_1, m_VertxShaderBlob);
}

D3D12_SHADER_BYTECODE MeshShader::CreatePixelShader()
{
	return CompileShaderCode("TestMeshShader.hlsl", SHADER_TYPE::ps_5_1, m_PixelShaderBlob);
}

D3D12_INPUT_LAYOUT_DESC MeshShader::GetInputLayout()
{
	const UINT elements = 4;
	D3D12_INPUT_ELEMENT_DESC* inputElements = new D3D12_INPUT_ELEMENT_DESC[elements];
	inputElements[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElements[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElements[2] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElements[3] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {};
	inputLayoutDesc.pInputElementDescs = inputElements;
	inputLayoutDesc.NumElements = elements;

	return inputLayoutDesc;
}
