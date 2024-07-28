#include "framework.h"
#include "TestMainScene.h"
#include "ResourceManager.h"
#include "ECS/ECSManager.h"
#include "Shader/Shader.h"
#include "ECS/TimeLine/TimeLine.h"
#include "Network/Client.h"

bool TestMainScene::LoadSceneExtra(ComPtr<ID3D12GraphicsCommandList> commandList)
{
    // load Sky material
    m_SkyMaterialIdx = m_ResourceManager->GetMaterial("_SkyRender", commandList);
    if (m_SkyMaterialIdx == -1) return false;

	m_SphereSkyMeshIndex = m_ResourceManager->GetMesh("Sphere", commandList);
	if (m_SphereSkyMeshIndex == -1) return false;

    return true;
}

void TestMainScene::OnPreRender(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_CPU_DESCRIPTOR_HANDLE resultDsv)
{
	// 추가된 부분
	//m_ResourceManager->SetPSO(commandList, m_SkyMaterialIdx);
	auto* skyMaterial = m_ResourceManager->GetMaterial(m_SkyMaterialIdx);
	skyMaterial->GetShader()->SetPipelineState(commandList);

	// set light data idx
	//skyMaterial->SetDataIndex(0, m_ResourceManager->m_LightIdx);
	// todo mainlight를 찾아야 함
	//skyMaterial->SetDataIndex(1, 0);
	//skyMaterial->SetDatas(commandList, static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT));

	int lightResIdx = m_ResourceManager->m_LightIdx;
	Mesh* sphere = m_ResourceManager->GetMesh(m_SphereSkyMeshIndex);
	commandList->ClearDepthStencilView(resultDsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	std::function<void(component::DayLight*, component::Light*)> func =
		[commandList, lightResIdx, sphere](component::DayLight* dayLight, component::Light* li) {

		if (dayLight->IsRender() == false) return;

		XMFLOAT4 noonLight = dayLight->GetNoonLight();
		XMFLOAT4 moonLight = dayLight->GetMoonLight();
		XMFLOAT4 sunsetLight = dayLight->GetSunSetLight();
		float angle = dayLight->GetLightAngle();

		LightData& light = li->GetLightData();
		//DebugPrintVector(light.m_Direction, "Dir: ");

		int lightIdx = li->GetIndex();

		commandList->SetGraphicsRoot32BitConstants(static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT), 4, &noonLight, static_cast<int>(DAYLIGHT_ROOTCONST::DAY_LIGHT));
		commandList->SetGraphicsRoot32BitConstants(static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT), 4, &moonLight, static_cast<int>(DAYLIGHT_ROOTCONST::MOON_LIGHT));
		commandList->SetGraphicsRoot32BitConstants(static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT), 4, &sunsetLight, static_cast<int>(DAYLIGHT_ROOTCONST::SUNSET_LIGHT));
		commandList->SetGraphicsRoot32BitConstants(static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT), 1, &lightResIdx, static_cast<int>(DAYLIGHT_ROOTCONST::LIGHT_RESOURCE_INDEX));
		commandList->SetGraphicsRoot32BitConstants(static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT), 1, &lightIdx, static_cast<int>(DAYLIGHT_ROOTCONST::MAIN_LIGHT_INDEX));
		commandList->SetGraphicsRoot32BitConstants(static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT), 1, &angle, static_cast<int>(DAYLIGHT_ROOTCONST::LIGHT_ANGLE));

		int vtxSize = sphere->GetVertexNum();
		sphere->SetVertexBuffer(commandList);
		commandList->DrawInstanced(vtxSize, 1, 0, 0);
		};

	m_ECSManager->Execute(func);

	commandList->ClearDepthStencilView(resultDsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
}

void TestMainScene::OnSelfHost()
{
	auto& client = Client::GetInstance();
	const SOCKET* playerSock = client.getPSock();

	if (playerSock[0]) {
		std::string playername = client.GetHostPlayerName();
		std::string othername = client.GetGuestPlayerName();

		std::function<void(component::Server*, component::Name*, component::Pawn*)>
			findAndSetID = [&playername, &othername, &playerSock](component::Server* ser, component::Name* na, component::Pawn* pawnComp) {
			if (na->getName() == playername) ser->setID(playerSock[0]);
			if (na->getName() == othername) pawnComp->SetControlServer(true);
			};
		m_ECSManager->Execute(findAndSetID);
	}
	else 
		ERROR_QUIT("ERROR_NO SOCKET");

	// todo
	// set can to send mode
	std::function<void(component::Server*, component::Drink*)> sendMode = [](component::Server* ser, component::Drink* dr) { ser->SetSendMode(true); };
	m_ECSManager->Execute(sendMode);
}

void TestMainScene::OnSelfGuest()
{
	auto& client = Client::GetInstance();
	const SOCKET* playerSock = client.getPSock();

	if (playerSock[0]) {
		std::string playername = client.GetGuestPlayerName();

		// set self
		std::function<void(component::Server*, component::Name*)>
			findAndSetID = [&playername, &playerSock](component::Server* ser, component::Name* na) {
			if (na->getName() == playername) ser->setID(playerSock[0]);
			};
		m_ECSManager->Execute(findAndSetID);

		std::string otherPlayer = client.GetHostPlayerName();
		// set other
		std::function<void(component::Server*, component::Name*, component::Pawn*)>
			findAndSetIDOnOther = [&otherPlayer, &playerSock](component::Server* ser, component::Name* na, component::Pawn* pawnComp) {
			if (na->getName() == otherPlayer) {
				ser->setID(playerSock[1]);
				pawnComp->SetControlServer(true);
			}
			};
		m_ECSManager->Execute(findAndSetIDOnOther);

		//	ID
		//	60~65 음료수
		//	70~72 CCTV
		//	80 RC

		// todo
		// set cctv, rc to send mode
		std::function<void(component::Server*, component::Name*)>
			toSendMode = [&otherPlayer, &playerSock](component::Server* ser, component::Name* na) {
			if (70 <= ser->getID() && ser->getID() <= 80) ser->SetSendMode(true);
			};
		m_ECSManager->Execute(toSendMode);

	}
	else
		ERROR_QUIT("ERROR_NO SOCKET");
}

void TestMainScene::OnGuestEnter()
{
	auto& client = Client::GetInstance();
	const SOCKET* playerSock = client.getPSock();

	if (playerSock[1]) {
		std::string playername = client.GetGuestPlayerName();

		std::function<void(component::Server*, component::Name*)>
			findAndSetID = [&playername, &playerSock](component::Server* ser, component::Name* na) {
			if (na->getName() == playername) ser->setID(playerSock[1]);
			};
		m_ECSManager->Execute(findAndSetID);
	}
	else
		ERROR_QUIT("ERROR_NO SOCKET");
}

bool TestMainScene::Enter(ComPtr<ID3D12GraphicsCommandList> commandList)
{
    Scene::Enter(commandList);
 
	// on main scene enter
	// check is host
	
	return true;
}

void TestMainScene::Update(float deltaTime)
{
    Scene::Update(deltaTime);
}

void TestMainScene::Exit()
{
}

bool TestMainScene::ProcessInput(UINT msg, WPARAM wParam, LPARAM lParam)
{
    return false;
}

void TestMainScene::OnServerConnected()
{
	short type = Client::GetInstance().getCharType();
	
	// if logged in

	// if host
	std::string playername;
	if (type == 1)
	{
		// start host
		OnSelfHost();
	}
	else {
		// start guest
		OnSelfGuest();
	}
}

void TestMainScene::OnGameStarted()
{
	ECSManager* manager = m_ECSManager.get();

	// reset player positions
	std::function<void(component::Player*, component::Transform*)> movePlayers = [manager](component::Player* pl, component::Transform* tr) {
		tr->SetPosition(pl->GetOriginalPosition());
		tr->SetRotation(pl->GetOriginalRotate());
		};
	manager->Execute(movePlayers);

	// get current pawn
	component::PlayerController* ctrler = nullptr;
	std::function<void(component::PlayerController*)> getCtrler = [&ctrler](component::PlayerController* control) { ctrler = control; };
	manager->Execute(getCtrler);
	component::Pawn* controlledPawn = ctrler->GetControllingPawn();
	Entity* originCam = controlledPawn->GetCameraEntity();
	component::Transform* originEntityTransform = manager->GetComponent<component::Transform>(controlledPawn->GetSelfEntity());
	component::Transform* originCamTransform = manager->GetComponent<component::Transform>(originCam);

	XMFLOAT3 finalPos;
	XMFLOAT4X4 parent = originEntityTransform->GetWorldTransform();
	XMFLOAT3 camPos = originCamTransform->GetPosition();
	XMFLOAT3 endPos = { 4470.0f, 160.84f, 920.0f };

	XMVECTOR camPosV = XMLoadFloat3(&camPos);
	XMMATRIX parentMat = XMLoadFloat4x4(&parent);

	XMStoreFloat3(&finalPos, XMVector3Transform(camPosV, parentMat));

	XMFLOAT3 endRot = { 0.0f, -90.0f, 0.0f };

	// get pawn to possess
	component::Pawn* startCutScenePawn[3] = { nullptr, };
	std::function<void(component::Pawn*, component::Name*)> getChangePawn = 
		[
			&startCutScenePawn
		]
		
		(component::Pawn* pawn, component::Name* name)
		{ 
			if (name->getName() == "GameStartCutScenePawn00") startCutScenePawn[0] = pawn;
			else if (name->getName() == "GameStartCutScenePawn01") startCutScenePawn[1] = pawn;
			else if (name->getName() == "GameStartCutScenePawn02") startCutScenePawn[2] = pawn;
		};
	manager->Execute(getChangePawn);
	ctrler->Possess(startCutScenePawn[0], manager);

	XMFLOAT3 endPositions[3] = {
		//{4919, 132.9, 931},
		{4056.5, 198.7, 1187.5},
		{2318.0, 234.0, 3559.0},
		finalPos
	};
	XMFLOAT3 endRotations[3] = {
		//{0, -90.0, 0},
		{20.418, 148.493, 0},
		{-16.392, -178.304, 0},
		endRot
	};

	//XMFLOAT3 endPositions[3] = {
	//{4056.5, 198.7, 1187.5},
	//{2318.0, 234.0, 3559.0},
	//finalPos
	//};
	//XMFLOAT3 endRotations[3] = {
	//	{20.418, 148.493, 0},
	//	{-16.392, -178.304, 0},
	//	endRot
	//};


	// cut scene 진행
	// 00 -> 00end포인트 이동(EndEvent로 아래)
	// 00end에서 애니메이션 재생 및 1초 대기 (EndEvent로 아래)
	// 1초 대기 이후 대화창 (원기학의 도착했습니다.) 및 2.5초 대기 (EndEvent로 아래)
	// 

	std::function TwoToEnd = [ctrler, controlledPawn, manager]() { ctrler->Possess(controlledPawn, manager); };
	std::function OneToTwo = [ctrler, TwoToEnd, startCutScenePawn, this, manager, endPositions, endRotations]() {

		Entity* playerEnt = manager->GetEntity(Client::GetInstance().GetHostPlayerName());
		component::AnimationController* ctrl = manager->GetComponent<component::AnimationController>(playerEnt);

		ctrl->ChangeAnimationTo(ANIMATION_STATE::BLENDED_MOVING_STATE);

		ctrler->Possess(startCutScenePawn[2], manager);
		CutScenePawnMoving(startCutScenePawn[2], TwoToEnd, endPositions[2], endRotations[2], 2.5f, true);
		};

	// end events
	std::function wait2SecAndZeroToOne = [ctrler, startCutScenePawn, manager, this, OneToTwo, endPositions, endRotations]() {
		
		std::function<void()> changeToOne = [ctrler, startCutScenePawn, manager, this, OneToTwo, endPositions, endRotations]() {
			ctrler->Possess(startCutScenePawn[1], manager);
			CutScenePawnMoving(startCutScenePawn[1], OneToTwo, endPositions[1], endRotations[1], 4.0f, true);
			};
		
		
		// find host player
		Entity* playerEnt = manager->GetEntity(Client::GetInstance().GetHostPlayerName());
		component::AnimationController* ctrl = manager->GetComponent<component::AnimationController>(playerEnt);

		ctrl->ChangeAnimationTo(ANIMATION_STATE::GET_PHONE_CALL);

		AddEndEventAfterTime(changeToOne, 8.0f);
		};



	// start to start cut scene pawn
	ctrler->Possess(startCutScenePawn[0], manager);
	CutScenePawnMoving(startCutScenePawn[0], wait2SecAndZeroToOne, endPositions[0], endRotations[0], 5.0f, true);


	// ui set
	std::function<void()> showConver = [this]() { ShowConversationUI(2, 2); };
	AddEndEventAfterTime(showConver, 10.0f);

	std::function<void()> showConver2 = [this]() { ShowConversationUI(2, 3); };
	AddEndEventAfterTime(showConver2, 12.0f);

	std::function<void()> showConver3 = [this]() { ShowConversationUI(2, 4); };
	AddEndEventAfterTime(showConver3, 14.0f);

	std::function<void()> showConver4 = [this]() { ShowConversationUI(3, 5); };
	AddEndEventAfterTime(showConver4, 16.0f);

	std::function<void()> showConver5 = [this]() { ShowConversationUI(2, 6); };
	AddEndEventAfterTime(showConver5, 18.0f);

	std::function<void()> showConver6 = [this]() { HideConversationUI(); };
	AddEndEventAfterTime(showConver6, 20.0f);
}

void TestMainScene::CutScenePawnMoving(component::Pawn* pawn, const std::function<void()>& endEvent, const XMFLOAT3& endPos, const XMFLOAT3& endRot, float endTime, bool eventOn)
{
	component::Transform* pawnTr = m_ECSManager->GetComponent<component::Transform>(pawn->GetSelfEntity());

	// position
	TimeLine<XMFLOAT3>* positionToEnd = new TimeLine<XMFLOAT3>(pawnTr->GetPositionPtr());
	XMFLOAT3 startPos = pawnTr->GetPosition();
	positionToEnd->AddKeyFrame(startPos, 0);
	positionToEnd->AddKeyFrame(startPos, 0.2f);
	positionToEnd->AddKeyFrame(endPos, endTime - 0.2f);
	positionToEnd->AddKeyFrame(endPos, endTime);
	if (eventOn) positionToEnd->SetEndEvent(endEvent);
	m_ECSManager->AddTimeLine(positionToEnd);

	// rotation
	TimeLine<XMFLOAT3>* rotateToEnd = new TimeLine<XMFLOAT3>(pawnTr->GetRotationPtr());
	XMFLOAT3 startRot = pawnTr->GetRotation();

	rotateToEnd->AddKeyFrame(startRot, 0);
	rotateToEnd->AddKeyFrame(startRot, 0.2f);
	rotateToEnd->AddKeyFrame(endRot, endTime - 0.5f);
	rotateToEnd->AddKeyFrame(endRot, endTime - 1.0f);
	m_ECSManager->AddTimeLine(rotateToEnd);
}

void TestMainScene::AddEndEventAfterTime(const std::function<void()>& endEvent, float endTime)
{
	// position
	TimeLine<float>* temp = new TimeLine<float>(nullptr);
	temp->AddKeyFrame(0.0f, 0.0f);
	temp->AddKeyFrame(0.0f, endTime);
	temp->SetEndEvent(endEvent);
	m_ECSManager->AddTimeLine(temp);
}

void TestMainScene::ShowConversationUI(int talker, int conversation)
{
	std::function<void(component::UICanvas*, component::UIConversation*)> ui = [talker, conversation](component::UICanvas* canv, component::UIConversation* conv) {
		conv->SetTalker(talker);
		conv->SetConversation(conversation);

		canv->ShowUI();
		};

	m_ECSManager->Execute(ui);
}

void TestMainScene::HideConversationUI()
{
	std::function<void(component::UICanvas*, component::UIConversation*)> ui = [](component::UICanvas* canv, component::UIConversation* conv) {
		canv->HideUI();
		};

	m_ECSManager->Execute(ui);
}

//void TestMainScene::Render(std::vector<ComPtr<ID3D12GraphicsCommandList>>& commandLists, D3D12_CPU_DESCRIPTOR_HANDLE resultRtv, D3D12_CPU_DESCRIPTOR_HANDLE resultDsv)
//{
//	// heap set
//	SetResourceHeap(commandLists[0]);
//
//	// animate first
//	AnimateToSO(commandLists[0]);
//
//	// mrt render end
//	RenderOnMRT(commandLists[0], resultDsv);
//
//
//	
//
//	///////////////////////////////////////////////////////////////////////////////////////////
//
//	// update light, and shadowm map 
//	UpdateLightData(commandLists[0]);
//
//	// post processing
//	PostProcessing(commandLists[0], resultRtv, resultDsv);
//}
