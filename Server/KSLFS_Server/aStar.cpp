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
			return currentNode->printPath(goal);
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

void compare_length_next_path(PATH*& path, const DirectX::XMFLOAT3 NPC, const DirectX::XMFLOAT3 goal)
{
	float goal_npc_length = (NPC.x - goal.x) * (NPC.x - goal.x) + (NPC.z - goal.z) * (NPC.z - goal.z);
	while (goal_npc_length < (path->pos.x - goal.x) * (path->pos.x - goal.x) + (path->pos.z - goal.z) * (path->pos.z - goal.z))	// 목적지 - npc 까지의 거리보다, 목적지 - 시작노드 까지의 거리다 더 길다 == npc가 시작노드로 되돌아 가야한다.
	{
		PATH* next = path->next;
		delete path;
		path = next;
		if (path == nullptr)
			break;
	}
}

//void compare_length_next_path(const DirectX::XMFLOAT3& NPC, const DirectX::XMFLOAT3& goal)
//{

//}


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
	graph[32] = new NODE(32, -4.205, 5.08, -8.518);
	graph[33] = new NODE(33, -4.205, 6.919, -12.363);
	graph[34] = new NODE(34, -6.214, 6.919, -13.36);
	graph[35] = new NODE(35, -2.013, 6.919, -13.36);
	graph[36] = new NODE(36, -6.214, 8.648, -7.357);
	graph[37] = new NODE(37, -2.24, 8.648, -7.357);
	graph[38] = new NODE(38, -4.013, 8.648, -8.376);
	graph[39] = new NODE(39, -4.013, 8.648, -5.062);

	graph[40] = new NODE(40, -33.422, 8.648, -5.062);
	graph[41] = new NODE(41, -4.013, 8.648, 6.265);
	graph[42] = new NODE(42, 14.2, 8.648, 6.265);
	graph[43] = new NODE(43, 26.149, 8.648, 6.265);
	graph[44] = new NODE(44, 14.2, 8.648, -5.06);
	graph[45] = new NODE(45, 24.08, 8.648, -5.06);
	graph[46] = new NODE(46, 27.386, 8.648, -9.672);
	graph[47] = new NODE(47, -26.676, 8.648, 6.265);
	graph[48] = new NODE(48, -4.013, 10.505, -12.439);
	graph[49] = new NODE(49, -6.347, 10.505, -13.807);

	graph[50] = new NODE(50, -1.984, 10.505, -13.807);
	graph[51] = new NODE(51, -6.347, 12.306, -8.708);
	graph[52] = new NODE(52, -4.261, 14.113, -12.242);
	graph[53] = new NODE(53, -2.026, 12.306, -8.554);
	graph[54] = new NODE(54, -4.261, 12.306, 6.153);
	graph[55] = new NODE(55, -33.529, 12.306, -5.05);
	graph[56] = new NODE(56, -4.261, 12.306, -5.05);
	graph[57] = new NODE(57, -26.703, 12.306, 6.153);
	graph[58] = new NODE(58, 25.78, 12.306, 6.153);
	graph[59] = new NODE(59, 24.6, 12.306, -5.05);

	graph[60] = new NODE(60, 26.33, 12.306, -9.356);
	graph[61] = new NODE(61, -4.261, 12.306, -8.554);
	graph[62] = new NODE(62, -6.576, 14.113, -13.549);
	graph[63] = new NODE(63, -1.927, 14.113, -13.549);
	graph[64] = new NODE(64, -6.576, 15.908, -8.343);
	graph[65] = new NODE(65, -1.867, 15.908, -8.343);
	graph[66] = new NODE(66, -4.27, 15.908, -8.343);
	graph[67] = new NODE(67, -4.27, 15.908, -5.195);
	graph[68] = new NODE(68, -4.27, 15.908, 6.06);
	graph[69] = new NODE(69, -33.673, 15.908, -5.195);

	graph[70] = new NODE(70, -26.62, 15.908, 6.06);
	graph[71] = new NODE(71, 14.14, 15.908, 6.06);
	graph[72] = new NODE(72, 25.73, 15.908, 6.06);
	graph[73] = new NODE(73, 13.99, 15.908, -5.195);
	graph[74] = new NODE(74, 24.83, 15.908, -5.195);
	graph[75] = new NODE(75, 27.28, 15.908, -9.49);
	graph[76] = new NODE(76, -4.27, 17.698, -12.491);
	graph[77] = new NODE(77, -6.354, 17.698, -13.565);
	graph[78] = new NODE(78, -1.832, 17.698, -13.565);

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
	graph[16]->neighbors.emplace_back(graph[32]);

	graph[17]->neighbors.emplace_back(graph[14]);
	graph[17]->neighbors.emplace_back(graph[16]);
	graph[17]->neighbors.emplace_back(graph[18]);
	graph[16]->neighbors.emplace_back(graph[32]);

	graph[18]->neighbors.emplace_back(graph[16]);
	graph[18]->neighbors.emplace_back(graph[17]);
	graph[18]->neighbors.emplace_back(graph[19]);
	graph[18]->neighbors.emplace_back(graph[20]);
	graph[18]->neighbors.emplace_back(graph[26]);
	graph[18]->neighbors.emplace_back(graph[32]);

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

	graph[32]->neighbors.emplace_back(graph[16]);
	graph[32]->neighbors.emplace_back(graph[17]);
	graph[32]->neighbors.emplace_back(graph[26]);
	graph[32]->neighbors.emplace_back(graph[33]);

	graph[33]->neighbors.emplace_back(graph[32]);
	graph[33]->neighbors.emplace_back(graph[34]);
	graph[33]->neighbors.emplace_back(graph[35]);

	graph[34]->neighbors.emplace_back(graph[33]);
	graph[34]->neighbors.emplace_back(graph[35]);
	graph[34]->neighbors.emplace_back(graph[36]);

	graph[35]->neighbors.emplace_back(graph[33]);
	graph[35]->neighbors.emplace_back(graph[34]);
	graph[35]->neighbors.emplace_back(graph[37]);

	graph[36]->neighbors.emplace_back(graph[37]);
	graph[36]->neighbors.emplace_back(graph[38]);
	graph[36]->neighbors.emplace_back(graph[37]);
	graph[36]->neighbors.emplace_back(graph[39]);

	graph[37]->neighbors.emplace_back(graph[35]);
	graph[37]->neighbors.emplace_back(graph[38]);
	graph[37]->neighbors.emplace_back(graph[36]);
	graph[37]->neighbors.emplace_back(graph[39]);

	graph[38]->neighbors.emplace_back(graph[36]);
	graph[38]->neighbors.emplace_back(graph[37]);
	graph[38]->neighbors.emplace_back(graph[39]);
	graph[38]->neighbors.emplace_back(graph[48]);

	graph[39]->neighbors.emplace_back(graph[41]);
	graph[39]->neighbors.emplace_back(graph[40]);
	graph[39]->neighbors.emplace_back(graph[44]);
	graph[39]->neighbors.emplace_back(graph[36]);
	graph[39]->neighbors.emplace_back(graph[37]);
	graph[39]->neighbors.emplace_back(graph[38]);

	graph[40]->neighbors.emplace_back(graph[39]);

	graph[41]->neighbors.emplace_back(graph[39]);
	graph[41]->neighbors.emplace_back(graph[42]);
	graph[41]->neighbors.emplace_back(graph[47]);

	graph[42]->neighbors.emplace_back(graph[41]);
	graph[42]->neighbors.emplace_back(graph[43]);
	graph[42]->neighbors.emplace_back(graph[44]);

	graph[43]->neighbors.emplace_back(graph[42]);

	graph[44]->neighbors.emplace_back(graph[39]);
	graph[44]->neighbors.emplace_back(graph[42]);
	graph[44]->neighbors.emplace_back(graph[45]);

	graph[45]->neighbors.emplace_back(graph[44]);
	graph[45]->neighbors.emplace_back(graph[46]);

	graph[46]->neighbors.emplace_back(graph[45]);

	graph[47]->neighbors.emplace_back(graph[41]);

	graph[48]->neighbors.emplace_back(graph[38]);
	graph[48]->neighbors.emplace_back(graph[49]);
	graph[48]->neighbors.emplace_back(graph[50]);

	graph[49]->neighbors.emplace_back(graph[48]);
	graph[49]->neighbors.emplace_back(graph[50]);
	graph[49]->neighbors.emplace_back(graph[51]);

	graph[50]->neighbors.emplace_back(graph[48]);
	graph[50]->neighbors.emplace_back(graph[49]);
	graph[50]->neighbors.emplace_back(graph[53]);

	graph[51]->neighbors.emplace_back(graph[49]);
	graph[51]->neighbors.emplace_back(graph[61]);
	graph[51]->neighbors.emplace_back(graph[56]);

	graph[52]->neighbors.emplace_back(graph[61]);
	graph[52]->neighbors.emplace_back(graph[62]);
	graph[52]->neighbors.emplace_back(graph[63]);

	graph[53]->neighbors.emplace_back(graph[50]);
	graph[53]->neighbors.emplace_back(graph[56]);
	graph[53]->neighbors.emplace_back(graph[61]);

	graph[54]->neighbors.emplace_back(graph[56]);
	graph[54]->neighbors.emplace_back(graph[57]);
	graph[54]->neighbors.emplace_back(graph[58]);

	graph[55]->neighbors.emplace_back(graph[56]);

	graph[56]->neighbors.emplace_back(graph[51]);
	graph[56]->neighbors.emplace_back(graph[53]);
	graph[56]->neighbors.emplace_back(graph[61]);
	graph[56]->neighbors.emplace_back(graph[55]);
	graph[56]->neighbors.emplace_back(graph[54]);
	graph[56]->neighbors.emplace_back(graph[59]);

	graph[57]->neighbors.emplace_back(graph[54]);

	graph[58]->neighbors.emplace_back(graph[54]);

	graph[59]->neighbors.emplace_back(graph[56]);
	graph[59]->neighbors.emplace_back(graph[60]);

	graph[60]->neighbors.emplace_back(graph[59]);

	graph[61]->neighbors.emplace_back(graph[51]);
	graph[61]->neighbors.emplace_back(graph[53]);
	graph[61]->neighbors.emplace_back(graph[56]);
	graph[61]->neighbors.emplace_back(graph[52]);

	graph[62]->neighbors.emplace_back(graph[52]);
	graph[62]->neighbors.emplace_back(graph[63]);
	graph[62]->neighbors.emplace_back(graph[64]);

	graph[63]->neighbors.emplace_back(graph[52]);
	graph[63]->neighbors.emplace_back(graph[62]);
	graph[63]->neighbors.emplace_back(graph[65]);

	graph[64]->neighbors.emplace_back(graph[62]);
	graph[64]->neighbors.emplace_back(graph[66]);
	graph[64]->neighbors.emplace_back(graph[67]);

	graph[65]->neighbors.emplace_back(graph[63]);
	graph[65]->neighbors.emplace_back(graph[66]);
	graph[65]->neighbors.emplace_back(graph[67]);

	graph[66]->neighbors.emplace_back(graph[64]);
	graph[66]->neighbors.emplace_back(graph[65]);
	graph[66]->neighbors.emplace_back(graph[67]);
	graph[66]->neighbors.emplace_back(graph[76]);
	
	graph[67]->neighbors.emplace_back(graph[64]);
	graph[67]->neighbors.emplace_back(graph[65]);
	graph[67]->neighbors.emplace_back(graph[66]);
	graph[67]->neighbors.emplace_back(graph[68]);
	graph[67]->neighbors.emplace_back(graph[69]);
	graph[67]->neighbors.emplace_back(graph[73]);

	graph[68]->neighbors.emplace_back(graph[67]);
	graph[68]->neighbors.emplace_back(graph[70]);
	graph[68]->neighbors.emplace_back(graph[71]);

	graph[69]->neighbors.emplace_back(graph[67]);

	graph[70]->neighbors.emplace_back(graph[68]);

	graph[71]->neighbors.emplace_back(graph[68]);
	graph[71]->neighbors.emplace_back(graph[73]);
	graph[71]->neighbors.emplace_back(graph[72]);

	graph[72]->neighbors.emplace_back(graph[71]);

	graph[73]->neighbors.emplace_back(graph[67]);
	graph[73]->neighbors.emplace_back(graph[71]);
	graph[73]->neighbors.emplace_back(graph[74]);

	graph[74]->neighbors.emplace_back(graph[73]);
	graph[74]->neighbors.emplace_back(graph[75]);

	graph[75]->neighbors.emplace_back(graph[74]);

	graph[76]->neighbors.emplace_back(graph[66]);
	graph[76]->neighbors.emplace_back(graph[77]);
	graph[76]->neighbors.emplace_back(graph[78]);

	graph[77]->neighbors.emplace_back(graph[76]);
	graph[77]->neighbors.emplace_back(graph[78]);

	graph[78]->neighbors.emplace_back(graph[76]);
	graph[78]->neighbors.emplace_back(graph[77]);

}