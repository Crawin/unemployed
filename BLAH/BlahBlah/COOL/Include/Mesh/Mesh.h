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

	int m_SubSetIndexBuffer = -1;						// �ε��� ����

	// name
	std::string m_Name = "";

	// bounding box
	XMFLOAT3 m_AABBCenter = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 m_AABBExtents = XMFLOAT3(0.0f, 0.0f, 0.0f);
	BoundingOrientedBox m_ModelBoundingBox;

	std::vector<Mesh> m_Childs;

	void BuildMesh(std::ifstream& file);

public:
	Mesh() {}
	~Mesh() {}

	bool LoadFile(const char* fileName);

	void SetPositionBuffer(int idx)		{ m_PositionBuffer = idx; }
	void SetNormalBuffer(int idx)		{ m_NormalBuffer = idx; }
	void SetTangentBuffer(int idx)		{ m_TangentBuffer = idx; }
	void SetTexCoord0Buffer(int idx)	{ m_TexCoord0Buffer = idx; }
	//void SetTexCoord1Buffer(int idx)	{ m_TexCoord1Buffer = idx; }
	void InsertIndexBuffer(int idx)		{ m_SubSetIndexBuffer = idx; }
};

