#include "framework.h"
#include "Renderer/COOLResource.h"
#include "Renderer/Renderer.h"
#include "ResourceManager.h"
#include "ECS/Entity.h"
#include "ECS/Component.h"
#include "ECS/ECSManager.h"
#include <json/json.h>

//#include "Material/Material.h"
//#include "Mesh/Mesh.h"
#include "Shader/Shader.h"

#define SCENE_PATH		"SceneData\\Scene\\"

#define MESH_PATH		"SceneData\\Mesh\\"
#define SHADER_PATH		"SceneData\\Shader\\"
#define TEXTURE_PATH	"SceneData\\Material\\Texture\\"
#define MATERIAL_PATH	"SceneData\\Material\\"


ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
	for (auto& mat : m_Materials)
		delete mat;
	for (auto& mes : m_Meshes)
		delete mes;
	for (auto& cmp : m_Components)
		delete cmp;
	
	//sharedptr
	m_Shaders.clear();


	for (auto& obj : m_Entities)
		delete obj;

	m_Resources.clear();
	m_VertexIndexDatas.clear();
}

bool ResourceManager::LoadObjectFile(const std::string& fileName, bool isCam)
{
	// 대충 오브젝트를 로드한다.
	Json::Value root;
	Json::Reader reader;

	// 일단 고전 유니티 방식으로 하자
	// ecs는 아니지만 컴포넌트가 있는 방식이다.
	std::ifstream file(fileName);
	reader.parse(file, root);
	
	auto entptr = LoadObjectJson(root, nullptr);

	if (entptr == nullptr) 
		return false;

	if (isCam) m_RootEntities.push_back(entptr);
	else m_Cameras.push_back(entptr);

	return true;
}

#define CHILDREN "Children"

Entity* ResourceManager::LoadObjectJson(Json::Value& root, Entity* parent)
{
	Entity* ent = new Entity(m_Entities.size());

	if (parent) parent->AddChild(ent);

	for (auto& jsonComp : root.getMemberNames()) {
		if (CHECK_COMPONENT(jsonComp) == false) {
			if (jsonComp != CHILDREN) DebugPrint(std::format("No Such Component Registered!! {}", jsonComp));
			continue;
		}

		component::Component* cmp = GET_COMPONENT(jsonComp);
		cmp->Create(root, this);
		ent->AddComponent(cmp);
		m_Components.push_back(cmp);
	}
	
	// if Children
	if (root[CHILDREN].isNull() == false)
		for (auto& val : root[CHILDREN]) {
			auto child = LoadObjectJson(val, ent);
			if (child == nullptr)
				return nullptr;
		}
	
	// 여기서 ECS Manager에 entity를 삽입하지 않는다.
	// late init이 끝난 후 entity를 삽입함
	m_Entities.push_back(ent);
	//m_ECSManager->AddEntity(ent);

	return ent;
}

int ResourceManager::LoadMesh(const std::string& name, ComPtr<ID3D12GraphicsCommandList> commandList)
{
	std::string fileName = MESH_PATH + name + ".bin";
	std::ifstream meshFile(fileName, std::ios::binary);
	if (meshFile.is_open() == false) {
		DebugPrint(std::format("Failed to open mesh file!! fileName: {}", fileName));
		return -1;
	}

	Mesh* mesh = new Mesh;
	mesh->m_Name = ExtractFileName(fileName);
	mesh->BuildMesh(commandList, meshFile, this);

	DebugPrint(std::format("loaded file name: {}", mesh->m_Name));
	return GetMesh(mesh->m_Name);

}

int ResourceManager::LoadMaterial(const std::string& name, ComPtr<ID3D12GraphicsCommandList> commandList)
{
	// 초기화 단계일 때만 매터리얼을 생성한다.
	std::string fileName = MATERIAL_PATH + name + ".json";

	Material* material = new Material;
	std::string shaderName;
	if (false == material->LoadFile(commandList, fileName, this, shaderName)) {
		DebugPrint(std::format("Failed to load material!! Name: {}", name));
		delete material;
		return -1;
	}

	// get shader
	auto shader = GetShader(shaderName, commandList);
	material->SetShader(shader);
	m_Materials.push_back(material);

	return m_Materials.size() - 1;
}

std::shared_ptr<Shader> ResourceManager::LoadShader(const std::string& name, ComPtr<ID3D12GraphicsCommandList> commandList)
{
	std::shared_ptr<Shader> shader = std::make_shared<Shader>();

	std::string fileName = SHADER_PATH + name + ".json";

	if (false == Renderer::GetInstance().CreateShader(commandList, fileName, shader)) {
		DebugPrint(std::format("Failed to load shader file!! fileName: {}", fileName));
		return nullptr;
	}

	m_Shaders.push_back(shader);

	DebugPrint(std::format("loaded file name: {}", shader->m_Name));


	return shader;
}

bool ResourceManager::Init(ComPtr<ID3D12GraphicsCommandList> commandList, const std::string& sceneName)
{
	// 1. 씬에 배치 될 오브젝트들을 찾는다.
	// 2. 씬에 배치 될 오브젝트를 차례로 로드한다.
	// 
	// 오브젝트 로드????
	// A 오브젝트 로드한다.
	// 사용 mesh와 material을 찾는다.
	// 해당 mesh와 material이 없으면 새로 만든다.
	//

	//ecs manager를 만든다
	// ecsmanager 의 소유는 scene으로 이동함
	//m_ECSManager = new ECSManager<COMPONENT_COUNT>;

	std::string scenePath = SCENE_PATH + sceneName;

	// Load Objects
	CHECK_CREATE_FAILED(LoadObjects(sceneName, commandList), "Object Load Failed!!");

	// Load Lights

	// Load Cameras
	CHECK_CREATE_FAILED(LoadCameras(sceneName, commandList), "Camera Load Fail!!");

	// Load Mesh, Material, Texture, Shader to use
	CHECK_CREATE_FAILED(LateInit(commandList), "LateInit Fail!");

	// build resource heap
	CHECK_CREATE_FAILED(Renderer::GetInstance().CreateResourceDescriptorHeap(m_ShaderResourceHeap, m_Resources), "CreateResourceDescriptorHeap Fail!!");

	return true;
}

void ResourceManager::SetECSManager(std::shared_ptr<ECSManager> ptr)
{
	m_ECSManager = ptr;
}

bool ResourceManager::LateInit(ComPtr<ID3D12GraphicsCommandList> commandList)
{
	// load mesh
	for (const auto& rendererLoadInfo : m_ToLoadRenderDatas) {
		// mesh의 json에서는 mesh의 이름을
		// satodia.satodia_body 와 같은 방식으로 이루어져있다.
		// 뜻: satodia.bin 파일에 존재하는 satodia_body

		std::string meshFileName = ExtractFileName(rendererLoadInfo.m_MeshName);
		
		auto it = std::find(rendererLoadInfo.m_MeshName.begin(), rendererLoadInfo.m_MeshName.end(), '.');
		++it;
		std::string meshName(it, rendererLoadInfo.m_MeshName.end());

		// 해당 mesh가 없을 때 부모 mesh를 load

		int res = GetMesh(meshName);
		if (res == -1)
		{
			// check first
			if (GetMesh(meshFileName) != -1) {
				DebugPrint(std::format("ERROR!!!!, No Such mesh in file!! mesh: {}\t file: {}", meshName, meshFileName));
				return false;
			}
			
			// load parent
			LoadMesh(meshFileName, commandList);
		}

		res = GetMesh(meshName);

		if (res == -1) {
			DebugPrint(std::format("ERROR!!!!, No Such mesh in file!! mesh: {}\t file: {}", meshName, meshFileName));
			return false;
		}

		rendererLoadInfo.m_Renderer->SetMesh(res);

		// load material
		std::string materialFileName = rendererLoadInfo.m_MaterialName;
		res = GetMaterial(materialFileName, commandList);

		if (res == -1) {
			DebugPrint(std::format(
				"ERROR!!, no such material, name: {}\nSet to uv_checker",
				materialFileName));
			res = GetMaterial(materialFileName, commandList);
		}


		rendererLoadInfo.m_Renderer->SetMaterial(res);

	}

	for (auto& ent : m_Entities)
		m_ECSManager->AddEntity(ent);

	//m_ToLoadRenderDatas.clear();
	return true;
}

bool ResourceManager::LoadObjects(const std::string& sceneName, ComPtr<ID3D12GraphicsCommandList> commandList)
{
	std::string objPath = SCENE_PATH + sceneName + "\\Object\\";
	for (const auto& file : std::filesystem::directory_iterator(objPath)) {
		if (file.is_directory()) continue;

		std::string fileName = objPath + file.path().filename().string();
		CHECK_CREATE_FAILED(LoadObjectFile(fileName), std::format("Object Load Failed!! Object Name: {}", fileName));
	}
	return true;
}

bool ResourceManager::LoadCameras(const std::string& sceneName, ComPtr<ID3D12GraphicsCommandList> commandList)
{
	std::string camPath = SCENE_PATH + sceneName + "\\Camera\\";
	for (const auto& file : std::filesystem::directory_iterator(camPath)) {
		if (file.is_directory()) continue;

		std::string fileName = camPath + file.path().filename().string();
		CHECK_CREATE_FAILED(LoadObjectFile(fileName, true), std::format("Object Load Failed!! Object Name: {}", fileName));
	}

	if (m_MainCamera == nullptr) return false;

	return true;
}

int ResourceManager::CreateObjectResource(UINT size, const std::string resName, void** toMapData)
{
	COOLResourcePtr res = Renderer::GetInstance().CreateEmptyBuffer(
		D3D12_HEAP_TYPE_UPLOAD,
		D3D12_RESOURCE_STATE_GENERIC_READ/*D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER*/,
		size,
		resName,
		toMapData);

	m_VertexIndexDatas.push_back(res);

	return m_VertexIndexDatas.size() - 1;
}

void ResourceManager::SetDatas()
{

}

int ResourceManager::GetTexture(ComPtr<ID3D12GraphicsCommandList> commandList, const std::string& name)
{
	// 없다면 새로 추가한다
	if (false == m_TextureIndexMap.contains(name)) {
		std::string dds = TEXTURE_PATH + name + ".dds";
		std::wstring fileNamewstr(dds.begin(), dds.end());

		auto res = Renderer::GetInstance().CreateTextureFromDDSFile(commandList, fileNamewstr.c_str());
		m_Resources.push_back(res);

		m_TextureIndexMap[name] = m_Resources.size()- 1;
	}

	return m_TextureIndexMap[name];
}

int ResourceManager::GetMaterial(const std::string& name, ComPtr<ID3D12GraphicsCommandList> commandList)
{
	for (int i = 0; i < m_Materials.size(); ++i) 
		if (m_Materials[i]->GetName() == name)
			return i;

	if (commandList) return LoadMaterial(name, commandList);
	return -1;
}

std::shared_ptr<Shader> ResourceManager::GetShader(const std::string& name, ComPtr<ID3D12GraphicsCommandList> commandList)
{
	for (auto& shader : m_Shaders)
		if (shader->GetName() == name)
			return shader;

	// 없다.
	if (commandList) return LoadShader(name, commandList);
	return nullptr;
}

int ResourceManager::GetMeshToLoad(const std::string& name)
{
	// todo 
	// 여기서 다른 resourcemanager에게 mesh를 전달받을 수 있음
	// 지금은 -1만 return

	return -1;

	for (int i = 0; i < m_ToLoadRenderDatas.size(); ++i)
		if (m_ToLoadRenderDatas[i].m_MeshName == name)
			return i;
	
	// no such mesh
	return -1;
}

void ResourceManager::AddLateLoad(const std::string& mesh, const std::string& material, component::Renderer* renderer)
{
	ToLoadRendererInfo info;
	info.m_MeshName = mesh;
	info.m_MaterialName = material;
	info.m_Renderer = renderer;

	m_ToLoadRenderDatas.push_back(info);
}

int ResourceManager::GetMaterialToLoad(const std::string& name)
{
	for (int i = 0; i < m_ToLoadMaterials.size(); ++i)
		if (m_ToLoadMaterials[i] == name)
			return i;

	m_ToLoadMaterials.push_back(name);

	return m_ToLoadMaterials.size() - 1;
}

D3D12_GPU_VIRTUAL_ADDRESS ResourceManager::GetVertexDataGPUAddress(int idx)
{
	if (0 <= idx && idx < m_VertexIndexDatas.size())
		return m_VertexIndexDatas[idx]->GetResource()->GetGPUVirtualAddress();

	return -1;
}

int ResourceManager::GetMesh(const std::string& name, ComPtr<ID3D12GraphicsCommandList> commandList)
{
	// 찾았다
	for (int i = 0; i < m_Meshes.size(); ++i)
		if (m_Meshes[i]->GetName() == name)
			return i;

	// todo 찾지 못했다면 당장 생성하지 말고 보관해두었다가 나중에 생성하게 하자
	// commandlist 인자도 지워야지
	
	// 
	// 
	// 못찾았다
	if (commandList) return LoadMesh(name, commandList);

	return -1;
}

