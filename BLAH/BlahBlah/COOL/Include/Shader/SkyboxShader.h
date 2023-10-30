#pragma once
#include "Shader.h"


class SkyboxShader :
    public Shader
{
public:
	SkyboxShader(int queue, std::string_view shaderName);
	virtual ~SkyboxShader();

	virtual bool InitShader(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, ComPtr<ID3D12RootSignature> rootSignature, ComPtr<ID3D12DescriptorHeap> resHeap = nullptr);

	virtual void Render(ComPtr<ID3D12GraphicsCommandList> commandList);

private:
	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();

};

