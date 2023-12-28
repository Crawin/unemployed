#include "framework.h"
#include "Renderer.h"
#include "COOLResource.h"
#include "Camera/Camera.h"
//#include "Shader/Shader.h"

#define TEST_SHADER

#ifdef TEST_SHADER
#include "Shader/TestShader.h"
#include "Shader/SkyboxShader.h"
#include "Shader/MeshShader.h"
#include "Material/Material.h"
#include "Mesh/Mesh.h"
#endif

Renderer::Renderer()
{

}

Renderer::~Renderer()
{
	// 각종 객체들이 아래 함수 호출까지는 살아있는 상태이며 소멸자를 완전히 나가면서 소멸된다.
	// 확실하게 체크하기 위해서는 어케해야할까?
	// Renderer 말고 Application에서 하자

	// 혹시 모르니 여기서 모두 해제
	m_Resources.clear();
	m_VertexIndexDatas.clear();

	// 죽기 전에 살아있는 애들 확인하고 간다
#if defined(_DEBUG)
	IDXGIDebug1* debug = NULL;
	DXGIGetDebugInterface1(0, __uuidof(IDXGIDebug1), (void**)&debug);
	HRESULT hResult = debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
	debug->Release();
#endif

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
		m_RenderTargetBuffer[i]->SetName(std::format("RenderTarget[{}]", i));
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
	m_DepthStencilBuffer = CreateEmpty2DResource(D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_DEPTH_WRITE, m_ScreenSize, "depthstencil buffer");
	m_DepthStencilBuffer.get()->SetDimension(D3D12_SRV_DIMENSION_TEXTURE2D);
	m_DepthStencilBuffer.get()->SetName("depthstencil buffer");

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CPU_DESCRIPTOR_HANDLE dsvCPUHandle = m_DsvHeap->GetCPUDescriptorHandleForHeapStart();
	m_Device->CreateDepthStencilView(m_DepthStencilBuffer->GetResource(), &dsvDesc, dsvCPUHandle);

	//RegisterShaderResource(m_DepthStencilBuffer);
	return true;
}

bool Renderer::CreateRootSignature()
{
	// t0 
	// 
	const int resourceType = 5;
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

	//  ...ROOT_SIGNATURE_IDX enum 참고
	// idx 0: descriptor table, descriptor table
	rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParams[0].DescriptorTable.NumDescriptorRanges = resourceType;
	rootParams[0].DescriptorTable.pDescriptorRanges = descRange;
	rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	// idx 1: index of descriptor table, root constants
	rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParams[1].Constants.Num32BitValues = 16;
	rootParams[1].Constants.ShaderRegister = 0;
	rootParams[1].Constants.RegisterSpace = 0;
	rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	// idx 2: camera matrix (view, projection(ortho)), cbv
	rootParams[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParams[2].Descriptor.ShaderRegister = 1;
	rootParams[2].Descriptor.RegisterSpace = 0;
	rootParams[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	// idx 3: index of descriptor table, root constants
	rootParams[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParams[3].Constants.Num32BitValues = 16;
	rootParams[3].Constants.ShaderRegister = 2;
	rootParams[3].Constants.RegisterSpace = 0;
	rootParams[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	// idx 4: etc (delta time, 이것도 cbv로
	rootParams[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParams[4].Descriptor.ShaderRegister = 3;
	rootParams[4].Descriptor.RegisterSpace = 0;
	rootParams[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
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

	auto currentCPUPtr = m_ResourceHeap->GetCPUDescriptorHandleForHeapStart();
	//auto currentGPUPtr = m_ResourceHeap->GetGPUDescriptorHandleForHeapStart();

	// m_Resource에 있는 리소스들 디스크립터를 만들고 힙에다가 등록
	for (int i = 0; i < m_Resources.size(); ++i) {
		//for (auto resource : m_Resources) {
		ID3D12Resource* res = m_Resources[i].get()->GetResource();
		D3D12_RESOURCE_DESC resDesc = res->GetDesc();

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = resDesc.Format;
		srvDesc.ViewDimension = m_Resources[i].get()->GetDimension();

		switch (srvDesc.ViewDimension) {
		case D3D12_SRV_DIMENSION_TEXTURE2D:
			srvDesc.Texture2D.MipLevels = 1;
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

			DebugPrint("ViewDimension: D3D12_SRV_DIMENSION_TEXTURECUBE, 버퍼로 되어있다\n리소스 이름: " + m_Resources[i].get()->GetName());
			break;

		default:
			DebugPrint("ViewDimension이 알맞지 않음 \n리소스 이름: " + m_Resources[i].get()->GetName());
			return false;
		}

		ID3D12Resource* resource = m_Resources[i]->GetResource();
		m_Device->CreateShaderResourceView(resource, &srvDesc, currentCPUPtr);
		currentCPUPtr.ptr += m_CbvSrvDescIncrSize;
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

	//auto tshader = std::make_shared<SkyboxShader>(1000, "SkyboxShader");
	//m_Shaders.push_back(tshader);

	auto tttShader = std::make_shared<MeshShader>(1001, "meshShader");
	m_Shaders.push_back(tttShader);

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

	m_MainCommandAllocator->Reset();
	m_MainCommandList->Reset(m_MainCommandAllocator.Get(), nullptr);

	//CHECK_CREATE_FAILED(CreateTestRootSignature(), "CreateRootSignature Failed!!");
	CHECK_CREATE_FAILED(CreateRootSignature(), "CreateRootSignature Failed!!");
	CHECK_CREATE_FAILED(LoadShaders(), "LoadShaders Failed!!");

	// scene
	// 씬 로드 부분 이라고 가정
#ifdef TEST_SHADER
	{
		auto commandAllocator = m_MainCommandAllocator;
		auto commandList = m_MainCommandList;

		// 씬 로드 플로우????? 임시임
		//	0. 루트시그니쳐가 있어야함
		//	1. 씬에서 사용할 쉐이더를 로드한다 <- 지금은 위에서 함
		//	2. 씬에서 사용할 매터리얼 로드
		//		a. 얘가 필요한 텍스쳐들 로드, 로드하면 인덱스를 리턴함
		//		b. 매터리엘 멤버에 보면 배열 있음(m_TextureIndex). 거기에 텍스쳐 인덱스 넣기
		//		c. 완료 후 쉐이더에 연결
		//	3. 오브젝트들 로드 및 매터리얼과 짝지어서 맵퍼같은거에 등록 <- 
		//  사실 2번 3번 순서는 큰 상관 없을거같음

		//m_MainCommandAllocator->Reset();
		//commandList->Reset(commandAllocator.Get(), nullptr);


		// 아래가 1번
		// 하드코딩하긴 귀찮으니까 텍스트파일로 빼서 하자
		// ex) material_load_list.json
		//		여기 안에 메터리얼 이름부터 해서 로드할 텍스쳐 이름이 쫙 있는거임, 이거 좀 쩌는듯
		//		
		//		{name: 임시, shader: 무슨무슨쉐이더,albedo: bbb.dds, ...}, {name: asdf, ...} ...

		// 뭐 대충 메터리얼 로드하는 부분이라고 가정함
		// 로드 해야 할 메터리얼 파일들이 txt로 리스트로 줄마다 나열되어있음
		// SkyBox.json
		// asdf.json
		// ...

		// 2번이긴 하지?
		const char* materialLoadList[] = { "SkyBox.json", };

		for (int i = 0; i < _countof(materialLoadList); ++i) {
			Material* temp = new Material("임시");
			temp->LoadTexture(commandList, materialLoadList[i]);

			int skyBox = CreateTextureFromDDSFile(commandList, L"bg.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);//D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
			int temphair = CreateTextureFromDDSFile(commandList, L"satoh.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			int tempbody = CreateTextureFromDDSFile(commandList, L"satob.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			int uvchecker = CreateTextureFromDDSFile(commandList, L"uvchecker2.dds", D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

			temp->SetAlbedoTextureIndex(skyBox);

			m_Shaders.back()->AddMaterial(temp);
		}

		// 3번이긴 하지?
		//Mesh tempMesh;
		//tempMesh.LoadFile(commandList, "satodia.bin");
		//m_Meshes.push_back(tempMesh);
		m_Camera = new Camera(90.0f, ((float)(m_ScreenSize.cx) / (float)(m_ScreenSize.cy)), 0.1f, 1000.0f);
		m_Camera->SetWorldPosition({ 0.0f, 30.0f, -150.0f });
		m_Camera->Init();

		// 업로드버퍼같은거 실행
		ExecuteAndEraseUploadHeap(commandList);
	}
#endif // _DEBUG

	CHECK_CREATE_FAILED(CreateResourceDescriptorHeap(), "CreateResourceDescriptorHeap");

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

COOLResourcePtr Renderer::CreateEmpty2DResource(D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_STATES resourceState, const SIZE& size, std::string_view name)
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

	return COOLResourcePtr(new COOLResource(temp, resourceState, heapType, name));
}

COOLResourcePtr Renderer::CreateEmptyBufferResource(D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_STATES resourceState, UINT bytes, std::string_view name)
{
	ID3D12Resource* temp;

	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
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

	return COOLResourcePtr(new COOLResource(temp, resourceState, heapType, name));
}

int Renderer::CreateTextureFromDDSFile(ComPtr<ID3D12GraphicsCommandList> commandList, const wchar_t* fileName, D3D12_RESOURCE_STATES resourceState)
{
	ID3D12Resource* texture = nullptr;
	std::unique_ptr<uint8_t[]> ddsData;
	std::vector<D3D12_SUBRESOURCE_DATA> subResources;
	DDS_ALPHA_MODE ddsAlphaMode = DDS_ALPHA_MODE_UNKNOWN;
	bool isCubeMap = false;

	std::wstring temp(fileName);
	std::string strTemp(temp.begin(), temp.end());

	// dds를 로드한다
	HRESULT res = DirectX::LoadDDSTextureFromFileEx(m_Device.Get(), fileName, 0, D3D12_RESOURCE_FLAG_NONE, DDS_LOADER_DEFAULT, &texture, ddsData, subResources, &ddsAlphaMode, &isCubeMap);

	// 실패시 -1 리턴
	if (res == E_FAIL) {
		// fileName이 wstring이라 wstring -> string으로 바꿔줘야한다
		std::wstring wstring{ fileName };
		wstring += L"Load Fail!! Check path";

		std::string str;
		str.assign(wstring.begin(), wstring.end());

		DebugPrint(str);
		return -1;
	}

	if (ddsAlphaMode != DDS_ALPHA_MODE_UNKNOWN) {
		DebugPrint("!!!!!\n");
	}

	// 리소스 디스크립터
	UINT64 numObSUbresource = GetRequiredIntermediateSize(texture, 0, subResources.size());

	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Alignment = 0;
	resDesc.Width = numObSUbresource;
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;										// 텍스쳐어레이같은건 안쓸 예정 아마도
	resDesc.MipLevels = 1;
	resDesc.Format = DXGI_FORMAT_UNKNOWN;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	// 업로드 힙에다 생성해둔다
	D3D12_HEAP_PROPERTIES heapProperty = {};
	heapProperty.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProperty.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperty.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperty.CreationNodeMask = 1;
	heapProperty.VisibleNodeMask = 1;

	// 업로드용 리소스 생성, 나중에 이거 지워야함
	ID3D12Resource* uploadResource = nullptr;
	m_Device->CreateCommittedResource(&heapProperty, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadResource));
	m_UploadResources.push_back(uploadResource);

	uploadResource->SetName((temp + L" - upload buffer").c_str());

	// 복사 및 상태 전이
	UpdateSubresources(commandList.Get(), texture, uploadResource, 0, 0, subResources.size(), &subResources[0]);

	D3D12_RESOURCE_BARRIER resourceBarrier = {};
	resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	resourceBarrier.Transition.pResource = texture;
	resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	resourceBarrier.Transition.StateAfter = resourceState;
	resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	commandList->ResourceBarrier(1, &resourceBarrier);

	COOLResourcePtr newResource(new COOLResource(texture, resourceState, D3D12_HEAP_TYPE_DEFAULT));
	newResource->SetDimension(D3D12_SRV_DIMENSION_TEXTURE2D);

	newResource->SetName(strTemp);

	return RegisterShaderResource(newResource);
}

int Renderer::LoadMeshFromFile(ComPtr<ID3D12GraphicsCommandList> commandList, const char* fileName)
{
	std::vector<int> data;

	auto uploadResource = CreateEmptyBufferResource(D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, data.size());


	return 0;
}

int Renderer::CreateEmptyBuffer(D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_STATES resourceState, UINT bytes, std::string_view name, void** toMapData)
{
	UINT createBytes = ((bytes + 255) & ~255);
	auto resource = CreateEmptyBufferResource(heapType, resourceState, createBytes, name);

	if (toMapData) {
		resource->SetMapOn();
		resource->GetResource()->Map(0, nullptr, toMapData);
	}

	if (resourceState == D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER ||
		resourceState == D3D12_RESOURCE_STATE_INDEX_BUFFER ||
		toMapData != nullptr) {
		m_VertexIndexDatas.push_back(resource);
		return m_VertexIndexDatas.size() - 1;
	}
	else {
		m_Resources.push_back(resource);
		return m_Resources.size() - 1;
	}

	return -1;
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

void Renderer::SetViewportScissorRect(ComPtr<ID3D12GraphicsCommandList> commandList)
{
	D3D12_VIEWPORT vp;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.Width = static_cast<float>(m_ScreenSize.cx);
	vp.Height = static_cast<float>(m_ScreenSize.cy);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;

	RECT scRect = { 0, 0, m_ScreenSize.cx, m_ScreenSize.cy };

	SetViewportScissorRect(commandList, 1, vp, scRect);
}

void Renderer::SetViewportScissorRect(ComPtr<ID3D12GraphicsCommandList> commandList, UINT numOfViewPort, const D3D12_VIEWPORT& viewport, const RECT& scissorRect)
{
	commandList->RSSetViewports(numOfViewPort, &viewport);
	commandList->RSSetScissorRects(numOfViewPort, &scissorRect);
}

UINT Renderer::RegisterShaderResource(COOLResourcePtr resource)
{
	m_Resources.push_back(resource);
	return m_Resources.size() - 1;
}

void Renderer::ExecuteAndEraseUploadHeap(ComPtr<ID3D12GraphicsCommandList> commandList)
{
	commandList->Close();
	ID3D12CommandList* ppd3dCommandLists[] = { commandList.Get() };
	m_CommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	UINT64 fenceValue = ++m_FenceValues[m_CurSwapChainIndex];
	HRESULT hResult = m_CommandQueue->Signal(m_Fence.Get(), fenceValue);
	if (m_Fence->GetCompletedValue() < fenceValue) {
		hResult = m_Fence->SetEventOnCompletion(fenceValue, m_FenceEvent);
		WaitForSingleObject(m_FenceEvent, INFINITE);
	}

	for (int i = 0; i < m_UploadResources.size(); ++i) {
		if (m_UploadResources[i]) {
			m_UploadResources[i]->Release();
			m_UploadResources[i] = nullptr;
		}
	}
	m_UploadResources.clear();
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

	// 리소스 set
	m_MainCommandList->SetDescriptorHeaps(1, m_ResourceHeap.GetAddressOf());
	m_MainCommandList->SetGraphicsRootDescriptorTable(0, m_ResourceHeap->GetGPUDescriptorHandleForHeapStart());

	SetViewportScissorRect(m_MainCommandList);

	// present를 기다려야 하지 않을까

	m_RenderTargetBuffer[m_CurSwapChainIndex]->TransToState(m_MainCommandList, D3D12_RESOURCE_STATE_RENDER_TARGET);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvCpuDesHandle = m_RtvHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_CPU_DESCRIPTOR_HANDLE dsvCpuDesHandle = m_DsvHeap->GetCPUDescriptorHandleForHeapStart();
	rtvCpuDesHandle.ptr += m_CurSwapChainIndex * m_RtvDescIncrSize;

	float clearColor[4] = { 0.1f, 0.1f, 0.35f, 1.0f };
	//float clearColor[4] = { 0, };// { 1.0f, 1.0f, 1.0f, 1.0f };

	m_MainCommandList->ClearRenderTargetView(rtvCpuDesHandle, clearColor, 0, nullptr);
	m_MainCommandList->ClearDepthStencilView(dsvCpuDesHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	m_MainCommandList->OMSetRenderTargets(1, &rtvCpuDesHandle, true, &dsvCpuDesHandle);

	// real render
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	XMFLOAT3 tempMove = { 0.0f, 0.0f, 0.0f };
	if (GetAsyncKeyState('W') & 0x8000) tempMove.z += 1.0f;
	if (GetAsyncKeyState('S') & 0x8000) tempMove.z -= 1.0f;
	if (GetAsyncKeyState('A') & 0x8000) tempMove.x -= 1.0f;
	if (GetAsyncKeyState('D') & 0x8000) tempMove.x += 1.0f;
	if (GetAsyncKeyState('Q') & 0x8000) tempMove.y -= 1.0f;
	if (GetAsyncKeyState('E') & 0x8000) tempMove.y += 1.0f;

	float size = Vector3::Length(tempMove);
	if (size > 0.5f) {
		XMFLOAT3 normal = Vector3::Normalize(tempMove);
		tempMove = Vector3::ScalarProduct(normal, 1.0f / 60.0f);
		XMFLOAT3 pos = Vector3::Add(m_Camera->GeWorldPosition(), tempMove);
		m_Camera->SetWorldPosition(pos);

		// debug print;
		//XMFLOAT4 stat = { -74.0509, 132.178, -0.982319, 1.0f };
		//XMFLOAT4X4 view = m_Camera->GetViewMat();
		//XMFLOAT4X4 proj = m_Camera->GetProjMat();

		//XMVECTOR statVector = XMLoadFloat4(&stat);

		//XMMATRIX viewMatrix = XMLoadFloat4x4(&view);
		//XMMATRIX projMatrix = XMLoadFloat4x4(&proj);

		//statVector = XMVector4Transform(statVector, viewMatrix);
		//statVector = XMVector4Transform(statVector, projMatrix);

		//XMFLOAT4 pTrans;
		//XMStoreFloat4(&pTrans, statVector);

		//DebugPrint(std::format("pTrans {}, {}, {}", pTrans.x, pTrans.y, pTrans.z));
	}


	m_Camera->Render(m_MainCommandList);

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

D3D12_GPU_VIRTUAL_ADDRESS Renderer::GetResourceGPUAddress(int idx)
{
	if (0 <= idx && idx < m_Resources.size())
		return m_Resources[idx]->GetResource()->GetGPUVirtualAddress();

	return 0;
}

D3D12_GPU_VIRTUAL_ADDRESS Renderer::GetVertexDataGPUAddress(int idx)
{
	if (0 <= idx && idx < m_VertexIndexDatas.size())
		return m_VertexIndexDatas[idx]->GetResource()->GetGPUVirtualAddress();

	return 0;
}

COOLResourcePtr Renderer::GetResourceFromIndex(int idx)
{
	if (0 <= idx && idx < m_Resources.size())
		return m_Resources[idx];

	return nullptr;
}

COOLResourcePtr Renderer::GetVertexDataFromIndex(int idx)
{
	if (0 <= idx && idx < m_VertexIndexDatas.size())
		return m_VertexIndexDatas[idx];

	return nullptr;
}

/*
void Renderer::MouseInput(int xin, int yin)
{
	float x = yin * 0.10f;
	float y = xin * 0.10f;

	XMFLOAT3 look = m_Camera->GetLook();
	XMFLOAT3 right = m_Camera->GetRight();
	XMFLOAT3 up = m_Camera->GetUp();

	// x
	if (xin != 0) {
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&right), XMConvertToRadians(x));
		right = Vector3::TransformNormal(right, xmmtxRotate);
		up = Vector3::TransformNormal(up, xmmtxRotate);
		look = Vector3::TransformNormal(look, xmmtxRotate);
	}
	// y
	if (yin != 0) {
		XMFLOAT3 xmf3Up = { 0.0f, 1.0f, 0.0f };
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&xmf3Up), XMConvertToRadians(y));
		right = Vector3::TransformNormal(right, xmmtxRotate);
		look = Vector3::TransformNormal(look, xmmtxRotate);
		up = Vector3::TransformNormal(up, xmmtxRotate);
	}


	//m_Camera->SetRight(right);
	//m_Camera->SetLook(look);
	//m_Camera->SetUp(up);

	//look = Vector3::TransformNormal(look, xmmtxRotate);
	DebugPrint(std::format("look: {}, {}, {}", look.x, look.y, look.z));
}
*/
Renderer* Renderer::GetRendererPtr()
{
	std::cout << "ptr: " << &Renderer::GetInstance() << std::endl;
	return &Renderer::GetInstance();
}
