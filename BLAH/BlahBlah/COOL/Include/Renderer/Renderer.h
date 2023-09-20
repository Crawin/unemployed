#pragma once
#include <vector>
// dx12 렌더러

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

	// 생성된 할당자,리스트의 인덱스를 outIndex로 돌려줌
	bool CreateCommandAllocatorAndList(size_t& outIndex);

	// 윈도우 전체에 설정
	void SetViewportScissorRect();
	// 지정해준 사이즈로 설정
	void SetViewportScissorRect(UINT numOfViewPort, const D3D12_VIEWPORT& viewport, const RECT& scissorRect);

	void ResourceTransition(ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES transTo);

	// render
	void Render();

private:
	// 스왑체인 관련
	static const UINT m_NumSwapChainBuffers = 2;
	UINT m_CurSwapChainIndex = 0;

	// 윈도우
	HWND m_hWnd = 0;
	SIZE m_ScreenSize = { 1280,720 };
	bool m_Windowed = true;

	// msaa4x
	UINT m_MsaaQualityLevels = 0;
	bool m_MsaaEnable = true;

	// 디스크립터 증가 사이즈
	UINT m_CbvSrvDescIncrSize = 0;
	UINT m_RtvDescIncrSize = 0;
	UINT m_DsvDescIncrSize = 0;

	ComPtr<IDXGIFactory4> m_Factory;
	ComPtr<ID3D12Device> m_Device;
	ComPtr<ID3D12Fence> m_Fence;
	ComPtr<IDXGISwapChain3> m_SwapChain;

	UINT64 m_FenceValues[m_NumSwapChainBuffers];
	HANDLE m_FenceEvent;

	// 여기 아래는 분해해도 되나?

	ComPtr<ID3D12DescriptorHeap> m_RtvHeap;
	ComPtr<ID3D12DescriptorHeap> m_DsvHeap;

	ComPtr<COOLResource> m_RenderTargetBuffer[m_NumSwapChainBuffers];
	ComPtr<COOLResource> m_DepthStencilBuffer;

	size_t m_MainCommandIdx = 0;
	ComPtr<ID3D12CommandQueue> m_CommandQueue;
	// 아모른직다 가능성은 열어두자, 그림자맵 정도는 동시에 만들어도?

	std::vector<ComPtr<ID3D12CommandAllocator>> m_CommandAllocators;
	std::vector<ComPtr<ID3D12GraphicsCommandList>> m_GraphicsCommandLists;

	ComPtr<ID3D12CommandAllocator> m_MainCommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_MainCommandList;


	// 프로그램의 루트시그니처는 변하지 않을 예정
	ComPtr<ID3D12RootSignature> m_RootSignature;

	std::vector<std::shared_ptr<Shader>> m_Shaders;
};

