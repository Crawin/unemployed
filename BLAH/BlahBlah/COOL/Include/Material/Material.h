#pragma once



class Shader;

class Material
{
	friend class MaterialManager;
public:
	Material();
	Material(std::string_view name);
	~Material();

	const std::string& GetName() const { return m_Name; }

	void SetName(std::string_view name) { m_Name = name; }

	void SetAlbedoTextureIndex(int idx) { m_TextureIndex[0] = idx; }
	//void SetAlbedoTextureIndex(int idx) { m_TextureIndex._11 = idx; }

	bool LoadTexture(ComPtr<ID3D12GraphicsCommandList> cmdList, const char* fileName);

	void SetDatas(ComPtr<ID3D12GraphicsCommandList> cmdList, UINT paramIdx);

private:
	std::string m_Name = "이름없음";

	int m_TextureIndex[16] = {};

	Shader* m_Shader = nullptr;

};

