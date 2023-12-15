#pragma once

class Scene;
// 사용 예시
// 1. 게임을 하다가(인게임씬) 다른 씬으로 이동해야한다(다음 스테이지, 메인메뉴, 게임오버씬)
//		인게임씬에서 게임오버가 되는 상황에 ChangeScene(게임오버씬)
//		1. 인게임씬.Exit() 호출
//		2. 게임오버씬 Enter() 호출
//		3. 인게임씬.Update()는 return;

class SceneManager
{
	SceneManager();
	~SceneManager();

	Scene* m_CurrentScene = nullptr;
	Scene* m_PrevScene = nullptr;
	Scene* m_NextScene = nullptr;

public:
	static SceneManager& GetInstance() {
		static SceneManager inst;
		return inst;
	}

	// 씬 전환에 로딩이 있다면 어쩌지? 로딩 씬을 강제로 넣어줘야 하지 않을까?
	// 이건 생각해보자
	void ChangeScene(Scene* newScene);

	void ProcessInput(UINT msg, WPARAM wParam, LPARAM lParam);

};

