#include "../framework.h"
#include "Renderer.h"
#include "COOLResource.h"
//#include "../Shader/Shader.h"

#define TEST_SHADER

#ifdef TEST_SHADER
#include "../Shader/TestShader.h"
#include "../Shader/SkyboxShader.h"
#endif

Renderer::Renderer()
{

}

Renderer::~Renderer()
{
	// 각종 객체들이 아래 함수 호출까지는 살아있는 상태이며 소멸자를 완전히 나가면서 소멸된다.
	// 확실하게 체크하기 위해서는 어케해야할까?
	// Renderer 말고 Application에서 하자

/*
	// 죽기 전에 살아있는 애들 확인하고 간다
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
	CHECK_CREATE_FAILED(debugController, "debugController 생성 실패\n");

	debugController->EnableDebugLayer();
	DXGIFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	// 팩토리님을 생성한다.
	CreateDXGIFactory2(DXGIFactoryFlags, IID_PPV_ARGS(m_Factory.GetAddressOf()));
	CHECK_CREATE_FAILED(m_Factory, " m_Factory 생성 실패\n");

	// 기본 어답터로 생성해본다
	// 왜인진 모르겠는데 내 컴퓨터(손정원)에선 디바이스 생성에 예외가 뜨는데 왜지? 예외만 뜨고 멀쩡함
	// 예외 발생(0x00007FFD140B2BDC, COOL.exe): Microsoft C++ 예외: Poco::NotFoundException
	D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(m_Device.GetAddressOf()));

	if (!m_Device) {
		ComPtr<IDXGIAdapter1> pAdapter = nullptr;
		for (UINT i = 0; DXGI_ERROR_NOT_FOUND != m_Factory->EnumAdapters1(i, &pAdapter); i++) {
			DXGI_ADAPTER_DESC1 adapterDesc;
			pAdapter->GetDesc1(&adapterDesc);
			if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
		}

		D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(m_Device.GetAddressOf()));
	}

	CHECK_CREATE_FAILED(m_Device, " m_Device 생성 실패");

	// 각 뷰 종류들의 크기를 구해놓음
	m_CbvSrvDescIncrSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_RtvDescIncrSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_DsvDescIncrSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	// MSAA 지원 수준을 체크한다
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
	CHECK_CREATE_FAILED(m_Fence, " m_Fence 생성 실패\n");

	return true;
}

bool Renderer::CreateCommandQueueAndList()
{
	// CommandQueue가 있어야함
	// CommandQueue에 CommandList를 담아서 보냄
	// CommandList에다가 명령을 담는데 이때 명령을 할당할 때 CommandAllocator가 필요함
	// Allocator는 여러개의 CommandList를 open한 상태로 쓸 수 없음, 즉 1:1 매칭

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};					// 초기화는 해주자
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;			// 다이렉트 <- 지피유가 직접 실행함
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;			//

	m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_CommandQueue.GetAddressOf()));
	CHECK_CREATE_FAILED(m_CommandQueue, "m_CommandQueue 생성 실패");

	CHECK_CREATE_FAILED(CreateCommandAllocatorAndList(m_MainCommandIdx), std::format("idx: {} 커맨드리스트 생성 실패!", m_MainCommandIdx));

	return true;
}

bool Renderer::CreateSwapChain()
{
	// 백버퍼랑 뭐시기랑 어쨋든 해주는거
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
	CHECK_CREATE_FAILED(m_SwapChain, "스왑체인 생성 실패!");

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
	CHECK_CREATE_FAILED(m_RtvHeap, "m_RtvHeap 생성 실패!");

	descriptorDesc.NumDescriptors = 1;
	descriptorDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

	m_Device->CreateDescriptorHeap(&descriptorDesc, IID_PPV_ARGS(m_DsvHeap.GetAddressOf()));
	CHECK_CREATE_FAILED(m_DsvHeap, "m_DsvHeap 생성 실패!");

	return true;
}


bool Renderer::CreateRTV()
{
	// 뷰를 만들으려면 리소스가 존재해야한다
	// 그러나 이건 따로 만들 필요가 없음
	// 스왑체인을 통해 만들었기 때문에 이미 존재함

	D3D12_CPU_DESCRIPTOR_HANDLE rtvDescriptorHandle = m_RtvHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < m_NumSwapChainBuffers; ++i) {
		ID3D12Resource* tempResource = nullptr;

		m_SwapChain->GetBuffer(
			i,
			IID_PPV_ARGS(&tempResource));
		CHECK_CREATE_FAILED(tempResource, "m_SwapChain->GetBuffer Failed!!");

		m_RenderTargetBuffer[i] = COOLResourcePtr(new COOLResource(tempResource, D3D12_RESOURCE_STATE_PRESENT));
		m_RenderTargetBuffer[i].get()->SetDimension(D3D12_SRV_DIMENSION_TEXTURE2D);

		m_Device->CreateRenderTargetView(m_RenderTargetBuffer[i]->GetResource(), nullptr, rtvDescriptorHandle);
		rtvDescriptorHandle.ptr += m_RtvDescIncrSize;

		RegisterShaderResource(m_RenderTargetBuffer[i]);

	}

	return true;
}

bool Renderer::CreateDSV()
{
	// Depth/Stencil은 우리가 리소스를 따로 만들어주어야함
	// 보통 리소스를 디폴트힙에다 만들면
	// 값을 넣어 복사해주기 위한 업로드힙이 별도로 필요
	// 근데 얘는 필요없음 비어있는애
	m_DepthStencilBuffer = CreateEmpty2DResource(D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_DEPTH_WRITE, m_ScreenSize);
	m_DepthStencilBuffer.get()->SetDimension(D3D12_SRV_DIMENSION_TEXTURE2D);
	m_DepthStencilBuffer.get()->SetName("depthstencil buffer");

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CPU_DESCRIPTOR_HANDLE dsvCPUHandle = m_DsvHeap->GetCPUDescriptorHandleForHeapStart();
	m_Device->CreateDepthStencilView(m_DepthStencilBuffer->GetResource(), &dsvDesc, dsvCPUHandle);

	RegisterShaderResource(m_DepthStencilBuffer);
	return true;
}

bool Renderer::CreateRootSignature()
{
	// t0 
	// 
	const int resourceType = 3;
	D3D12_DESCRIPTOR_RANGE descRange[resourceType] = {};
	for (int i = 0; i < resourceType; ++i) {
		descRange[i].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		descRange[i].NumDescriptors = UINT_MAX;
		descRange[i].BaseShaderRegister = 0;
		descRange[i].RegisterSpace = i;
		//descRange[i].Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE;		// D3D12_DESCRIPTOR_RANGE1
		descRange[i].OffsetInDescriptorsFromTableStart = 0;								// 좀 더 알아봐야함
	}
	// 사용 시
	// 디스크립터힙 내부: (Texture2D, TextureCube, Texture2D, ...) 
	// 쉐이더 에서 TexCubeList[1]로 사용 해야 함

	const int numParams = ROOT_SIGNATURE_IDX_MAX;
	D3D12_ROOT_PARAMETER rootParams[numParams] = {};
	// idx 0: descriptor table ...ROOT_SIGNATURE_IDX enum 참고
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

	// 루트시그니처 생성
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
	//descriptorDesc.NumDescriptors = UINT_MAX;												// 힙의 크기를 모른채로 시작하고싶은데 방법이 없을까
	descriptorDesc.NumDescriptors = m_Resources.size();										// 렌더타겟과 ds도 넣고싶음 -> 들어가긴 하네
	descriptorDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descriptorDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descriptorDesc.NodeMask = 0;

	m_Device->CreateDescriptorHeap(&descriptorDesc, IID_PPV_ARGS(m_ResourceHeap.GetAddressOf()));
	CHECK_CREATE_FAILED(m_ResourceHeap, "m_ResourceHeap 생성 실패!");

	// m_Resource에 있는 리소스들 디스크립터를 만들고 힙에다가 등록
	for (auto resource : m_Resources) {
		ID3D12Resource* res = resource.get()->GetResource();
		D3D12_RESOURCE_DESC resDesc = res->GetDesc();

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = resDesc.Format;
		srvDesc.ViewDimension = resource.get()->GetDimension();

		switch (srvDesc.ViewDimension) {
		case D3D12_SRV_DIMENSION_TEXTURE2D:
			srvDesc.Texture2D.MipLevels = -1;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.PlaneSlice = 0;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			break;

		case D3D12_SRV_DIMENSION_TEXTURECUBE:
			srvDesc.TextureCube.MipLevels = 1;
			srvDesc.TextureCube.MostDetailedMip = 0;
			srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
			break;

		case D3D12_SRV_DIMENSION_BUFFER:
			srvDesc.Buffer.FirstElement = 0;
			srvDesc.Buffer.NumElements = 0;								// 나중에 버퍼 쓸 때 다시 확인
			srvDesc.Buffer.StructureByteStride = 0;
			srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

			DebugPrint("ViewDimension: D3D12_SRV_DIMENSION_TEXTURECUBE, 버퍼로 되어있다\n리소스 이름: " + resource.get()->GetName());
			break;

		default:
			DebugPrint("ViewDimension이 알맞지 않음 \n리소스 이름: " + resource.get()->GetName());
			return false;
		}

	}
	// D3D12_SRV_DIMENSION_UNKNOWN = 0,
	// D3D12_SRV_DIMENSION_BUFFER = 1,
	// D3D12_SRV_DIMENSION_TEXTURE1D = 2,
	// D3D12_SRV_DIMENSION_TEXTURE1DARRAY = 3,
	// D3D12_SRV_DIMENSION_TEXTURE2D = 4,
	// D3D12_SRV_DIMENSION_TEXTURE2DARRAY = 5,
	// D3D12_SRV_DIMENSION_TEXTURE2DMS = 6,
	// D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY = 7,
	// D3D12_SRV_DIMENSION_TEXTURE3D = 8,
	// D3D12_SRV_DIMENSION_TEXTURECUBE = 9,
	// D3D12_SRV_DIMENSION_TEXTURECUBEARRAY = 10,
	// D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE = 11

	return true;
}

bool Renderer::LoadShaders()
{
	// 모든 쉐이더를 로드한다
	// 쓸 것들을 파일로 저장해둔다? 
	// 각기 다른 쉐이더 종류, 블렌드스테이트, 레스터라이저스테이트, ds 스테이트 등등 모두 다를태니
	// 쉐이더는 미리 로드 해두고 거기에 맞는 오브젝트들이 있다면 렌더하는 방향으로 갈까?
	// 프로그램 최초 로드시에만 쉐이더를 생성한다 나쁘지 않음


#ifdef TEST_SHADER
	auto shader = std::make_shared<TestShader>(1, "TestShader");
	m_Shaders.push_back(shader);

	auto tshader = std::make_shared<SkyboxShader>(1000, "SkyboxShader");
	m_Shaders.push_back(tshader);

	for (auto& sha : m_Shaders) {
		// todo Shader가 필요한 리소스를 불러와 m_ResourceHeap에다가 넣음
		CHECK_CREATE_FAILED(sha->InitShader(m_Device, m_MainCommandList, m_RootSignature), sha->GetName());
	}
#endif

	// 렌더큐 기준으로 오름차순 정렬한다
	std::sort(m_Shaders.begin(), m_Shaders.end());

	return true;
	return false;
}

bool Renderer::Init(const SIZE& wndSize, HWND hWnd)
{
	// dx12의 초기화 과정
	// 
	// 디바이스 생성
	// 펜스 생성, 서술자 크기 얻기
	// 멀티샘플링 어디까지 확인
	// 커맨드리스트, 큐 생성
	// 스왑체인 생성
	// rtv dsv 힙 생성
	// rtv, dsv 생성

	m_ScreenSize = wndSize;
	m_hWnd = hWnd;

	CHECK_CREATE_FAILED(CreateDevice(), "CreateDevice Failed!!");
	CHECK_CREATE_FAILED(CreateFence(), "CreateFence Failed!!");
	CHECK_CREATE_FAILED(CreateCommandQueueAndList(), "CreateCommandQueueAndList Failed!!");
	CHECK_CREATE_FAILED(CreateSwapChain(), "CreateSwapChain Failed!!");
	CHECK_CREATE_FAILED(CreateRTVAndDSVDescrHeap(), "CreateRTVAndDescrHeap Failed!!");
	CHECK_CREATE_FAILED(CreateRTV(), "CreateRTV Failed!!");
	CHECK_CREATE_FAILED(CreateDSV(), "CreateDSV Failed!!");

	m_MainCommandAllocator = m_CommandAllocators[CMDID::MAIN];
	m_MainCommandList = m_GraphicsCommandLists[CMDID::MAIN];

	// scene
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
	CHECK_CREATE_FAILED(allocator, "m_CommandAllocator 생성 실패");

	m_Device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		allocator.Get(),
		nullptr,
		IID_PPV_ARGS(commandlist.GetAddressOf()));
	CHECK_CREATE_FAILED(commandlist, "m_GraphicsCommandList 생성 실패");

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
	resDesc.DepthOrArraySize = 1;										// 텍스쳐어레이같은건 안쓸 예정 아마도
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
	resDesc.DepthOrArraySize = 1;
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

UINT Renderer::RegisterShaderResource(COOLResourcePtr resource)
{
	m_Resources.push_back(resource);
	return m_Resources.size() - 1;
}

void Renderer::Render()
{
	// pre render
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	for (int i = 0; i < m_CommandAllocators.size(); ++i) {
		m_CommandAllocators[i]->Reset();
		m_GraphicsCommandLists[i]->Reset(m_CommandAllocators[i].Get(), nullptr);
	}

	// root signature set
	m_MainCommandList->SetGraphicsRootSignature(m_RootSignature.Get());

	SetViewportScissorRect();

	// present를 기다려야 하지 않을까

	m_RenderTargetBuffer[m_CurSwapChainIndex]->TransToState(m_MainCommandList, D3D12_RESOURCE_STATE_RENDER_TARGET);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvCpuDesHandle = m_RtvHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_CPU_DESCRIPTOR_HANDLE dsvCpuDesHandle = m_DsvHeap->GetCPUDescriptorHandleForHeapStart();
	rtvCpuDesHandle.ptr += m_CurSwapChainIndex * m_RtvDescIncrSize;

	//float clearColor[4] = { 0.3f, 0.9f, 0.3f, 1.0f };
	float clearColor[4] = { 0, };// { 1.0f, 1.0f, 1.0f, 1.0f };

	m_MainCommandList->ClearRenderTargetView(rtvCpuDesHandle, clearColor, 0, nullptr);
	m_MainCommandList->ClearDepthStencilView(dsvCpuDesHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	m_MainCommandList->OMSetRenderTargets(1, &rtvCpuDesHandle, true, &dsvCpuDesHandle);

	// real render
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	for (auto shader : m_Shaders) {
		shader->Render(m_MainCommandList);
	}

	// render end
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	m_RenderTargetBuffer[m_CurSwapChainIndex]->TransToState(m_MainCommandList, D3D12_RESOURCE_STATE_PRESENT);

	for (auto& command : m_GraphicsCommandLists)
		command->Close();

	// CommandList CommandQueue Execute
	ID3D12CommandList* p[] = { m_MainCommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(p), p);

	// wait for gpu
	UINT64 fenceValue = ++m_FenceValues[m_CurSwapChainIndex];
	HRESULT hResult = m_CommandQueue->Signal(m_Fence.Get(), fenceValue);
	if (m_Fence->GetCompletedValue() < fenceValue) {
		hResult = m_Fence->SetEventOnCompletion(fenceValue, m_FenceEvent);
		WaitForSingleObject(m_FenceEvent, INFINITE);
	}

	// present
	m_SwapChain->Present(0, 0);

	m_CurSwapChainIndex = m_SwapChain->GetCurrentBackBufferIndex();
	fenceValue = ++m_FenceValues[m_CurSwapChainIndex];
	hResult = m_CommandQueue->Signal(m_Fence.Get(), fenceValue);
	if (m_Fence->GetCompletedValue() < fenceValue) {
		hResult = m_Fence->SetEventOnCompletion(fenceValue, m_FenceEvent);
		WaitForSingleObject(m_FenceEvent, INFINITE);
	}

}
