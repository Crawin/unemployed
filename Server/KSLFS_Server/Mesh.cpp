#include "IOCP_Common.h"
#include "Mesh.h"

void Mesh::LoadMeshData(std::ifstream& meshFile)
{
	if (false)
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
		m_AABBExtents_Divide = DirectX::XMFLOAT3(abs(m_AABBExtents.x * 0.5), abs(m_AABBExtents.y * 0.5), abs(m_AABBExtents.z * 0.5));
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
	else
	{					// 24.06.27 완료 할것.
		// 메쉬 파일 구조
		// 1. 이름 길이					// int
		// 2. 이름						// char*
		// 3. 바운딩박스					// float3 x 3
		// 4. 부모 상대 변환 행렬			// float4x4
		// 5. 버텍스 타입				// int
		// 6. 버텍스 정보				// int, int*(pos, nor, tan, uv)			// int pos int nor int tan int uv, + uint4(bleidng bone) + float4 (blending weight)
		// 7. 인덱스 정보				// int int*int
		// 8. 서브메쉬 개수				// int
		// 9. 서브메쉬(이름길이 이름 버텍스정보 서브메쉬...개수)		// 재귀로 파고들어라
		//

		// 1. 이름 길이
		using namespace DirectX;
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
		XMFLOAT3 min, max;
		meshFile.read((char*)&min, sizeof(XMFLOAT3));
		meshFile.read((char*)&max, sizeof(XMFLOAT3));
		meshFile.read((char*)&m_AABBCenter, sizeof(XMFLOAT3));

		XMVECTOR ext = (XMLoadFloat3(&max) - XMLoadFloat3(&min)) / 2.0f;
		XMStoreFloat3(&m_AABBExtents, ext);

		// 4. 부모 상대 변환 행렬		// float4x4
		meshFile.read((char*)&m_LocalTransform, sizeof(XMFLOAT4X4));

		auto centervec = XMLoadFloat3(&m_AABBCenter);
		auto centervec4d = XMVectorSetW(centervec, 1);

		auto worldmat = XMLoadFloat4x4(&m_LocalTransform);

		auto worldlocation = XMVector4Transform(centervec4d, worldmat);
		
		XMStoreFloat3(&m_AABBCenter, worldlocation);
		//XMStoreFloat4x4(&m_LocalTransform, XMMatrixIdentity());
		

		// bounding box
		m_AABBExtents.x = abs(m_AABBExtents.x);
		m_AABBExtents.y = abs(m_AABBExtents.y);
		m_AABBExtents.z = abs(m_AABBExtents.z);
		m_ModelBoundingBox = BoundingOrientedBox(m_AABBCenter, m_AABBExtents, XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));

		// 5. 버텍스 타입 // 사용 할 듯 하다. ex) 스킨메쉬 vs 일반메쉬
		unsigned int vtxType;
		meshFile.read((char*)&vtxType, sizeof(unsigned int));

		// 6. 버텍스 정보
		unsigned int vertexLen = 0;

		meshFile.read((char*)&vertexLen, sizeof(unsigned int));
		if (vertexLen > 0) {

			switch (vtxType) {
			case 0:			// NO_VERTEX
				std::cout << "노 버텍스" << std::endl;
				break;

			case 1:			// NORMAL
			{
				//std::cout << "노멀" << std::endl;
				UINT size = sizeof(Vertex) * vertexLen;
				size = ((size + 255) & ~255);
				char* data = new char[size];
				meshFile.read(data, sizeof(Vertex) * vertexLen);
				
				vertecies = reinterpret_cast<Vertex*>(data);
				//for (int i = 0; i < vertexLen; ++i)
				//{
				//	std::cout << i << ": ";
				//	std::cout << "Position: (" << vertecies[i].position.x << ", " << vertecies[i].position.y << ", " << vertecies[i].position.z << ")" << std::endl;
				//	std::cout << "Normal: (" << vertecies[i].normal.x << ", " << vertecies[i].normal.y << ", " << vertecies[i].normal.z << ")" << std::endl;
				//	std::cout << "Tangent: (" << vertecies[i].tangent.x << ", " << vertecies[i].tangent.y << ", " << vertecies[i].tangent.z << ")" << std::endl;
				//	std::cout << "UV: (" << vertecies[i].uv.x << ", " << vertecies[i].uv.y << ")" << std::endl;
				//	std::cout << std::endl;
				//}
				break;
			}
			case 2:			// SKINNED
				std::cout << "스킨드" << std::endl;
				//LoadVertices<SkinnedVertex>(commandList, file, manager, vertexLen);
				//m_BoneIdx = GetBone(commandList, fileName, manager);
				//m_IsSkinned = true;
				break;
			}

			m_VertexNum = vertexLen;
		}

		//manager->AddMesh(this);

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
		if (length[i] < 0)
			continue;
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

void Mesh::planeNum_intersects_direction_vector_v3(const DirectX::BoundingOrientedBox& player, const Mesh& target, DirectX::XMFLOAT3& playerSpeed, DirectX::XMFLOAT3* newSpeed, DirectX::XMFLOAT3* newPosition, std::chrono::steady_clock::time_point& sendTime, std::chrono::nanoseconds& ping)
{
	float length[6];
	DirectX::XMFLOAT3 planes[6];
	planes[0] = { target.m_AABBCenter.x - target.m_AABBExtents_Divide.x, 0, 0 };	// 좌측면
	planes[1] = { target.m_AABBCenter.x + target.m_AABBExtents_Divide.x, 0, 0 };	// 우측면
	planes[2] = { 0, target.m_AABBCenter.y - target.m_AABBExtents_Divide.y, 0 };	// 하면
	planes[3] = { 0, target.m_AABBCenter.y + target.m_AABBExtents_Divide.y, 0 };	// 상면
	planes[4] = { 0, 0, target.m_AABBCenter.z - target.m_AABBExtents_Divide.z };	// 전면
	planes[5] = { 0, 0, target.m_AABBCenter.z + target.m_AABBExtents_Divide.z };	// 후면

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
	DirectX::XMVECTOR ray = DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&playerPos), DirectX::XMLoadFloat3(&npcPos));	// 경비병 -> 플레이어의 벡터
	if (m_Childs[0].m_Childs[floor+1].ray_collision(npcPos, ray))
		return true;
	return false;
}

bool Mesh::sight_block(DirectX::XMFLOAT3& playerPos, DirectX::XMFLOAT3& npcPos)
{
	DirectX::XMVECTOR ray = DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&playerPos), DirectX::XMLoadFloat3(&npcPos));	// 경비병 -> 플레이어의 벡터
	if (ray_collision(npcPos, ray))
		return true;
	return false;
}

bool Mesh::ray_collision(DirectX::XMFLOAT3& startPos, DirectX::XMVECTOR& ray)
{
	//DirectX::XMFLOAT4 orient(0, 0, 0, 1);
	float ray_length = DirectX::XMVectorGetX(DirectX::XMVector3Length(ray));

	float dist = 0;
	if (ray_length > 0)
	{
		if (this->m_ModelBoundingBox.Intersects(DirectX::XMLoadFloat3(&startPos), DirectX::XMVector3Normalize(ray), dist))
		{
			if (dist < ray_length)
			{
				return true;			// 충돌했다.
			}
		}
	}
	return false;						// 충돌하지 않았다.

	//for (auto& wall : m_Childs)
	//{
	//	DirectX::BoundingOrientedBox obb(wall.m_AABBCenter, wall.m_AABBExtents_Divide, orient);
	//	float dist = 0;
	//	if (ray_length > 0) {
	//		if (obb.Intersects(DirectX::XMLoadFloat3(&startPos),DirectX::XMVector3Normalize(ray), dist))
	//		{
	//			if (dist < ray_length)
	//			{
	//				//std::cout << wall.m_Name << "때문에 안보영" << std::endl;
	//				return false;
	//			}
	//		}
	//		//std::cout << "dist : "<<dist << std::endl;
	//	}
	//}
	//return true;
}

bool Mesh::collision_v3(DirectX::BoundingOrientedBox& player, DirectX::XMFLOAT3* newPosition, DirectX::XMFLOAT3& playerSpeed, DirectX::XMFLOAT3* newSpeed ,unsigned short* ptrFloor, std::chrono::steady_clock::time_point& sendTime, std::chrono::nanoseconds& ping)
{
	int player_floor = m_Childs[0].m_Childs[0].floor_collision(player, newPosition, newSpeed,reinterpret_cast<short*>(ptrFloor));	// 층 충돌 기존 unsignd 인걸 short로 변경했기에 오류 발생 가능성이 매우 높음

	if (m_Childs[0].m_Childs[player_floor].map_collision(player, newPosition, playerSpeed, newSpeed,sendTime,ping))				// 해당 층의 벽과 충돌
	{
		return true;
	}
	return false;
}

const int Mesh::floor_collision(const DirectX::BoundingOrientedBox& player, DirectX::XMFLOAT3* newPosition, DirectX::XMFLOAT3* newSpeed, short* ptrFloor)
{
	DirectX::XMFLOAT4 orient(0, 0, 0, 1);

	int i = 0;
	for (auto& level : m_Childs)
	{
		for (auto& floor : level.m_Childs)
		{
			DirectX::BoundingOrientedBox obb(floor.m_AABBCenter, floor.m_AABBExtents_Divide, orient);
			if (player.Intersects(obb))
			{
				if (player.Center.y >= floor.m_AABBCenter.y)
				{
					newPosition->y = floor.m_AABBCenter.y + floor.m_AABBExtents_Divide.y;
					//newSpeed->y = 0;
					//std::cout << i << "층에 닿음" << std::endl;
					*ptrFloor = i;
					return i;
				}
			}
		}
		++i;
	}			// 층의 바닥과 충돌하였는지
	// 바닥과 충돌하지 않으면
	*ptrFloor = -1;
	return -1;

	//for (int i = 0; i < 2; ++i)
	//{
	//	if (player.Center.y >= m_Childs[i].m_AABBCenter.y && player.Center.y < m_Childs[i + 1].m_AABBCenter.y)
	//	{
	//		//std::cout << i + 1 << "과 " << i + 2 << "층 사이에 있음" << std::endl;
	//		*ptrFloor = i+1;
	//		newPosition->y = player.Center.y - player.Extents.y;
	//		return i + 1;
	//	}
	//}			// 층과 층 사이에 있는지
}

const bool Mesh::floor_collision(const DirectX::BoundingOrientedBox& player, float& ptrFloor, float& posY)
{
	for (auto& floor : this->m_Childs)
	{
		if (floor.m_ModelBoundingBox.Intersects(player))
		{
			if (ptrFloor != 6)
				posY = floor.m_AABBCenter.y + floor.m_AABBExtents.y;
			else
				posY = floor.m_AABBCenter.y;
			return true;
		}
	}
	if (ptrFloor == 0)	ptrFloor = 1;
	else	ptrFloor += 0.5;

	return false;
}

const bool Mesh::floor_collision(const DirectX::BoundingOrientedBox& player, float& ptrFloor)
{
	for (auto& floor : this->m_Childs)
	{
		if (floor.m_ModelBoundingBox.Intersects(player))
		{
			return true;
		}
	}
	if (ptrFloor == 0)	ptrFloor = 1;
	else	ptrFloor += 0.5;

	return false;
}

const bool Mesh::stair_collision(const DirectX::BoundingOrientedBox& player, float& posY, const int& i)
{
	for (auto& stair : this->m_Childs)
	{
		if (stair.m_ModelBoundingBox.Intersects(player))
		{
			switch (i)
			{
			case 0:				// +Z
				posY = (stair.m_AABBExtents.y / stair.m_AABBExtents.z) * (player.Center.z - (stair.m_AABBCenter.z - stair.m_AABBExtents.z)) + (stair.m_AABBCenter.y - stair.m_AABBExtents.y);
				break;
			case 1:				// -Z
				posY = (-stair.m_AABBExtents.y / stair.m_AABBExtents.z) * (player.Center.z - (stair.m_AABBCenter.z + stair.m_AABBExtents.z)) + (stair.m_AABBCenter.y - stair.m_AABBExtents.y);
				break;
			case 2:				// +X
				posY = (stair.m_AABBExtents.y / stair.m_AABBExtents.x) * (player.Center.x - (stair.m_AABBCenter.x - stair.m_AABBExtents.x)) + (stair.m_AABBCenter.y - stair.m_AABBExtents.y);
				break;
			case 3:				// -X
				posY = (-stair.m_AABBExtents.y / stair.m_AABBExtents.x) * (player.Center.x - (stair.m_AABBCenter.x + stair.m_AABBExtents.x)) + (stair.m_AABBCenter.y - stair.m_AABBExtents.y);
				break;
			}
			std::cout << "경비 y : " << posY << std::endl;
			return true;
		}
	}
	return false;
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
			//std::cout << wall.m_Name << "과 충돌" << std::endl;
			planeNum_intersects_direction_vector_v3(player, wall, playerSpeed, newSpeed, newPosition,sendTime, ping);
			return true;
		}
	}
	return false;
}
