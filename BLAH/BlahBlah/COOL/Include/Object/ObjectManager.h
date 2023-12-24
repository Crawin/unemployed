#pragma once

// �갡 �� ��
// ī�޶� �ø��� �ؼ� ������ ������Ʈ�� �ε����� �ѱ�
// 
// DOD(Data Oriented Design, ĳ�ù̽��� ���̱� ���� ���� ��� �ϴ� �͵� �� ���� �͵��� ���ӵ� �޸𸮿� �ִ´�. 
// boundingorientedbox�� ũ�Ⱑ 40, xmfloat4x4�� ũ�Ⱑ 64�̴�. 
// ���� ĳ�ð� �������� ���� ũ�Ⱑ 64����Ʈ�ε� ���� ȿ���� ������? ���� ������ ������
// ���� ��� ū �������� �����͸� dod�� ���� ū �̵��� ����.
// �̵��� ���� �ʹٸ� position���� �͸� ���°� �̵��� �� �ϴ�.
// 
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
	ObjectManager();
	~ObjectManager();

	bool LoadFolder(const std::string& pathName);

	ObjectBase* GetObjectFromName(const std::string& name);

	void Update(float deltaTime);

private:
	int m_NextID = 0;

	std::vector<ObjectBase*> m_Objects;

};

