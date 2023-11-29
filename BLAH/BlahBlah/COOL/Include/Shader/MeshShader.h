#pragma once
#include "Shader.h"

class Mesh;

class MeshShader :
    public Shader
{
public:
	MeshShader(int queue, std::string_view name) : Shader(queue, name) {}

	virtual bool InitShader(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, ComPtr<ID3D12RootSignature> rootSignature, ComPtr<ID3D12DescriptorHeap> resHeap = nullptr);
	virtual void Render(ComPtr<ID3D12GraphicsCommandList> commandList);

private:
	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();
	virtual D3D12_INPUT_LAYOUT_DESC GetInputLayout();

	// test

	Mesh tempMesh;
};

