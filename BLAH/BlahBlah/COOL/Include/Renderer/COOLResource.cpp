#include "../framework.h"
#include "COOLResource.h"

std::map<COOLResource*, D3D12_RESOURCE_STATES> COOLResource::m_CurStateMap{};

COOLResource::COOLResource()
	: ID3D12Resource()
{
	m_CurStateMap.emplace(this, D3D12_RESOURCE_STATE_COMMON);
}

COOLResource::~COOLResource()
{
	m_CurStateMap.erase(this);
}

void COOLResource::TransToState(ComPtr<ID3D12GraphicsCommandList> cmdLst, D3D12_RESOURCE_STATES newState)
{
	auto& curState = m_CurStateMap[this];

	if (newState == curState) {
		DebugPrint(std::format("ERROR! Same resource state: {}", static_cast<int>(newState)));
		return;
	}

	D3D12_RESOURCE_BARRIER resourceBarrier = {};
	resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	resourceBarrier.Transition.pResource = this;
	resourceBarrier.Transition.StateBefore = curState;
	resourceBarrier.Transition.StateAfter = newState;
	resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	cmdLst->ResourceBarrier(1, &resourceBarrier);

	curState = newState;
	//m_CurrentState = newState;
}
