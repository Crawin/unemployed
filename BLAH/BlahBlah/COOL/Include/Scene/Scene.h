#pragma once

class Scene
{
public:
	Scene();
	virtual ~Scene();

private:
	std::string m_SceneName = "이름을 짓지 않았다";

public:
	// 씬매니저
	//static Scene& GetInstance() {
	//	static Scene inst;
	//	return inst;
	//}

	virtual bool Init();

	// 최초 진입시 할 행동 ex) 맵 로드, npc 생성 등등
	virtual void Enter() = 0;
	// 메인루프 중 할 행동 ex) npc의 이동, 로딩씬이면 로딩 프로그레스바 퍼센트 올리기
	virtual void Update() = 0;
	// 씬 끝나면 할 행동 ex) 객체 해제, 이런거?
	virtual void Exit() = 0;
	// 입력 처리, 굳이 나눌 필요가 있을까
	virtual void ProcessMouseInput(UINT msg, int x, int y) = 0;
	virtual void ProcessKeyboardInput(unsigned char key) = 0;

	//void Render();
};

