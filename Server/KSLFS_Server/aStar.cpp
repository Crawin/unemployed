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
	g_um_graph[0] = new NODE(0, -1.10343, 0, 6.714495);
	g_um_graph[1] = new NODE(1, 12.64, 0, 11.44);
	g_um_graph[2] = new NODE(2, 25.35, 0, 11.24);
	g_um_graph[3] = new NODE(3, -4.59, 0, -8.74);
	g_um_graph[4] = new NODE(4, 25.41, 0, 6.42);
	g_um_graph[5] = new NODE(5, 30.81, 0, 10.99);
	g_um_graph[6] = new NODE(6, 31.18, 0, -8.53);
	g_um_graph[7] = new NODE(7, -4.346, 2.912, -14.193);
	g_um_graph[8] = new NODE(8, -1.89, 2.912, -14.19);
	g_um_graph[9] = new NODE(9, -1.89, 5.13, -6.624);
	g_um_graph[10] = new NODE(10, 17.37, 5.13, -6.624);

	g_um_graph[0]->neighbors.emplace_back(g_um_graph[1]);
	g_um_graph[0]->neighbors.emplace_back(g_um_graph[3]);

	g_um_graph[1]->neighbors.emplace_back(g_um_graph[2]);
	g_um_graph[1]->neighbors.emplace_back(g_um_graph[4]);
	g_um_graph[1]->neighbors.emplace_back(g_um_graph[0]);

	g_um_graph[2]->neighbors.emplace_back(g_um_graph[1]);
	g_um_graph[2]->neighbors.emplace_back(g_um_graph[4]);
	g_um_graph[2]->neighbors.emplace_back(g_um_graph[5]);

	g_um_graph[3]->neighbors.emplace_back(g_um_graph[0]);
	g_um_graph[3]->neighbors.emplace_back(g_um_graph[7]);

	g_um_graph[4]->neighbors.emplace_back(g_um_graph[2]);

	g_um_graph[5]->neighbors.emplace_back(g_um_graph[2]);
	g_um_graph[5]->neighbors.emplace_back(g_um_graph[6]);

	g_um_graph[6]->neighbors.emplace_back(g_um_graph[5]);

	g_um_graph[7]->neighbors.emplace_back(g_um_graph[3]);
	g_um_graph[7]->neighbors.emplace_back(g_um_graph[8]);

	g_um_graph[8]->neighbors.emplace_back(g_um_graph[7]);
	g_um_graph[8]->neighbors.emplace_back(g_um_graph[9]);

	g_um_graph[9]->neighbors.emplace_back(g_um_graph[8]);
	g_um_graph[9]->neighbors.emplace_back(g_um_graph[10]);

	g_um_graph[10]->neighbors.emplace_back(g_um_graph[9]);
}