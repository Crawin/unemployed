//#include "Renderer/Renderer.h"
#include "framework.h"
#include "Scene.h"
#include "ECS/ECSManager.h"
#include "ResourceManager.h"
#include "Shader/Shader.h"

#ifdef _DEBUG
#include "App/InputManager.h"
#endif
//#define SCENE_PATH "SceneData\\Scene\\"

Scene::Scene()
{
	m_ResourceManager = new ResourceManager;
}

Scene::~Scene()
{
	if (m_ResourceManager) delete m_ResourceManager;
}


bool Scene::LoadScene(ComPtr<ID3D12GraphicsCommandList> commandList, const std::string& sceneName)
{
	m_ECSManager = std::make_shared<ECSManager>();

	m_ResourceManager->SetECSManager(m_ECSManager);

	CHECK_CREATE_FAILED(AddSystem(), std::format("Error on add system!!, name: {}", sceneName));

	CHECK_CREATE_FAILED(m_ResourceManager->Init(commandList, sceneName), std::format("Can't Load Scene, name: {}", sceneName));
	
	CHECK_CREATE_FAILED(LoadSceneExtra(commandList), std::format("Can't Load Scene Extra, name: {}", sceneName));

	m_ECSManager->InitSystem();

	return true;
}

bool Scene::AddSystem()
{
	// default systems
	// 넣는 순서에 따라 system이 돌아가는게 달라짐
	// 우선순위들 정리
	// 1. 각 객체들의 transform을 바꾸는 system
	// 998. todo 충돌처리
	// 999. 부모에 따라 transform을 바꾸는 system(LocalToWorldTransform)
	
	// sync position by server
	m_ECSManager->InsertSystem(new ECSsystem::SyncPosition);
	
	// move collide check, handle simulate, move and send
	m_ECSManager->InsertSystem(new ECSsystem::UpdateInput);
	m_ECSManager->InsertSystem(new ECSsystem::SimulatePhysics);
	m_ECSManager->InsertSystem(new ECSsystem::MoveByPhysics);
	m_ECSManager->InsertSystem(new ECSsystem::CollideHandle);
	m_ECSManager->InsertSystem(new ECSsystem::SendToServer);

	// transform sync
	m_ECSManager->InsertSystem(new ECSsystem::LocalToWorldTransform);
	m_ECSManager->InsertSystem(new ECSsystem::SyncWithTransform);


	// 나중에 해도 상관 없는 것들
	m_ECSManager->InsertSystem(new ECSsystem::AnimationPlayTimeAdd);
	m_ECSManager->InsertSystem(new ECSsystem::ChangeAnimationTest);
	m_ECSManager->InsertSystem(new ECSsystem::DayLight);
	m_ECSManager->InsertSystem(new ECSsystem::HandleInteraction);
	m_ECSManager->InsertSystem(new ECSsystem::HandleUIComponent);


	return true;
}

bool Scene::LoadSceneExtra(ComPtr<ID3D12GraphicsCommandList> commandList)
{
	//m_ResourceManager->GetMaterial("", commandList);

	return true;
}

void Scene::SetResourceHeap(ComPtr<ID3D12GraphicsCommandList> commandList)
{
	m_ResourceManager->SetResourceHeap(commandList);

}

void Scene::AnimateToSO(ComPtr<ID3D12GraphicsCommandList> commandList)
{
	ResourceManager* manager = m_ResourceManager;

	// Set Animation PSO
	m_ResourceManager->GetAnimationShader()->SetPipelineState(commandList);

	// animate and set animed data
	std::function<void(component::Renderer*, component::AnimationExecutor*)> animate = [&commandList, manager](component::Renderer* renderComponent, component::AnimationExecutor* executor) {
		int meshIdx = renderComponent->GetMesh();
		Mesh* mesh = manager->GetMesh(meshIdx);
		if (mesh && mesh->IsSkinned() && mesh->GetVertexNum() > 0) {
			// Stream Out data set to  Stream Out
			int bufidx = executor->GetStreamOutBuffer();
			manager->SetResourceState(commandList, RESOURCE_TYPES::VERTEX, bufidx, D3D12_RESOURCE_STATE_STREAM_OUT);
			auto resPtr = executor->GetStreamOutBuffer();

			// should set stream buf pos to zero
			*(executor->m_StreamSize) = 0;

			// SO set
			const auto& view = executor->GetStreamOutBufferView();
			D3D12_STREAM_OUTPUT_BUFFER_VIEW bufView[] = { view };
			commandList->SOSetTargets(0, _countof(bufView), bufView);

			// set bone here
			int boneIdx = manager->GetBone(mesh->GetBoneIdx())->GetBoneDataIdx();
			commandList->SetGraphicsRoot32BitConstants(static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT), 1, &boneIdx, static_cast<int>(ANIM_ROOTCONST::BONE_INDEX));

			// set animation here
			executor->SetData(commandList, manager);

			// animate here
			mesh->SetVertexBuffer(commandList);
			mesh->Animate(commandList);

			// 완료 후 Vertex Buffer로 변경
			manager->SetResourceState(commandList, RESOURCE_TYPES::VERTEX, bufidx, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		}
		};

	m_ECSManager->Execute(animate);

	//std::function<void(component::Animation*)> debug = [](component::Animation* anim) {

	//	DebugPrint(std::format("{}", *(anim->m_StreamSize)));
	//	};

	//m_ECSManager->Execute(debug);
}

void Scene::RenderOnMRT(ComPtr<ID3D12GraphicsCommandList> commandList, component::Camera* camera)
{
	auto& res = m_ResourceManager;

	int camIdx = camera->GetCameraIndex();

	// default deffered renderer
	m_ResourceManager->SetMRTStates(commandList, D3D12_RESOURCE_STATE_RENDER_TARGET, camIdx);

	// clear mrt
	float clearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_ResourceManager->ClearMRTS(commandList, clearColor, camIdx);

	// OM set
	auto rtMRT = m_ResourceManager->GetDefferedRenderTargetStart(camIdx);
	auto dsv = m_ResourceManager->GetDefferedDSV(camIdx);
	commandList->OMSetRenderTargets(static_cast<int>(MULTIPLE_RENDER_TARGETS::MRT_END), &rtMRT, true, &dsv);

	OnPreRender(commandList, dsv);
	camera->SetCameraData(commandList);

	BoundingFrustum& cameraFustum = camera->GetBoundingFrustum();
	BoundingOrientedBox tempOBB;

	XMFLOAT3 camPos = camera->GetWorldPosition();
	XMFLOAT3 camDir = camera->GetWorldDirection();

	// make function
	std::function<void(component::Renderer*)> func = [&commandList, &res, &cameraFustum, &tempOBB](component::Renderer* renderComponent) {
		int materialIdx = renderComponent->GetMaterial();
		int meshIdx = renderComponent->GetMesh();

		Material* material = res->GetMaterial(materialIdx);
		Mesh* mesh = res->GetMesh(meshIdx);

		const BoundingOrientedBox& meshOBB = mesh->GetBoundingBox();
		meshOBB.Transform(tempOBB, XMLoadFloat4x4(&renderComponent->GetWorldMatrix()));

		// frustum culling
		if (cameraFustum.Intersects(tempOBB) == false) return;


		material->GetShader()->SetPipelineState(commandList);
		material->SetDatas(commandList, static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT));

		//XMFLOAT4X4 t = Matrix4x4::Identity();
		//res->m_Meshes[mesh]->SetVertexBuffer(commandLists[0]);

		const auto& view = renderComponent->GetVertexBufferView();
		D3D12_VERTEX_BUFFER_VIEW bufView[] = { view };
		commandList->IASetVertexBuffers(0, _countof(bufView), bufView);

		mesh->Render(commandList, renderComponent->GetWorldMatrix());
		};

	// execute function
	m_ECSManager->Execute<component::Renderer>(func);

	OnPostRender(commandList, dsv);

	// render sky here


	// set state common
	m_ResourceManager->SetMRTStates(commandList, D3D12_RESOURCE_STATE_COMMON, camIdx);
}

void Scene::UpdateLightData(ComPtr<ID3D12GraphicsCommandList> commandList)
{
	// render shadow map

	// light map
	// 1. light map을 만드는 모든 light들에 대해 프러스텀 컬링을 실시함 (renderer를 찾아 모아옴)
	// 2. lightmap 상태 변경(tex -> present)
	// 3. OM SET
	// 4. render
	// 4 - 1. shader에 light 데이터가 가야하는데 어케하지? light data를 
	// 4 - 2.
	// 5. present -> tex;


	// for lambda capture
	std::shared_ptr<ECSManager> ecs = m_ECSManager;
	auto& res = m_ResourceManager;

	// set light data to shader
	int count = 0;
	LightData* data = m_ResourceManager->m_MappedLightData;

	// unoccupy
	res->UnOccupyShadowRTVs();

	// get Camera
	std::vector<component::Camera*> camVec;
	std::function<void(component::Camera*)> getCam = [&camVec](component::Camera* cam) { camVec.push_back(cam); };
	m_ECSManager->Execute(getCam);

	// prepare for light evaluate
	int& activeLights = m_ActiveLightSize;
	activeLights = 0;
	component::Camera* cam = camVec[0];
	XMFLOAT3 camPos = cam->GetWorldPosition();
	XMFLOAT3 camDir = cam->GetWorldDirection();
	std::vector<component::Light*> toCastShadowLights;
	toCastShadowLights.reserve(m_ResourceManager->m_ShadowMaps.size());

	// evaluate lights for shadow mapping
	std::function<void(component::Light*)> shadowMapEvaluate = [&camPos, &camDir, &toCastShadowLights](component::Light* lightComp) {
		// unlink first
		lightComp->GetLightData().m_CameraIdx = -1;

		// if allways not cast, return
		if (lightComp->IsCastShadow() == false) return;

		// is main light -> cast allways cast shadow
		lightComp->GetLightData().m_CastShadow = lightComp->IsMainLight();
		lightComp->CalculateScore(camPos, camDir);

		toCastShadowLights.push_back(lightComp);

		};
	m_ECSManager->Execute(shadowMapEvaluate);

	// sort by score
	std::sort(toCastShadowLights.begin(), toCastShadowLights.end(), [](component::Light* a, component::Light* b) { return a->GetScore() > b->GetScore(); });

	// link light to shadow map
	int maxShadowMaps = m_ResourceManager->m_ShadowMaps.size();
	for (int i = 0; i < maxShadowMaps && i < toCastShadowLights.size(); ++i) {
		// if too low, end
		if (toCastShadowLights[i]->GetScore() < -100.0f) break;

		LightData& light = toCastShadowLights[i]->GetLightData();

		light.m_CastShadow = TRUE;
		int shadowMapIdx = -1;
		// link lights
		switch (static_cast<LIGHT_TYPES>(light.m_LightType)) {
		case LIGHT_TYPES::DIRECTIONAL_LIGHT:
		{
			++activeLights;

			// todo 
			// 하드코딩 경고!!!!!!!!!!!!!!!!!!!!!!!!!
			auto& p = camVec[0]->GetPosition();

			XMFLOAT3 up = light.m_Direction;
			up.x *= -5000.0;
			up.y *= -5000.0;
			up.z *= -5000.0;

			XMFLOAT3 pos = { p.x + up.x, p.y + up.y, p.z + up.z };
			light.m_Position = pos;
		}

		// fallthrough 
		case LIGHT_TYPES::SPOT_LIGHT:
			// 해당 함수가 불리면 shadow mapping을 한다.
			shadowMapIdx = res->GetUnOccupiedShadowMapRenderTarget(static_cast<LIGHT_TYPES>(light.m_LightType));
			res->UpdateShadowMapView(shadowMapIdx, light);

			// set result
			light.m_CameraIdx = res->GetShadowMapCamIdx(shadowMapIdx);
			light.m_ShadowMapResults[0] = shadowMapIdx + res->GetShadowMapRTVStartIdx();

			// todo
			// 현재는 shadowmap 0번이 shadowmap 렌더타겟 0번을 그대로 가리키고 있기 때문에
			res->SetShadowMapRTVIdx(shadowMapIdx, shadowMapIdx);
			break;

		case LIGHT_TYPES::POINT_LIGHT:
			DebugPrint("No current shadow map setting for point light now");
			break;

		};
	}

	// memcpy to gpu
	std::function<void(component::Light*)> updateLight = [&count, data](component::Light* lightComp) {
		LightData& light = lightComp->GetLightData();
		memcpy(data + count++, &light, sizeof(LightData));
		};
	m_ECSManager->Execute(updateLight);


	// 
	// OMSet
	// Render
	// LOOP
	// 
	// set light map pso
	int shadowMat = m_ResourceManager->GetShadowMappingMaterial();
	//m_ResourceManager->SetPSO(commandList, shadowMat);
	Material* mat = m_ResourceManager->GetMaterial(shadowMat);
	mat->GetShader()->SetPipelineState(commandList);


	// Set Resource To RTV
	m_ResourceManager->SetShadowMapStates(commandList, D3D12_RESOURCE_STATE_RENDER_TARGET);

	// Clear RTV Clear DSV
	const float clearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_ResourceManager->ClearShadowMaps(commandList, clearColor);
	auto dsv = m_ResourceManager->GetShadowMapDepthStencil();
		//m_ResourceManager->m_ShadowMapDSVHeap->GetCPUDescriptorHandleForHeapStart();

	// set viewport
	// 하드코딩 경보
	D3D12_VIEWPORT vp;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.Width = static_cast<float>(8192);
	vp.Height = static_cast<float>(8192);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;

	RECT scRect = { 0, 0, 8192, 8192 };

	commandList->RSSetViewports(1, &vp);
	commandList->RSSetScissorRects(1, &scRect);
	
	// normal render code

	BoundingFrustum* cameraFustum;
	BoundingOrientedBox tempOBB;

	std::function<void(component::Renderer*)> render = [commandList, res, &cameraFustum, &tempOBB](component::Renderer* rend) {
		Mesh* mesh = res->GetMesh(rend->GetMesh());

		const BoundingOrientedBox& meshOBB = mesh->GetBoundingBox();
		meshOBB.Transform(tempOBB, XMLoadFloat4x4(&rend->GetWorldMatrix()));

		// frustum culling
		if (cameraFustum->Intersects(tempOBB) == false) return;

		const auto& view = rend->GetVertexBufferView();
		D3D12_VERTEX_BUFFER_VIEW bufView[] = { view };
		commandList->IASetVertexBuffers(0, _countof(bufView), bufView);

		mesh->Render(commandList, rend->GetWorldMatrix());
		};

	// 모든 활성화된 셰도우맵에 대해서
	for (int i = 0; i < m_ResourceManager->m_ShadowMaps.size(); ++i) {
		if (m_ResourceManager->m_ShadowMapOccupied[i] == false) break;
		int idx = res->GetShadowMapRTVIdx(i);
		D3D12_CPU_DESCRIPTOR_HANDLE targetRtv = res->GetShadowMapRenderTarget(idx);

		if (idx < 0) {
			DebugPrint("ERROR~!!!!");
		}
		
		commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
		commandList->OMSetRenderTargets(1, &targetRtv, true, &dsv);

		// set camera
		res->SetShadowMapCamera(commandList, idx);

		// 일단 임시로 update함

		cameraFustum = res->GetShadowMapFrustum(idx);
		ecs->Execute(render);
	}

	vp.Width = static_cast<float>(1280);
	vp.Height = static_cast<float>(720);

	scRect = { 0, 0, 1280, 720};

	commandList->RSSetViewports(1, &vp);
	commandList->RSSetScissorRects(1, &scRect);
	// set render target

	
	// set to common
	m_ResourceManager->SetShadowMapStates(commandList, D3D12_RESOURCE_STATE_COMMON);

}

void Scene::PostProcessing(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_CPU_DESCRIPTOR_HANDLE resultRtv, D3D12_CPU_DESCRIPTOR_HANDLE resultDsv)
{
	std::vector<component::Camera*> camVec;
	std::function<void(component::Camera*)> getCam = [&commandList, &camVec](component::Camera* cam) {	camVec.push_back(cam); };
	m_ECSManager->Execute(getCam);

	camVec[0]->SetCameraData(commandList);
	int camIdx = camVec[0]->GetCameraIndex();

	commandList->ClearDepthStencilView(resultDsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	commandList->OMSetRenderTargets(1, &resultRtv, true, &resultDsv);
	//m_ResourceManager->SetMRTStates(commandList, D3D12_RESOURCE_STATE_COMMON, camIdx);
	//m_ResourceManager->SetShadowMapStates(commandList, D3D12_RESOURCE_STATE_COMMON);

	m_ResourceManager->SetCameraToPostProcessing(camIdx);

	int postMat = m_ResourceManager->GetPostProcessingMaterial();
	Material* mat = m_ResourceManager->GetMaterial(postMat);

	// todo light size
	mat->SetDataIndex(static_cast<int>(MRT_POST_ROOTCONST::LIGHT_SIZE), m_ResourceManager->m_LightSize);
	mat->SetDataIndex(static_cast<int>(MRT_POST_ROOTCONST::LIGHT_IDX), m_ResourceManager->m_LightIdx);
	mat->GetShader()->SetPipelineState(commandList);
	//m_ResourceManager->SetPSO(commandList, postMat);
	mat->SetDatas(commandList, static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT));

	commandList->DrawInstanced(6, 1, 0, 0);

#ifdef _DEBUG
	// test code
	// todo 적절한 위치로 옮기도록 하자
	if (InputManager::GetInstance().GetDebugMode()) {
		commandList->ClearDepthStencilView(resultDsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
		int debugMat = m_ResourceManager->GetDebuggingMaterial();

		//m_ResourceManager->SetPSO(commandList, debugMat);
		m_ResourceManager->GetMaterial(debugMat)->GetShader()->SetPipelineState(commandList);

		int a[2] = { 0, 0 };
		
		if (GetAsyncKeyState(VK_F9) & 0x8000)
			a[0] = m_ResourceManager->GetShadowMapRTVStartIdx();

		commandList->SetGraphicsRoot32BitConstants(static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT), 1, a, 0);
		//m_ResourceManager->m_Materials[postMat]->SetDatas(commandLists[0], static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT));

		commandList->DrawInstanced(6, 1, 0, 0);
	}
#endif
}

void Scene::DrawUI(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_CPU_DESCRIPTOR_HANDLE resultRtv, D3D12_CPU_DESCRIPTOR_HANDLE resultDsv)
{
	commandList->ClearDepthStencilView(resultDsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	auto& ecsManager = m_ECSManager;
	auto& resManager = m_ResourceManager;

	// set screen size
	commandList->SetGraphicsRoot32BitConstants(static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT), 2, &Renderer::GetInstance().GetScreenSize(), static_cast<int>(UI_ROOT_CONST::SCREEN_SIZE));

	std::function<void(component::UIRenderer*, component::UITransform*)> renderUI = [commandList, resManager](component::UIRenderer* rend, component::UITransform* trans) {
		Material* material = resManager->GetMaterial(rend->GetMaterial());
		//mat.Set
		int textureIdx = material->GetTexture(0);

		material->GetShader()->SetPipelineState(commandList);

		commandList->SetGraphicsRoot32BitConstants(static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT), 1, &textureIdx, static_cast<int>(UI_ROOT_CONST::TEXTURE_IDX));
		commandList->SetGraphicsRoot32BitConstants(static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT), 2, &rend->GetSpriteSize(), static_cast<int>(UI_ROOT_CONST::SPRITE_SIZE));
		commandList->SetGraphicsRoot32BitConstants(static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT), 1, &rend->GetCurSprite(), static_cast<int>(UI_ROOT_CONST::CUR_SPRITE));
		commandList->SetGraphicsRoot32BitConstants(static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT), 2, &trans->GetCenter(), static_cast<int>(UI_ROOT_CONST::UI_CENTER));
		commandList->SetGraphicsRoot32BitConstants(static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT), 2, &trans->GetSize(), static_cast<int>(UI_ROOT_CONST::UI_SIZE));

		commandList->DrawInstanced(6, 1, 0, 0);
		};

	using namespace component;

	std::function<void(UICanvas*, Children*)> forAllUICanvas = [&ecsManager, &renderUI](UICanvas* canvas, Children* cild) {
		if (canvas->IsActive()) {
			Entity* ent = cild->GetEntity();
			const std::vector<Entity*>& children = ent->GetChildren();

			for (Entity* child : children) {
				auto bit = child->GetBitset();
				int innerId = child->GetInnerID();

				ecsManager->ExecuteFromEntity(bit, innerId, renderUI);

				//// get ui trans and 
				//UITransform* childTrans = ecsManager->GetComponent<UITransform>(bit, innerId);
				//UIRenderer* childRender = ecsManager->GetComponent<UIRenderer>(bit, innerId);
				//if (childTrans != nullptr && childRender != nullptr);

			}
		}
		};

	m_ECSManager->Execute(forAllUICanvas);

}

bool Scene::Enter(ComPtr<ID3D12GraphicsCommandList> commandList)
{
	if (LoadScene(commandList, m_SceneName) == false)
	{
		ERROR_QUIT(std::format("ERROR!! Scene load error, {}", m_SceneName));
	}

	return true;
}

void Scene::Update(float deltaTime)
{
	m_ECSManager->UpdateSystem(deltaTime);
}

void Scene::Render(std::vector<ComPtr<ID3D12GraphicsCommandList>>& commandLists, D3D12_CPU_DESCRIPTOR_HANDLE resultRtv, D3D12_CPU_DESCRIPTOR_HANDLE resultDsv)
{
	// heap set
	SetResourceHeap(commandLists[0]);

	// animate first
	AnimateToSO(commandLists[0]);


	// get Camera
	std::vector<component::Camera*> camVec;
	std::function<void(component::Camera*)> getCam = [&camVec](component::Camera* cam) { camVec.push_back(cam); };
	m_ECSManager->Execute(getCam);

	int camIdx = camVec[0]->GetCameraIndex();

	// wait for end here

	// render on multi
	// mrt render end
	for(int i = 0; i < camVec.size(); ++i)
		RenderOnMRT(commandLists[0], camVec[i]);

	// wait for end
	///////////////////////////////////////////////////////////////////////////////////////////

	// update light, and shadowm map 
	UpdateLightData(commandLists[0]);

	// post processing
	PostProcessing(commandLists[0], resultRtv, resultDsv);

	DrawUI(commandLists[0], resultRtv, resultDsv);
}
