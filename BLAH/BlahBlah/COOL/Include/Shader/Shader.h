#pragma once

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
	friend class ResourceManager;

public:
	Shader();
	Shader(int queue, std::string_view name);
	virtual ~Shader();

	std::string_view GetName() const { return m_Name; }

	void ChangeRenderQueue(int n) { m_RenderQueue = n; }
	void EnableShader() { m_Enable = true; }
	void DisableShader() { m_Enable = false; }

	void AddMaterial(Material* mat) { m_Materials.push_back(mat); }

	//virtual bool InitShader(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, ComPtr<ID3D12RootSignature> rootSignature, ComPtr<ID3D12DescriptorHeap> resHeap = nullptr);

	std::strong_ordering operator<=>(const Shader& other) { return m_RenderQueue <=> other.m_RenderQueue; }

protected:
	// PSO 생성에 필요한 함수, 변수들
	// json에서 받아서 변경한다.

	//virtual D3D12_SHADER_BYTECODE CreateVertexShader(const std::string& fileName);			// 재정의 필요
	//virtual D3D12_SHADER_BYTECODE CreateHullShader(const std::string& fileName);			// 없음
	//virtual D3D12_SHADER_BYTECODE CreateDomainShader(const std::string& fileName);			// 없음
	//virtual D3D12_SHADER_BYTECODE CreateGeometryShader(const std::string& fileName);		// 없음
	//virtual D3D12_SHADER_BYTECODE CreatePixelShader(const std::string& fileName);			// 재정의 필요
	virtual D3D12_STREAM_OUTPUT_DESC GetStreamOutputDesc(int presetID);						// 없음
	virtual D3D12_BLEND_DESC GetBlendDesc(int presetID);;									// Normal
	virtual D3D12_RASTERIZER_DESC GetRasterizerStateDesc(int presetID);						// Normal
	virtual D3D12_DEPTH_STENCIL_DESC GetDepthStencilState(int presetID);					// Normal
	virtual D3D12_INPUT_LAYOUT_DESC GetInputLayout(int presetID);							// 깡통
	virtual DXGI_SAMPLE_DESC GetSampleDesc();												// Normal

	UINT m_SampleMask = UINT_MAX;
	D3D12_PRIMITIVE_TOPOLOGY_TYPE m_PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	D3D_PRIMITIVE_TOPOLOGY m_PrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	UINT m_NumRenderTargets = 1;
	DXGI_FORMAT m_RTFormats[8] = { 
		DXGI_FORMAT_R8G8B8A8_UNORM, 
		DXGI_FORMAT_R8G8B8A8_UNORM, 
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_FORMAT_R32G32B32A32_FLOAT,			// position buff
		DXGI_FORMAT_R32_FLOAT,					// shadow map
		DXGI_FORMAT_R8G8B8A8_UNORM,
	};
	DXGI_FORMAT m_DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	D3D12_PIPELINE_STATE_FLAGS m_PsoFlags = D3D12_PIPELINE_STATE_FLAG_NONE;

	ComPtr<ID3DBlob> m_VertxShaderBlob;
	ComPtr<ID3DBlob> m_HullShaderBlob;
	ComPtr<ID3DBlob> m_DomainShaderBlob;
	ComPtr<ID3DBlob> m_GeometryShaderBlob;
	ComPtr<ID3DBlob> m_PixelShaderBlob;

	static D3D12_SHADER_BYTECODE CompileShaderCode(std::string_view fileName, SHADER_TYPE shaderType, ComPtr<ID3DBlob>& shaderBlob);

//private:
	//bool CreateShader(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, ComPtr<ID3D12RootSignature> rootSignature);

public:
	bool CreateShader(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, ComPtr<ID3D12RootSignature> rootSignature, const std::string& fileName);

public:
	virtual void SetPipelineState(ComPtr<ID3D12GraphicsCommandList> commandList);

protected:
	std::vector<Material*> m_Materials;

private:
	static int m_GID;

	int m_Id = 0;
	int m_RenderQueue = 0;
	std::string m_Name = "Shader";

	bool m_Enable = true;

	ComPtr<ID3D12PipelineState> m_PipelineState;
};

