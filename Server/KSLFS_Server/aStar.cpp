#include <iostream>
#include <vector>
#include <unordered_map>
#include <queue>
#include <DirectXMath.h>
#include "aStar.h"

int find_near_NODE(DirectX::XMFLOAT3& pos)
{
	std::vector<std::pair<int, float>> nodes;
	for (auto& node : g_um_graph) {
		auto a = (node.second->pos.x - pos.x) * (node.second->pos.x - pos.x) + (node.second->pos.z - pos.z) * (node.second->pos.z - pos.z) + (node.second->pos.y - pos.y) * (node.second->pos.y - pos.y);
		nodes.emplace_back(node.first, a);
	}

	float nearest = 1E+37;
	int nearest_ID = 0;
	for (auto& length : nodes)
	{
		if (length.second < nearest)
		{
			nearest = length.second;
			nearest_ID = length.first;
		}
	}
	return nearest_ID;
}

PATH* aStarSearch(DirectX::XMFLOAT3& curr, DirectX::XMFLOAT3& goal)
{
	int start_id = find_near_NODE(curr);
	//std::cout << "���� ����� ���� ���� [" << start_id << "] �Դϴ�." << std::endl;
	int goal_id = find_near_NODE(goal);
	//std::cout << "���� ����� ���� ���� [" << goal_id << "] �Դϴ�." << std::endl;

	std::priority_queue<NODE*> openList;

	NODE* startNode = g_um_graph[start_id];
	NODE* goalNode = g_um_graph[goal_id];

	startNode->heuristic(goalNode);
	openList.push(startNode);

	std::unordered_map<int, bool> closedList;

	while (!openList.empty())
	{
		NODE* currentNode = openList.top();
		openList.pop();

		if (currentNode == goalNode) {
			return currentNode->printPath();
		}

		// ���� ��带 ���� ��Ͽ� �߰�
		closedList[currentNode->id] = true;

		// ���� ����� �̿� ������ Ž��
		for (const auto& neighbor : currentNode->neighbors) {
			NODE* neighborNode = neighbor;

			// �̿� ��尡 ���� ��Ͽ� ������ ���
			if (closedList[neighborNode->id]) {
				continue;
			}

			double tentativeG = currentNode->g +
				((currentNode->pos.x - neighborNode->pos.x) * (currentNode->pos.x - neighborNode->pos.x) +
					(currentNode->pos.y - neighborNode->pos.y) * (currentNode->pos.y - neighborNode->pos.y) +
					(currentNode->pos.z - neighborNode->pos.z) * (currentNode->pos.z - neighborNode->pos.z));

			// ���ο� ����� ��� �Ǵ� �� ���� ����� ���
			if (tentativeG < neighborNode->g || neighborNode->parent == nullptr) {
				neighborNode->parent = currentNode;
				neighborNode->g = tentativeG;
				neighborNode->heuristic(goalNode);

				openList.push(neighborNode);
			}
		}
	}

	// ��ǥ ������ �������� ���� ���
	std::cout << "��ǥ ��忡 ������ �� �����ϴ�." << std::endl;
	return nullptr;
}

void MakeGraph()
{
	g_um_graph[0] = new NODE(0, 31.6, 0, -4);
	g_um_graph[1] = new NODE(1, 31.16, 0, 10.74);
	g_um_graph[2] = new NODE(2, 25.25, 0, 6.84);
	g_um_graph[3] = new NODE(3, 25.13, 0, 10.85);
	g_um_graph[4] = new NODE(4, 26.98, 0, 6.82);
	g_um_graph[5] = new NODE(5, 24.8, 0, 3.52);
	g_um_graph[6] = new NODE(6, 27.13, 0, 3.65);
	g_um_graph[7] = new NODE(7, 27.15, 0, -9.13);
	g_um_graph[8] = new NODE(8, 0.84, 0, 10.98);
	g_um_graph[9] = new NODE(9, -0.49, 0, 6.65);
	g_um_graph[10] = new NODE(10, -4.03, 0, 6.77);

	g_um_graph[11] = new NODE(11, -4.29, 0, 12.36);
	g_um_graph[12] = new NODE(12, -4.137, 0, -6.713);
	g_um_graph[13] = new NODE(13, -4.214, 2.84, -13.746);
	g_um_graph[14] = new NODE(14, -1.96, 2.84, -13.81);
	g_um_graph[15] = new NODE(15, -6.59, 2.84, -13.79);
	g_um_graph[16] = new NODE(16, -6.41, 5.08, -7.94);
	g_um_graph[17] = new NODE(17, -1.89, 5.08, -8);
	g_um_graph[18] = new NODE(18, -4.26, 5.08, -5.15);
	g_um_graph[19] = new NODE(19, -33.3, 5.08, -5.17);
	g_um_graph[20] = new NODE(20, -4.18, 5.08, 6.08);

	g_um_graph[21] = new NODE(21, -26.72, 5.08, 5.83);
	g_um_graph[22] = new NODE(22, 16.38, 5.08, 6.14);
	g_um_graph[23] = new NODE(23, 22.48, 5.08, 6.14);
	g_um_graph[24] = new NODE(24, 22.71, 5.08, 2.57);
	g_um_graph[25] = new NODE(25, 16.54, 5.08, 2.73);
	g_um_graph[26] = new NODE(26, 15.35, 5.08, -5.35);
	g_um_graph[27] = new NODE(27, 22.62, 5.08, -4.5);
	g_um_graph[28] = new NODE(28, 27.72, 5.08, -9.69);
	g_um_graph[29] = new NODE(29, 20.73, 5.08, -0.77);
	g_um_graph[30] = new NODE(30, 27.369, 5.08, -0.873);
	g_um_graph[31] = new NODE(31, 12.9,0, 11.11);


	g_um_graph[0]->neighbors.emplace_back(g_um_graph[1]);

	g_um_graph[1]->neighbors.emplace_back(g_um_graph[0]);
	g_um_graph[1]->neighbors.emplace_back(g_um_graph[3]);

	g_um_graph[2]->neighbors.emplace_back(g_um_graph[3]);
	g_um_graph[2]->neighbors.emplace_back(g_um_graph[4]);
	g_um_graph[2]->neighbors.emplace_back(g_um_graph[5]);

	g_um_graph[3]->neighbors.emplace_back(g_um_graph[1]);
	g_um_graph[3]->neighbors.emplace_back(g_um_graph[2]);
	g_um_graph[3]->neighbors.emplace_back(g_um_graph[31]);

	g_um_graph[4]->neighbors.emplace_back(g_um_graph[2]);
	g_um_graph[4]->neighbors.emplace_back(g_um_graph[6]);

	g_um_graph[5]->neighbors.emplace_back(g_um_graph[2]);
	g_um_graph[5]->neighbors.emplace_back(g_um_graph[6]);

	g_um_graph[6]->neighbors.emplace_back(g_um_graph[4]);
	g_um_graph[6]->neighbors.emplace_back(g_um_graph[5]);
	g_um_graph[6]->neighbors.emplace_back(g_um_graph[7]);

	g_um_graph[7]->neighbors.emplace_back(g_um_graph[6]);

	g_um_graph[8]->neighbors.emplace_back(g_um_graph[31]);
	g_um_graph[8]->neighbors.emplace_back(g_um_graph[9]);

	g_um_graph[9]->neighbors.emplace_back(g_um_graph[8]);
	g_um_graph[9]->neighbors.emplace_back(g_um_graph[10]);

	g_um_graph[10]->neighbors.emplace_back(g_um_graph[9]);
	g_um_graph[10]->neighbors.emplace_back(g_um_graph[11]);
	g_um_graph[10]->neighbors.emplace_back(g_um_graph[12]);

	g_um_graph[11]->neighbors.emplace_back(g_um_graph[10]);

	g_um_graph[12]->neighbors.emplace_back(g_um_graph[10]);
	g_um_graph[12]->neighbors.emplace_back(g_um_graph[13]);

	g_um_graph[13]->neighbors.emplace_back(g_um_graph[12]);
	g_um_graph[13]->neighbors.emplace_back(g_um_graph[14]);
	g_um_graph[13]->neighbors.emplace_back(g_um_graph[15]);

	g_um_graph[14]->neighbors.emplace_back(g_um_graph[13]);
	g_um_graph[14]->neighbors.emplace_back(g_um_graph[17]);

	g_um_graph[15]->neighbors.emplace_back(g_um_graph[13]);
	g_um_graph[15]->neighbors.emplace_back(g_um_graph[16]);

	g_um_graph[16]->neighbors.emplace_back(g_um_graph[15]);
	g_um_graph[16]->neighbors.emplace_back(g_um_graph[17]);
	g_um_graph[16]->neighbors.emplace_back(g_um_graph[18]);

	g_um_graph[17]->neighbors.emplace_back(g_um_graph[14]);
	g_um_graph[17]->neighbors.emplace_back(g_um_graph[16]);
	g_um_graph[17]->neighbors.emplace_back(g_um_graph[18]);

	g_um_graph[18]->neighbors.emplace_back(g_um_graph[16]);
	g_um_graph[18]->neighbors.emplace_back(g_um_graph[17]);
	g_um_graph[18]->neighbors.emplace_back(g_um_graph[19]);
	g_um_graph[18]->neighbors.emplace_back(g_um_graph[20]);
	g_um_graph[18]->neighbors.emplace_back(g_um_graph[26]);

	g_um_graph[19]->neighbors.emplace_back(g_um_graph[18]);

	g_um_graph[20]->neighbors.emplace_back(g_um_graph[18]);
	g_um_graph[20]->neighbors.emplace_back(g_um_graph[21]);
	g_um_graph[20]->neighbors.emplace_back(g_um_graph[22]);

	g_um_graph[21]->neighbors.emplace_back(g_um_graph[20]);

	g_um_graph[22]->neighbors.emplace_back(g_um_graph[20]);
	g_um_graph[22]->neighbors.emplace_back(g_um_graph[23]);
	g_um_graph[22]->neighbors.emplace_back(g_um_graph[25]);

	g_um_graph[23]->neighbors.emplace_back(g_um_graph[22]);
	g_um_graph[23]->neighbors.emplace_back(g_um_graph[24]);

	g_um_graph[24]->neighbors.emplace_back(g_um_graph[23]);
	g_um_graph[24]->neighbors.emplace_back(g_um_graph[25]);
	g_um_graph[24]->neighbors.emplace_back(g_um_graph[29]);

	g_um_graph[25]->neighbors.emplace_back(g_um_graph[22]);
	g_um_graph[25]->neighbors.emplace_back(g_um_graph[24]);
	g_um_graph[25]->neighbors.emplace_back(g_um_graph[29]);

	g_um_graph[26]->neighbors.emplace_back(g_um_graph[18]);
	g_um_graph[26]->neighbors.emplace_back(g_um_graph[27]);
	g_um_graph[26]->neighbors.emplace_back(g_um_graph[29]);
	g_um_graph[26]->neighbors.emplace_back(g_um_graph[25]);

	g_um_graph[27]->neighbors.emplace_back(g_um_graph[26]);
	g_um_graph[27]->neighbors.emplace_back(g_um_graph[28]);
	g_um_graph[27]->neighbors.emplace_back(g_um_graph[29]);

	g_um_graph[28]->neighbors.emplace_back(g_um_graph[27]);

	g_um_graph[29]->neighbors.emplace_back(g_um_graph[24]);
	g_um_graph[29]->neighbors.emplace_back(g_um_graph[25]);
	g_um_graph[29]->neighbors.emplace_back(g_um_graph[26]);
	g_um_graph[29]->neighbors.emplace_back(g_um_graph[27]);
	g_um_graph[29]->neighbors.emplace_back(g_um_graph[30]);

	g_um_graph[30]->neighbors.emplace_back(g_um_graph[29]);

	g_um_graph[31]->neighbors.emplace_back(g_um_graph[3]);
	g_um_graph[31]->neighbors.emplace_back(g_um_graph[8]);
}