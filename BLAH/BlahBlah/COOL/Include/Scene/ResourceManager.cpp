#include "framework.h"
#include "Renderer/COOLResource.h"
#include "Renderer/Renderer.h"
#include "ResourceManager.h"

#include "Material/Material.h"
#include "Mesh/Mesh.h"

#define SHADER_PATH "SceneData\\Shader\\"
#define MESH_PATH "SceneData\\Mesh\\"
#define TEXTURE_PATH "SceneData\\Texture\\"
#define MATERIAL_PATH "SceneData\\Material\\"

ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
	for (auto& mat : m_Materials)
		delete mat;
	for (auto& mes : m_Meshes)
		delete mes;
}

void ResourceManager::BuildMesh(ComPtr<ID3D12GraphicsCommandList> commandList, std::ifstream& file, Mesh* mesh)
{
	mesh->BuildMesh(commandList, file);

	m_Meshes.push_back(mesh);
	//m_MeshMap[mesh->m_Name] = mesh;


	// 8. 서브메쉬 개수
	unsigned int childNum;
	file.read((char*)&childNum, sizeof(unsigned int));
	mesh->m_Childs.reserve(childNum);

	// 9. 서브메쉬(재귀)
	for (unsigned int i = 0; i < childNum; ++i) {
		Mesh* child = new Mesh;
		BuildMesh(commandList, file, child);
		mesh->m_Childs.push_back(child);
	}
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
		BuildMesh(commandList, meshFile, mesh);

		m_Meshes.push_back(mesh);

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

