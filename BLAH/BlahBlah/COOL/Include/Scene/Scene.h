#pragma once

// 씬 Enum
// 이거에 따라서 로드 할 씬의 종류가 달라짐
enum SCENE_TYPE {
	LOGO = 0,
	LOADING,			// loading
	MAIN,				// Main Scene
	SCENE_TYPE_MAX
};

// https://docs.unity3d.com/Packages/com.unity.render-pipelines.universal@17.0/manual/index.html
// urp 참조
// 씬이름.json에 포함되어야 할 내용
// 0. 지 이름과 타입
// 1. next scene의 json 파일 명
// 2. 로딩씬이 필요한지?
// 3. 카메라(이름, 위치, IsMain, 큐), isMain 이라면 scene에서 얘의 결과로 최종 렌더에 사용
// 3. 쉐이더 리스트 (이름, 큐, 카메라와 연결 되어있음, unity urp의 pass와 비슷한 역할이다)
// 4. 메터리얼 리스트 (쉐이더와 연결 되어있음)
// 5. 오브젝트 리스트 (타입(카메라,플레이어캐릭터,npc,건물)/메쉬/메터리얼)

// 의문점
// 다른 카메라에 포작되게 하려면 어떤 식으로 해야할까
// 다른 카메라에서 지가 원하는 메터리얼로 렌더하게 생성?

// 모든 로드는 씬에서 한다.

class MeshManager;
class ObjectManager;
class MaterialManager;
class ShaderManager;

class ObjectBase;
class Camera;

class Scene
{
	friend class SceneManager;
public:
	Scene();
	virtual ~Scene();

private:
	std::string m_SceneName = "noname";

protected:
	// 서로 교환해야 하는 데이터가 많다.
	// SceneData 클래스로 묶고 한번에 사용할까?
	MeshManager* m_MeshManager = nullptr;
	MaterialManager* m_MaterialManager = nullptr;
	ObjectManager* m_ObjectManager = nullptr;
	ShaderManager* m_ShaderManager = nullptr;

	//virtual bool Init();
private:
	bool LoadScene(ComPtr<ID3D12GraphicsCommandList> commandList, const std::string& sceneName);

public:

	// 최초 진입시 할 행동 ex) 맵 로드, npc 생성 등등
	// true: 로딩 씬 필요 / 로딩하는 쓰레드를 생성한다. 로딩씬에게 알려줌
	// false: 로딩 씬 불필요
	virtual bool Enter(ComPtr<ID3D12GraphicsCommandList> commandList);
	// 메인루프 중 할 행동 ex) npc의 이동, 로딩씬이면 로딩 프로그레스바 퍼센트 올리기
	virtual void Update(float deltaTime) = 0;
	// 씬 끝나면 할 행동 ex) 객체 해제, 이런거?
	virtual void Exit() = 0;
	// 입력 처리. 해당 씬이 활성화되어 있을 때 할 입력 처리
	virtual bool ProcessInput(UINT msg, WPARAM wParam, LPARAM lParam) = 0;

	//void Render();
};

