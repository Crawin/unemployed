#pragma once

class Material;
class Shader;
class ShaderManager;

class MaterialManager
{
public:
	MaterialManager();
	~MaterialManager();

	void RegisterShaderManager(ShaderManager* shaderManager) { m_ShaderManager = shaderManager; }

private:
	// load file
	bool LoadFile(ComPtr<ID3D12GraphicsCommandList> commandList, const std::string& fileName);

public:
	// load files in folder path
	bool LoadFolder(ComPtr<ID3D12GraphicsCommandList> commandList, const std::string& pathName);

	// get material
	Material* GetMaterial(const std::string& name);

	// texturemap 에서 로드 혹은 새로 받아옴
	int LoadOrGetTexture(ComPtr<ID3D12GraphicsCommandList> commandList, const std::string& name);

private:
	// 텍스쳐 중복 로드 방지
	std::map<std::string, int> m_TextureIndexMap;

	std::vector <Material*> m_Materials;

	std::string m_CurrentPath = "";

	// delete 금지, 여기서 소유 중인 객체가 아니다.
	// shared_ptr로 변경할까
	ShaderManager* m_ShaderManager = nullptr;

	//std::map <Shader*, Material*> m_MaterialMap;

};

