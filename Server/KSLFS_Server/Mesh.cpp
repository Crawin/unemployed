#include "IOCP_Common.h"
#include "Mesh.h"

void Mesh::LoadMeshData(std::ifstream& meshFile)
{
	// �޽� ���� ����
// 1. �̸� ����					// int
// 2. �̸�						// char*
// 3. �ٿ���ڽ�					// float3 x 3
// 4. �θ� ��� ��ȯ ���			// float4x4
// 5. ���ؽ� Ÿ��				// int
// 6. ���ؽ� ����				// int, int*(pos, nor, tan, uv)			// int pos int nor int tan int uv
// 7. �ε��� ����				// int int*int
// 8. ����޽� ����				// int
// 9. ����޽�(�̸����� �̸� ���ؽ����� ����޽�...����)		// ��ͷ� �İ����
//

// 1. �̸� ����
	unsigned int size;
	meshFile.read((char*)&size, sizeof(unsigned int));

	// 2. �̸�
	if (size > 0) {
		char* name = new char[size + 1];
		name[size] = '\0';
		meshFile.read(name, size);
		m_Name = std::string(name);
		delete[] name;
	}

	// 3. �ٿ���ڽ�	
	DirectX::XMFLOAT3 min, max;
	meshFile.read((char*)&min, sizeof(DirectX::XMFLOAT3));
	meshFile.read((char*)&max, sizeof(DirectX::XMFLOAT3));
	meshFile.read((char*)&m_AABBCenter, sizeof(DirectX::XMFLOAT3));
	m_AABBExtents = DirectX::XMFLOAT3(max.x - min.x, max.y - min.y, max.z - min.z);
	m_AABBExtents_Divide = DirectX::XMFLOAT3(m_AABBExtents.x / 2, m_AABBExtents.y / 2, m_AABBExtents.z / 2);
	// 4. �θ� ��� ��ȯ ���		// float4x4
	meshFile.read((char*)&m_LocalTransform, sizeof(DirectX::XMFLOAT4X4));

	// 5. ���ؽ� Ÿ�� // ��� �� �� �ϴ�. ex) ��Ų�޽� vs �Ϲݸ޽�
	unsigned int vtxType;
	meshFile.read((char*)&vtxType, sizeof(unsigned int));

	// 6. ���ؽ� ����
	unsigned int vertexLen = 0;
	meshFile.read((char*)&vertexLen, sizeof(unsigned int));

	if (vertexLen > 0) {

		std::vector<Vertex> vertex(vertexLen);
		meshFile.read((char*)(&vertex[0]), sizeof(Vertex) * vertexLen);

		m_VertexNum = vertexLen;
	}

	// 8. ����޽� ����
	unsigned int childNum;
	meshFile.read((char*)&childNum, sizeof(unsigned int));
	m_Childs.reserve(childNum);

	// 9. ����޽�(���)
	for (unsigned int i = 0; i < childNum; ++i) {
		Mesh newMesh;
		DirectX::XMMATRIX root = DirectX::XMLoadFloat4x4(&m_RootTransform);
		DirectX::XMMATRIX local = DirectX::XMLoadFloat4x4(&m_LocalTransform);
		DirectX::XMStoreFloat4x4(&newMesh.m_RootTransform, DirectX::XMMatrixMultiply(root, local));
		newMesh.LoadMeshData(meshFile);
		m_Childs.push_back(newMesh);
	}
}

DirectX::XMFLOAT3 Mesh::GetCenter()
{
	return m_AABBCenter;
}

DirectX::XMFLOAT3 Mesh::GetExtents()
{
	return m_AABBExtents;
}

bool Mesh::collision(const DirectX::BoundingOrientedBox& player)
{
	DirectX::XMFLOAT4 orient(0, 0, 0, 1);
	DirectX::BoundingOrientedBox obb(m_AABBCenter, m_AABBExtents_Divide, orient);

	if (player.Intersects(obb) && m_Name != "")
	{
		return true;
	}
	else
	{
		for (auto& child : m_Childs)
		{
			if (child.collision(player))
				return true;
		}
	}
	return false;
}

bool Mesh::collision_v2(const DirectX::BoundingOrientedBox& player,DirectX::XMFLOAT3& playerSpeed, DirectX::XMFLOAT3* newSpeed)
{
	DirectX::XMFLOAT4 orient(0, 0, 0, 1);
	DirectX::BoundingOrientedBox obb(m_AABBCenter, m_AABBExtents_Divide, orient);


	if (player.Intersects(obb) && m_Name != "")
	{
		//DirectX::XMVECTOR speed = DirectX::XMLoadFloat3(&playerSpeed);
		//playerSpeed.x* (x - player.Center.x) + playerSpeed.y * (y - player.Center.y) + playerSpeed.z * (z - player.Center.z) = 0;

		//float subX = abs(m_AABBCenter.x - player.Center.x);
		//float subZ = abs(m_AABBCenter.z - player.Center.z);
		//if (subX > subZ)
		//{
		//	DirectX::XMVECTOR crash_normal_vector = { (player.Center.x -m_AABBCenter.x) / subX * 1,0,0 };
		//	DirectX::XMVECTOR player_speed_vector = DirectX::XMLoadFloat3(&playerSpeed);
		//	//DirectX::XMVECTOR projection = DirectX::XMVector3Dot(player_speed_vector, crash_normal_vector);

		//	DirectX::XMStoreFloat3(newSpeed, DirectX::XMVectorAdd(player_speed_vector, crash_normal_vector));
		//}
		//else
		//{
		//	DirectX::XMVECTOR crash_normal_vector = { 0,0,(player.Center.z - m_AABBCenter.z) / subZ * 1 };
		//	DirectX::XMVECTOR player_speed_vector = DirectX::XMLoadFloat3(&playerSpeed);
		//	DirectX::XMStoreFloat3(newSpeed, DirectX::XMVectorAdd(player_speed_vector, crash_normal_vector));
		//}
		
		planeNum_intersects_direction_vector(player, playerSpeed, newSpeed);
		return true;
	}
	else
	{
		for (auto& child : m_Childs)
		{
			if (child.collision_v2(player,playerSpeed,newSpeed))
				return true;
		}
	}
	return false;
}

void Mesh::planeNum_intersects_direction_vector(const DirectX::BoundingOrientedBox& player, DirectX::XMFLOAT3& playerSpeed, DirectX::XMFLOAT3* newSpeed)
{
	// �ϴ� �ϵ��ڵ� �غ���
// ���� �����
	float length[6];
	//�־��� �� = player.Center;
	//������ ������
	//x = player.Center.x + playerSpeed.x * t 
	//m_AABBCenter.x- m_AABBExtents_Divide.x = player.Center.x + playerSpeed.x * t
	//	playerSpeed.x * t = m_AABBCenter.x - m_AABBExtents_Divide.x - player.Center.x
	// P(x,y,z) = P0 + t * v;
	// t = (P(x,y,z) - p0) / v;
	//DirectX::XMFLOAT3 planes[6] = {
	//	m_AABBCenter.x - m_AABBExtents_Divide.x, 0, 0,

	//}
	DirectX::XMFLOAT3 planes[6];
	planes[0] = { m_AABBCenter.x - m_AABBExtents_Divide.x, 0, 0 };	// ������
	planes[1] = { m_AABBCenter.x + m_AABBExtents_Divide.x, 0, 0 };	// ������
	planes[2] = { 0, m_AABBCenter.y - m_AABBExtents_Divide.y, 0 };	// �ϸ�
	planes[3] = { 0, m_AABBCenter.y + m_AABBExtents_Divide.y, 0 };	// ���
	planes[4] = { 0, 0, m_AABBCenter.z - m_AABBExtents_Divide.z };	// ����
	planes[5] = { 0, 0, m_AABBCenter.z + m_AABBExtents_Divide.z };	// �ĸ�

	DirectX::XMVECTOR playerpos = DirectX::XMLoadFloat3(&player.Center);
	for (int i = 0; i < 6; ++i)
	{
		float t;
		if (planes[i].x != 0)
		{
			if (playerSpeed.x != 0)
				t = (planes[i].x - player.Center.x) / playerSpeed.x;
			else
			{
				*newSpeed = playerSpeed;
				return;
			}
		}
		else if (planes[i].y != 0)
		{
			if(playerSpeed.y != 0)
				t = (planes[i].y - player.Center.y) / playerSpeed.y;
			else
			{
				*newSpeed = playerSpeed;
				return;
			}
		}
		else if (planes[i].z != 0)
		{
			if (playerSpeed.z != 0)
				t = (planes[i].z - player.Center.z) / playerSpeed.z;
			else
			{
				*newSpeed = playerSpeed;
				return;
			}
		}
		DirectX::XMFLOAT3 plane = { player.Center.x + t * playerSpeed.x, player.Center.y + t * playerSpeed.y, player.Center.z + t * playerSpeed.z };
		DirectX::XMVECTOR plane_vec = DirectX::XMLoadFloat3(&plane);
		DirectX::XMVECTOR playerToPlane = DirectX::XMVectorSubtract(plane_vec, playerpos);
		length[i] = DirectX::XMVectorGetX(DirectX::XMVector3Length(playerToPlane));
	}

	float minimum = FLT_MAX;
	int num;

	for (int i = 0; i < 6; ++i)
	{
		if (minimum > length[i])
		{
			minimum = length[i];
			num = i;
		}
	}
	// �浹�� �� ���ϱ� �Ϸ�

	DirectX::XMFLOAT3 normal[6];
	normal[0] = { -1,0,0 };
	normal[1] = { 1,0,0 };
	normal[2] = { 0,-1,0 };
	normal[3] = { 0,1,0 };
	normal[4] = { 0,0,-1 };
	normal[5] = { 0,0,1 };
	
	DirectX::XMVECTOR normal_vector = DirectX::XMLoadFloat3(&normal[num]);
	DirectX::XMVECTOR speed_vector = DirectX::XMLoadFloat3(&playerSpeed);
	float normal_length = -DirectX::XMVectorGetX(DirectX::XMVector3Dot(normal_vector, speed_vector));
	DirectX::XMFLOAT3 plane_normal = { normal[num].x * normal_length, normal[num].y * normal_length, normal[num].z * normal_length };
	DirectX::XMVECTOR plane_vector = DirectX::XMLoadFloat3(&plane_normal);
	DirectX::XMVECTOR result_speed = DirectX::XMVectorAdd(speed_vector, plane_vector);
	DirectX::XMStoreFloat3(newSpeed, result_speed);
}