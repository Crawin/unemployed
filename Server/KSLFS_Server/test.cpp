#include <iostream>
#include <vector>
#include <unordered_map>
#include <queue>

class NODE {
public:
	int id;								// ����� ���� ���̵�
	std::vector<NODE*> neighbors;		// �̿� ����
	NODE* parent;						// �θ� ���
	double g;							// ���� �������� ���� �������� ���� ���
	double h;							// ���� ��忡�� ��ǥ �������� �޸���ƽ ����ġ (���� ��ǥ���� ��ǥ ��ǥ ������)
	double f;							// f = g + h
	float x, y, z;							// ��ǥ
	NODE(int id, float posX, float posY, float posZ) :id(id), x(posX),y(posY), z(posZ), g(0), h(0), f(0), parent(nullptr) {}
	bool operator<(const NODE* n) const {
		return this->f > n->f;
	}
	void heuristic(NODE* goal) {
		this->h = (goal->x - this->x) * (goal->x - this->x) + (goal->z - this->z) * (goal->z - this->z);
		this->f = this->g + this->h;
	}

	void printPath()
	{
		std::vector<int> path;
		NODE* node = this;
		while (node) {
			path.push_back(node->id);
			node = node->parent;
		}
		std::reverse(path.begin(), path.end());

		for (int id : path) {
			std::cout << id << " ";
		}
		std::cout << std::endl;
	}
	void init()
	{
		g = 0;
		h = 0;
		f = 0;
		parent = nullptr;
	}
};

std::unordered_map<int, NODE*> graph;

int find_near_NODE(float posX,float posY, float posZ)
{
	std::vector<std::pair<int, float>> nodes;
	for (auto& node : graph) {
		auto a = (node.second->x - posX) * (node.second->x - posX) + (node.second->z - posZ) * (node.second->z - posZ) + (node.second->y - posY) * (node.second->y - posY);
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

void aStarSearch(float currX, float currY, float currZ, float goalX, float goalY, float goalZ)
{
	int start_id = find_near_NODE(currX, currY, currZ);
	std::cout << "���� ����� ���� ���� [" << start_id << "] �Դϴ�." << std::endl;
	int goal_id = find_near_NODE(goalX, goalY, goalZ);
	std::cout << "���� ����� ���� ���� [" << goal_id << "] �Դϴ�." << std::endl;

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
			currentNode->printPath();
			return;
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
				((currentNode->x - neighborNode->x) * (currentNode->x - neighborNode->x) +
					(currentNode->y - neighborNode->y) * (currentNode->y - neighborNode->y) +
					(currentNode->z - neighborNode->z) * (currentNode->z - neighborNode->z));

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
}

int main()
{
	graph[0] = new NODE(0, -1.10343,0, 6.714495);
	graph[1] = new NODE(1, 12.64,0, 11.44);
	graph[2] = new NODE(2, 25.35,0, 11.24);
	graph[3] = new NODE(3, -4.59,0, -8.74);
	graph[4] = new NODE(4, 25.41,0, 6.42);
	graph[5] = new NODE(5, 30.81,0, 10.99);
	graph[6] = new NODE(6, 31.18,0, -8.53);
	graph[7] = new NODE(7, -4.346, 2.912, -14.193);
	graph[8] = new NODE(8, -1.89, 2.912, -14.19);
	graph[9] = new NODE(9, -1.89, 5.13 , -6.624);
	graph[10] = new NODE(10, 17.37, 5.13, -6.624);

	graph[0]->neighbors.push_back(graph[1]);
	graph[0]->neighbors.push_back(graph[3]);

	graph[1]->neighbors.push_back(graph[2]);
	graph[1]->neighbors.push_back(graph[4]);
	graph[1]->neighbors.push_back(graph[0]);

	graph[2]->neighbors.push_back(graph[1]);
	graph[2]->neighbors.push_back(graph[4]);
	graph[2]->neighbors.push_back(graph[5]);

	graph[3]->neighbors.push_back(graph[0]);
	graph[3]->neighbors.push_back(graph[7]);

	graph[4]->neighbors.push_back(graph[2]);

	graph[5]->neighbors.push_back(graph[2]);
	graph[5]->neighbors.push_back(graph[6]);

	graph[6]->neighbors.push_back(graph[5]);

	graph[7]->neighbors.push_back(graph[3]);
	graph[7]->neighbors.push_back(graph[8]);

	graph[8]->neighbors.push_back(graph[7]);
	graph[8]->neighbors.push_back(graph[9]);

	graph[9]->neighbors.push_back(graph[8]);
	graph[9]->neighbors.push_back(graph[10]);

	graph[10]->neighbors.push_back(graph[9]);

	float currX = 0;
	float currY = 0;
	float currZ = 0;

	float inputX, inputZ, inputY;
	while (true)
	{
		std::cout << "����ġ: (" << currX << ", " << currY << ", " << currZ << ") ��ǥ ��ġ�� �Է��Ͻÿ� : ";
		std::cin >> inputX;
		std::cin >> inputY;
		std::cin >> inputZ;
		aStarSearch(currX, currY, currZ, inputX, inputY, inputZ);
		for (auto& node : graph)
			node.second->init();
		currX = inputX;
		currY = inputY;
		currZ = inputZ;
	}

}