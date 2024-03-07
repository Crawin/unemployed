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
	COOLResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES initState, D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_DEFAULT, std::string_view name = "Resource");
	~COOLResource();

	const std::string GetName() const { return m_Name; }
	ID3D12Resource* GetResource() { return m_Resource; }
	D3D12_RESOURCE_STATES GetState() const { return m_CurStateMap; }
	D3D12_SRV_DIMENSION GetDimension() const { return m_Dimension; }

	void SetName(std::wstring wstr) { m_Name = std::string(wstr.begin(), wstr.end()); }
	void SetName(std::string_view str) { m_Name = str; std::wstring temp(m_Name.begin(), m_Name.end());  m_Resource->SetName(temp.c_str()); }
	void SetDimension(D3D12_SRV_DIMENSION dim) { m_Dimension = dim; }

	void TransToState(ComPtr<ID3D12GraphicsCommandList> cmdLst, D3D12_RESOURCE_STATES newState);

	void DontReleaseOnDesctruct() { m_Release = false; }

	void SetMapOn() { m_Mapped = true; }

private:
	std::string m_Name;
	ID3D12Resource* m_Resource = nullptr;
	D3D12_RESOURCE_STATES m_CurStateMap = D3D12_RESOURCE_STATE_COMMON;
	D3D12_HEAP_TYPE m_HeapType = D3D12_HEAP_TYPE_DEFAULT;
	D3D12_SRV_DIMENSION m_Dimension = D3D12_SRV_DIMENSION_TEXTURE2D;

	// release를 미루지 않겠습니다.
	bool m_Release = true;

	// map되어있는 오브젝트이다.
	bool m_Mapped = false;
};

