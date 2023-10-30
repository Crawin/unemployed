#include "framework.h"
#include "Material.h"

void Material::SetDatas(ComPtr<ID3D12GraphicsCommandList> cmdList, UINT paramIdx)
{
	cmdList->SetGraphicsRoot32BitConstants(paramIdx, _countof(m_TextureIndex), &m_TextureIndex, 0);
}
