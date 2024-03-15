#pragma once
#include "Material/Material.h"
#include "Mesh/Mesh.h"
#include "Mesh/Bone.h"
//#include "Object/ObjectBase.h"

class COOLResource;
class Material;
class Mesh;
class Shader;
class Bone;
//class Camera;

class ECSManager;

using COOLResourcePtr = std::shared_ptr<COOLResource>;

class Entity;

namespace Json { class Value; }

namespace component { 
	class Component;
	class Renderer;
	class Camera;
}

struct ToLoadRendererInfo {
	std::string m_MeshName;
	std::string m_MaterialName;
	component::Renderer* m_Renderer = nullptr;
};

// 이따구로 하는 방법 말고는 없을까...
// todo 생각해보자

class ResourceManager
{
public:
	ResourceManager();
	~ResourceManager();

private:
	// 임시, todo 나중에 지울것
	friend class Scene;
	friend class SceneManager;

	friend bool Material::LoadFile(ComPtr<ID3D12GraphicsCommandList> cmdList, const std::string& fileName, ResourceManager* manager, std::string& shaderName);

	friend void Mesh::BuildMesh(ComPtr<ID3D12GraphicsCommandList> commandList, std::ifstream& file, const std::string& fileName, ResourceManager* manager);

	template<typename VERTEX>
	friend void Mesh::LoadVertices(ComPtr<ID3D12GraphicsCommandList> commandList, std::ifstream& file, ResourceManager* manager, int vtxSize);


	friend void Bone::LoadBone(ComPtr<ID3D12GraphicsCommandList> commandList, std::ifstream& file, ResourceManager* manager);

private:

	// for material, returns index
	int GetTexture(ComPtr<ID3D12GraphicsCommandList> commandList, const std::string& name);

	// for mesh
	void AddMesh(Mesh* mesh) { m_Meshes.push_back(mesh); }

	template <class T>
	int CreateBufferFromVector(ComPtr<ID3D12GraphicsCommandList> commandList, const std::vector<T>& data, D3D12_RESOURCE_STATES resourceState, std::string_view name = "buffer", RESOURCE_TYPES toInsert = RESOURCE_TYPES::VERTEX)
	{
		COOLResourcePtr ptr = Renderer::GetInstance().CreateBufferFromVector(commandList, data, resourceState, name);

		switch (toInsert) {
		case RESOURCE_TYPES::SHADER:
			m_Resources.push_back(ptr);
			return static_cast<int>(m_Resources.size() - 1);
			break;
		case RESOURCE_TYPES::VERTEX:
			m_VertexIndexDatas.push_back(ptr);
			return static_cast<int>(m_VertexIndexDatas.size() - 1);
			break;
		case RESOURCE_TYPES::OBJECT:
			m_ObjectDatas.push_back(ptr);
			return static_cast<int>(m_ObjectDatas.size() - 1);
			break;
		}

		return -1;
	}

	int CreateEmptyBuffer(ComPtr<ID3D12GraphicsCommandList> commandList, int size, int stride, D3D12_RESOURCE_STATES resourceState, std::string_view name = "buffer", RESOURCE_TYPES toInsert = RESOURCE_TYPES::SHADER);

	// object 폴더의 file 불러오는 함수
	bool LoadObjectFile(const std::string& sceneName, bool isCam = false);

	// Json을 파싱해 오브젝트를 실제로 만드는 함수
	Entity* LoadObjectJson(Json::Value& root, Entity* parent = nullptr);

	int LoadMesh(const std::string& name, ComPtr<ID3D12GraphicsCommandList> commandList);

	int LoadMaterial(const std::string& name, ComPtr<ID3D12GraphicsCommandList> commandList);

	int LoadBone(const std::string& name, ComPtr<ID3D12GraphicsCommandList> commandList);

	std::shared_ptr<Shader> LoadShader(const std::string& name, ComPtr<ID3D12GraphicsCommandList> commandList);

	// mesh/material 에 로드 할 것들이 남아 있다면 로드
	bool LateInit(ComPtr<ID3D12GraphicsCommandList> commandList);

	// objects
	bool LoadObjects(const std::string& sceneName, ComPtr<ID3D12GraphicsCommandList> commandList);

	// camera
	bool LoadCameras(const std::string& sceneName, ComPtr<ID3D12GraphicsCommandList> commandList);

	// for deferred renderer
	bool MakeExtraRenderTarget();

public:
	void SetECSManager(std::shared_ptr<ECSManager> ptr);

	// 씬 생성시 최초 초기화
	bool Init(ComPtr<ID3D12GraphicsCommandList> commandList, const std::string& sceneName);

	// for object
	int CreateObjectResource(UINT size, const std::string resName, void** toMapData);

	// resource
	D3D12_GPU_VIRTUAL_ADDRESS GetResourceDataGPUAddress(RESOURCE_TYPES resType, int idx);

	// 렌더 전 디스크립터테이블 set
	void SetDatas();
	
	// get datas
	int GetMesh(const std::string& name, ComPtr<ID3D12GraphicsCommandList> commandList = nullptr);
	int GetMaterial(const std::string& name, ComPtr<ID3D12GraphicsCommandList> commandList = nullptr);
	int GetBone(const std::string& name, ComPtr<ID3D12GraphicsCommandList> commandList);
	std::shared_ptr<Shader> GetShader(const std::string& name, ComPtr<ID3D12GraphicsCommandList> commandList = nullptr);

	// for late init
	void AddLateLoad(const std::string& mesh, const std::string& material, component::Renderer* renderer);
	
	// for multiple render target / post processing
	void SetMRTStates(ComPtr<ID3D12GraphicsCommandList> cmdList, D3D12_RESOURCE_STATES toState);
	void ClearMRTS(ComPtr<ID3D12GraphicsCommandList> cmdList, const float color[4]);
	D3D12_CPU_DESCRIPTOR_HANDLE GetDefferedRenderTargetStart() const;
	int GetPostProcessingMaterial() const;

	// manual 
	void SetResourceState(ComPtr<ID3D12GraphicsCommandList> cmdList, RESOURCE_TYPES resType, int idx, D3D12_RESOURCE_STATES toState);

	void SetMainCamera(component::Camera* cam) { m_MainCamera = cam; }
	
#ifdef _DEBUG
	int GetDebuggingMaterial() const { return m_DebuggingMaterial; };
#endif

private:
	// 메시 데이터
	std::vector<COOLResourcePtr> m_VertexIndexDatas;

	// object 데이터
	std::vector<COOLResourcePtr> m_ObjectDatas;

	// 텍스쳐 데이터
	std::vector<COOLResourcePtr> m_Resources;

	// 업로드리소스, 초기화 완료 후 전체 삭제
	std::vector<ID3D12Resource*> m_UploadResources;

	// 리소스힙, t0번 슬롯에 set
	ComPtr<ID3D12DescriptorHeap> m_ShaderResourceHeap;
	ComPtr<ID3D12DescriptorHeap> m_MRTHeap;

	////////////////////////////////////////////////////////
	// ECS SYSTEM
	////////////////////////////////////////////////////////
	// Object
	std::vector<Entity*> m_Entities;
	//std::vector<Entity*> m_RootEntities;

	// component
	//std::vector<component::Component*> m_Components;

	// ECS System
	std::shared_ptr<ECSManager> m_ECSManager = nullptr;

	////////////////////////////////////////////////////////
	// ECS SYSTEM END
	////////////////////////////////////////////////////////

	// mesh
	std::vector<Mesh*> m_Meshes;
	std::vector<Bone*> m_Bones;

	// material, texture
	std::vector<Material*> m_Materials;
	std::map<std::string, int> m_TextureIndexMap;

	// shader
	std::vector<std::shared_ptr<Shader>> m_Shaders;

	// animation stream out shader
	std::shared_ptr<Shader> m_AnimationShader;

	// Deffered render targets
	// 해당갯수 만큼 m_Resources에 넣음
	int m_DefferedRenderTargets = static_cast<int>(MULTIPLE_RENDER_TARGETS::MRT_END);
	int m_DefferedRTVStartIdx = -1;
	//D3D12_CPU_DESCRIPTOR_HANDLE m_DefferedRTVStart = D3D12_CPU_DESCRIPTOR_HANDLE();

	// postProcessingMaterial;
	const char* m_PostProcessing = "PostProcessing";
	int m_PostProcessingMaterial = -1;

#ifdef _DEBUG
	const char* m_Debuggging = "ForDebug";
	int m_DebuggingMaterial = -1;
#endif

	// 로드 해야 할 mesh, material들을 저장만 해둔 후 나중에 로드 한다.
	// 추가 설명
	// mesh idx / material idx 는 오브젝트 로드시 부여되기에
	// 추후에 late init 이후에 renderer에게 전달하기 위해
	// 
	// 
	std::vector<ToLoadRendererInfo> m_ToLoadRenderDatas;
	std::vector<std::string> m_ToLoadMaterials;

	//std::vector<Entity*> m_Cameras;
	component::Camera* m_MainCamera = nullptr;
};

