#pragma once

// �갡 �� ��
// ī�޶� �ø��� �ؼ� ������ ������Ʈ�� �ε����� �ѱ�
// 
// DOD(Data Oriented Design, ĳ�ù̽��� ���̱� ���� ���� ��� �ϴ� �͵� �� ���� �͵��� ���ӵ� �޸𸮿� �ִ´�. 
// boundingorientedbox�� ũ�Ⱑ 40, xmfloat4x4�� ũ�Ⱑ 64�̴�. 
// ĳ�ÿ� �������� ���� ũ�Ⱑ 64����Ʈ�ε� ���� ȿ���� ������? ���� ������ ������
// 
// 
//		ex) �ø��� �浹üũ �� ���� �ٿ�� �ڽ�
// �׷� �갡 ������ ���� ����?
// ������Ʈ���� �迭
// ������Ʈ���� OBB �迭
// ������Ʈ���� ��� �迭 <- �ʿ��ұ�? �ϴ� �ּ�ó�� ����
// ������ �����ؾ� �ϱ� ������ �̱������� ���� �ʴ´�.
//		������ �����ؾ� �ϴ� ����
//		A�� �÷��� �� B�� �ε� �� ���δ�.


class ObjectBase;

class ObjectManager
{
public:
	ObjectManager() {}
	~ObjectManager() {}

	void Init() {}

	void InsertObject(ObjectBase* obj);

	void Update(float deltaTime);

private:
	int m_NextID = 0;

	std::vector<ObjectBase*> m_Objects;
	std::vector<BoundingOrientedBox> m_ObjectOBBs;
	std::vector<XMFLOAT4X4> m_ObjectWorldMatrices;
};

