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
	DirectX::XMFLOAT3 m_AABBExtents_Divide = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);

	// todo mesh loader에 아래 데이터 추가 
	DirectX::XMFLOAT3 m_Position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	DirectX::XMFLOAT4 m_Rotation = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	DirectX::XMFLOAT3 m_Scale = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);

	// 부모와 상대적인 변환행렬
	DirectX::XMFLOAT4X4 m_LocalTransform = { 1,0,0,0 ,0,1,0,0, 0,0,1,0, 0,0,0,1 };
	DirectX::XMFLOAT4X4 m_RootTransform = { 1,0,0,0 ,0,1,0,0, 0,0,1,0, 0,0,0,1 };

	int m_VertexNum = 0;
public:
	std::vector<Mesh> m_Childs;
	void LoadMeshData(std::ifstream&);
	DirectX::XMFLOAT3 GetCenter();
	DirectX::XMFLOAT3 GetExtents();
	bool collision(const DirectX::BoundingOrientedBox&);
	bool collision_v2(const DirectX::BoundingOrientedBox&,DirectX::XMFLOAT3&, DirectX::XMFLOAT3*);
	void planeNum_intersects_direction_vector(const DirectX::BoundingOrientedBox&,DirectX::XMFLOAT3&, DirectX::XMFLOAT3*);
};