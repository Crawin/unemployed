﻿#pragma once



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

	void SetTexture(int idx, int tex) { m_TextureIndex[idx] = tex; }
	void SetDataIndex(int idx, int data) { m_TextureIndex[idx] = data; }
	void SetExtraDataIndex(int idx, float data) { m_ExtraData[idx] = data; }
	void SetAlbedoTextureIndex(int idx) { m_TextureIndex[0] = idx; }
	//void SetAlbedoTextureIndex(int idx) { m_TextureIndex._11 = idx; }

	bool LoadFile(ComPtr<ID3D12GraphicsCommandList> cmdList, const std::string& fileName, ResourceManager* manager, std::string& shaderName);

	void SetDatas(ComPtr<ID3D12GraphicsCommandList> cmdList, UINT paramIdx);

	void SetShader(std::shared_ptr<Shader> shader) { m_Shader = shader; }

	std::shared_ptr<Shader> GetShader() { return m_Shader; }

	int GetTexture(int idx) const { return m_TextureIndex[idx]; }

private:
	UINT m_TextureIndex[5] = {};
	float m_ExtraData[3] = {};

	std::string m_Name = "이름없음";
	
	std::shared_ptr<Shader> m_Shader = nullptr;

};

