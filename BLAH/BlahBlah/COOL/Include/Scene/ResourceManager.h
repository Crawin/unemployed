#pragma once

class COOLResource;
class Material;
class Mesh;
class Shader;

using COOLResourcePtr = std::shared_ptr<COOLResource>;

class ResourceManager
{
public:
	ResourceManager();
	~ResourceManager();

private:

public:
	// for object
	int CreateObjectResource(UINT size, const std::string resName, void** toMapData);

	// for material, returns index
	int GetTexture(ComPtr<ID3D12GraphicsCommandList> commandList, const std::string& name);
	int GetMaterial(const std::string& name, ComPtr<ID3D12GraphicsCommandList> commandList = nullptr);
	
	// for mesh
	int GetMesh(const std::string& name, ComPtr<ID3D12GraphicsCommandList> commandList = nullptr);
	void AddMesh(Mesh* mesh) { m_Meshes.push_back(mesh); }

	// resource
	D3D12_GPU_VIRTUAL_ADDRESS GetVertexDataGPUAddress(int idx);
	
	template <class T>
	int CreateBufferFromVector(ComPtr<ID3D12GraphicsCommandList> commandList, const std::vector<T>& data, D3D12_RESOURCE_STATES resourceState, std::string_view name = "buffer")
	{
		COOLResourcePtr ptr = Renderer::GetInstance().CreateBufferFromVector(commandList, data, resourceState, name);

		m_VertexIndexDatas.push_back(ptr);
		
		return m_VertexIndexDatas.size() - 1;
	}

	void SetDatas();


private:
	// 메시 데이터
	std::vector<COOLResourcePtr> m_VertexIndexDatas;

	// 텍스쳐 데이터
	std::vector<COOLResourcePtr> m_Resources;

	// 업로드
	std::vector<ID3D12Resource*> m_UploadResources;

	// 리소스힙, t1번 슬롯에 set
	ComPtr<ID3D12DescriptorHeap> m_ShaderResourceHeap;

	// mesh
	std::vector<Mesh*> m_Meshes;

	// material, texture
	std::vector<Material*> m_Materials;
	std::map<std::string, int> m_TextureIndexMap;

	// shader
	std::vector<Shader*> m_Shaders;

};

