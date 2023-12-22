#pragma once

// 오브젝트의 로드는 씬에서 한다.

class Scene
{
public:
	Scene();
	virtual ~Scene();

private:
	std::string m_SceneName = "이름을 짓지 않았다";

public:
	// 씬매니저로 이동
	//static Scene& GetInstance() {
	//	static Scene inst;
	//	return inst;
	//}

	virtual bool Init();

	// 최초 진입시 할 행동 ex) 맵 로드, npc 생성 등등
	// true: 로딩 씬 필요 / 로딩하는 쓰레드를 생성한다. 로딩씬에게 알려줌
	// false: 로딩 씬 불필요
	virtual bool Enter() = 0;
	// 메인루프 중 할 행동 ex) npc의 이동, 로딩씬이면 로딩 프로그레스바 퍼센트 올리기
	virtual void Update() = 0;
	// 씬 끝나면 할 행동 ex) 객체 해제, 이런거?
	virtual void Exit() = 0;
	// 입력 처리. 해당 씬이 활성화되어 있을 때 할 입력 처리
	virtual bool ProcessInput(UINT msg, WPARAM wParam, LPARAM lParam) = 0;

	//void Render();
};

