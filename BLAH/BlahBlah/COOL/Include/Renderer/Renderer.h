#pragma once

class Shader;
class COOLResource;
class Mesh;

using COOLResourcePtr = std::shared_ptr<COOLResource>;


class Renderer
{
public:
	static Renderer& GetInstance() {
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

private:
	// -------------------  Device가 하는 일들 단순 묶음 -------------------

	COOLResourcePtr CreateEmpty2DResource(D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_STATES resourceState, const SIZE& size);
	COOLResourcePtr CreateEmptyBufferResource(D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_STATES resourceState, UINT bytes);
	//COOLResourcePtr CreateBufferResource(D3D12_HEAP_TYPE heapType, void* data, UINT bytes, COOLResourcePtr& uploadBuffer) {};	// 일단 없앰


public:
	// ------------------- commandlist가 하는 일들 묶음 -------------------

	// create texture, returns index of texture..	씬 생성단계에서만 불러줘야 한다. 업로드버퍼가 생기니 주의
	int CreateTextureFromDDSFile(ComPtr<ID3D12GraphicsCommandList> commandList, const wchar_t* fileName, D3D12_RESOURCE_STATES resourceState);

	int LoadMeshFromFile(ComPtr<ID3D12GraphicsCommandList> commandList, const char* fileName);

	// create buffer, returns index of texture..	씬 생성단계에서만 불러줘야 한다. 업로드버퍼가 생기니 주의
	template <class T>
	int CreateBufferFromVector(ComPtr<ID3D12GraphicsCommandList> commandList, const std::vector<T>& data, D3D12_RESOURCE_STATES resourceState)
	{
		UINT bytes = data.size() * sizeof(T);
		auto resource = CreateEmptyBufferResource(D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_COPY_DEST, bytes);
		auto uploadResource = CreateEmptyBufferResource(D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, bytes);
		uploadResource->DontReleaseOnDesctruct();

		//D3D12_SUBRESOURCE_DATA subresourceData = {};
		//subresourceData.pData = &(data[0]);
		//subresourceData.SlicePitch = subresourceData.RowPitch = bytes;
		//UpdateSubresources<1>(commandList.Get(), resource->GetResource(), uploadResource->GetResource(), 0, 0, 1, &subresourceData);

		//resource->TransToState(commandList, resourceState);

		m_UploadResources.push_back(uploadResource->GetResource());
		if (resourceState == D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER ||
			resourceState == D3D12_RESOURCE_STATE_INDEX_BUFFER) {
			m_VertexIndexDatas.push_back(resource);
			return m_VertexIndexDatas.size() - 1;
		}
		else {
			m_Resources.push_back(resource);
			return m_Resources.size() - 1;
		}

		//::ZeroMemory(&d3dSubResourceData, sizeof(D3D12_SUBRESOURCE_DATA));
		//d3dSubResourceData.pData = pData;
		//d3dSubResourceData.SlicePitch = d3dSubResourceData.RowPitch = nBytes;
		//::UpdateSubresources<1>(pd3dCommandList, pd3dBuffer, *ppd3dUploadBuffer, 0, 0, 1, &d3dSubResourceData);

		//UpdateSubresources(commandList.Get(), texture, uploadResource, 0, 0, subResources.size(), &subResources[0]);
	}

	// 리소스 복사는 subresourceData로 하자 이건 보류 사용금지!!!!!!!!!!!!!!!!
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
	std::vector<COOLResourcePtr> m_VertexIndexDatas;

	// 리소스힙 관련
	std::vector<ID3D12Resource*> m_UploadResources;

	std::vector<COOLResourcePtr> m_Resources;
	ComPtr<ID3D12DescriptorHeap> m_ResourceHeap;

	// 임시
	std::vector<Mesh> m_Meshes;

};

