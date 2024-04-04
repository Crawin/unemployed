#pragma once
struct Vertex
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT3 tangent;
	DirectX::XMFLOAT2 uv;
};

class Mesh {
private:
	// name
	std::string m_Name = "";

	// bounding box
	DirectX::XMFLOAT3 m_AABBCenter = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	DirectX::XMFLOAT3 m_AABBExtents = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);

	// todo mesh loader�� �Ʒ� ������ �߰� 
	DirectX::XMFLOAT3 m_Position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	DirectX::XMFLOAT4 m_Rotation = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	DirectX::XMFLOAT3 m_Scale = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);

	// �θ�� ������� ��ȯ���
	DirectX::XMFLOAT4X4 m_LocalTransform;
	DirectX::XMFLOAT4X4 m_RootTransform;

	int m_VertexNum = 0;
public:
	std::vector<Mesh> m_Childs;
	void LoadMeshData(std::ifstream&);
	DirectX::XMFLOAT3 GetCenter();
	DirectX::XMFLOAT3 GetExtents();
};