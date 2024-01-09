#pragma once

// 얘가 할 일
// 카메라 컬링을 해서 선별된 오브젝트의 인덱스를 넘김
// 
// DOD(Data Oriented Design, 캐시미스를 줄이기 위해 연속 사용 하는 것들 중 작은 것들을 연속된 메모리에 넣는다. 
// boundingorientedbox의 크기가 40, xmfloat4x4의 크기가 64이다. 
// 보통 캐시가 가져오는 블럭의 크기가 64바이트인데 과연 효과가 있을까? 차라리 본인이 가지게
// 실험 결과 큰 사이즈의 데이터를 dod로 만들어도 큰 이득은 없다.
// 이득을 보고 싶다면 position같은 것만 묶는게 이득일 듯 하다.
// 
// 
// 
//		ex) 컬링과 충돌체크 때 사용될 바운딩 박스
// 그럼 얘가 가지고 있을 것은?
// 오브젝트들의 배열
// 오브젝트들의 OBB 배열
// 오브젝트들의 행렬 배열 <- 필요할까? 일단 주석처리 하자
// 씬마다 존재해야 하기 때문에 싱글톤으로 하지 않는다.
//		씬마다 존재해야 하는 이유
//		A씬 플레이 중 B씬 로드 시 꼬인다.


class MaterialManager;
class MeshManager;

class ObjectBase;

class ObjectManager
{
public:
	ObjectManager();
	~ObjectManager();

	void RegisterMaterialManager(MaterialManager* materialManager) { m_MaterialManager = materialManager; }
	void RegisterMeshManager(MeshManager* meshManager) { m_MeshManager = meshManager; }

	bool LoadFile(const std::string& fileName);

	bool LoadFolder(const std::string& pathName);

	ObjectBase* GetObjectFromName(const std::string& name);

	void Update(float deltaTime);

private:
	int m_NextID = 0;

	std::vector<ObjectBase*> m_Objects;

	// delete 금지! 여기서 관리하는 객체가 아님
	MaterialManager* m_MaterialManager = nullptr;

	// delete 금지! 여기서 관리하는 객체가 아님
	MeshManager* m_MeshManager = nullptr;

};

