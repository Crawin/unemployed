#include "framework.h"
#include "Material.h"

void Material::SetDatas(ComPtr<ID3D12GraphicsCommandList> cmdList, UINT paramIdx)
{
	cmdList->SetGraphicsRoot32BitConstants(paramIdx, sizeof(m_Float4x4), &m_Float4x4, 0);
}
