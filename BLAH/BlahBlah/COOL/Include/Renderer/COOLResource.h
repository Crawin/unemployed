#pragma once


// ID3D12Resource�� ���� ���ҽ��� ���� ���¸� �����ϰ��ְ� �Ѵ�
/*
class COOLResource : public ID3D12Resource {
	COOLResource();
	virtual ~COOLResource();

	// ����������� �߰��ϴ� �޸� ������ ����
	// �߰��� ��������� ������ �����Ѵ�
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

