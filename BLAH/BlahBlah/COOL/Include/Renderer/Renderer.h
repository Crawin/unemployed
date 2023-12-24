#pragma once

class Shader;
class COOLResource;
class Mesh;
class Camera;

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

	// ������ �Ҵ���,����Ʈ�� �ε����� outIndex�� ������
	bool CreateCommandAllocatorAndList(size_t& outIndex);

private:
	// -------------------  Device�� �ϴ� �ϵ� �ܼ� ���� -------------------

	COOLResourcePtr CreateEmpty2DResource(D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_STATES resourceState, const SIZE& size, std::string_view name = "empty2D");
	COOLResourcePtr CreateEmptyBufferResource(D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_STATES resourceState, UINT bytes, std::string_view name = "empty");
	//COOLResourcePtr CreateBufferResource(D3D12_HEAP_TYPE heapType, void* data, UINT bytes, COOLResourcePtr& uploadBuffer) {};	// �ϴ� ����


public:
	// ------------------- commandlist�� �ϴ� �ϵ� ���� -------------------

	// create texture, returns index of texture..	�� �����ܰ迡���� �ҷ���� �Ѵ�. ���ε���۰� ����� ����
	int CreateTextureFromDDSFile(ComPtr<ID3D12GraphicsCommandList> commandList, const wchar_t* fileName, D3D12_RESOURCE_STATES resourceState);

	int LoadMeshFromFile(ComPtr<ID3D12GraphicsCommandList> commandList, const char* fileName);

	// create buffer, returns index of texture..	�� �����ܰ迡���� �ҷ���� �Ѵ�. ���ε���۰� ����� ����
	template <class T>
	int CreateBufferFromVector(ComPtr<ID3D12GraphicsCommandList> commandList, const std::vector<T>& data, D3D12_RESOURCE_STATES resourceState, std::string_view name = "buffer")
	{
		UINT bytes = data.size() * sizeof(T);
		auto resource = CreateEmptyBufferResource(D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_COPY_DEST, bytes, name);
		auto uploadResource = CreateEmptyBufferResource(D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, bytes, std::format("{} upload resource", name));
		uploadResource->DontReleaseOnDesctruct();

		D3D12_SUBRESOURCE_DATA subresourceData = {};
		subresourceData.pData = &(data[0]);
		subresourceData.SlicePitch = subresourceData.RowPitch = bytes;
		UpdateSubresources<1>(commandList.Get(), resource->GetResource(), uploadResource->GetResource(), 0, 0, 1, &subresourceData);

		resource->TransToState(commandList, resourceState);

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

	// �׳� ��¥ ����ִ� ���ҽ� ����, toMapData�� ������ �ڵ����� ������ ���α���
	int CreateEmptyBuffer(D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_STATES resourceState, UINT bytes, std::string_view name = "empty", void** toMapData = nullptr);

	// ���ҽ� ����� subresourceData�� ���� �̰� ���� ������!!!!!!!!!!!!!!!!
	void CopyResource(ComPtr<ID3D12GraphicsCommandList> commandList, COOLResourcePtr src, COOLResourcePtr dest);

	// ------------------- ��Ÿ��� -------------------
	
	// �Ʒ� �� �Լ� ���Ŀ� �ٸ� Ŭ������ ���� �� (Camera���� ������)
	// ������ ��ü�� ����
	void SetViewportScissorRect(ComPtr<ID3D12GraphicsCommandList> commandList);
	// �������� ������� ����
	void SetViewportScissorRect(ComPtr<ID3D12GraphicsCommandList> commandList, UINT numOfViewPort, const D3D12_VIEWPORT& viewport, const RECT& scissorRect);

	// ------------------- ���ҽ� �����ϴ� ���ŵ� ���� -------------------

	// �ش� ���ҽ��� �ε��� ��ȣ�� �ǵ�����
	UINT RegisterShaderResource(COOLResourcePtr resource);

	// Ŀ�ǵ� ����Ʈ execute �ϰ� ���ε����� ������ �����.
	void ExecuteAndEraseUploadHeap(ComPtr<ID3D12GraphicsCommandList> commandList);

	// render, ���� �������� �ٲ� �����̴� ��� ���� ����
	void Render();

	Renderer* GetRendererPtr();

private:
	static const UINT m_NumSwapChainBuffers = 2;
	UINT m_CurSwapChainIndex = 0;

	// ������ ����
	HWND m_hWnd = 0;
	SIZE m_ScreenSize = { 1280,720 };
	bool m_Windowed = true;

	// msaa4x
	UINT m_MsaaQualityLevels = 0;
	bool m_MsaaEnable = true;

	// ��ũ���� ���� ������ ����
	UINT m_CbvSrvDescIncrSize = 0;
	UINT m_RtvDescIncrSize = 0;
	UINT m_DsvDescIncrSize = 0;

	// ����̽�, �ǵ� �ʿ� ����.
	ComPtr<IDXGIFactory4> m_Factory;
	ComPtr<ID3D12Device> m_Device;
	ComPtr<ID3D12Fence> m_Fence;
	ComPtr<IDXGISwapChain3> m_SwapChain;

	// ����Ÿ���� ���� ��ŭ�� �潺 ��ü
	UINT64 m_FenceValues[m_NumSwapChainBuffers] = { 0 };
	HANDLE m_FenceEvent = 0;;

	// rtv, dsv ��ũ���� ��
	ComPtr<ID3D12DescriptorHeap> m_RtvHeap;
	ComPtr<ID3D12DescriptorHeap> m_DsvHeap;

	// ����Ÿ�ٹ���, �������ٽǹ���
	COOLResourcePtr m_RenderTargetBuffer[m_NumSwapChainBuffers];
	COOLResourcePtr m_DepthStencilBuffer;

	// Ŀ�ǵ�ť
	size_t m_MainCommandIdx = 0;
	ComPtr<ID3D12CommandQueue> m_CommandQueue;

	// Ŀ�ǵ帮��Ʈ��
	std::vector<ComPtr<ID3D12CommandAllocator>> m_CommandAllocators;
	std::vector<ComPtr<ID3D12GraphicsCommandList>> m_GraphicsCommandLists;

	// �ַ� ����ϴ� Ŀ�ǵ帮��Ʈ
	ComPtr<ID3D12CommandAllocator> m_MainCommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_MainCommandList;

	// ��Ʈ�ñ״���, ����� �� ����.
	ComPtr<ID3D12RootSignature> m_RootSignature;

	// ���̴���. ���̴� ���� �� ����� ���� ����ٱ�?
	std::vector<std::shared_ptr<Shader>> m_Shaders;
	
	// ���� ��� �ִ� ���ҽ�. �� ����� ���� �̰� ������ �Ѵ�.
	std::vector<COOLResourcePtr> m_VertexIndexDatas;
	std::vector<COOLResourcePtr> m_Resources;

	// ���ҽ���
	ComPtr<ID3D12DescriptorHeap> m_ResourceHeap;
	std::vector<ID3D12Resource*> m_UploadResources;

	// �ӽ�
	std::vector<Mesh> m_Meshes;
	Camera* m_Camera;

public:
	D3D12_GPU_VIRTUAL_ADDRESS GetResourceGPUAddress(int idx);
	D3D12_GPU_VIRTUAL_ADDRESS GetVertexDataGPUAddress(int idx);

	COOLResourcePtr GetResourceFromIndex(int idx);
	COOLResourcePtr GetVertexDataFromIndex(int idx);

	ComPtr<ID3D12CommandAllocator> GetCommandAllocator(size_t idx) { return  m_CommandAllocators[idx]; }
	ComPtr<ID3D12GraphicsCommandList> GetCommandList(size_t idx) { return  m_GraphicsCommandLists[idx]; }
	// �ӽ�
	//void MouseInput(int x, int y);

};

