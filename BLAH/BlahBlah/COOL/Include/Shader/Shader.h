#pragma once
#include <vector>
// material�� ������ �ִ°� �ڸ��� ���
class Material;

enum class SHADER_TYPE {
	vs_5_1 = 0,
	hs_5_1,
	ds_5_1,
	gs_5_1,
	ps_5_1,
};

class Shader
{
public:
	Shader() = delete;
	Shader(int id, int queue, std::string name);
	virtual ~Shader();

	static int GetGID() { return m_GID++; }

	std::string_view GetName() const { return m_Name; }
	void ChangeRenderQueue(int n) { m_RenderQueue = n; }

	virtual bool InitShader(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, ComPtr<ID3D12RootSignature> rootSignature) = 0;

protected:
	// PSO ������ �ʿ��� �Լ�, ������
	// �ʿ信 ���� �������̵��� �ٲ㼭 �����Ѵ�

	virtual D3D12_SHADER_BYTECODE CreateVertexShader() = 0;		// ������ �ʿ�
	virtual D3D12_SHADER_BYTECODE CreateHullShader();			// ����
	virtual D3D12_SHADER_BYTECODE CreateDomainShader();			// ����
	virtual D3D12_SHADER_BYTECODE CreateGeometryShader();		// ����
	virtual D3D12_SHADER_BYTECODE CreatePixelShader() = 0;		// ������ �ʿ�
	virtual D3D12_STREAM_OUTPUT_DESC GetStreamOutputDesc();		// ����
	virtual D3D12_BLEND_DESC GetBlendDesc();					// Normal
	virtual D3D12_RASTERIZER_DESC GetRasterizerStateDesc();		// Normal
	virtual D3D12_DEPTH_STENCIL_DESC GetDepthStencilState();	// Normal
	virtual D3D12_INPUT_LAYOUT_DESC GetInputLayout() = 0;		// ������ �ʿ�
	virtual DXGI_SAMPLE_DESC GetSampleDesc();					// Normal

	UINT m_SampleMask = UINT_MAX;
	D3D12_PRIMITIVE_TOPOLOGY_TYPE m_PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	D3D_PRIMITIVE_TOPOLOGY m_PrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	UINT m_NumRenderTargets = 1;
	DXGI_FORMAT m_RTFormats[8] = { DXGI_FORMAT_R8G8B8A8_UNORM, };
	DXGI_FORMAT m_DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	D3D12_PIPELINE_STATE_FLAGS m_PsoFlags = D3D12_PIPELINE_STATE_FLAG_NONE;

	ComPtr<ID3DBlob> m_VertxShaderBlob;
	ComPtr<ID3DBlob> m_HullShaderBlob;
	ComPtr<ID3DBlob> m_DomainShaderBlob;
	ComPtr<ID3DBlob> m_GeometryShaderBlob;
	ComPtr<ID3DBlob> m_PixelShaderBlob;

	static D3D12_SHADER_BYTECODE CompileShaderCode(std::string_view fileName, SHADER_TYPE shaderType, ComPtr<ID3DBlob>& shaderBlob);

//private:
	bool CreateShader(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, ComPtr<ID3D12RootSignature> rootSignature);

public:
	virtual void Render(ComPtr<ID3D12GraphicsCommandList> commandList);


private:
	static int m_GID;

	int m_Id = 0;
	int m_RenderQueue = 0;
	std::string m_Name = "Shader";

	ComPtr<ID3D12PipelineState> m_PipelineState;
	std::vector<std::shared_ptr<Material>> m_Materials;
};

