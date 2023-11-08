#pragma once

class Shader;
class COOLResource;

using COOLResourcePtr = std::shared_ptr<COOLResource>;


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
	bool CreateResourceDescriptorHeap();
	bool LoadShaders();

public:
	bool Init(const SIZE& wndSize, HWND hWnd);

	// 생성된 할당자,리스트의 인덱스를 outIndex로 돌려줌
	bool CreateCommandAllocatorAndList(size_t& outIndex);

	// -------------------  Device가 하는 일들 단순 묶음 -------------------

	COOLResourcePtr CreateEmpty2DResource(D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_STATES resourceState, const SIZE& size);
	COOLResourcePtr CreateEmptyBufferResource(D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_STATES resourceState, UINT bytes);
	//COOLResourcePtr CreateBufferResource(D3D12_HEAP_TYPE heapType, void* data, UINT bytes, COOLResourcePtr& uploadBuffer) {};	// 일단 없앰

	// ------------------- commandlist가 하는 일들 묶음 -------------------

	// create texture, returns index of texture..	씬 생성단계에서만 불러줘야 한다. 업로드버퍼가 생기니 주의
	int CreateTextureFromDDSFile(ComPtr<ID3D12GraphicsCommandList> commandList, const wchar_t* fileName, D3D12_RESOURCE_STATES resourceState);

	// 리소스 복사는 subresourceData로 하자 이건 보류
	void CopyResource(ComPtr<ID3D12GraphicsCommandList> commandList, COOLResourcePtr src, COOLResourcePtr dest);

	// ------------------- 기타등등 -------------------
	
	// 아래 두 함수 추후에 다른 클래스로 빼야 함 (Camera같은 곳으로)
	// 윈도우 전체에 설정
	void SetViewportScissorRect();
	// 지정해준 사이즈로 설정
	void SetViewportScissorRect(UINT numOfViewPort, const D3D12_VIEWPORT& viewport, const RECT& scissorRect);

	// ------------------- 리소스 관리하는 저거들 묶음 -------------------

	// 해당 리소스의 인덱스 번호를 되돌려줌
	UINT RegisterShaderResource(COOLResourcePtr resource);


	// render
	void Render();

	Renderer* GetRendererPtr();

private:
	static const UINT m_NumSwapChainBuffers = 2;
	UINT m_CurSwapChainIndex = 0;

	HWND m_hWnd = 0;
	SIZE m_ScreenSize = { 1280,720 };
	bool m_Windowed = true;

	// msaa4x
	UINT m_MsaaQualityLevels = 0;
	bool m_MsaaEnable = true;

	UINT m_CbvSrvDescIncrSize = 0;
	UINT m_RtvDescIncrSize = 0;
	UINT m_DsvDescIncrSize = 0;

	ComPtr<IDXGIFactory4> m_Factory;
	ComPtr<ID3D12Device> m_Device;
	ComPtr<ID3D12Fence> m_Fence;
	ComPtr<IDXGISwapChain3> m_SwapChain;

	UINT64 m_FenceValues[m_NumSwapChainBuffers] = { 0 };
	HANDLE m_FenceEvent = 0;;

	ComPtr<ID3D12DescriptorHeap> m_RtvHeap;
	ComPtr<ID3D12DescriptorHeap> m_DsvHeap;

	COOLResourcePtr m_RenderTargetBuffer[m_NumSwapChainBuffers];
	COOLResourcePtr m_DepthStencilBuffer;

	size_t m_MainCommandIdx = 0;
	ComPtr<ID3D12CommandQueue> m_CommandQueue;

	std::vector<ComPtr<ID3D12CommandAllocator>> m_CommandAllocators;
	std::vector<ComPtr<ID3D12GraphicsCommandList>> m_GraphicsCommandLists;

	ComPtr<ID3D12CommandAllocator> m_MainCommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_MainCommandList;


	ComPtr<ID3D12RootSignature> m_RootSignature;

	std::vector<std::shared_ptr<Shader>> m_Shaders;

	// 리소스힙 관련
	std::vector<ID3D12Resource*> m_UploadResources;

	std::vector<COOLResourcePtr> m_Resources;
	ComPtr<ID3D12DescriptorHeap> m_ResourceHeap;
};

