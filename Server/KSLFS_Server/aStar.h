#pragma once
class PATH
{
public:
	DirectX::XMFLOAT3 pos;
	PATH* next = nullptr;
};

class NODE {
public:
	int id;								// ����� ���� ���̵�
	std::vector<NODE*> neighbors;		// �̿� ����
	NODE* parent;						// �θ� ���
	double g;							// ���� �������� ���� �������� ���� ���
	double h;							// ���� ��忡�� ��ǥ �������� �޸���ƽ ����ġ (���� ��ǥ���� ��ǥ ��ǥ ������)
	double f;							// f = g + h
	DirectX::XMFLOAT3 pos;							// ��ǥ
	NODE(int id, float posX, float posY, float posZ) :id(id), pos(posX*100,posY*100,posZ*100), g(0), h(0), f(0), parent(nullptr) {}

	bool operator<(const NODE* n) const {
		return this->f > n->f;
	}
	
	bool operator==(const NODE* n)const {
		return (this->pos.x == n->pos.x && this->pos.y == n->pos.y && this->pos.z == n->pos.z);
	}

	void heuristic(NODE* goal) {
		this->h = (goal->pos.x - this->pos.x) * (goal->pos.x - this->pos.x) + (goal->pos.y - this->pos.y) * (goal->pos.y - this->pos.y) + (goal->pos.z - this->pos.z) * (goal->pos.z - this->pos.z);
		this->f = this->g + this->h;
	}

	PATH* printPath()
	{
		PATH* path = nullptr;
		NODE* node = this;
		while (node) {
			PATH* newpath = new PATH;
			newpath->pos = node->pos;
			newpath->next = path;
			path = newpath;
			std::cout << "<= " << node->id;
			node = node->parent;
		}
		std::cout << std::endl;
		return path;
	}

	PATH* printPath(const DirectX::XMFLOAT3& goal)
	{
		PATH* path = new PATH;
		path->pos = goal;
		path->next = nullptr;
		NODE* node = this;
		while (node) {
			PATH* newpath = new PATH;
			newpath->pos = node->pos;
			newpath->next = path;
			path = newpath;
			std::cout << "<= " << node->id;
			node = node->parent;
		}
		std::cout << std::endl;
		return path;
	}

	void init()
	{
		g = 0;
		h = 0;
		f = 0;
		parent = nullptr;
	}
};

extern std::unordered_map<int, NODE*> g_um_graph;

int find_near_NODE(DirectX::XMFLOAT3& pos);
int find_near_NODE(DirectX::XMFLOAT3& pos, std::unordered_map<int, NODE*>& astar_graph);

PATH* aStarSearch(DirectX::XMFLOAT3& curr, DirectX::XMFLOAT3& goal);
PATH* aStarSearch(DirectX::XMFLOAT3& curr, DirectX::XMFLOAT3& goal, std::unordered_map<int, NODE*>& graph);

void compare_length_next_path(PATH*& path, const DirectX::XMFLOAT3 NPC, const DirectX::XMFLOAT3 goal);

void MakeGraph();
void MakeGraph(std::unordered_map<int, NODE*>& graph);