#pragma once

class Material
{
public:
	Material() {}
	~Material() {}

	void SetAlbedoTextureIndex(int idx) { m_Float4x4._11 = idx; }
	//void SetAlbedoTextureIndex(int idx) { m_Float4x4._11 = idx; }

	void SetDatas(ComPtr<ID3D12GraphicsCommandList> cmdList, UINT paramIdx);

private:
	XMFLOAT4X4 m_Float4x4 = XMFLOAT4X4(0);
};

