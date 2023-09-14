#include "../framework.h"
#include "Renderer.h"

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
	CreateDXGIFactory2(DXGIFactoryFlags, IID_PPV_ARGS(&m_Factory));
	CHECK_CREATE_FAILED(m_Factory, " m_Factory ���� ����\n");

	// �⺻ ����ͷ� �����غ���
	D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_Device));

	if (!m_Device) {
		// �����ߴٸ� ���丮���� ����͸� ã���ش�.
		ComPtr<IDXGIAdapter1> pAdapter = nullptr;
		for (UINT i = 0; DXGI_ERROR_NOT_FOUND != m_Factory->EnumAdapters1(i, &pAdapter); i++) {
			DXGI_ADAPTER_DESC1 adapterDesc;
			pAdapter->GetDesc1(&adapterDesc);
			if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
		}

		D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_Device));
	}

	CHECK_CREATE_FAILED(m_Device, " m_Device ���� ����\n");

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
	// dx12�� �ʱ�ȭ ����
	// 
	// ����̽� ����
	// �潺 ����, ������ ũ�� ���
	// ��Ƽ���ø� ������ Ȯ��
	// Ŀ�ǵ帮��Ʈ, ť ����
	// ����ü�� ����

	CHECK_CREATE_FAILED(CreateDevice(), "CreateDevice Failed!!");
	//CHECK_CREATE_FAILED(CreateFence(), "CreateFence Failed!!");
	CHECK_CREATE_FAILED(CreateSwapChain(), "CreateSwapChain Failed!!");
	CHECK_CREATE_FAILED(CreateCommandQueueAndList(), "CreateCommandQueueAndList Failed!!");

	return true;
}
