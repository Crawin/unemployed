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

int find_near_NODE(DirectX::XMFLOAT3& pos, std::unordered_map<int,NODE*>& astar_graph)
{
	std::vector<std::pair<int, float>> nodes;
	for (auto& node : astar_graph) {
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

PATH* aStarSearch(DirectX::XMFLOAT3& curr, DirectX::XMFLOAT3& goal, std::unordered_map<int, NODE*>& graph)
{
	int start_id = find_near_NODE(curr, graph);
	//std::cout << "가장 가까운 시작 노드는 [" << start_id << "] 입니다." << std::endl;
	int goal_id = find_near_NODE(goal, graph);
	//std::cout << "가장 가까운 도착 노드는 [" << goal_id << "] 입니다." << std::endl;

	std::priority_queue<NODE*> openList;

	NODE* startNode = graph[start_id];
	NODE* goalNode = graph[goal_id];

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

void MakeGraph(std::unordered_map<int, NODE*>& graph)
{
	graph[0] = new NODE(0, 31.6, 0, -4);
	graph[1] = new NODE(1, 31.16, 0, 10.74);
	graph[2] = new NODE(2, 25.25, 0, 6.84);
	graph[3] = new NODE(3, 25.13, 0, 10.85);
	graph[4] = new NODE(4, 26.98, 0, 6.82);
	graph[5] = new NODE(5, 24.8, 0, 3.52);
	graph[6] = new NODE(6, 27.13, 0, 3.65);
	graph[7] = new NODE(7, 27.15, 0, -9.13);
	graph[8] = new NODE(8, 0.84, 0, 10.98);
	graph[9] = new NODE(9, -0.49, 0, 6.65);
	graph[10] = new NODE(10, -4.03, 0, 6.77);

	graph[11] = new NODE(11, -4.29, 0, 12.36);
	graph[12] = new NODE(12, -4.137, 0, -6.713);
	graph[13] = new NODE(13, -4.214, 2.84, -13.746);
	graph[14] = new NODE(14, -1.96, 2.84, -13.81);
	graph[15] = new NODE(15, -6.59, 2.84, -13.79);
	graph[16] = new NODE(16, -6.41, 5.08, -7.94);
	graph[17] = new NODE(17, -1.89, 5.08, -8);
	graph[18] = new NODE(18, -4.26, 5.08, -5.15);
	graph[19] = new NODE(19, -33.3, 5.08, -5.17);
	graph[20] = new NODE(20, -4.18, 5.08, 6.08);

	graph[21] = new NODE(21, -26.72, 5.08, 5.83);
	graph[22] = new NODE(22, 16.38, 5.08, 6.14);
	graph[23] = new NODE(23, 22.48, 5.08, 6.14);
	graph[24] = new NODE(24, 22.71, 5.08, 2.57);
	graph[25] = new NODE(25, 16.54, 5.08, 2.73);
	graph[26] = new NODE(26, 15.35, 5.08, -5.35);
	graph[27] = new NODE(27, 22.62, 5.08, -4.5);
	graph[28] = new NODE(28, 27.72, 5.08, -9.69);
	graph[29] = new NODE(29, 20.73, 5.08, -0.77);
	graph[30] = new NODE(30, 27.369, 5.08, -0.873);
	graph[31] = new NODE(31, 12.9, 0, 11.11);


	graph[0]->neighbors.emplace_back(graph[1]);

	graph[1]->neighbors.emplace_back(graph[0]);
	graph[1]->neighbors.emplace_back(graph[3]);

	graph[2]->neighbors.emplace_back(graph[3]);
	graph[2]->neighbors.emplace_back(graph[4]);
	graph[2]->neighbors.emplace_back(graph[5]);

	graph[3]->neighbors.emplace_back(graph[1]);
	graph[3]->neighbors.emplace_back(graph[2]);
	graph[3]->neighbors.emplace_back(graph[31]);

	graph[4]->neighbors.emplace_back(graph[2]);
	graph[4]->neighbors.emplace_back(graph[6]);

	graph[5]->neighbors.emplace_back(graph[2]);
	graph[5]->neighbors.emplace_back(graph[6]);

	graph[6]->neighbors.emplace_back(graph[4]);
	graph[6]->neighbors.emplace_back(graph[5]);
	graph[6]->neighbors.emplace_back(graph[7]);

	graph[7]->neighbors.emplace_back(graph[6]);

	graph[8]->neighbors.emplace_back(graph[31]);
	graph[8]->neighbors.emplace_back(graph[9]);

	graph[9]->neighbors.emplace_back(graph[8]);
	graph[9]->neighbors.emplace_back(graph[10]);

	graph[10]->neighbors.emplace_back(graph[9]);
	graph[10]->neighbors.emplace_back(graph[11]);
	graph[10]->neighbors.emplace_back(graph[12]);

	graph[11]->neighbors.emplace_back(graph[10]);

	graph[12]->neighbors.emplace_back(graph[10]);
	graph[12]->neighbors.emplace_back(graph[13]);

	graph[13]->neighbors.emplace_back(graph[12]);
	graph[13]->neighbors.emplace_back(graph[14]);
	graph[13]->neighbors.emplace_back(graph[15]);

	graph[14]->neighbors.emplace_back(graph[13]);
	graph[14]->neighbors.emplace_back(graph[17]);

	graph[15]->neighbors.emplace_back(graph[13]);
	graph[15]->neighbors.emplace_back(graph[16]);

	graph[16]->neighbors.emplace_back(graph[15]);
	graph[16]->neighbors.emplace_back(graph[17]);
	graph[16]->neighbors.emplace_back(graph[18]);

	graph[17]->neighbors.emplace_back(graph[14]);
	graph[17]->neighbors.emplace_back(graph[16]);
	graph[17]->neighbors.emplace_back(graph[18]);

	graph[18]->neighbors.emplace_back(graph[16]);
	graph[18]->neighbors.emplace_back(graph[17]);
	graph[18]->neighbors.emplace_back(graph[19]);
	graph[18]->neighbors.emplace_back(graph[20]);
	graph[18]->neighbors.emplace_back(graph[26]);

	graph[19]->neighbors.emplace_back(graph[18]);

	graph[20]->neighbors.emplace_back(graph[18]);
	graph[20]->neighbors.emplace_back(graph[21]);
	graph[20]->neighbors.emplace_back(graph[22]);

	graph[21]->neighbors.emplace_back(graph[20]);

	graph[22]->neighbors.emplace_back(graph[20]);
	graph[22]->neighbors.emplace_back(graph[23]);
	graph[22]->neighbors.emplace_back(graph[25]);

	graph[23]->neighbors.emplace_back(graph[22]);
	graph[23]->neighbors.emplace_back(graph[24]);

	graph[24]->neighbors.emplace_back(graph[23]);
	graph[24]->neighbors.emplace_back(graph[25]);
	graph[24]->neighbors.emplace_back(graph[29]);

	graph[25]->neighbors.emplace_back(graph[22]);
	graph[25]->neighbors.emplace_back(graph[24]);
	graph[25]->neighbors.emplace_back(graph[29]);

	graph[26]->neighbors.emplace_back(graph[18]);
	graph[26]->neighbors.emplace_back(graph[27]);
	graph[26]->neighbors.emplace_back(graph[29]);
	graph[26]->neighbors.emplace_back(graph[25]);

	graph[27]->neighbors.emplace_back(graph[26]);
	graph[27]->neighbors.emplace_back(graph[28]);
	graph[27]->neighbors.emplace_back(graph[29]);

	graph[28]->neighbors.emplace_back(graph[27]);

	graph[29]->neighbors.emplace_back(graph[24]);
	graph[29]->neighbors.emplace_back(graph[25]);
	graph[29]->neighbors.emplace_back(graph[26]);
	graph[29]->neighbors.emplace_back(graph[27]);
	graph[29]->neighbors.emplace_back(graph[30]);

	graph[30]->neighbors.emplace_back(graph[29]);

	graph[31]->neighbors.emplace_back(graph[3]);
	graph[31]->neighbors.emplace_back(graph[8]);
}