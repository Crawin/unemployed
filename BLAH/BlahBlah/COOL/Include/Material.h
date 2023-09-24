#pragma once

class Material
{
public:
	Material() {}
	~Material() {}

	void SetAlbedoTextureIndex(int idx) { m_TextureIndex[0] = idx; }
	//void SetAlbedoTextureIndex(int idx) { m_TextureIndex._11 = idx; }

	void SetDatas(ComPtr<ID3D12GraphicsCommandList> cmdList, UINT paramIdx);

private:
	int m_TextureIndex[16];
	//XMFLOAT4X4 m_TextureIndex = XMFLOAT4X4(0);
};

