#pragma once



class Shader;
class ResourceManager;

class Material
{
	friend class ResourceManager;
public:
	Material();
	Material(std::string_view name);
	~Material();

	const std::string& GetName() const { return m_Name; }

	void SetName(std::string_view name) { m_Name = name; }

	void SetAlbedoTextureIndex(int idx) { m_TextureIndex[0] = idx; }
	//void SetAlbedoTextureIndex(int idx) { m_TextureIndex._11 = idx; }

	bool LoadFile(ComPtr<ID3D12GraphicsCommandList> cmdList, const std::string& fileName, ResourceManager* manager, std::string& shaderName);

	void SetDatas(ComPtr<ID3D12GraphicsCommandList> cmdList, UINT paramIdx);

private:
	std::string m_Name = "이름없음";

	int m_TextureIndex[16] = {};

	std::shared_ptr<Shader> m_Shader = nullptr;

};

