#include "../framework.h"
#include "Renderer.h"
#include "COOLResource.h"
//#include "../Shader/Shader.h"

#define TEST_SHADER

#ifdef TEST_SHADER
#include "../Shader/TestShader.h"
#endif

Renderer::Renderer()
{

}

Renderer::~Renderer()
{
	// ���� ��ü���� �Ʒ� �Լ� ȣ������� ����ִ� �����̸� �Ҹ��ڸ� ������ �����鼭 �Ҹ�ȴ�.
	// Ȯ���ϰ� üũ�ϱ� ���ؼ��� �����ؾ��ұ�?
	// Renderer ���� Application���� ����

/*
	// �ױ� ���� ����ִ� �ֵ� Ȯ���ϰ� ����
#if defined(_DEBUG)
	IDXGIDebug1* debug = NULL;
	DXGIGetDebugInterface1(0, __uuidof(IDXGIDebug1), (void**)&debug);
	HRESULT hResult = debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
	debug->Release();
#endif
*/
}

bool Renderer::CreateDevice()
{
	UINT DXGIFactoryFlags = 0;
#ifdef _DEBUG
	ComPtr<ID3D12Debug> debugController = nullptr;

	D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
	CHECK_CREATE_FAILED(debugController, "debugController ���� ����\n");

	debugController->EnableDebugLayer();
	DXGIFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	// ���丮���� �����Ѵ�.
	CreateDXGIFactory2(DXGIFactoryFlags, IID_PPV_ARGS(m_Factory.GetAddressOf()));
	CHECK_CREATE_FAILED(m_Factory, " m_Factory ���� ����\n");

	// �⺻ ����ͷ� �����غ���
	// ������ �𸣰ڴµ� �� ��ǻ��(������)���� ����̽� ������ ���ܰ� �ߴµ� ����? ���ܸ� �߰� ������
	// ���� �߻�(0x00007FFD140B2BDC, COOL.exe): Microsoft C++ ����: Poco::NotFoundException
	D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(m_Device.GetAddressOf()));

	if (!m_Device) {
		// �����ߴٸ� ���丮���� ����͸� ã���ش�.
		ComPtr<IDXGIAdapter1> pAdapter = nullptr;
		for (UINT i = 0; DXGI_ERROR_NOT_FOUND != m_Factory->EnumAdapters1(i, &pAdapter); i++) {
			DXGI_ADAPTER_DESC1 adapterDesc;
			pAdapter->GetDesc1(&adapterDesc);
			if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
		}

		D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(m_Device.GetAddressOf()));
	}

	CHECK_CREATE_FAILED(m_Device, " m_Device ���� ����\n");

	// �� �� �������� ũ�⸦ ���س���
	m_CbvSrvDescIncrSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_RtvDescIncrSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_DsvDescIncrSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	// MSAA ���� ������ üũ�Ѵ�
	// ��ǻ� dx12�� ������ �����ߴٸ� ũ�� �ʿ�� ���ٰ� ��
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msLevel;
	msLevel.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msLevel.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	msLevel.NumQualityLevels = 0;
	msLevel.SampleCount = 4;

	m_Device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msLevel, sizeof(msLevel));
	m_MsaaQualityLevels = msLevel.NumQualityLevels;
	m_MsaaEnable = (m_MsaaQualityLevels > 1) ? true : false;

	return true;
}

bool Renderer::CreateFence()
{
	m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_Fence.GetAddressOf()));
	CHECK_CREATE_FAILED(m_Fence, " m_Fence ���� ����\n");

	return true;
}

bool Renderer::CreateCommandQueueAndList()
{
	// CommandQueue�� �־����
	// CommandQueue�� CommandList�� ��Ƽ� ����
	// CommandList���ٰ� ������ ��µ� �̶� ������ �Ҵ��� �� CommandAllocator�� �ʿ���
	// Allocator�� �������� CommandList�� open�� ���·� �� �� ����, �� 1:1 ��Ī

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};					// �ʱ�ȭ�� ������
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;			// ���̷�Ʈ <- �������� ���� ������
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;			//
	
	m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_CommandQueue.GetAddressOf()));
	CHECK_CREATE_FAILED(m_CommandQueue, "m_CommandQueue ���� ����");

	CHECK_CREATE_FAILED(CreateCommandAllocatorAndList(m_MainCommandIdx), std::format("idx: {} Ŀ�ǵ帮��Ʈ ���� ����!", m_MainCommandIdx));

	m_MainCommandAllocator = m_CommandAllocators[CMDID::MAIN];
	m_MainCommandList = m_GraphicsCommandLists[CMDID::MAIN];

	return true;
}

bool Renderer::CreateSwapChain()
{
	// ����۶� ���ñ�� ��¶�� ���ִ°�
	m_SwapChain.Reset();

	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferDesc.Width = m_ScreenSize.cx;
	swapChainDesc.BufferDesc.Height = m_ScreenSize.cy;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.SampleDesc.Count = (m_MsaaEnable) ? 4 : 1;
	swapChainDesc.SampleDesc.Quality = (m_MsaaEnable) ? (m_MsaaQualityLevels - 1) : 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = m_NumSwapChainBuffers;
	swapChainDesc.OutputWindow = m_hWnd;
	swapChainDesc.Windowed = m_Windowed;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	m_Factory->CreateSwapChain(m_CommandQueue.Get(), &swapChainDesc, (IDXGISwapChain**)m_SwapChain.GetAddressOf());
	CHECK_CREATE_FAILED(m_SwapChain, "����ü�� ���� ����!");

	return true;
}

bool Renderer::CreateRTVAndDSVDescrHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC descriptorDesc = {};
	descriptorDesc.NumDescriptors = m_NumSwapChainBuffers;
	descriptorDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	descriptorDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	descriptorDesc.NodeMask = 0;

	m_Device->CreateDescriptorHeap(&descriptorDesc, IID_PPV_ARGS(m_RtvHeap.GetAddressOf()));
	CHECK_CREATE_FAILED(m_RtvHeap, "m_RtvHeap ���� ����!");

	descriptorDesc.NumDescriptors = 1;
	descriptorDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

	m_Device->CreateDescriptorHeap(&descriptorDesc, IID_PPV_ARGS(m_DsvHeap.GetAddressOf()));
	CHECK_CREATE_FAILED(m_RtvHeap, "m_RtvHeap ���� ����!");

	return true;
}

bool Renderer::CreateRTV()
{
	// �並 ���������� ���ҽ��� �����ؾ��Ѵ�
	// �׷��� �̰� ���� ���� �ʿ䰡 ����
	// ����ü���� ���� ������� ������ �̹� ������
	// 

	D3D12_CPU_DESCRIPTOR_HANDLE rtvDescriptorHandle = m_RtvHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < m_NumSwapChainBuffers; ++i) {
		ID3D12Resource* tempResource = nullptr;

		m_SwapChain->GetBuffer(
			i,
			IID_PPV_ARGS(&tempResource));
		CHECK_CREATE_FAILED(tempResource, "m_SwapChain->GetBuffer Failed!!");
		
		m_RenderTargetBuffer[i] = COOLResourcePtr(new COOLResource(tempResource, D3D12_RESOURCE_STATE_PRESENT));

		m_Device->CreateRenderTargetView(m_RenderTargetBuffer[i]->GetResource(), nullptr, rtvDescriptorHandle);
		rtvDescriptorHandle.ptr += m_RtvDescIncrSize;
	}

	return true;
}

bool Renderer::CreateDSV()
{
	// Depth/Stencil�� �츮�� ���ҽ��� ���� ������־����
	// ���� ���ҽ��� ����Ʈ������ �����
	// ���� �־� �������ֱ� ���� ���ε����� ������ �ʿ�
	// �ٵ� ��� �ʿ����
	m_DepthStencilBuffer = CreateEmpty2DResource(D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_DEPTH_WRITE, m_ScreenSize);

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CPU_DESCRIPTOR_HANDLE dsvCPUHandle = m_DsvHeap->GetCPUDescriptorHandleForHeapStart();
	m_Device->CreateDepthStencilView(m_DepthStencilBuffer->GetResource(), &dsvDesc, dsvCPUHandle);
	return true;
}

bool Renderer::CreateRootSignature()
{
	// t0 ��������, �ٸ� �����̽�
	// 
	const int resourceType = 3;
	D3D12_DESCRIPTOR_RANGE descRange[resourceType] = {};
	for (int i = 0; i < resourceType; ++i) {
		descRange[i].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		descRange[i].NumDescriptors = UINT_MAX;
		descRange[i].BaseShaderRegister = 0;
		descRange[i].RegisterSpace = i;
		descRange[i].OffsetInDescriptorsFromTableStart = 0;
		//descRange[i].Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE;		// D3D12_DESCRIPTOR_RANGE1
		descRange[i].OffsetInDescriptorsFromTableStart = 0;								// �� �� ����
	}

	const int numParams = ROOT_SIGNATURE_IDX_MAX;
	D3D12_ROOT_PARAMETER rootParams[numParams] = {};
	// idx 0: descriptor table ��� ���̴� ���ҽ��� ��ũ�������̺��� �������, ���̴����� ����� �ε����� �Ѱ���
	rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParams[0].DescriptorTable.NumDescriptorRanges = 1;
	rootParams[0].DescriptorTable.pDescriptorRanges = descRange;
	rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	//
	// sampler desc
	const int samplers = 2;
	D3D12_STATIC_SAMPLER_DESC samperDesc[samplers] = {};
	samperDesc[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samperDesc[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samperDesc[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samperDesc[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samperDesc[0].MipLODBias = 0;
	samperDesc[0].MaxAnisotropy = 1;
	samperDesc[0].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	samperDesc[0].MinLOD = 0;
	samperDesc[0].MaxLOD = D3D12_FLOAT32_MAX;
	samperDesc[0].ShaderRegister = 0;
	samperDesc[0].RegisterSpace = 0;
	samperDesc[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	samperDesc[1].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samperDesc[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samperDesc[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samperDesc[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samperDesc[1].MipLODBias = 0;
	samperDesc[1].MaxAnisotropy = 1;
	samperDesc[1].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	samperDesc[1].MinLOD = 0;
	samperDesc[1].MaxLOD = D3D12_FLOAT32_MAX;
	samperDesc[1].ShaderRegister = 1;
	samperDesc[1].RegisterSpace = 0;
	samperDesc[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	//
	// create root signature
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlag =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT; /* |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;     */

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.NumParameters = _countof(rootParams);
	rootSignatureDesc.pParameters = rootParams;
	rootSignatureDesc.NumStaticSamplers = _countof(samperDesc);
	rootSignatureDesc.pStaticSamplers = samperDesc;
	rootSignatureDesc.Flags = rootSignatureFlag;

	// ��Ʈ�ñ״�ó ����
	ComPtr<ID3DBlob> signatureBlob = nullptr;;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, signatureBlob.GetAddressOf(), errorBlob.GetAddressOf());
	
	if (errorBlob) {
		char* pErrorString = NULL;
		pErrorString = (char*)errorBlob->GetBufferPointer();
		TCHAR pstrDebug[256] = { 0 };

		mbstowcs(pstrDebug, pErrorString, 256);
		//OutputDebugString(pstrDebug);

		DebugPrint(pErrorString);
	}

	m_Device->CreateRootSignature(
		0,
		signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(m_RootSignature.GetAddressOf()));

	CHECK_CREATE_FAILED(m_RootSignature, "CreateRootSignature Failed!!");
	return true;

	return true;
}

bool Renderer::CreateTestRootSignature()
{
	D3D12_ROOT_PARAMETER rootParam = {};

	D3D12_DESCRIPTOR_RANGE descriptorRanges[1];


	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.NumParameters = 0;
	rootSignatureDesc.NumStaticSamplers = 0;
	rootSignatureDesc.pParameters = nullptr;
	rootSignatureDesc.pStaticSamplers = nullptr;
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// ��Ʈ�ñ״�ó ����
	ComPtr<ID3DBlob> signatureBlob = nullptr;;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, signatureBlob.GetAddressOf(), errorBlob.GetAddressOf());
	m_Device->CreateRootSignature(
		0,
		signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(m_RootSignature.GetAddressOf()));

	CHECK_CREATE_FAILED(m_RootSignature, "CreateRootSignature Failed!!");
	return true;
}

bool Renderer::CreateResourceDescriptorHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC descriptorDesc = {};
	descriptorDesc.NumDescriptors = m_NumSwapChainBuffers;
	descriptorDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descriptorDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	descriptorDesc.NodeMask = 0;
	
	m_Device->CreateDescriptorHeap(&descriptorDesc, IID_PPV_ARGS(m_ResourceHeap.GetAddressOf()));
	CHECK_CREATE_FAILED(m_ResourceHeap, "m_ResourceHeap ���� ����!");

	return true;
}

bool Renderer::LoadShaders()
{
	// ��� ���̴��� �ε��Ѵ�
	// �� �͵��� ���Ϸ� �����صд�? 
	// ���� �ٸ� ���̴� ����, �����彺����Ʈ, �����Ͷ�����������Ʈ, ds ������Ʈ ��� ��� �ٸ��´�
	// ���̴��� �̸� �ε� �صΰ� �ű⿡ �´� ������Ʈ���� �ִٸ� �����ϴ� �������� ����?
	// ���α׷� ���� �ε�ÿ��� ���̴��� �����Ѵ� ������ ����


#ifdef TEST_SHADER
	auto shader = std::make_shared<TestShader>(Shader::GetGID(), 1, "TestShader");
	m_Shaders.push_back(shader);
	
	for (auto& sha : m_Shaders) {
		// todo shader���ٰ� ���ҽ��� �Ѱ��༭ �� �˾Ƽ� �߰��ϰ� �Ѵ�
		CHECK_CREATE_FAILED(sha->InitShader(m_Device, m_MainCommandList, m_RootSignature), sha->GetName());
	}
#endif

	// ����ť�� �����Ѵ�
	std::sort(m_Shaders.begin(), m_Shaders.end());

	return true;
	return false;
}

bool Renderer::Init(const SIZE& wndSize, HWND hWnd)
{
	// dx12�� �ʱ�ȭ ����
	// 
	// ����̽� ����
	// �潺 ����, ������ ũ�� ���
	// ��Ƽ���ø� ������ Ȯ��
	// Ŀ�ǵ帮��Ʈ, ť ����
	// ����ü�� ����
	// rtv dsv �� ����
	// rtv, dsv ����

	m_ScreenSize = wndSize;
	m_hWnd = hWnd;

	CHECK_CREATE_FAILED(CreateDevice(), "CreateDevice Failed!!");
	CHECK_CREATE_FAILED(CreateFence(), "CreateFence Failed!!");
	CHECK_CREATE_FAILED(CreateCommandQueueAndList(), "CreateCommandQueueAndList Failed!!");
	CHECK_CREATE_FAILED(CreateSwapChain(), "CreateSwapChain Failed!!");
	CHECK_CREATE_FAILED(CreateRTVAndDSVDescrHeap(), "CreateRTVAndDescrHeap Failed!!");
	CHECK_CREATE_FAILED(CreateRTV(), "CreateRTV Failed!!");
	CHECK_CREATE_FAILED(CreateDSV(), "CreateDSV Failed!!");

	// scene??????
	//CHECK_CREATE_FAILED(CreateTestRootSignature(), "CreateRootSignature Failed!!");
	CHECK_CREATE_FAILED(CreateRootSignature(), "CreateRootSignature Failed!!");
	CHECK_CREATE_FAILED(CreateResourceDescriptorHeap(), "CreateResourceDescriptorHeap");
	CHECK_CREATE_FAILED(LoadShaders(), "LoadShaders Failed!!");

	return true;
}

bool Renderer::CreateCommandAllocatorAndList(size_t& outIndex)
{
	ComPtr<ID3D12CommandAllocator>& allocator = m_CommandAllocators.emplace_back();
	ComPtr<ID3D12GraphicsCommandList>& commandlist = m_GraphicsCommandLists.emplace_back();

	outIndex = m_CommandAllocators.size() - 1;

	m_Device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(allocator.GetAddressOf()));
	CHECK_CREATE_FAILED(allocator, "m_CommandAllocator ���� ����");

	m_Device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		allocator.Get(),
		nullptr,
		IID_PPV_ARGS(commandlist.GetAddressOf()));
	CHECK_CREATE_FAILED(commandlist, "m_GraphicsCommandList ���� ����");

	// Reset�� ȣ�� �ϰ� ������ ��ƾ� �ϴµ�, ���� ���¿��� Reset�� ȣ�� ������
	commandlist->Close();

	return true;
}

COOLResourcePtr Renderer::CreateEmpty2DResource(D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_STATES resourceState, const SIZE& size)
{
	ID3D12Resource* temp;

	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Alignment = 0;
	resDesc.Width = size.cx;
	resDesc.Height = size.cy;
	resDesc.DepthOrArraySize = 1;										// tex �迭 ũ�� Ȥ�� tex3D��� ����
	resDesc.MipLevels = 1;
	resDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	resDesc.SampleDesc.Count = (m_MsaaEnable) ? 4 : 1;
	resDesc.SampleDesc.Quality = (m_MsaaEnable) ? (m_MsaaQualityLevels - 1) : 0;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES heapProperty = {};
	heapProperty.Type = heapType;
	heapProperty.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperty.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperty.CreationNodeMask = 1;
	heapProperty.VisibleNodeMask = 1;

	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.DepthStencil.Stencil = 0;

	m_Device->CreateCommittedResource(
		&heapProperty,
		D3D12_HEAP_FLAG_NONE,				// heapFlags
		&resDesc,
		resourceState,						// initialResState
		&clearValue,
		IID_PPV_ARGS(&temp));

	return COOLResourcePtr(new COOLResource(temp, resourceState, heapType));
}

COOLResourcePtr Renderer::CreateEmptyBufferResource(D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_STATES resourceState, UINT bytes)
{
	ID3D12Resource* temp;

	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Alignment = 0;
	resDesc.Width = bytes;
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;										// tex �迭 ũ�� Ȥ�� tex3D��� ����
	resDesc.MipLevels = 1;
	resDesc.Format = DXGI_FORMAT_UNKNOWN;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	D3D12_HEAP_PROPERTIES heapProperty = {};
	heapProperty.Type = heapType;
	heapProperty.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperty.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperty.CreationNodeMask = 1;
	heapProperty.VisibleNodeMask = 1;

	m_Device->CreateCommittedResource(
		&heapProperty,
		D3D12_HEAP_FLAG_NONE,				// heapFlags
		&resDesc,
		resourceState,						// initialResState
		nullptr,
		IID_PPV_ARGS(&temp));

	return COOLResourcePtr(new COOLResource(temp, resourceState, heapType, "buffer"));
}

void Renderer::CopyResource(ComPtr<ID3D12GraphicsCommandList> commandList, COOLResourcePtr src, COOLResourcePtr dest)
{
	// if state is not copy dest
	auto curDestState = src->GetState();
	if (curDestState != D3D12_RESOURCE_STATE_COPY_DEST) 
		dest->TransToState(commandList, D3D12_RESOURCE_STATE_COPY_DEST);

	// do copy
	//D3D12_SUBRESOURCE_DATA 

	// return to state
	dest->TransToState(commandList, curDestState);
}

void Renderer::SetViewportScissorRect()
{
	D3D12_VIEWPORT vp;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.Width = static_cast<float>(m_ScreenSize.cx);
	vp.Height = static_cast<float>(m_ScreenSize.cy);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;

	RECT scRect = { 0, 0, m_ScreenSize.cx, m_ScreenSize.cy };

	SetViewportScissorRect(1, vp, scRect);
}

void Renderer::SetViewportScissorRect(UINT numOfViewPort, const D3D12_VIEWPORT& viewport, const RECT& scissorRect)
{
	m_MainCommandList->RSSetViewports(numOfViewPort, &viewport);
	m_MainCommandList->RSSetScissorRects(numOfViewPort, &scissorRect);
}

void Renderer::Render()
{
	// pre render
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// �Ҵ���, ����Ʈ ����
	for (int i = 0; i < m_CommandAllocators.size(); ++i) {
		m_CommandAllocators[i]->Reset();
		m_GraphicsCommandLists[i]->Reset(m_CommandAllocators[i].Get(), nullptr);
	}

	// root signature set
	m_MainCommandList->SetGraphicsRootSignature(m_RootSignature.Get());

	// ����Ʈ, ������Ʈ ����
	SetViewportScissorRect();

	// ����Ÿ���� ������Ʈ�� ��ٸ���
	
	m_RenderTargetBuffer[m_CurSwapChainIndex]->TransToState(m_MainCommandList, D3D12_RESOURCE_STATE_RENDER_TARGET);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvCpuDesHandle = m_RtvHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_CPU_DESCRIPTOR_HANDLE dsvCpuDesHandle = m_DsvHeap->GetCPUDescriptorHandleForHeapStart();
	rtvCpuDesHandle.ptr += m_CurSwapChainIndex * m_RtvDescIncrSize;

	// rtv�� dsv�� �ʱ�ȭ�Ѵ�
	float clearColor[4] = { 0.3f, 0.9f, 0.3f, 1.0f };
	m_MainCommandList->ClearRenderTargetView(rtvCpuDesHandle, clearColor, 0, nullptr);
	m_MainCommandList->ClearDepthStencilView(dsvCpuDesHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// ����Ÿ���� om�� ����, MRT�� ����ϰ� �ȴٸ� ���� 1�� �ٲ۴�
	m_MainCommandList->OMSetRenderTargets(1, &rtvCpuDesHandle, true, &dsvCpuDesHandle);

	// real render
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// ���̴����� CommandList�� �Ѱ��ָ� ����
	for (auto shader : m_Shaders) {
		// ������ �ϰ��ʹٸ�
		shader->Render(m_MainCommandList);
	}

	// render end
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	m_RenderTargetBuffer[m_CurSwapChainIndex]->TransToState(m_MainCommandList, D3D12_RESOURCE_STATE_PRESENT);

	for (auto& command : m_GraphicsCommandLists)
		command->Close();

	// CommandList�� ��Ƽ� CommandQueue�� �ְ� Execute
	ID3D12CommandList* p[] = { m_MainCommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(p), p);
	
	// GPU �Ϸ���� ���
	UINT64 fenceValue = ++m_FenceValues[m_CurSwapChainIndex];
	HRESULT hResult = m_CommandQueue->Signal(m_Fence.Get(), fenceValue);
	if (m_Fence->GetCompletedValue() < fenceValue) {
		hResult = m_Fence->SetEventOnCompletion(fenceValue, m_FenceEvent);
		WaitForSingleObject(m_FenceEvent, INFINITE);
	}

	// ����ü���� present
	m_SwapChain->Present(0, 0);

	m_CurSwapChainIndex = m_SwapChain->GetCurrentBackBufferIndex();
	fenceValue = ++m_FenceValues[m_CurSwapChainIndex];
	hResult = m_CommandQueue->Signal(m_Fence.Get(), fenceValue);
	if (m_Fence->GetCompletedValue() < fenceValue) {
		hResult = m_Fence->SetEventOnCompletion(fenceValue, m_FenceEvent);
		WaitForSingleObject(m_FenceEvent, INFINITE);
	}

}
