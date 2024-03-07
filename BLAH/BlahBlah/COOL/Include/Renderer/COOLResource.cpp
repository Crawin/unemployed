#include "framework.h"
#include "COOLResource.h"

COOLResource::COOLResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES initState, D3D12_HEAP_TYPE heapType, std::string_view name) :
	m_Resource{ resource },
	m_CurStateMap{ initState },
	m_HeapType{ heapType },
	m_Name{ name }
{
	SetName(name);
}

COOLResource::~COOLResource()
{
	if (m_Resource && m_Release) {
		if (m_Mapped) m_Resource->Unmap(0, nullptr);
		m_Resource->Release();
	}
}

void COOLResource::TransToState(ComPtr<ID3D12GraphicsCommandList> cmdLst, D3D12_RESOURCE_STATES newState)
{
	if (newState == m_CurStateMap) {
		DebugPrint(std::format("ERROR! Same resource state: {}", static_cast<int>(newState)));
		return;
	}

	D3D12_RESOURCE_BARRIER resourceBarrier = {};
	resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	resourceBarrier.Transition.pResource = m_Resource;
	resourceBarrier.Transition.StateBefore = m_CurStateMap;
	resourceBarrier.Transition.StateAfter = newState;
	resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	cmdLst->ResourceBarrier(1, &resourceBarrier);

	m_CurStateMap = newState;
	//m_CurrentState = newState;
}
