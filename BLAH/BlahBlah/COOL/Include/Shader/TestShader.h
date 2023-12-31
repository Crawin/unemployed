#pragma once
#include "Shader.h"

class TestShader : public Shader
{
public:
	TestShader(int queue, std::string_view name);
	virtual ~TestShader();

	virtual bool InitShader(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, ComPtr<ID3D12RootSignature> rootSignature, ComPtr<ID3D12DescriptorHeap> resHeap = nullptr);

	virtual void Render(ComPtr<ID3D12GraphicsCommandList> commandList);

private:
	virtual D3D12_SHADER_BYTECODE CreateVertexShader();		// 재정의 필요
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();		// 재정의 필요
	virtual D3D12_INPUT_LAYOUT_DESC GetInputLayout();		// 재정의 필요
};

