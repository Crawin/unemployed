#pragma once

// ��¥ �޽� �����͸� ������ ����
// �� ���������� ���� �׷��� ������Ʈ���� �˾Ƽ� �ϻ�
// ���ε���۴� �������� �������
// CPU�� ���ؽ� ������ ��� ������ �־�� �ұ�?
// �׷���
// �ϴ� ������ �ְ� �� ������ ���ֹ�����

// Mesh �ε� ����
// �װŹ��� �ϴ� ���Ͽ��� ����
// ���� ������ �ӽ� ���ҽ�(���ε���) ������
// �ӽø��ҽ��� ��¥ ���۷� ������
// �� �ڿ� �ӽø��ҽ��� �����ϴµ� �װ� �������� ���ٰ���


class Mesh
{
private:
	int m_PositionBuffer = -1;
	int m_NormalBuffer = -1;
	int m_TangentBuffer = -1;							// Ȥ�� �� �븻������ ����
	int m_TexCoord0Buffer = -1;
	//int m_TexCoord1Buffer = -1;						// Ȥ�� ��

	int m_IndexBuffer = -1;						// �ε��� ����

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

	// �θ�� ������� ��ȯ���
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

