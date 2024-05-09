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
	m_AABBExtents_Divide = DirectX::XMFLOAT3(abs(m_AABBExtents.x *0.5), abs(m_AABBExtents.y *0.5), abs(m_AABBExtents.z *0.5));
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

	if (player.Intersects(obb) && m_VertexNum != 0)
	{
		std::cout << this->m_Name << std::endl;
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
		if (length[i] < 0)
			continue;
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

void Mesh::planeNum_intersects_direction_vector_v3(const DirectX::BoundingOrientedBox& player, const Mesh& target, DirectX::XMFLOAT3& playerSpeed, DirectX::XMFLOAT3* newSpeed, DirectX::XMFLOAT3* newPosition, std::chrono::steady_clock::time_point& sendTime, std::chrono::nanoseconds& ping)
{
	float length[6];
	DirectX::XMFLOAT3 planes[6];
	planes[0] = { target.m_AABBCenter.x - target.m_AABBExtents_Divide.x, 0, 0 };	// ������
	planes[1] = { target.m_AABBCenter.x + target.m_AABBExtents_Divide.x, 0, 0 };	// ������
	planes[2] = { 0, target.m_AABBCenter.y - target.m_AABBExtents_Divide.y, 0 };	// �ϸ�
	planes[3] = { 0, target.m_AABBCenter.y + target.m_AABBExtents_Divide.y, 0 };	// ���
	planes[4] = { 0, 0, target.m_AABBCenter.z - target.m_AABBExtents_Divide.z };	// ����
	planes[5] = { 0, 0, target.m_AABBCenter.z + target.m_AABBExtents_Divide.z };	// �ĸ�

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
			if (playerSpeed.y != 0)
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
		if (length[i] < 0)
			continue;
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

	//auto deltaTime = std::chrono::high_resolution_clock::now() - sendTime + ping;
	//auto b = std::chrono::duration<float, std::chrono::seconds::period>(deltaTime);
	//auto c = b.count();
	//newPosition->x += newSpeed->x * c;
	//newPosition->z += newSpeed->z * c;
	//normal[num].x 
	//newPosition->x = 
}

bool Mesh::can_see(DirectX::XMFLOAT3& playerPos,DirectX::XMFLOAT3& npcPos,const unsigned short& floor)
{
	DirectX::XMVECTOR ray = DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&playerPos), DirectX::XMLoadFloat3(&npcPos));	// ��� -> �÷��̾��� ����
	if (m_Childs[0].m_Childs[floor].ray_collision(npcPos, ray))
		return true;
	return false;
}

bool Mesh::ray_collision(DirectX::XMFLOAT3& startPos, DirectX::XMVECTOR& ray)
{
	DirectX::XMFLOAT4 orient(0, 0, 0, 1);
	float ray_length = DirectX::XMVectorGetX(DirectX::XMVector3Length(ray));
	for (auto& wall : m_Childs)
	{
		DirectX::BoundingOrientedBox obb(wall.m_AABBCenter, wall.m_AABBExtents_Divide, orient);
		float dist = 0;
		if (ray_length > 0) {
			if (obb.Intersects(DirectX::XMLoadFloat3(&startPos),DirectX::XMVector3Normalize(ray), dist))
			{
				if (dist < ray_length)
				{
					//std::cout << wall.m_Name << "������ �Ⱥ���" << std::endl;
					return false;
				}
			}
			//std::cout << "dist : "<<dist << std::endl;
		}
	}
	return true;
}

bool Mesh::collision_v3(DirectX::BoundingOrientedBox& player, DirectX::XMFLOAT3* newPosition, DirectX::XMFLOAT3& playerSpeed, DirectX::XMFLOAT3* newSpeed ,unsigned short* ptrFloor, std::chrono::steady_clock::time_point& sendTime, std::chrono::nanoseconds& ping)
{
	int player_floor = m_Childs[0].m_Childs[0].floor_collision(player, newPosition, newSpeed, ptrFloor);	// �� �浹

	if (m_Childs[0].m_Childs[player_floor].map_collision(player, newPosition, playerSpeed, newSpeed,sendTime,ping))				// �ش� ���� ���� �浹
	{
		return true;
	}
	return false;
}

const int Mesh::floor_collision(const DirectX::BoundingOrientedBox& player, DirectX::XMFLOAT3* newPosition, DirectX::XMFLOAT3* newSpeed, unsigned short* ptrFloor)
{
	DirectX::XMFLOAT4 orient(0, 0, 0, 1);

	int i = 1;
	for (auto& floor : m_Childs)
	{
		DirectX::BoundingOrientedBox obb(floor.m_AABBCenter, floor.m_AABBExtents_Divide, orient);
		if (player.Intersects(obb))
		{
			if (player.Center.y >= floor.m_AABBCenter.y)
			{
				newPosition->y = floor.m_AABBCenter.y + floor.m_AABBExtents_Divide.y;
				//newSpeed->y = 0;
				//std::cout << i << "���� ����" << std::endl;
				*ptrFloor = i;
				return i;
			}
		}
		++i;
	}			// ���� �ٴڰ� �浹�Ͽ�����

	for (int i = 0; i < 2; ++i)
	{
		if (player.Center.y >= m_Childs[i].m_AABBCenter.y && player.Center.y < m_Childs[i + 1].m_AABBCenter.y)
		{
			//std::cout << i + 1 << "�� " << i + 2 << "�� ���̿� ����" << std::endl;
			*ptrFloor = i+1;
			newPosition->y = player.Center.y - player.Extents.y;
			return i + 1;
		}
	}			// ���� �� ���̿� �ִ���
}

bool Mesh::map_collision(DirectX::BoundingOrientedBox& player, DirectX::XMFLOAT3* newPosition, DirectX::XMFLOAT3& playerSpeed, DirectX::XMFLOAT3* newSpeed, std::chrono::steady_clock::time_point& sendTime, std::chrono::nanoseconds& ping)
{
	DirectX::XMFLOAT4 orient(0, 0, 0, 1);
	for (auto& wall : m_Childs)
	{
		DirectX::BoundingOrientedBox obb(wall.m_AABBCenter, wall.m_AABBExtents_Divide, orient);
		auto deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - sendTime + ping).count();
		player.Center.x += playerSpeed.x * deltaTime;
		player.Center.y += playerSpeed.y * deltaTime;
		player.Center.z += playerSpeed.z * deltaTime;
		if (player.Intersects(obb))
		{
			//std::cout << wall.m_Name << "�� �浹" << std::endl;
			planeNum_intersects_direction_vector_v3(player, wall, playerSpeed, newSpeed, newPosition,sendTime, ping);
			return true;
		}
	}
	return false;
}
