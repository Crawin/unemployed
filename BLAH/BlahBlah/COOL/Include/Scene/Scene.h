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

// 리소스 매니저라는 클래스가 있다 (기존 Mesh, Material, Shader를 통합함)
// Object를 로드 할 때 ResourceManager에 필요한 리소스를 추가해 가져오거나 재활용 함
class ResourceManager;

class ECSManager;

class Scene
{
	friend class SceneManager;
public:
	Scene();
	virtual ~Scene();

private:
	std::string m_SceneName = "noname";

protected:
	ResourceManager* m_ResourceManager = nullptr;

	// ECS System
	std::shared_ptr<ECSManager> m_ECSManager = nullptr;

	int m_ActiveLightSize = 0;

	//virtual bool Init();
private:
	bool LoadScene(ComPtr<ID3D12GraphicsCommandList> commandList, const std::string& sceneName);

protected:
	virtual bool AddSystem();

	virtual bool LoadSceneExtra(ComPtr<ID3D12GraphicsCommandList> commandList);

	void SetResourceHeap(ComPtr<ID3D12GraphicsCommandList> commandList);

	// render seq 01
	void AnimateToSO(ComPtr<ID3D12GraphicsCommandList> commandList);

	// pre render on mrt
	virtual void OnPreRender(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_CPU_DESCRIPTOR_HANDLE resultDsv) {}

	// render seq
	void RenderOnMRT(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_CPU_DESCRIPTOR_HANDLE resultDsv);

	// post render on mrt
	virtual void OnPostRender(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_CPU_DESCRIPTOR_HANDLE resultDsv) {}

	// render seq final - 2
	void UpdateLightData(ComPtr<ID3D12GraphicsCommandList> commandList);

	// render seq final - 1
	void PostProcessing(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_CPU_DESCRIPTOR_HANDLE resultRtv, D3D12_CPU_DESCRIPTOR_HANDLE resultDsv);

	// render seq final
	void DrawUI(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_CPU_DESCRIPTOR_HANDLE resultRtv, D3D12_CPU_DESCRIPTOR_HANDLE resultDsv);

public:

	// 최초 진입시 할 행동 ex) 맵 로드, npc 생성 등등
	// true: 로딩 씬 필요 / 로딩하는 쓰레드를 생성한다. 로딩씬에게 알려줌
	// false: 로딩 씬 불필요
	virtual bool Enter(ComPtr<ID3D12GraphicsCommandList> commandList);
	// 메인루프 중 할 행동 ex) npc의 이동, 로딩씬이면 로딩 프로그레스바 퍼센트 올리기
	virtual void Update(float deltaTime);
	// 씬 끝나면 할 행동 ex) 객체 해제, 이런거?
	virtual void Exit() = 0;
	// 입력 처리. 해당 씬이 활성화되어 있을 때 할 입력 처리
	virtual bool ProcessInput(UINT msg, WPARAM wParam, LPARAM lParam) = 0;

	// 최종 결과를 resultRtv, resultDsv에 넘긴다
	virtual void Render(std::vector<ComPtr<ID3D12GraphicsCommandList>>& commandLists, D3D12_CPU_DESCRIPTOR_HANDLE resultRtv, D3D12_CPU_DESCRIPTOR_HANDLE resultDsv);
};

