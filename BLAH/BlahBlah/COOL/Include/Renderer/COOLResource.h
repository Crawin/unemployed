#pragma once


// ID3D12Resource를 감싸 리소스의 현재 상태를 유지하고있게 한다
/*
class COOLResource : public ID3D12Resource {
	COOLResource();
	virtual ~COOLResource();

	// 멤버변수를를 추가하니 메모리 오류가 난다
	// 추가된 멤버변수가 생성을 방해한다
	//D3D12_RESOURCE_STATES m_CurrentState = D3D12_RESOURCE_STATE_COMMON;

public:
	void SetInitialState(D3D12_RESOURCE_STATES init) { m_CurStateMap[this] = init; }
	void TransToState(ComPtr<ID3D12GraphicsCommandList> cmdLst, D3D12_RESOURCE_STATES newState);

private:
	static std::map<COOLResource*, D3D12_RESOURCE_STATES> m_CurStateMap;
};
*/

class COOLResource {
public:
	COOLResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES initState, D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_DEFAULT, std::string name = "Resource");
	~COOLResource();

	ID3D12Resource* GetResource() { return m_Resource; }
	D3D12_RESOURCE_STATES GetState() const { return m_CurStateMap; }

	void TransToState(ComPtr<ID3D12GraphicsCommandList> cmdLst, D3D12_RESOURCE_STATES newState);

private:
	std::string m_Name;
	ID3D12Resource* m_Resource = nullptr;
	D3D12_RESOURCE_STATES m_CurStateMap = D3D12_RESOURCE_STATE_COMMON;
	D3D12_HEAP_TYPE m_HeapType = D3D12_HEAP_TYPE_DEFAULT;
};

