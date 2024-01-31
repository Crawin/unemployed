#pragma once

class COOLResource;
class Material;
class Mesh;

using COOLResourcePtr = std::shared_ptr<COOLResource>;

class ResourceManager
{
public:
	ResourceManager();
	~ResourceManager();

private:
	void BuildMesh(ComPtr<ID3D12GraphicsCommandList> commandList, std::ifstream& file, Mesh* mesh);

public:
	// for object
	int CreateObjectResource(UINT size, const std::string resName, void** toMapData);

	void SetDatas();

	// for material, returns index
	int GetTexture(ComPtr<ID3D12GraphicsCommandList> commandList, const std::string& name);
	int GetMaterial(const std::string& name, ComPtr<ID3D12GraphicsCommandList> commandList = nullptr);
	
	// for mesh

	int GetMesh(const std::string& name, ComPtr<ID3D12GraphicsCommandList> commandList = nullptr);

	template<class Type>
	int Get() 
	{

	}

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

};

