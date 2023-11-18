#pragma once

class Material
{
public:
	Material() {}
	Material(std::string_view name) : m_Name{ name } {};
	~Material() {}

	void SetName(std::string_view name) { m_Name = name; }

	void SetAlbedoTextureIndex(int idx) { m_TextureIndex[0] = idx; }
	//void SetAlbedoTextureIndex(int idx) { m_TextureIndex._11 = idx; }

	bool LoadTexture(ComPtr<ID3D12GraphicsCommandList> cmdList, const char* fileName);

	void SetDatas(ComPtr<ID3D12GraphicsCommandList> cmdList, UINT paramIdx);

private:
	std::string m_Name = "이름을 지어주세요";

	int m_TextureIndex[16] = {};

};

