#include "framework.h"
#include "Shader.h"
#include "Material/Material.h"
#include <json/json.h>

#define FILE_PATH "Include/Shader/HLSL/"

int Shader::m_GID = 0;

Shader::Shader()
{
	m_Id = m_GID++;
}

Shader::Shader(int queue, std::string_view name)
{
	m_Id = m_GID++;
}

Shader::~Shader()
{
	//for (auto& mats : m_Materials)
	//	delete mats;
}
/*
D3D12_SHADER_BYTECODE Shader::CreateVertexShader(const std::string& fileName)
{
	return D3D12_SHADER_BYTECODE();
}

D3D12_SHADER_BYTECODE Shader::CreateHullShader(const std::string& fileName)
{
	D3D12_SHADER_BYTECODE shader = {};
	return shader;
}

D3D12_SHADER_BYTECODE Shader::CreateDomainShader(const std::string& fileName)
{
	D3D12_SHADER_BYTECODE shader = {};
	return shader;
}

D3D12_SHADER_BYTECODE Shader::CreateGeometryShader(const std::string& fileName)
{
	D3D12_SHADER_BYTECODE shader = {};
	return shader;
}

D3D12_SHADER_BYTECODE Shader::CreatePixelShader(const std::string& fileName)
{
	return D3D12_SHADER_BYTECODE();
}
*/
D3D12_STREAM_OUTPUT_DESC Shader::GetStreamOutputDesc(int presetID)
{
	D3D12_STREAM_OUTPUT_DESC desc = {};

	switch (presetID) {
	case 0:
	default:
		// no use
		return desc;
	}

	return desc;
}

D3D12_BLEND_DESC Shader::GetBlendDesc(int presetID)
{
	D3D12_BLEND_DESC desc = {};
	desc.AlphaToCoverageEnable = false;
	desc.IndependentBlendEnable = false;		// 끄면 mrt에서도 rt0을 기준으로 한다
	desc.RenderTarget[0].BlendEnable = false;
	desc.RenderTarget[0].LogicOpEnable = false;
	desc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	desc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
	desc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	desc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	desc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	desc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	desc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	desc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	switch (presetID) {
	case 0:
	default:
		// default : no alpha blend
		return desc;
	}

	return desc;
}

D3D12_RASTERIZER_DESC Shader::GetRasterizerStateDesc(int presetID)
{
	D3D12_RASTERIZER_DESC desc = {};
	desc.FillMode = D3D12_FILL_MODE_SOLID;
	desc.CullMode = D3D12_CULL_MODE_BACK;
	desc.FrontCounterClockwise = false;
	desc.DepthBias = 0;
	desc.DepthBiasClamp = 0.0f;
	desc.SlopeScaledDepthBias = 0.0f;
	desc.DepthClipEnable = true;
	desc.MultisampleEnable = false;
	desc.AntialiasedLineEnable = false;
	desc.ForcedSampleCount = 0;
	desc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	switch (presetID) {
	case 0:
	default:
		// default
		// fill = solid / cull = back
		return desc;
	}

	return desc;
}

D3D12_DEPTH_STENCIL_DESC Shader::GetDepthStencilState(int presetID)
{
	D3D12_DEPTH_STENCIL_DESC desc = {};
	desc.DepthEnable = TRUE;
	desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	desc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	desc.StencilEnable = FALSE;
	desc.StencilReadMask = 0x00;
	desc.StencilWriteMask = 0x00;
	desc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	desc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	desc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	desc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	desc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	desc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	desc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	desc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	switch (presetID) {
	case 0:
	default:
		// default
		// depth stencil = on
		return desc;

	//case 1:
		// depth stencil off;
	}

	return desc;
}

D3D12_INPUT_LAYOUT_DESC Shader::GetInputLayout(int presetID)
{
	D3D12_INPUT_LAYOUT_DESC temp = {};
	temp.pInputElementDescs = nullptr;

	switch (presetID) {
	case 0:
	default:
		// no Input Layout
		return temp;

	case 1:
	{
		// Vertex, default type
		const UINT elements = 4;
		D3D12_INPUT_ELEMENT_DESC* inputElements = new D3D12_INPUT_ELEMENT_DESC[elements];
		inputElements[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		inputElements[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		inputElements[2] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		inputElements[3] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

		temp.pInputElementDescs = inputElements;
		temp.NumElements = elements;
		return temp;

		// case 2: SkinedMeshVertex
	}
	}
	return temp;
}

DXGI_SAMPLE_DESC Shader::GetSampleDesc()
{
	DXGI_SAMPLE_DESC desc = {};
	desc.Count = 1;
	return desc;
}

D3D12_SHADER_BYTECODE Shader::CompileShaderCode(std::string_view fileName, SHADER_TYPE shaderType, ComPtr<ID3DBlob>& shaderBlob)
{
	// 없으면 뛰어넘음
	if (fileName == "") {
		//DebugPrint("No Shader! SKIP!!");
		return D3D12_SHADER_BYTECODE();
	}

	UINT nCompileFlags = 0;
#if defined(_DEBUG)
	nCompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES;
#endif

	std::string file(FILE_PATH);
	file += fileName;
	std::wstring lpsStr(file.begin(), file.end());

	// shader type and name
	std::string shaderName;
	switch (shaderType) {
	case SHADER_TYPE::vs_5_1: shaderName = "vs"; break;
	case SHADER_TYPE::hs_5_1: shaderName = "hs"; break;
	case SHADER_TYPE::ds_5_1: shaderName = "ds"; break;
	case SHADER_TYPE::gs_5_1: shaderName = "gs"; break;
	case SHADER_TYPE::ps_5_1: shaderName = "ps"; break;
	}
	
	std::string shaderTypeToStr = shaderName + "_5_1";
	
	ComPtr<ID3DBlob> errorBlob;
	D3DCompileFromFile(
		lpsStr.c_str(),
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		shaderName.c_str(),
		shaderTypeToStr.c_str(),
		nCompileFlags, 
		0, 
		&shaderBlob,
		&errorBlob);

	char* pErrorString = NULL;
	if (errorBlob) {
		pErrorString = (char*)errorBlob->GetBufferPointer();
		TCHAR pstrDebug[256] = { 0 };

		mbstowcs(pstrDebug, pErrorString, 256);
		//OutputDebugString(pstrDebug);

		// 임시로 지운다.
		DebugPrint(pErrorString);
	}

	D3D12_SHADER_BYTECODE result;
	result.BytecodeLength = shaderBlob->GetBufferSize();
	result.pShaderBytecode = shaderBlob->GetBufferPointer();
	return result;
}
/*
bool Shader::CreateShader(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, ComPtr<ID3D12RootSignature> rootSignature)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.pRootSignature = rootSignature.Get();
	psoDesc.VS = CreateVertexShader();															// 없음
	psoDesc.HS = CreateHullShader();															// 기본값(없음)
	psoDesc.DS = CreateDomainShader();															// 기본값(없음)
	psoDesc.GS = CreateGeometryShader();														// 기본값(없음)
	psoDesc.PS = CreatePixelShader();															// 없음
	psoDesc.StreamOutput = GetStreamOutputDesc();												// 기본값(없음)
	psoDesc.BlendState = GetBlendDesc();														// 기본값
	psoDesc.SampleMask = m_SampleMask;															// UINT_MAX
	psoDesc.RasterizerState = GetRasterizerStateDesc();											// 기본값
	psoDesc.DepthStencilState = GetDepthStencilState();											// 기본값
	psoDesc.InputLayout = GetInputLayout();														// 없음
	psoDesc.PrimitiveTopologyType = m_PrimitiveTopologyType;									// D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE
	psoDesc.NumRenderTargets = m_NumRenderTargets;												// 1
	for (UINT i = 0; i < m_NumRenderTargets; ++i) psoDesc.RTVFormats[i] = m_RTFormats[i];		// DXGI_FORMAT_R8G8B8A8_UNORM
	psoDesc.DSVFormat = m_DSVFormat;															// DXGI_FORMAT_R8G8B8A8_UNORM
	psoDesc.SampleDesc = GetSampleDesc();														// count = 1
	psoDesc.Flags = m_PsoFlags;																	// D3D12_PIPELINE_STATE_FLAG_NONE

	device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_PipelineState.GetAddressOf()));
	CHECK_CREATE_FAILED(m_PipelineState, "m_PipelineState Create Failed!!");

	// comptr이라 release 필요 없이 널포인터로 만들어주면 된다 아마도? 체크 함 해봐야함
	if (m_VertxShaderBlob) m_VertxShaderBlob = nullptr;
	if (m_HullShaderBlob) m_HullShaderBlob = nullptr;
	if (m_DomainShaderBlob) m_DomainShaderBlob = nullptr;
	if (m_GeometryShaderBlob) m_GeometryShaderBlob = nullptr;
	if (m_PixelShaderBlob) m_PixelShaderBlob = nullptr;

	return true;
}
*/
#define COMPILE_SHADER

bool Shader::CreateShader(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, ComPtr<ID3D12RootSignature> rootSignature, const std::string& fileName)
{
	// load file
	std::ifstream file(fileName);

	if (file.is_open() == false)
	{
		DebugPrint(std::format("No Such File Name!!! file name: {}", fileName));
		return false;
	}

	// load file to json
	Json::Value root;
	Json::Reader reader;

	if (false == reader.parse(file, root)) {
		DebugPrint(std::format("Failed to open shader json file!! fileName: {}", fileName));
		DebugPrint(reader.getFormatedErrorMessages());
		return false;
	};

	m_Name = root["name"].asString();

	// num of rendertargets
	m_NumRenderTargets = root["NumRenderTargets"].asInt();
	m_PrimitiveTopologyType = static_cast<D3D12_PRIMITIVE_TOPOLOGY_TYPE>(root["PrimitiveTopologyType"].asInt());
	m_PrimitiveTopology = static_cast<D3D12_PRIMITIVE_TOPOLOGY>(root["PrimitiveTopology"].asInt());

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.pRootSignature = rootSignature.Get();
#ifdef COMPILE_SHADER
	psoDesc.VS = CompileShaderCode(root["VS"].asString(), SHADER_TYPE::vs_5_1, m_VertxShaderBlob);
	psoDesc.HS = CompileShaderCode(root["HS"].asString(), SHADER_TYPE::hs_5_1, m_HullShaderBlob);
	psoDesc.DS = CompileShaderCode(root["DS"].asString(), SHADER_TYPE::ds_5_1, m_DomainShaderBlob);
	psoDesc.GS = CompileShaderCode(root["GS"].asString(), SHADER_TYPE::gs_5_1, m_GeometryShaderBlob);
	psoDesc.PS = CompileShaderCode(root["PS"].asString(), SHADER_TYPE::ps_5_1, m_PixelShaderBlob);
#else
	// todo 
	// 컴파일 된 쉐이더로 해야한다.
	psoDesc.VS = CompileShaderCode(root["VS"].asString(), SHADER_TYPE::vs_5_1, m_VertxShaderBlob);
	psoDesc.HS = CompileShaderCode(root["HS"].asString(), SHADER_TYPE::hs_5_1, m_HullShaderBlob);
	psoDesc.DS = CompileShaderCode(root["DS"].asString(), SHADER_TYPE::ds_5_1, m_DomainShaderBlob);
	psoDesc.GS = CompileShaderCode(root["GS"].asString(), SHADER_TYPE::gs_5_1, m_GeometryShaderBlob);
	psoDesc.PS = CompileShaderCode(root["PS"].asString(), SHADER_TYPE::ps_5_1, m_PixelShaderBlob);
#endif
	psoDesc.SampleMask = m_SampleMask;															// UINT_MAX
	psoDesc.StreamOutput = GetStreamOutputDesc(root["StreamOutput"].asInt());					// 기본값(없음)
	psoDesc.BlendState = GetBlendDesc(root["BlendState"].asInt());								// 기본값
	psoDesc.RasterizerState = GetRasterizerStateDesc(root["RasterizerState"].asInt());			// 기본값
	psoDesc.DepthStencilState = GetDepthStencilState(root["DepthStencilState"].asInt());		// 기본값
	psoDesc.InputLayout = GetInputLayout(root["InputLayout"].asInt());							// 없음
	psoDesc.PrimitiveTopologyType = m_PrimitiveTopologyType;									// D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE
	psoDesc.NumRenderTargets = m_NumRenderTargets;												// 1
	for (UINT i = 0; i < m_NumRenderTargets; ++i) psoDesc.RTVFormats[i] = m_RTFormats[i];		// DXGI_FORMAT_R8G8B8A8_UNORM
	psoDesc.DSVFormat = m_DSVFormat;															// DXGI_FORMAT_R8G8B8A8_UNORM
	psoDesc.SampleDesc = GetSampleDesc();														// count = 1
	psoDesc.Flags = m_PsoFlags;																	// D3D12_PIPELINE_STATE_FLAG_NONE

	device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_PipelineState.GetAddressOf()));
	CHECK_CREATE_FAILED(m_PipelineState, "m_PipelineState Create Failed!!");

	if (m_VertxShaderBlob) m_VertxShaderBlob = nullptr;
	if (m_HullShaderBlob) m_HullShaderBlob = nullptr;
	if (m_DomainShaderBlob) m_DomainShaderBlob = nullptr;
	if (m_GeometryShaderBlob) m_GeometryShaderBlob = nullptr;
	if (m_PixelShaderBlob) m_PixelShaderBlob = nullptr;

	return true;
}

void Shader::Render(ComPtr<ID3D12GraphicsCommandList> commandList)
{
	if (false == m_Enable)
		return;

	// set pso
	commandList->IASetPrimitiveTopology(m_PrimitiveTopology);
	commandList->SetPipelineState(m_PipelineState.Get());

}
