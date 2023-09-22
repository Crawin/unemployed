#pragma once
#include <vector>
// dx12 ������

class Shader;
class COOLResource;


struct GraphicsCommand {
	ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_GraphicsCommandList;
};

class Renderer
{
public:
	static Renderer& Instance() {
		static Renderer inst;
		return inst;
	}

	enum CMDID {
		MAIN = 0,
		MAX = 1
	};

private:
	Renderer();
	~Renderer();

	bool CreateDevice();
	bool CreateFence();
	bool CreateCommandQueueAndList();
	bool CreateSwapChain();
	bool CreateRTVAndDSVDescrHeap();
	bool CreateRTV();
	bool CreateDSV();

	bool CreateRootSignature();
	bool CreateTestRootSignature();
	bool LoadShaders();

public:
	bool Init(const SIZE& wndSize, HWND hWnd);

	// ������ �Ҵ���,����Ʈ�� �ε����� outIndex�� ������
	bool CreateCommandAllocatorAndList(size_t& outIndex);

	// ������ ��ü�� ����
	void SetViewportScissorRect();
	// �������� ������� ����
	void SetViewportScissorRect(UINT numOfViewPort, const D3D12_VIEWPORT& viewport, const RECT& scissorRect);

	void ResourceTransition(ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES transTo);

	// render
	void Render();

private:
	// ����ü�� ����
	static const UINT m_NumSwapChainBuffers = 2;
	UINT m_CurSwapChainIndex = 0;

	// ������
	HWND m_hWnd = 0;
	SIZE m_ScreenSize = { 1280,720 };
	bool m_Windowed = true;

	// msaa4x
	UINT m_MsaaQualityLevels = 0;
	bool m_MsaaEnable = true;

	// ��ũ���� ���� ������
	UINT m_CbvSrvDescIncrSize = 0;
	UINT m_RtvDescIncrSize = 0;
	UINT m_DsvDescIncrSize = 0;

	ComPtr<IDXGIFactory4> m_Factory;
	ComPtr<ID3D12Device> m_Device;
	ComPtr<ID3D12Fence> m_Fence;
	ComPtr<IDXGISwapChain3> m_SwapChain;

	UINT64 m_FenceValues[m_NumSwapChainBuffers];
	HANDLE m_FenceEvent;

	// ���� �Ʒ��� �����ص� �ǳ�?

	ComPtr<ID3D12DescriptorHeap> m_RtvHeap;
	ComPtr<ID3D12DescriptorHeap> m_DsvHeap;

	ComPtr<COOLResource> m_RenderTargetBuffer[m_NumSwapChainBuffers];
	ComPtr<COOLResource> m_DepthStencilBuffer;

	size_t m_MainCommandIdx = 0;
	ComPtr<ID3D12CommandQueue> m_CommandQueue;
	// �Ƹ����� ���ɼ��� �������, �׸��ڸ� ������ ���ÿ� ����?

	std::vector<ComPtr<ID3D12CommandAllocator>> m_CommandAllocators;
	std::vector<ComPtr<ID3D12GraphicsCommandList>> m_GraphicsCommandLists;

	ComPtr<ID3D12CommandAllocator> m_MainCommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_MainCommandList;


	// ���α׷��� ��Ʈ�ñ״�ó�� ������ ���� ����
	ComPtr<ID3D12RootSignature> m_RootSignature;

	std::vector<std::shared_ptr<Shader>> m_Shaders;
};

