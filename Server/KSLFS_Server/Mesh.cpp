#include "IOCP_Common.h"
#include "Mesh.h"

void Mesh::LoadMeshData(std::ifstream& meshFile)
{
	// 메쉬 파일 구조
// 1. 이름 길이					// int
// 2. 이름						// char*
// 3. 바운딩박스					// float3 x 3
// 4. 부모 상대 변환 행렬			// float4x4
// 5. 버텍스 타입				// int
// 6. 버텍스 정보				// int, int*(pos, nor, tan, uv)			// int pos int nor int tan int uv
// 7. 인덱스 정보				// int int*int
// 8. 서브메쉬 개수				// int
// 9. 서브메쉬(이름길이 이름 버텍스정보 서브메쉬...개수)		// 재귀로 파고들어라
//

// 1. 이름 길이
	unsigned int size;
	meshFile.read((char*)&size, sizeof(unsigned int));

	// 2. 이름
	if (size > 0) {
		char* name = new char[size + 1];
		name[size] = '\0';
		meshFile.read(name, size);
		m_Name = std::string(name);
		delete[] name;
	}

	// 3. 바운딩박스	
	DirectX::XMFLOAT3 min, max;
	meshFile.read((char*)&min, sizeof(DirectX::XMFLOAT3));
	meshFile.read((char*)&max, sizeof(DirectX::XMFLOAT3));
	meshFile.read((char*)&m_AABBCenter, sizeof(DirectX::XMFLOAT3));
	m_AABBExtents = DirectX::XMFLOAT3(max.x - min.x, max.y - min.y, max.z - min.z);
	m_AABBExtents_Divide = DirectX::XMFLOAT3(m_AABBExtents.x / 2, m_AABBExtents.y / 2, m_AABBExtents.z / 2);
	// 4. 부모 상대 변환 행렬		// float4x4
	meshFile.read((char*)&m_LocalTransform, sizeof(DirectX::XMFLOAT4X4));

	// 5. 버텍스 타입 // 사용 할 듯 하다. ex) 스킨메쉬 vs 일반메쉬
	unsigned int vtxType;
	meshFile.read((char*)&vtxType, sizeof(unsigned int));

	// 6. 버텍스 정보
	unsigned int vertexLen = 0;
	meshFile.read((char*)&vertexLen, sizeof(unsigned int));

	if (vertexLen > 0) {

		std::vector<Vertex> vertex(vertexLen);
		meshFile.read((char*)(&vertex[0]), sizeof(Vertex) * vertexLen);

		m_VertexNum = vertexLen;
	}

	// 8. 서브메쉬 개수
	unsigned int childNum;
	meshFile.read((char*)&childNum, sizeof(unsigned int));
	m_Childs.reserve(childNum);

	// 9. 서브메쉬(재귀)
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
	// 일단 하드코딩 해보자
// 왼쪽 면부터
	float length[6];
	//주어진 점 = player.Center;
	//직선의 방정식
	//x = player.Center.x + playerSpeed.x * t 
	//m_AABBCenter.x- m_AABBExtents_Divide.x = player.Center.x + playerSpeed.x * t
	//	playerSpeed.x * t = m_AABBCenter.x - m_AABBExtents_Divide.x - player.Center.x
	// P(x,y,z) = P0 + t * v;
	// t = (P(x,y,z) - p0) / v;
	//DirectX::XMFLOAT3 planes[6] = {
	//	m_AABBCenter.x - m_AABBExtents_Divide.x, 0, 0,

	//}
	DirectX::XMFLOAT3 planes[6];
	planes[0] = { m_AABBCenter.x - m_AABBExtents_Divide.x, 0, 0 };	// 좌측면
	planes[1] = { m_AABBCenter.x + m_AABBExtents_Divide.x, 0, 0 };	// 우측면
	planes[2] = { 0, m_AABBCenter.y - m_AABBExtents_Divide.y, 0 };	// 하면
	planes[3] = { 0, m_AABBCenter.y + m_AABBExtents_Divide.y, 0 };	// 상면
	planes[4] = { 0, 0, m_AABBCenter.z - m_AABBExtents_Divide.z };	// 전면
	planes[5] = { 0, 0, m_AABBCenter.z + m_AABBExtents_Divide.z };	// 후면

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
	// 충돌한 면 구하기 완료

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