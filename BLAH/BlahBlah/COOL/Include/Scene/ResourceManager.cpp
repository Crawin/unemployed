#include "framework.h"
#include "Renderer/COOLResource.h"
#include "Renderer/Renderer.h"
#include "ResourceManager.h"
#include "ECS/ECSManager.h"
#include "Lighting/ShadowMap.h"
#include <json/json.h>

//#include "Material/Material.h"
//#include "Mesh/Mesh.h"
#include "Shader/Shader.h"

#define SCENE_PATH			"SceneData\\Scene\\"

#define MESH_PATH			"SceneData\\Mesh\\"
#define SHADER_PATH			"SceneData\\Shader\\"
#define TEXTURE_PATH		"SceneData\\Material\\Texture\\"
#define MATERIAL_PATH		"SceneData\\Material\\"
#define BONE_PATH			"SceneData\\Mesh\\bone_"
#define ANIMATION_SET_PATH	"SceneData\\Animation\\"
#define ANIMATION_PATH		"SceneData\\Animation\\Data\\"


ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
	for (auto& mat : m_Materials)
		delete mat;
	for (auto& mes : m_Meshes)
		delete mes;
	for (auto& bon : m_Bones)
		delete bon;

	// sharedptr
	//for (auto& ani : m_Animations)
	//	delete ani;
	// esc매니저에서 하기 때문에 상관 없다
	//for (auto& cmp : m_Components)
	//	delete cmp;
	
	m_Animations.clear();

	//sharedptr
	m_Shaders.clear();

	// ecs manager가 가지고 있다.
	//for (auto& obj : m_Entities)
	//	delete obj;

	m_Resources.clear();
	m_VertexIndexDatas.clear();
	m_ObjectDatas.clear();
}

int ResourceManager::CreateBufferFromData(ComPtr<ID3D12GraphicsCommandList> commandList, char* data, int stride, int numOfData, D3D12_RESOURCE_STATES resourceState, std::string_view name, RESOURCE_TYPES toInsert)
{
	COOLResourcePtr ptr = Renderer::GetInstance().CreateBufferFromPointer(commandList, data, stride, numOfData, resourceState, name);

	switch (toInsert) {
	case RESOURCE_TYPES::SHADER:
		ptr->SetShaderResource();
		m_Resources.push_back(ptr);
		return static_cast<int>(m_Resources.size() - 1);
		break;
	case RESOURCE_TYPES::VERTEX:
		m_VertexIndexDatas.push_back(ptr);
		return static_cast<int>(m_VertexIndexDatas.size() - 1);
		break;
	case RESOURCE_TYPES::OBJECT:
		//ptr->SetConstant();
		m_ObjectDatas.push_back(ptr);
		return static_cast<int>(m_ObjectDatas.size() - 1);
		break;
	}

	return -1;
}

int ResourceManager::CreateEmptyBuffer(ComPtr<ID3D12GraphicsCommandList> commandList, int size, int stride, D3D12_RESOURCE_STATES resourceState, std::string_view name, RESOURCE_TYPES toInsert, D3D12_HEAP_TYPE heapType)
{
	COOLResourcePtr ptr = Renderer::GetInstance().CreateEmptyBuffer(
		heapType,
		D3D12_RESOURCE_STATE_COMMON,
		size * stride,
		name);

	ptr->TransToState(commandList, resourceState);

	switch (toInsert) {
	case RESOURCE_TYPES::SHADER:
		ptr->SetShaderResource();
		m_Resources.push_back(ptr);
		return static_cast<int>(m_Resources.size() - 1);
		break;
	case RESOURCE_TYPES::VERTEX:
		m_VertexIndexDatas.push_back(ptr);
		return static_cast<int>(m_VertexIndexDatas.size() - 1);
		break;
	case RESOURCE_TYPES::OBJECT:
		m_ObjectDatas.push_back(ptr);
		return static_cast<int>(m_ObjectDatas.size() - 1);
		break;
	}

	return -1;
}

bool ResourceManager::LoadObjectFile(const std::string& fileName, bool isCam)
{
	// 오브젝트를 로드한다.
	Json::Value root;
	Json::Reader reader;

	std::ifstream file(fileName);
	bool parseResult = reader.parse(file, root);
	
	if (parseResult == false) {
		DebugPrint(std::format("Object file Parse Failed!, fileName: {}\n{}", fileName, reader.getFormattedErrorMessages()));
		return false;
	}

	auto entptr = LoadObjectJson(root, nullptr);

	if (entptr == nullptr) 
		return false;

	// add root component
	component::Root* cmp = new component::Root;
	cmp->SetEntity(entptr);
	entptr->AddComponent(cmp);

	return true;
}

#define CHILDREN "Children"

Entity* ResourceManager::LoadObjectJson(Json::Value& root, Entity* parent)
{
	Entity* ent = new Entity;

	if (parent) parent->AddChild(ent);

	for (auto jsonComp : root.getMemberNames()) {
		component::Component* cmp = GET_COMPONENT(jsonComp);

		if (cmp == nullptr) {
			if (jsonComp != CHILDREN) DebugPrint(std::format("No Such Component Registered!! {}", jsonComp));
			continue;
		}

		cmp->Create(root, this);
		ent->AddComponent(cmp);
		ent->AddBit(cmp->GetBitset());

		DebugPrint(std::format("add {}", cmp->GetBitset().to_string()));

		// 동적할당 했으면 지워줘야 한다.
		// memcpy 이후 지우지 않고
		// 소멸 이후 지운다.
		// std::string, std::map과 같은 것들은 memcpy 후 모두 같은 것을 가리키고 있기 때문에
		// 
		m_Components.push_back(cmp);
	}
	DebugPrint("");

	// self entity
	component::SelfEntity* cmp = new component::SelfEntity;
	cmp->SetEntity(ent);
	ent->AddComponent(cmp);
	ent->AddBit(cmp->GetBitset());

	// if Children
	if (root[CHILDREN].isNull() == false) {
		for (auto& val : root[CHILDREN]) 
		{
			auto child = LoadObjectJson(val, ent);
			if (child == nullptr)
				return nullptr;
		}
		component::Children* cmp = new component::Children;
		cmp->SetEntity(ent);
		ent->AddComponent(cmp);
		ent->AddBit(cmp->GetBitset());
	}
	
	// 여기서 ECS Manager에 entity를 삽입하지 않는다.
	// late init이 끝난 후 entity를 삽입함
	m_Entities.push_back(ent);
	//m_ECSManager->AddEntity(ent);

	return ent;
}

int ResourceManager::LoadMesh(const std::string& name, ComPtr<ID3D12GraphicsCommandList> commandList)
{
	std::string fileName = MESH_PATH + name + ".bin";
	std::ifstream meshFile(fileName, std::ios::binary);
	if (meshFile.is_open() == false) {
		DebugPrint(std::format("Failed to open mesh file!! fileName: {}", fileName));
		return -1;
	}

	Mesh* mesh = new Mesh;
	mesh->m_Name = ExtractFileName(fileName);
	mesh->BuildMesh(commandList, meshFile, name, this);

	DebugPrint(std::format("loaded mesh file name: {}", mesh->m_Name));
	return GetMesh(mesh->m_Name);

}

int ResourceManager::LoadMaterial(const std::string& name, ComPtr<ID3D12GraphicsCommandList> commandList)
{
	// 초기화 단계일 때만 매터리얼을 생성한다.
	std::string fileName = MATERIAL_PATH + name + ".json";

	Material* material = new Material;
	std::string shaderName;
	if (false == material->LoadFile(commandList, fileName, this, shaderName)) {
		DebugPrint(std::format("Failed to load material!! Name: {}", name));
		delete material;
		return -1;
	}

	// get shader
	auto shader = GetShader(shaderName, commandList);
	material->SetShader(shader);
	m_Materials.push_back(material);

	DebugPrint(std::format("loaded material file name: {}", material->m_Name));
	return static_cast<int>(m_Materials.size() - 1);
}

int ResourceManager::LoadBone(const std::string& name, ComPtr<ID3D12GraphicsCommandList> commandList)
{
	std::string fileName = BONE_PATH + name + ".bin";
	std::ifstream boneFile(fileName, std::ios::binary);
	if (boneFile.is_open() == false) {
		DebugPrint(std::format("Failed to open bone file!! fileName: {}", fileName));
		return -1;
	}

	Bone* bone = new Bone;
	bone->m_Name = name;
	bone->LoadBone(commandList, boneFile, this);
	m_Bones.push_back(bone);

	DebugPrint(std::format("loaded bone file name: {}", bone->m_Name));
	return static_cast<int>(m_Bones.size() - 1);
}

int ResourceManager::LoadAnimation(const std::string& name, ComPtr<ID3D12GraphicsCommandList> commandList)
{
	// todo 이런거 템플릿으로 만들자 제발
	std::string fileName = ANIMATION_PATH + name + ".bin";
	std::ifstream animFile(fileName, std::ios::binary);
	if (animFile.is_open() == false) {
		DebugPrint(std::format("Failed to open anim file!! fileName: {}", fileName));
		return -1;
	}

	std::shared_ptr<Animation> anim = std::make_shared<Animation>();
	anim->m_Name = name;
	anim->LoadAnimation(commandList, animFile, this);
	m_Animations.push_back(anim);

	DebugPrint(std::format("loaded anim file name: {}", anim->m_Name));
	return static_cast<int>(m_Animations.size() - 1);
}

std::shared_ptr<Shader> ResourceManager::LoadShader(const std::string& name, ComPtr<ID3D12GraphicsCommandList> commandList)
{
	std::shared_ptr<Shader> shader = std::make_shared<Shader>();

	std::string fileName = SHADER_PATH + name + ".json";

	if (false == Renderer::GetInstance().CreateShader(commandList, fileName, shader)) {
		ERROR_QUIT(std::format("Failed to load shader file!! fileName: {}", fileName));
		return nullptr;
	}

	m_Shaders.push_back(shader);

	DebugPrint(std::format("loaded shader file name: {}", shader->m_Name));
	return shader;
}

bool ResourceManager::LoadDefferedResource(ComPtr<ID3D12GraphicsCommandList> commandList)
{
	// load Postprocessing material
	m_PostProcessingMaterial = GetMaterial(m_PostProcessing, commandList);
	if (m_PostProcessingMaterial == -1) return false;

	for (int i = 0; i < m_DefferedRenderTargets; ++i)
		m_Materials[m_PostProcessingMaterial]->SetTexture(i, i + m_DefferedRTVStartIdx);

#ifdef _DEBUG
	m_DebuggingMaterial = GetMaterial(m_Debuggging, commandList);
	if (m_DebuggingMaterial == -1) return false;

	// debug deffered default
	for (int i = 0; i < m_DefferedRenderTargets; ++i)
		m_Materials[m_DebuggingMaterial]->SetTexture(i, i + m_DefferedRTVStartIdx);
#endif

	return true;
}

bool ResourceManager::LoadLateInitMesh(ComPtr<ID3D12GraphicsCommandList> commandList)
{
	// load mesh
	for (const auto& rendererLoadInfo : m_ToLoadRenderDatas) {
		// mesh의 json에서는 mesh의 이름을
		// satodia.satodia_body 와 같은 방식으로 이루어져있다.
		// 뜻: satodia.bin 파일에 존재하는 satodia_body

		std::string meshFileName = ExtractFileName(rendererLoadInfo.m_MeshName);

		auto it = std::find(rendererLoadInfo.m_MeshName.begin(), rendererLoadInfo.m_MeshName.end(), '.');
		++it;
		std::string meshName(it, rendererLoadInfo.m_MeshName.end());

		// 해당 mesh가 없을 때 부모 mesh를 load

		int res = GetMesh(meshName);
		if (res == -1)
		{
			// check first
			if (GetMesh(meshFileName) != -1) {
				DebugPrint(std::format("ERROR!!!!, No Such mesh in file!! mesh: {}\t file: {}", meshName, meshFileName));
				return false;
			}

			// load parent
			LoadMesh(meshFileName, commandList);
		}

		res = GetMesh(meshName);

		if (res == -1) {
			DebugPrint(std::format("ERROR!!!!, No Such mesh in file!! mesh: {}\t file: {}", meshName, meshFileName));
			return false;
		}

		rendererLoadInfo.m_Renderer->SetMesh(res);
		rendererLoadInfo.m_Renderer->SetVertexBufferView(m_Meshes[res]->m_VertexBufferView);

		// load material
		std::string materialFileName = rendererLoadInfo.m_MaterialName;
		res = GetMaterial(materialFileName, commandList);

		if (res == -1) {
			DebugPrint(std::format(
				"ERROR!!, no such material, name: {}\nSet to uv_checker",
				materialFileName));
			res = GetMaterial(materialFileName, commandList);
		}


		rendererLoadInfo.m_Renderer->SetMaterial(res);

	}

	for (const auto& uiRendererInfo : m_ToLoadUIDatas) {
		// load material
		std::string materialFileName = uiRendererInfo.m_MaterialName;
		int res = GetMaterial(materialFileName, commandList);

		if (res == -1) {
			DebugPrint(std::format(
				"ERROR!!, no such material, name: {}\nSet to uv_checker",
				materialFileName));
			res = GetMaterial(materialFileName, commandList);
		}

		uiRendererInfo.m_Renderer->SetMaterial(res);
	}

	return true;
}

bool ResourceManager::LoadLateInitAnimation(ComPtr<ID3D12GraphicsCommandList> commandList)
{
	// load AnimationSet(Animation) first
	for (auto& animCont : m_ToLoadAnimCont) {
		Json::Value root;
		Json::Reader reader;

		std::string fileName = ANIMATION_SET_PATH + animCont.m_AnimSetName + ".json";

		std::ifstream file(fileName);
		bool parseResult = reader.parse(file, root);

		if (file.is_open() == false) {
			DebugPrint(std::format("Cant Open File!!, file Name: {}", fileName));
			return false;
		}

		if (parseResult == false) {
			DebugPrint(std::format("AnimationPlayer file Parse Failed!, fileName: {}\n{}", fileName, reader.getFormattedErrorMessages()));
			return false;
		}

		// name
		AnimationPlayer* player = new AnimationPlayer;
		player->m_Name = animCont.m_AnimSetName;
		for (auto& key : root.getMemberNames()) {
			std::string animName = root[key].asString();
			int anim = GetAnimation(animName, commandList);

			if (anim < 0) ERROR_QUIT("Failed To Load Animation");

			player->m_AnimationMap[ConvertStringToAnimationState(key)] = m_Animations[anim];

			// todo 하드코딩 경고!!!!!!!!!!!!!!!!!!!!
			// 나중에 꼭 꼭 고쳐야한다
			if (m_Animations[anim]->m_Name == "dia_falling_back" || m_Animations[anim]->m_Name == "dia_getting_up" ||
				m_Animations[anim]->m_Name == "PlayerFall" || m_Animations[anim]->m_Name == "PlayerGetUp") {
				m_Animations[anim]->m_Loop = false;
			}
		}
		m_AnimationPlayer.push_back(player);

		animCont.m_Controller->SetPlayer(player);
		DebugPrint(std::format("loaded animset file name: {}", player->m_Name));
	}

	// set animation executer
	for (auto& animExe : m_ToLoadAnimExe) {
		std::string name = animExe.m_AnimSetName;
		auto it = std::find_if(m_AnimationPlayer.begin(), m_AnimationPlayer.end(), [&name](AnimationPlayer* player) { return player->GetName() == name; });

		// failed to find;
		if (it == m_AnimationPlayer.end()) {
			DebugPrint(std::format("Can't find animation set!! name: {}", name));
			return false;
		}

		animExe.m_Executor->SetPlayer(*it);
	}

	for (auto& att : m_ToLoadAttach) {
		std::string name = att.m_AnimSetName;
		auto it = std::find_if(m_AnimationPlayer.begin(), m_AnimationPlayer.end(), [&name](AnimationPlayer* player) { return player->GetName() == name; });

		// failed to find;
		if (it == m_AnimationPlayer.end()) {
			DebugPrint(std::format("Can't find animation set!! name: {}", name));
			return false;
		}

		att.m_Attach->SetPlayer(*it);

		// find bone;
		std::string boneName = att.boneName;
		auto bone = std::find_if(m_Bones.begin(), m_Bones.end(), [&boneName](Bone* bone) { return bone->GetName() == boneName; });

		if (bone == m_Bones.end()) {
			DebugPrint(std::format("Can't find BONE!!, name: {}", boneName));
			return false;
		}
	}

	return true;
}

bool ResourceManager::MakeLightData(ComPtr<ID3D12GraphicsCommandList> commandList)
{
	if (m_LightSize > 0) {
		COOLResourcePtr res = Renderer::GetInstance().CreateEmptyBuffer(
			D3D12_HEAP_TYPE_UPLOAD,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			m_LightSize * sizeof(LightData),
			"LightingData",
			(void**)&m_MappedLightData);

		res->SetShaderResource();
		res->SetDimension(D3D12_SRV_DIMENSION_BUFFER);
		res->SetNumOfElement(m_LightSize);
		res->SetStride(sizeof(LightData));


		m_Resources.push_back(res);
		m_LightIdx = static_cast<int>(m_Resources.size() - 1);
	}

	return true;
}

bool ResourceManager::MakeShadowMaps()
{
	m_ShadowMaps.reserve(m_ShadowMapRenderTargets);

	// make shadow map object
	for (int i = 0; i < m_ShadowMapRenderTargets; ++i) {
		m_ShadowMaps.push_back(ShadowMap());

		CameraDataShader* data = nullptr;
		int idx = CreateObjectResource(
			sizeof(CameraDataShader) * 6,
			std::format("ShadowMap Camera Shader Data_{}", i),
			(void**)(&data),
			D3D12_HEAP_TYPE_UPLOAD,
			RESOURCE_TYPES::SHADER);
		
		auto gpuAddr = GetResourceDataGPUAddress(RESOURCE_TYPES::SHADER, idx);
		
		m_ShadowMaps[i].SetCameraShaderData(data);
		m_ShadowMaps[i].SetCameraShaderDataIdx(idx);
		m_ShadowMaps[i].SetCameraShaderDataGPUAddr(gpuAddr);
	}

	return true;
}

bool ResourceManager::LoadShadowMappingResource(ComPtr<ID3D12GraphicsCommandList> commandList)
{
	// load Postprocessing material
	m_ShadowMappingMaterial = GetMaterial(m_ShadowMapping, commandList);
	if (m_ShadowMappingMaterial == -1) return false;


#ifdef _DEBUG
	
	// debugggg
	
	//for (int i = 0; i < m_ShadowMapRenderTargets; ++i)
	//	m_Materials[m_ShadowMappingMaterial]->SetTexture(i, i + m_ShadowMapRTVStartIdx);
	

	//m_DebuggingMaterial = GetMaterial(m_Debuggging, commandList);
	//if (m_DebuggingMaterial == -1) return false;

	//// debug deffered default
	//for (int i = 0; i < m_DefferedRenderTargets; ++i)
	//	m_Materials[m_DebuggingMaterial]->SetTexture(i, i + m_DefferedRTVStartIdx);
#endif

	return true;
}

bool ResourceManager::Init(ComPtr<ID3D12GraphicsCommandList> commandList, const std::string& sceneName)
{
	// 1. 씬에 배치 될 오브젝트들을 찾는다.
	// 2. 씬에 배치 될 오브젝트를 차례로 로드한다.
	// 
	// 오브젝트 로드????
	// A 오브젝트 로드한다.
	// 사용 mesh와 material을 찾는다.
	// 해당 mesh와 material이 없으면 새로 만든다.
	//

	//ecs manager를 만든다
	// ecsmanager 의 소유는 scene으로 이동함
	//m_ECSManager = new ECSManager<COMPONENT_COUNT>;

	std::string scenePath = SCENE_PATH + sceneName;

	// make deferred renderer texture
	CHECK_CREATE_FAILED(MakeExtraRenderTarget(), "RenderTargets for deffered rendering make Failed!!");

	// Load Objects
	CHECK_CREATE_FAILED(LoadObjects(sceneName, commandList), "Object Load Failed!!");

	// Load Cameras
	CHECK_CREATE_FAILED(LoadCameras(sceneName, commandList), "Camera Load Fail!!");

	// Load Mesh, Material, Texture, Shader to use
	CHECK_CREATE_FAILED(LateInit(commandList), "LateInit Fail!");

	// build mrt rtv heap
	CHECK_CREATE_FAILED(
		Renderer::GetInstance().CreateRenderTargetView(m_MRTHeap, m_Resources, m_DefferedRTVStartIdx, m_DefferedRenderTargets), 
		"Create MRT RTV Failed!!");

	// build shadow map heap
	CHECK_CREATE_FAILED(
		Renderer::GetInstance().CreateRenderTargetView(m_ShadowMapHeap, m_Resources, m_ShadowMapRTVStartIdx, m_ShadowMapRenderTargets), 
		"Create ShadowMap RTV Failed!!");

	CHECK_CREATE_FAILED(
		Renderer::GetInstance().CreateDepthStencilView(m_ShadowMapDSVHeap, m_ShadowDSV),
		"Create ShadowMap DSV Failed!!");

	// build resource heap
	CHECK_CREATE_FAILED(
		Renderer::GetInstance().CreateResourceDescriptorHeap(m_ShaderResourceHeap, m_Resources), 
		"CreateResourceDescriptorHeap Fail!!");


	return true;
}

void ResourceManager::SetECSManager(std::shared_ptr<ECSManager> ptr)
{
	m_ECSManager = ptr;
}

bool ResourceManager::LateInit(ComPtr<ID3D12GraphicsCommandList> commandList)
{
	// deffered
	CHECK_CREATE_FAILED(LoadDefferedResource(commandList), "Load DefferedResource Failed!");
	
	// mesh + material
	CHECK_CREATE_FAILED(LoadLateInitMesh(commandList), "LateInit, LoadMesh Failed!");

	// load Animation
	CHECK_CREATE_FAILED(LoadLateInitAnimation(commandList), "Animation Late Load Failed!");

	// make lighting data
	CHECK_CREATE_FAILED(MakeLightData(commandList), "Light Making Failed!!");

	// shadow map pso
	CHECK_CREATE_FAILED(LoadShadowMappingResource(commandList), "Load ShadowMappingResource Failed!!");

	for (auto& ent : m_Entities)
		m_ECSManager->AddEntity(ent);

	// 람다에 this는 조금 그렇지만
	// 초기화 부분이라 봐준다
	// todo 한번 생각해보자

	// make animation executer so buf view
	std::function<void(component::Renderer*, component::AnimationExecutor*)> setSO = [&commandList, this](component::Renderer* render, component::AnimationExecutor* anim) {
		Mesh* mesh = m_Meshes[render->GetMesh()];

		if (mesh->GetVertexNum() <= 0) return;

		// make stream buff
		// result is Vertex
		int streamOutBuffer = CreateEmptyBuffer(
			commandList,
			mesh->m_VertexNum, sizeof(Vertex),
			D3D12_RESOURCE_STATE_STREAM_OUT,
			std::format("animated_{}", mesh->m_Name),
			RESOURCE_TYPES::VERTEX);

		//int filledSizeBuff = CreateEmptyBuffer(
		//	commandList,
		//	sizeof(UINT64), sizeof(UINT64),
		//	D3D12_RESOURCE_STATE_STREAM_OUT,
		//	std::format("StreamCount_animated_{}", mesh->m_Name),
		//	RESOURCE_TYPES::VERTEX);
		int filledSizeBuff = CreateObjectResource(
			sizeof(UINT64),
			std::format("StreamCount_animated_{}", mesh->m_Name),
			(void**)(&anim->m_StreamSize));

		// update renderer vertex buffer use to this
		D3D12_VERTEX_BUFFER_VIEW animatedBufferView = {};
		animatedBufferView.BufferLocation = GetResourceDataGPUAddress(RESOURCE_TYPES::VERTEX, streamOutBuffer);
		animatedBufferView.StrideInBytes = sizeof(Vertex);
		animatedBufferView.SizeInBytes = sizeof(Vertex) * mesh->m_VertexNum;
		render->SetVertexBufferView(animatedBufferView);

		D3D12_STREAM_OUTPUT_BUFFER_VIEW toAnimateBufferView = {};
		toAnimateBufferView.BufferLocation = GetResourceDataGPUAddress(RESOURCE_TYPES::VERTEX, streamOutBuffer);
		toAnimateBufferView.BufferFilledSizeLocation = GetResourceDataGPUAddress(RESOURCE_TYPES::OBJECT, filledSizeBuff);
		toAnimateBufferView.SizeInBytes = sizeof(Vertex) * mesh->m_VertexNum;

		anim->SetOriginalVertexBufferView(render->GetVertexBufferView());
		anim->SetStreamOutBufferView(toAnimateBufferView);
		anim->SetStreamOutBuffer(streamOutBuffer);
		};

	m_ECSManager->Execute(setSO);
	
	// make bounding box here
	std::function<void(component::Collider*, component::SelfEntity*)> setBox =
		[this](component::Collider* coll, component::SelfEntity* self) {
		Entity* ent = self->GetEntity();

		// auto init by renderer
		if (coll->GetCollided()) {
			coll->SetCollided(false);

			// syncMesh
			if (coll->IsCapsule()) {
				coll->SetCapsule(false);
				component::Renderer* rend = m_ECSManager->GetComponent<component::Renderer>(ent);

				if (rend == nullptr) ERROR_QUIT("ERROR!!!!, no renderer component on collider auto init!");

				// do st
				Mesh* mesh = m_Meshes[rend->GetMesh()];

				// todo 이거 짜야 한다!!!!!!!!!!!!!!!!!!!
				coll->SetOriginBoxCenter(mesh->GetBoundingBox());
			}
			else {
				component::Renderer* rend = m_ECSManager->GetComponent<component::Renderer>(ent);

				if (rend == nullptr) ERROR_QUIT("ERROR!!!!, no renderer component on collider auto init!");

				// do st
				Mesh* mesh = m_Meshes[rend->GetMesh()];

				coll->SetOriginBox(mesh->GetBoundingBox());
			}
		}
		};

	m_ECSManager->Execute(setBox);


	// set default animation to anim executor;
	for (auto& player : m_AnimationPlayer) {
		player->ChangeToAnimation(player->m_AnimationMap[ANIMATION_STATE::IDLE]);
		player->ChangeToAnimation(player->m_AnimationMap[ANIMATION_STATE::IDLE]);
	};

	// Make shader for animation stream out
	m_AnimationShader = LoadShader("Animation_StreamOut", commandList);
	CHECK_CREATE_FAILED(m_AnimationShader, "Shader Making Failed!!");

	// clear all LateLoad
	m_ToLoadRenderDatas.clear();
	m_ToLoadUIDatas.clear();
	m_ToLoadAnimCont.clear();
	m_ToLoadAnimExe.clear();
	m_ToLoadAttach.clear();
	
	return true;
}

bool ResourceManager::LoadObjects(const std::string& sceneName, ComPtr<ID3D12GraphicsCommandList> commandList)
{
	std::string objPath = SCENE_PATH + sceneName + "\\Object\\";
	for (const auto& file : std::filesystem::directory_iterator(objPath)) {
		if (file.is_directory()) continue;

		std::string fileName = objPath + file.path().filename().string();
		CHECK_CREATE_FAILED(LoadObjectFile(fileName), std::format("Object Load Failed!! Object Name: {}", fileName));
	}
	return true;
}

bool ResourceManager::LoadCameras(const std::string& sceneName, ComPtr<ID3D12GraphicsCommandList> commandList)
{
	std::string camPath = SCENE_PATH + sceneName + "\\Camera\\";
	for (const auto& file : std::filesystem::directory_iterator(camPath)) {
		if (file.is_directory()) continue;

		std::string fileName = camPath + file.path().filename().string();
		CHECK_CREATE_FAILED(LoadObjectFile(fileName, true), std::format("Object Load Failed!! Object Name: {}", fileName));
	}

	if (m_MainCamera == nullptr) return false;

	return true;
}

bool ResourceManager::MakeExtraRenderTarget()
{
	// MRT Render Targets
	m_DefferedRTVStartIdx = static_cast<int>(m_Resources.size());

	DXGI_FORMAT format[static_cast<int>(MULTIPLE_RENDER_TARGETS::MRT_END)] = {
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		DXGI_FORMAT_R32G32B32A32_FLOAT
	};

	for (int i = 0; i < static_cast<int>(MULTIPLE_RENDER_TARGETS::MRT_END); ++i) {
		m_Resources.emplace_back(Renderer::GetInstance().CreateEmpty2DResource(
			D3D12_HEAP_TYPE_DEFAULT,
			D3D12_RESOURCE_STATE_COMMON,
			Renderer::GetInstance().GetScreenSize(),
			std::format("DifferedTarget_{}", i),
			D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
			format[i])
		);

		m_Resources[m_DefferedRTVStartIdx + i]->SetShaderResource();
	}


	// make Shadow Map Objects
	CHECK_CREATE_FAILED(MakeShadowMaps(), "Shadow Map Making Failed!!");

	// ShadowMap Render Targets
	m_ShadowMapRTVStartIdx = static_cast<int>(m_Resources.size());
	for (int i = 0; i < m_ShadowMapRenderTargets; ++i) {
		m_Resources.emplace_back(Renderer::GetInstance().CreateEmpty2DResource(
			D3D12_HEAP_TYPE_DEFAULT,
			D3D12_RESOURCE_STATE_COMMON,
			m_ShadowMaps[i].GetRTSize(),
			//Renderer::GetInstance().GetScreenSize(),
			std::format("Shadow_Map_{}", i),
			D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
			DXGI_FORMAT_R32_FLOAT)
		);

		m_Resources[m_ShadowMapRTVStartIdx + i]->SetShaderResource();
	}
	
	// create Shadpw map dsv;
	//m_ShadowMapDSVStartIdx = static_cast<int>(m_Resources.size());

	m_ShadowDSV = Renderer::GetInstance().CreateEmpty2DResourceDSV(
		D3D12_HEAP_TYPE_DEFAULT, 
		D3D12_RESOURCE_STATE_DEPTH_WRITE, 
		m_ShadowMaps[0].GetRTSize(),
		//Renderer::GetInstance().GetScreenSize(),
		"shadowmap ds buffer");
	m_ShadowDSV->SetDimension(D3D12_SRV_DIMENSION_TEXTURE2D);
	m_ShadowDSV->SetName("shadowmap ds buffer");

	return true;
}

int ResourceManager::CreateObjectResource(UINT size, const std::string resName, void** toMapData, D3D12_HEAP_TYPE heapType, RESOURCE_TYPES toInsert)
{
	COOLResourcePtr res = Renderer::GetInstance().CreateEmptyBuffer(
		heapType,
		((heapType == D3D12_HEAP_TYPE_UPLOAD ? D3D12_RESOURCE_STATE_GENERIC_READ : D3D12_RESOURCE_STATE_COMMON))/*D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER*/,
		size,
		resName,
		toMapData);

	res->SetConstant();

	switch (toInsert) {
	case RESOURCE_TYPES::OBJECT:
		m_ObjectDatas.push_back(res);
		return static_cast<int>(m_ObjectDatas.size() - 1);
	case RESOURCE_TYPES::SHADER:
		m_Resources.push_back(res);
		return static_cast<int>(m_Resources.size() - 1);
	case RESOURCE_TYPES::VERTEX:
		m_VertexIndexDatas.push_back(res);
		return static_cast<int>(m_VertexIndexDatas.size() - 1);
	}
}

void ResourceManager::SetResourceHeap(ComPtr<ID3D12GraphicsCommandList> commandList)
{
	commandList->SetDescriptorHeaps(1, m_ShaderResourceHeap.GetAddressOf());
	commandList->SetGraphicsRootDescriptorTable(0, m_ShaderResourceHeap->GetGPUDescriptorHandleForHeapStart());
}

int ResourceManager::GetTexture(ComPtr<ID3D12GraphicsCommandList> commandList, const std::string& name)
{
	// 없다면 새로 추가한다
	if (false == m_TextureIndexMap.contains(name)) {
		std::string dds = TEXTURE_PATH + name + ".dds";
		std::wstring fileNamewstr(dds.begin(), dds.end());

		auto res = Renderer::GetInstance().CreateTextureFromDDSFile(commandList, fileNamewstr.c_str());
		m_Resources.push_back(res);

		m_TextureIndexMap[name] = static_cast<int>(m_Resources.size())- 1;
	}

	return m_TextureIndexMap[name];
}

int ResourceManager::GetMaterial(const std::string& name, ComPtr<ID3D12GraphicsCommandList> commandList)
{
	for (int i = 0; i < m_Materials.size(); ++i) 
		if (m_Materials[i]->GetName() == name)
			return i;

	if (commandList) return LoadMaterial(name, commandList);
	return -1;
}

int ResourceManager::GetBone(const std::string& name, ComPtr<ID3D12GraphicsCommandList> commandList)
{
	for (int i = 0; i < m_Bones.size(); ++i)
		if (m_Bones[i]->GetName() == name)
			return i;

	if (commandList) return LoadBone(name, commandList);
	return -1;
}

int ResourceManager::GetAnimation(const std::string& name, ComPtr<ID3D12GraphicsCommandList> commandList)
{
	for (int i = 0; i < m_Animations.size(); ++i)
		if (m_Animations[i]->GetName() == name)
			return i;

	if (commandList) return LoadAnimation(name, commandList);
	return -1;
}

std::shared_ptr<Shader> ResourceManager::GetShader(const std::string& name, ComPtr<ID3D12GraphicsCommandList> commandList)
{
	for (auto& shader : m_Shaders)
		if (shader->GetName() == name)
			return shader;

	// 없다.
	if (commandList) return LoadShader(name, commandList);
	return nullptr;
}

//void ResourceManager::SetPSO(ComPtr<ID3D12GraphicsCommandList> commandList, int materialIdx)
//{
//	m_Materials[materialIdx]->GetShader()->SetPipelineState(commandList);
//	//commandList->SetPipelineState(m_Materials[materialIdx]->GetShader()->m_PipelineState);
//}

//void ResourceManager::RenderMesh(ComPtr<ID3D12GraphicsCommandList> commandList, int meshIdx, const XMFLOAT4X4& parentMatrix)
//{
//	m_Meshes[meshIdx]->Render(commandList, parentMatrix);
//}

void ResourceManager::AddLateLoad(const std::string& mesh, const std::string& material, component::Renderer* renderer)
{
	ToLoadRendererInfo info;
	info.m_MeshName = mesh;
	info.m_MaterialName = material;
	info.m_Renderer = renderer;

	m_ToLoadRenderDatas.push_back(info);
}

void ResourceManager::AddLateLoadUI(const std::string& material, component::UIRenderer* renderer)
{
	ToLoadUIRendererInfo info;
	info.m_MaterialName = material;
	info.m_Renderer = renderer;

	m_ToLoadUIDatas.push_back(info);
}

void ResourceManager::AddLateLoadAnimController(const std::string& fileName, component::AnimationController* controller)
{
	m_ToLoadAnimCont.emplace_back(fileName, controller);
}

void ResourceManager::AddLateLoadAnimExecutor(const std::string& fileName, component::AnimationExecutor* executor)
{
	m_ToLoadAnimExe.emplace_back(fileName, executor);
}

void ResourceManager::AddLateLoadAttach(const std::string& fileName, component::Attach* executor, const std::string& bone, int idx)
{
	m_ToLoadAttach.emplace_back(fileName, executor, bone, idx);
}

void ResourceManager::AddLightData()
{
	++m_LightSize;
	//return static_cast<int>(m_LightDatas.size() - 1);
}

//LightData& ResourceManager::GetLightData(int idx)
//{
//	return m_LightDatas[idx];
//}

void ResourceManager::SetMRTStates(ComPtr<ID3D12GraphicsCommandList> cmdList, D3D12_RESOURCE_STATES toState)
{
	for (int i = m_DefferedRTVStartIdx; i < m_DefferedRTVStartIdx + m_DefferedRenderTargets; ++i) {
		m_Resources[i]->TransToState(cmdList, toState);
	}
}

void ResourceManager::ClearMRTS(ComPtr<ID3D12GraphicsCommandList> cmdList, const float color[4])
{
	auto inc = Renderer::GetInstance().GetRTVHeapIncSize();
	auto decs = m_MRTHeap->GetCPUDescriptorHandleForHeapStart();
	for (int i = 0; i < m_DefferedRenderTargets; ++i) {
		cmdList->ClearRenderTargetView(decs, color, 0, nullptr);
		decs.ptr += inc;
	}
}

D3D12_CPU_DESCRIPTOR_HANDLE ResourceManager::GetDefferedRenderTargetStart() const
{
	return m_MRTHeap->GetCPUDescriptorHandleForHeapStart();
}

int ResourceManager::GetPostProcessingMaterial() const
{
	return m_PostProcessingMaterial;
}

void ResourceManager::SetShadowMapStates(ComPtr<ID3D12GraphicsCommandList> cmdList, D3D12_RESOURCE_STATES toState)
{
	for (int i = m_ShadowMapRTVStartIdx; i < m_ShadowMapRTVStartIdx + m_ShadowMapRenderTargets; ++i) {
		m_Resources[i]->TransToState(cmdList, toState);
	}
}

void ResourceManager::ClearShadowMaps(ComPtr<ID3D12GraphicsCommandList> cmdList, const float color[4])
{
	auto inc = Renderer::GetInstance().GetRTVHeapIncSize();
	auto decs = m_ShadowMapHeap->GetCPUDescriptorHandleForHeapStart();
	for (int i = 0; i < m_ShadowMapRenderTargets; ++i) {
		cmdList->ClearRenderTargetView(decs, color, 0, nullptr);
		decs.ptr += inc;
	}

}

int ResourceManager::GetUnOccupiedShadowMapRenderTarget(LIGHT_TYPES lightType)
{
	// todo light type에 따라 light render target을 따로 주자
	// ex) Directional Light -> cascaded / point -> texCube
	for (int i = 0; i < _countof(m_ShadowMapOccupied); ++i) {
		if (m_ShadowMapOccupied[i] == false) {
			m_ShadowMapOccupied[i] = true;
			return i;
		}
	}

	return -1;
}

D3D12_CPU_DESCRIPTOR_HANDLE ResourceManager::GetShadowMapRenderTarget(int idx) const
{
	D3D12_CPU_DESCRIPTOR_HANDLE temp = m_ShadowMapHeap->GetCPUDescriptorHandleForHeapStart();
	
	temp.ptr += Renderer::GetInstance().GetRTVHeapIncSize() * idx;

	return temp;
}

D3D12_CPU_DESCRIPTOR_HANDLE ResourceManager::GetShadowMapDepthStencil() const
{
	return m_ShadowMapDSVHeap->GetCPUDescriptorHandleForHeapStart();
}

int ResourceManager::GetShadowMapCamIdx(int idx)
{
	return m_ShadowMaps[idx].GetCameraDataIdx();
}

void ResourceManager::SetShadowMapCamera(ComPtr<ID3D12GraphicsCommandList> cmdList, int idx) const
{
	m_ShadowMaps[idx].SetCameraData(cmdList);
}

void ResourceManager::UpdateShadowMapView(int idx, const LightData& light)
{
	m_ShadowMaps[idx].UpdateViewMatrixByLight(light);
}

int ResourceManager::GetShadowMappingMaterial() const
{
	return m_ShadowMappingMaterial;
}

BoundingFrustum* ResourceManager::GetShadowMapFrustum(int idx)
{
	return m_ShadowMaps[idx].GetBoundingFrustum();
}

int ResourceManager::GetShadowMapRTVIdx(int idx)
{
	return m_ShadowMaps[idx].GetRenderTargetIdx();
}

void ResourceManager::SetShadowMapRTVIdx(int idx, int rtvIdx)
{
	m_ShadowMaps[idx].SetRenderTargetIdx(rtvIdx);
}

void ResourceManager::UnOccupyShadowRTVs()
{
	for (int i = 0; i < _countof(m_ShadowMapOccupied); ++i)
		m_ShadowMapOccupied[i] = false;
}

void ResourceManager::SetResourceState(ComPtr<ID3D12GraphicsCommandList> cmdList, RESOURCE_TYPES resType, int idx, D3D12_RESOURCE_STATES toState)
{
	switch (resType) {
	case RESOURCE_TYPES::SHADER:
		m_Resources[idx]->TransToState(cmdList, toState);
		break;
	case RESOURCE_TYPES::VERTEX:
		m_VertexIndexDatas[idx]->TransToState(cmdList, toState);
		break;
	case RESOURCE_TYPES::OBJECT:
		m_ObjectDatas[idx]->TransToState(cmdList, toState);
		break;
	}
}

D3D12_GPU_VIRTUAL_ADDRESS ResourceManager::GetResourceDataGPUAddress(RESOURCE_TYPES resType, int idx)
{
	switch (resType) {
	case RESOURCE_TYPES::SHADER:
		if (0 <= idx && idx < m_Resources.size())
			return m_Resources[idx]->GetResource()->GetGPUVirtualAddress();
		break;
	case RESOURCE_TYPES::VERTEX:
		if (0 <= idx && idx < m_VertexIndexDatas.size())
			return m_VertexIndexDatas[idx]->GetResource()->GetGPUVirtualAddress();
		break;
	case RESOURCE_TYPES::OBJECT:
		if (0 <= idx && idx < m_ObjectDatas.size())
			return m_ObjectDatas[idx]->GetResource()->GetGPUVirtualAddress();
		break;
	}

	return -1;
}

//D3D12_GPU_VIRTUAL_ADDRESS ResourceManager::GetVertexDataGPUAddress(int idx)
//{
//	if (0 <= idx && idx < m_VertexIndexDatas.size())
//		return m_VertexIndexDatas[idx]->GetResource()->GetGPUVirtualAddress();
//
//	return -1;
//}
//
//D3D12_GPU_VIRTUAL_ADDRESS ResourceManager::GetObjectDataGPUAddress(int idx)
//{
//	if (0 <= idx && idx < m_ObjectDatas.size())
//		return m_ObjectDatas[idx]->GetResource()->GetGPUVirtualAddress();
//
//	return -1;
//}

int ResourceManager::GetMesh(const std::string& name, ComPtr<ID3D12GraphicsCommandList> commandList)
{
	// 찾았다
	for (int i = 0; i < m_Meshes.size(); ++i)
		if (m_Meshes[i]->GetName() == name)
			return i;

	// todo 찾지 못했다면 당장 생성하지 말고 보관해두었다가 나중에 생성하게 하자
	// commandlist 인자도 지워야지
	
	// 
	// 
	// 못찾았다
	if (commandList) return LoadMesh(name, commandList);

	return -1;
}

