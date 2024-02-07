#include "framework.h"
#include "Renderer/COOLResource.h"
#include "Renderer/Renderer.h"
#include "ResourceManager.h"

//#include "Material/Material.h"
//#include "Mesh/Mesh.h"
#include "Shader/Shader.h"

#define SCENE_PATH		"SceneData\\Scene\\"

#define MESH_PATH		"SceneData\\Mesh\\"
#define SHADER_PATH		"SceneData\\Shader\\"
#define TEXTURE_PATH	"SceneData\\Texture\\"
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
	for (auto& sha : m_Shaders)
		delete sha;
}

bool ResourceManager::Init(const std::string& sceneName)
{
	// 1. 씬에 배치 될 오브젝트들을 찾는다.
	// 2. 씬에 배치 될 오브젝트를 차례로 로드한다.
	// 
	// 오브젝트 로드????
	// A 오브젝트 로드한다.
	// 사용 mesh와 material을 찾는다.
	// 해당 mesh와 material이 없으면 새로 만든다.
	//

	std::string scenePath = SCENE_PATH + sceneName;

	for (const auto& file : std::filesystem::directory_iterator(scenePath)) {
		if (file.is_directory()) continue;

		std::string fileName = scenePath + file.path().filename().string();
		CHECK_CREATE_FAILED(LoadFile(fileName), "File Load Failed!!");
	}

	return false;
}

int ResourceManager::CreateObjectResource(UINT size, const std::string resName, void** toMapData)
{
	COOLResourcePtr res = Renderer::GetInstance().CreateEmptyBuffer(
		D3D12_HEAP_TYPE_UPLOAD,
		D3D12_RESOURCE_STATE_GENERIC_READ/*D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER*/,
		size,
		resName,
		toMapData);

	m_Resources.push_back(res);

	return m_Resources.size() - 1;
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
	
	// 없다!
	if (commandList) {
		// 초기화 단계일 때만 매터리얼을 생성한다.
		Material* material = new Material;
		std::string shaderName;
		if (false == material->LoadFile(commandList, name, this, shaderName)) {
			DebugPrint(std::format("Failed to load material!! Name: {}", name));
			delete material;
			return -1;
		}

		m_Materials.push_back(material);

		return m_Materials.size() - 1;
	}
	else {
		DebugPrint(
			std::format(
				"ERROR!!!!, no such material {}\nmake this material on initialize",
				name));
	}


	return -1;
}

D3D12_GPU_VIRTUAL_ADDRESS ResourceManager::GetVertexDataGPUAddress(int idx)
{
	if (0 <= idx && idx < m_Resources.size())
		return m_Resources[idx]->GetResource()->GetGPUVirtualAddress();

	return -1;
}

int ResourceManager::GetMesh(const std::string& name, ComPtr<ID3D12GraphicsCommandList> commandList)
{
	// 찾았다
	for (int i = 0; i < m_Meshes.size(); ++i)
		if (m_Meshes[i]->GetName() == name)
			return i;

	// 못찾았다
	if (commandList) {
		std::string fileName = MESH_PATH + name + ".bin";
		std::ifstream meshFile(fileName, std::ios::binary);
		if (meshFile.fail()) {
			DebugPrint(std::format("Failed to open mesh file!! fileName: {}", fileName));
			return -1;
		}

		Mesh* mesh = new Mesh;
		mesh->m_Name = ExtractFileName(fileName);
		mesh->BuildMesh(commandList, meshFile, this);

		DebugPrint(std::format("loaded file name: {}", mesh->m_Name));
		return true;

	}
	else {
		DebugPrint(
			std::format(
			"ERROR!!!!, no such mesh {}\nmake this mesh on initialize",
			name));
	}

	// 없다

	return -1;
}

