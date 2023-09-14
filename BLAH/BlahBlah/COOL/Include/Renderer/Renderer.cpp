#include "../framework.h"
#include "Renderer.h"

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
	CreateDXGIFactory2(DXGIFactoryFlags, IID_PPV_ARGS(&m_Factory));
	CHECK_CREATE_FAILED(m_Factory, " m_Factory 생성 실패\n");

	// 기본 어답터로 생성해본다
	D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_Device));

	if (!m_Device) {
		// 실패했다면 팩토리님이 어답터를 찾아준다.
		ComPtr<IDXGIAdapter1> pAdapter = nullptr;
		for (UINT i = 0; DXGI_ERROR_NOT_FOUND != m_Factory->EnumAdapters1(i, &pAdapter); i++) {
			DXGI_ADAPTER_DESC1 adapterDesc;
			pAdapter->GetDesc1(&adapterDesc);
			if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
		}

		D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_Device));
	}

	CHECK_CREATE_FAILED(m_Device, " m_Device 생성 실패\n");

	return true;
}

bool Renderer::CreateSwapChain()
{
	return false;
}

bool Renderer::CreateCommandQueueAndList()
{
	return false;
}

bool Renderer::Init()
{
	// dx12의 초기화 과정
	// 
	// 디바이스 생성
	// 펜스 생성, 서술자 크기 얻기
	// 멀티샘플링 어디까지 확인
	// 커맨드리스트, 큐 생성
	// 스왑체인 생성

	CHECK_CREATE_FAILED(CreateDevice(), "CreateDevice Failed!!");
	//CHECK_CREATE_FAILED(CreateFence(), "CreateFence Failed!!");
	CHECK_CREATE_FAILED(CreateSwapChain(), "CreateSwapChain Failed!!");
	CHECK_CREATE_FAILED(CreateCommandQueueAndList(), "CreateCommandQueueAndList Failed!!");

	return true;
}
