#pragma once

// 진짜 메쉬 데이터만 가지고 있음
// 뭐 계층구조니 뭐니 그런건 오브젝트에서 알아서 하삼
// 업로드버퍼는 렌더러가 들고있음
// CPU도 버텍스 정보를 계속 가지고 있어야 할까?
// 그러게
// 일단 가지고 있게 한 다음에 없애버리자

// Mesh 로드 순서
// 그거뭐냐 일단 파일에서 읽음
// 읽은 정보를 임시 리소스(업로드힙) 만들음
// 임시리소스를 진짜 버퍼로 복사함
// 맨 뒤에 임시리소스를 해제하는데 그건 렌더러가 해줄거임


class Mesh
{
private:
	int m_PositionBuffer = -1;
	int m_NormalBuffer = -1;
	int m_TangentBuffer = -1;							// 혹시 모를 노말맵핑을 위해
	int m_TexCoord0Buffer = -1;
	//int m_TexCoord1Buffer = -1;						// 혹시 모를

	int m_IndexBuffer = -1;						// 인덱스 버퍼

	D3D12_VERTEX_BUFFER_VIEW m_PositionBufferView = {};
	D3D12_VERTEX_BUFFER_VIEW m_NormalBufferView = {};
	D3D12_VERTEX_BUFFER_VIEW m_TangentBufferView = {};
	D3D12_VERTEX_BUFFER_VIEW m_TexCoord0BufferView = {};
	//D3D12_VERTEX_BUFFER_VIEW m_TexCoord1BufferView = {};

	D3D12_INDEX_BUFFER_VIEW m_IndexBufferView = {};

	// name
	std::string m_Name = "";

	// bounding box
	XMFLOAT3 m_AABBCenter = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 m_AABBExtents = XMFLOAT3(0.0f, 0.0f, 0.0f);
	BoundingOrientedBox m_ModelBoundingBox;

	// 부모와 상대적인 변환행렬
	XMFLOAT4X4 m_LocalTransform = Matrix4x4::Identity();
	XMFLOAT4X4 m_RootTransform = Matrix4x4::Identity();

	std::vector<Mesh> m_Childs;

	void BuildMesh(ComPtr<ID3D12GraphicsCommandList> commandList, std::ifstream& file);

	int m_VertexNum = 0;
	int m_IndexNum = 0;

public:
	Mesh() {}
	~Mesh() {}

	bool LoadFile(ComPtr<ID3D12GraphicsCommandList> commandList, const char* fileName);

	void SetPositionBuffer(int idx)		{ m_PositionBuffer = idx; }
	void SetNormalBuffer(int idx)		{ m_NormalBuffer = idx; }
	void SetTangentBuffer(int idx)		{ m_TangentBuffer = idx; }
	void SetTexCoord0Buffer(int idx)	{ m_TexCoord0Buffer = idx; }
	//void SetTexCoord1Buffer(int idx)	{ m_TexCoord1Buffer = idx; }
	void InsertIndexBuffer(int idx)		{ m_IndexBuffer = idx; }

	void Render(ComPtr<ID3D12GraphicsCommandList> commandList, XMFLOAT4X4& parent);
};

