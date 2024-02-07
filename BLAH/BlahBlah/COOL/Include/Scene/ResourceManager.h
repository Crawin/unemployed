#pragma once
#include "Material/Material.h"
#include "Mesh/Mesh.h"
#include "Object/ObjectBase.h"

class COOLResource;
class Material;
class Mesh;
class Shader;
class Camera;

using COOLResourcePtr = std::shared_ptr<COOLResource>;


// 이따구로 하는 방법 말고는 없을까...
// todo 생각해보자

class ResourceManager
{
public:
	ResourceManager();
	~ResourceManager();

private:
	// for material, returns index
	int GetTexture(ComPtr<ID3D12GraphicsCommandList> commandList, const std::string& name);
	int GetMaterial(const std::string& name, ComPtr<ID3D12GraphicsCommandList> commandList = nullptr);
	friend bool Material::LoadFile(ComPtr<ID3D12GraphicsCommandList> cmdList, const std::string& fileName, ResourceManager* manager, std::string& shaderName);

	// for mesh
	int GetMesh(const std::string& name, ComPtr<ID3D12GraphicsCommandList> commandList = nullptr);
	void AddMesh(Mesh* mesh) { m_Meshes.push_back(mesh); }
	friend void Mesh::BuildMesh(ComPtr<ID3D12GraphicsCommandList> commandList, std::ifstream& file, ResourceManager* manager);

	// for object
	int CreateObjectResource(UINT size, const std::string resName, void** toMapData);
	friend void Camera::Init(ResourceManager* resourceManager);
	
	template <class T>
	int CreateBufferFromVector(ComPtr<ID3D12GraphicsCommandList> commandList, const std::vector<T>& data, D3D12_RESOURCE_STATES resourceState, std::string_view name = "buffer")
	{
		COOLResourcePtr ptr = Renderer::GetInstance().CreateBufferFromVector(commandList, data, resourceState, name);

		m_VertexIndexDatas.push_back(ptr);
		
		return m_VertexIndexDatas.size() - 1;
	}

	bool LoadFile(const std::string& sceneName);

public:
	bool Init(const std::string& sceneName);

	// resource
	D3D12_GPU_VIRTUAL_ADDRESS GetVertexDataGPUAddress(int idx);

	// 렌더 전 디스크립터테이블 set
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

