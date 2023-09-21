#pragma once

class COOLDevice
{

public:
    //virtual HRESULT STDMETHODCALLTYPE CreateCommittedResource(
    //    _In_  const D3D12_HEAP_PROPERTIES* pHeapProperties,
    //    D3D12_HEAP_FLAGS HeapFlags,
    //    _In_  const D3D12_RESOURCE_DESC* pDesc,
    //    D3D12_RESOURCE_STATES InitialResourceState,
    //    _In_opt_  const D3D12_CLEAR_VALUE* pOptimizedClearValue,
    //    REFIID riidResource,
    //    _COM_Outptr_opt_  void** ppvResource) override;

private:
    ID3D12Device* m_Device;


};

