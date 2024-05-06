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
	//std::cout << "가장 가까운 시작 노드는 [" << start_id << "] 입니다." << std::endl;
	int goal_id = find_near_NODE(goal);
	//std::cout << "가장 가까운 도착 노드는 [" << goal_id << "] 입니다." << std::endl;

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

		// 현재 노드를 닫힌 목록에 추가
		closedList[currentNode->id] = true;

		// 현재 노드의 이웃 노드들을 탐색
		for (const auto& neighbor : currentNode->neighbors) {
			NODE* neighborNode = neighbor;

			// 이웃 노드가 닫힌 목록에 있으면 계속
			if (closedList[neighborNode->id]) {
				continue;
			}

			double tentativeG = currentNode->g +
				((currentNode->pos.x - neighborNode->pos.x) * (currentNode->pos.x - neighborNode->pos.x) +
					(currentNode->pos.y - neighborNode->pos.y) * (currentNode->pos.y - neighborNode->pos.y) +
					(currentNode->pos.z - neighborNode->pos.z) * (currentNode->pos.z - neighborNode->pos.z));

			// 새로운 노드인 경우 또는 더 나은 경로인 경우
			if (tentativeG < neighborNode->g || neighborNode->parent == nullptr) {
				neighborNode->parent = currentNode;
				neighborNode->g = tentativeG;
				neighborNode->heuristic(goalNode);

				openList.push(neighborNode);
			}
		}
	}

	// 목표 지점에 도달하지 못한 경우
	std::cout << "목표 노드에 도달할 수 없습니다." << std::endl;
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