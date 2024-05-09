#include <DirectXCollision.h>
#include <iostream>
#include <conio.h>
class character
{
public:
	int x;
	int y;
	int z;
	character(int posX, int posY, int posZ) :x(posX), y(posY), z(posZ) {}
};

void can_see(character& player, character& npc);
character wall(4, 0, 4);

int main()
{
	int map[10][10] = { 0, };

	character player(3, 0, 3);
	character npc(5, 0, 5);


	map[player.z][player.x] = 2;
	map[npc.z][npc.x] = 3;
	map[wall.z][wall.x] = 1;
	map[wall.z - 1][wall.x] = 1;
	map[wall.z + 1][wall.x] = 1;

	while (true)
	{
		system("cls");
		can_see(player, npc);
		for (int y = 0; y < 10; ++y)
		{
			std::cout << y << "\t";
			for (int x = 0; x < 10; ++x)
			{
				if (map[y][x] == 0)
					std::cout << "¡Û\t";
				else if (map[y][x] == 1)
					std::cout << "¡á\t";
				else if (map[y][x] == 2)
					std::cout << "¡Ú\t";
				else if (map[y][x] == 3)
					std::cout << "¡ß\t";
			}
			std::cout << std::endl;
		}

		int inputKey = _getch();
		switch (inputKey)
		{
		case 119:
			if (player.z > 0)
			{
				if (map[player.z - 1][player.x] == 0)
				{
					map[player.z][player.x] = 0;
					player.z -= 1;
					map[player.z][player.x] = 2;
				}
			}
			break;
		case 97:
			if (player.x > 0)
			{
				if (map[player.z][player.x - 1] == 0)
				{
					map[player.z][player.x] = 0;
					player.x -= 1;
					map[player.z][player.x] = 2;
				}
			}
			break;
		case 115:
			if (player.z < 9)
			{
				if (map[player.z + 1][player.x] == 0)
				{
					map[player.z][player.x] = 0;
					player.z += 1;
					map[player.z][player.x] = 2;
				}
			}
			break;
		case 100:
			if (player.x < 9)
			{
				if (map[player.z][player.x + 1] == 0)
				{
					map[player.z][player.x] = 0;
					player.x += 1;
					map[player.z][player.x] = 2;
				}
			}
			break;
		}
	}
}

void can_see(character& player, character& npc)
{
	DirectX::XMFLOAT3 playerCenter = { (float)player.x,(float)player.y,(float)player.z };
	DirectX::XMFLOAT4 orient = { 0,0,0,1 };
	//DirectX::XMFLOAT3 extent = { 0.5,0.5,0.5 };
	//DirectX::BoundingOrientedBox pBox(playerCenter, extent, orient);
	DirectX::XMFLOAT3 npcCenter = { (float)npc.x,(float)npc.y,(float)npc.z };
	//DirectX::BoundingOrientedBox npcBox(npcCenter, extent, orient);
	DirectX::XMFLOAT3 wallCenter = { (float)wall.x,(float)wall.y,(float)wall.z };
	DirectX::XMFLOAT3 wallExtent = { 0.5,0.5,1 };
	DirectX::BoundingOrientedBox wallBox(wallCenter, wallExtent, orient);
	
	float dist = 0;
	
	DirectX::XMVECTOR origin = DirectX::XMLoadFloat3(&npcCenter);
	DirectX::XMVECTOR direction = DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&playerCenter),origin);
	if (wallBox.Intersects(origin,DirectX::XMVector3Normalize(direction), dist))
	{
		std::cout<<dist <<"¾Èº¸¿µ"<<std::endl;
	}
	
}