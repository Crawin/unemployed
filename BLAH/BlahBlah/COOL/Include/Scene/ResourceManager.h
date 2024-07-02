#pragma once
#include "Renderer/Renderer.h"
#include "Material/Material.h"
#include "Light.h"
#include "Mesh/Mesh.h"
#include "Mesh/Bone.h"
#include "Animation/Animation.h"
#include "Animation/AnimationPlayer.h"
#include "Lighting/ShadowMap.h"
//#include "Object/ObjectBase.h"

class Renderer;
class COOLResource;
class Material;
class Mesh;
class Shader;
class Bone;
class ShadowMap;

class ECSManager;

using COOLResourcePtr = std::shared_ptr<COOLResource>;

class Entity;

namespace Json { class Value; }

namespace component { 
	class Component;
	class Renderer;
	class UIRenderer;
	class Camera;
	class AnimationController;
	class AnimationExecutor;
	class Attach;
	class Light;
}

struct ToLoadRendererInfo {
	std::string m_MeshName;
	std::string m_MaterialName;
	component::Renderer* m_Renderer = nullptr;
};

struct ToLoadUIRendererInfo {
	std::string m_MaterialName;
	component::UIRenderer* m_Renderer = nullptr;
};

struct ToLoadAnimControllerInfo {
	std::string m_AnimSetName;
	component::AnimationController* m_Controller;
};

struct ToLoadAnimExecutorInfo {
	std::string m_AnimSetName;
	component::AnimationExecutor* m_Executor;
};

struct ToLoadAttachInfo {
	std::string m_AnimSetName;
	component::Attach* m_Attach;
	std::string boneName;
	int idx;
};

struct CameraRenderTargets {
	int m_MRTStartIdx = -1;
	int m_MRTNum = -1;
	COOLResourcePtr m_DepthBuffer = nullptr;
	ComPtr<ID3D12DescriptorHeap> m_MRTHeap;
	ComPtr<ID3D12DescriptorHeap> m_DsvHeap;

	int m_ResultRenderTargetIndex = -1;
	ComPtr<ID3D12DescriptorHeap> m_ResultRenderTargetHeap;
};

//struct ToLoadLightDataInfo {
//	int idx;
//	component::Light* m_Light;
//};

// 이따구로 하는 방법 말고는 없을까...
// todo 생각해보자

class ResourceManager
{
public:
	ResourceManager();
	~ResourceManager();

private:
	// 임시, todo 나중에 지울것
	//friend class Scene;
	//friend class TestMainScene;
	friend class SceneManager;

	friend bool Material::LoadFile(ComPtr<ID3D12GraphicsCommandList> cmdList, const std::string& fileName, ResourceManager* manager, std::string& shaderName);

	friend void Mesh::BuildMesh(ComPtr<ID3D12GraphicsCommandList> commandList, std::ifstream& file, const std::string& fileName, ResourceManager* manager);

	template<typename VERTEX>
	friend void Mesh::LoadVertices(ComPtr<ID3D12GraphicsCommandList> commandList, std::ifstream& file, ResourceManager* manager, int vtxSize);

	friend void Bone::LoadBone(ComPtr<ID3D12GraphicsCommandList> commandList, std::ifstream& file, ResourceManager* manager);

	friend void Animation::LoadAnimation(ComPtr<ID3D12GraphicsCommandList> commandList, std::ifstream& file, ResourceManager* manager);

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
			ptr->SetShaderResource();
			m_Resources.push_back(ptr);
			return static_cast<int>(m_Resources.size() - 1);
			break;
		case RESOURCE_TYPES::VERTEX:
			m_VertexIndexDatas.push_back(ptr);
			return static_cast<int>(m_VertexIndexDatas.size() - 1);
			break;
		case RESOURCE_TYPES::OBJECT:
			//ptr->SetConstant();
			m_ObjectDatas.push_back(ptr);
			return static_cast<int>(m_ObjectDatas.size() - 1);
			break;
		}

		return -1;
	}

	int CreateBufferFromData(ComPtr<ID3D12GraphicsCommandList> commandList, char* data, int stride, int numOfData, D3D12_RESOURCE_STATES resourceState, std::string_view name = "buffer", RESOURCE_TYPES toInsert = RESOURCE_TYPES::VERTEX);

	int CreateEmptyBuffer(ComPtr<ID3D12GraphicsCommandList> commandList, int size, int stride, D3D12_RESOURCE_STATES resourceState, std::string_view name = "buffer", RESOURCE_TYPES toInsert = RESOURCE_TYPES::SHADER, D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_DEFAULT);

	bool LoadObjectFile(const std::string& sceneName, bool isCam = false);	// object 폴더의 file 불러오는 함수
	Entity* LoadObjectJson(Json::Value& root, Entity* parent = nullptr);	// Json을 파싱해 오브젝트를 실제로 만드는 함수
	int LoadMesh(const std::string& name, ComPtr<ID3D12GraphicsCommandList> commandList);
	int LoadMaterial(const std::string& name, ComPtr<ID3D12GraphicsCommandList> commandList);
	int LoadBone(const std::string& name, ComPtr<ID3D12GraphicsCommandList> commandList);
	int LoadAnimation(const std::string& name, ComPtr<ID3D12GraphicsCommandList> commandList);
	std::shared_ptr<Shader> LoadShader(const std::string& name, ComPtr<ID3D12GraphicsCommandList> commandList);

	bool LoadDefferedResource(ComPtr<ID3D12GraphicsCommandList> commandList);
	bool LoadLateInitMesh(ComPtr<ID3D12GraphicsCommandList> commandList);
	bool LoadLateInitAnimation(ComPtr<ID3D12GraphicsCommandList> commandList);
	bool MakeLightData(ComPtr<ID3D12GraphicsCommandList> commandList);					// todo 카메라마다 만들어줘야 할지 고민해보자
	bool MakeShadowMaps();
	bool LoadShadowMappingResource(ComPtr<ID3D12GraphicsCommandList> commandList);

	// mesh/material 에 로드 할 것들이 남아 있다면 로드
	bool LateInit(ComPtr<ID3D12GraphicsCommandList> commandList);

	// objects
	bool LoadObjects(const std::string& sceneName, ComPtr<ID3D12GraphicsCommandList> commandList);
	bool LoadCameras(const std::string& sceneName, ComPtr<ID3D12GraphicsCommandList> commandList);

	// for shadow map
	bool MakeShadowMapRenderTargets();

	// for mrts, returns start idx
	bool MakeMultipleRenderTargets(CameraRenderTargets& camData);

public:
	void SetECSManager(std::shared_ptr<ECSManager> ptr);

	// 씬 생성시 최초 초기화
	bool Init(ComPtr<ID3D12GraphicsCommandList> commandList, const std::string& sceneName);

	// for object
	int CreateObjectResource(UINT size, const std::string resName, void** toMapData, D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_UPLOAD, RESOURCE_TYPES toInsert = RESOURCE_TYPES::OBJECT);

	// resource
	D3D12_GPU_VIRTUAL_ADDRESS GetResourceDataGPUAddress(RESOURCE_TYPES resType, int idx);

	// 렌더 전 디스크립터테이블 set, 사용하지 않는듯?
	void SetResourceHeap(ComPtr<ID3D12GraphicsCommandList> commandList);
	
	// get datas
	int GetMesh(const std::string& name, ComPtr<ID3D12GraphicsCommandList> commandList = nullptr);
	int GetMaterial(const std::string& name, ComPtr<ID3D12GraphicsCommandList> commandList = nullptr);
	int GetBone(const std::string& name, ComPtr<ID3D12GraphicsCommandList> commandList);
	int GetAnimation(const std::string& name, ComPtr<ID3D12GraphicsCommandList> commandList);
	std::shared_ptr<Shader> GetShader(const std::string& name, ComPtr<ID3D12GraphicsCommandList> commandList = nullptr);

	Mesh* GetMesh(int idx) { return m_Meshes[idx]; }
	Material* GetMaterial(int idx) { return m_Materials[idx]; }
	Bone* GetBone(int idx) { return m_Bones[idx]; }
	std::shared_ptr<Shader> GetAnimationShader() { return m_AnimationShader; }
	// on scene::render
	//void SetPSO(ComPtr<ID3D12GraphicsCommandList> commandList, int materialIdx);
	//void RenderMesh(ComPtr<ID3D12GraphicsCommandList> commandList, int meshIdx, const XMFLOAT4X4& parentMatrix);

	// for late init
	void AddLateLoad(const std::string& mesh, const std::string& material, component::Renderer* renderer);
	void AddLateLoadUI(const std::string& material, component::UIRenderer* renderer);
	
	// for late init anim controller
	void AddLateLoadAnimController(const std::string& fileName, component::AnimationController* controller);

	// for late init anim executor
	void AddLateLoadAnimExecutor(const std::string& fileName, component::AnimationExecutor* executor);
	
	// for late init anim executor
	void AddLateLoadAttach(const std::string& fileName, component::Attach* executor, const std::string& bone, int idx);

	// for light components
	void AddLightData();

	// for camera components
	int AddCamera(int numRenderTargets);

	// for multiple render target / post processing
	void SetMRTStates(ComPtr<ID3D12GraphicsCommandList> cmdList, D3D12_RESOURCE_STATES toState, int cameraIdx);
	void ClearMRTS(ComPtr<ID3D12GraphicsCommandList> cmdList, const float color[4], int cameraIdx);
	D3D12_CPU_DESCRIPTOR_HANDLE GetDefferedRenderTargetStart(int cameraIdx) const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetDefferedDSV(int cameraIdx) const;

	int GetPostProcessingMaterial() const;
	int GetCameraRenderTargetIndex(int camIdx, MULTIPLE_RENDER_TARGETS rtType) const;
	void SetCameraToPostProcessing(int camIdx);

	// for shadowmap
	void SetShadowMapStates(ComPtr<ID3D12GraphicsCommandList> cmdList, D3D12_RESOURCE_STATES toState);
	void ClearShadowMaps(ComPtr<ID3D12GraphicsCommandList> cmdList, const float color[4]);
	int GetUnOccupiedShadowMapRenderTarget(LIGHT_TYPES lightType);				// todo light type이 point light일 때에도 대응 해야함
	D3D12_CPU_DESCRIPTOR_HANDLE GetShadowMapRenderTarget(int idx) const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetShadowMapDepthStencil() const;

	int GetShadowMapCamIdx(int idx);
	void SetShadowMapCamera(ComPtr<ID3D12GraphicsCommandList> cmdList, int idx) const;
	void UpdateShadowMapView(int idx, const LightData& light);
	int GetShadowMappingMaterial() const;
	BoundingFrustum* GetShadowMapFrustum(int idx);

	int GetShadowMapRTVIdx(int idx);
	void SetShadowMapRTVIdx(int idx, int rtvIdx);
	void UnOccupyShadowRTVs();

	int GetShadowMapRTVStartIdx() const { return m_ShadowMapRTVStartIdx; }

	// manual 
	void SetResourceState(ComPtr<ID3D12GraphicsCommandList> cmdList, RESOURCE_TYPES resType, int idx, D3D12_RESOURCE_STATES toState);

	void SetMainCamera(component::Camera* cam) { m_MainCamera = cam; }
	
#ifdef _DEBUG
	int GetDebuggingMaterial() const { return m_DebuggingMaterial; };
#endif

private:
	// 메시 데이터
	std::vector<COOLResourcePtr> m_VertexIndexDatas;

	// 상수버퍼 데이터
	std::vector<COOLResourcePtr> m_ObjectDatas;

	// 셰이더 데이터 t register
	std::vector<COOLResourcePtr> m_Resources;

	// 업로드리소스, 초기화 완료 후 전체 삭제
	std::vector<ID3D12Resource*> m_UploadResources;

	// 리소스힙, t0번 슬롯에 set
	ComPtr<ID3D12DescriptorHeap> m_ShaderResourceHeap;

	// 추가 렌더타겟들
	std::vector<CameraRenderTargets> m_CameraRenderTargets;
	//ComPtr<ID3D12DescriptorHeap> m_MRTHeap;
	ComPtr<ID3D12DescriptorHeap> m_ShadowMapHeap;
	ComPtr<ID3D12DescriptorHeap> m_ShadowMapDSVHeap;
	COOLResourcePtr m_ShadowDSV;
	////////////////////////////////////////////////////////
	// ECS SYSTEM
	////////////////////////////////////////////////////////
	// Object
	std::vector<Entity*> m_Entities;
	//std::vector<Entity*> m_RootEntities;

	// component
	std::vector<component::Component*> m_Components;

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
	std::vector<AnimationPlayer*> m_AnimationPlayer;
	std::vector<std::shared_ptr<Animation>> m_Animations;

	// Deffered render targets
	// 해당갯수 만큼 m_Resources에 넣음
	//static const int m_DefferedRenderTargets = static_cast<int>(MULTIPLE_RENDER_TARGETS::MRT_END);
	//int m_DefferedRTVStartIdx = -1;
	//D3D12_CPU_DESCRIPTOR_HANDLE m_DefferedRTVStart = D3D12_CPU_DESCRIPTOR_HANDLE();

	// cascaded + other
	static const int m_ShadowMapRenderTargets = 4;
	int m_ShadowMapRTVStartIdx = -1;
	int m_ShadowMapDSVStartIdx = -1;

	// postProcessingMaterial;
	const char* m_PostProcessing = "_PostProcessing";
	int m_PostProcessingMaterial = -1;

	// shadowMappingMaterial;
	const char* m_ShadowMapping = "_ShadowMapping";
	int m_ShadowMappingMaterial = -1;

#ifdef _DEBUG
	const char* m_Debuggging = "_ForDebug";
	int m_DebuggingMaterial = -1;
#endif

public:
	// light datas
	//std::vector<LightData> m_LightDatas;
	std::vector<ShadowMap> m_ShadowMaps;
	bool m_ShadowMapOccupied[m_ShadowMapRenderTargets] = {};

	LightData* m_MappedLightData;

	int m_LightSize = 0;
	int m_LightIdx = -1;

private:

	// 로드 해야 할 mesh, material들을 저장만 해둔 후 나중에 로드 한다.
	// 추가 설명
	// mesh idx / material idx 는 오브젝트 로드시 부여되기에
	// 추후에 late init 이후에 renderer에게 전달하기 위해
	// 
	// 
	std::vector<ToLoadRendererInfo> m_ToLoadRenderDatas;
	std::vector<ToLoadUIRendererInfo> m_ToLoadUIDatas;

	// ToLoadRenderer의 Animation판
	// 최초 컴포넌트 생성시에 여기에다 넣어두고
	// late init에서 로드
	//
	std::vector<ToLoadAnimControllerInfo> m_ToLoadAnimCont;
	std::vector<ToLoadAnimExecutorInfo> m_ToLoadAnimExe;
	std::vector<ToLoadAttachInfo> m_ToLoadAttach;


	//std::vector<Entity*> m_Cameras;
	component::Camera* m_MainCamera = nullptr;
};

