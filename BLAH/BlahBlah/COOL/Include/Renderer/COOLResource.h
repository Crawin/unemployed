#pragma once

#include <map>

// ID3D12Resource�� ���� ���ҽ��� ���� ���¸� �����ϰ��ְ� �Ѵ�
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

