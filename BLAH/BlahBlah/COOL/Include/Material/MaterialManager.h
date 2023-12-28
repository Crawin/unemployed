#pragma once

class Material;
class Shader;

class MaterialManager
{
public:
	MaterialManager();
	~MaterialManager();

	
private:
	// load file
	bool LoadFile(ComPtr<ID3D12GraphicsCommandList> commandList, const std::string& fileName);

public:
	// load files in folder path
	bool LoadFolder(ComPtr<ID3D12GraphicsCommandList> commandList, const std::string& pathName);

	Material* GetMaterial(const std::string& name);


private:
	// 텍스쳐 중복 로드 방지
	std::map<std::string, int> m_TextureIndexMap;

	std::vector <Material*> m_Materials;

	std::string m_CurrentPath = "";

	//std::map <Shader*, Material*> m_MaterialMap;

};

