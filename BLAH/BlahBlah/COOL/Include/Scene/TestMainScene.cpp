﻿#include "framework.h"
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

		std::function<void(component::Server*, component::Name*)>
			findAndSetID = [&playername, &playerSock](component::Server* ser, component::Name* na) {
			if (na->getName() == playername) ser->setID(playerSock[0]);
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
		std::function<void(component::Server*, component::Name*)>
			findAndSetIDOnOther = [&otherPlayer, &playerSock](component::Server* ser, component::Name* na) {
			if (na->getName() == otherPlayer) ser->setID(playerSock[1]);
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
