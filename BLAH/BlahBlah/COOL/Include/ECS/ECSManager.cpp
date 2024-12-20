﻿#include "framework.h"
#include "ECSManager.h"
#include "Network/Client.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Component Container
/////////////////////////////////////////////////////////////////////////////////////////////////////

ComponentContainer::ComponentContainer(size_t stride)
	: m_Stride{ stride }
{
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Component Set
/////////////////////////////////////////////////////////////////////////////////////////////////////

inline ComponentSet::ComponentSet(COMP_BITSET bit)
{
	// 미리 해당 bitset에 맞춰 componentcontainer를 만들어둔다.
	for (int i = 0; i < COMPONENT_COUNT; ++i) {
		if (bit.test(i)) {
			// add
			size_t componentSize = GET_COMP_SIZE(i);
			
			COMP_BITSET tempBit(1);
			tempBit <<= i;

			m_Set.emplace(tempBit, componentSize);
		}
	}
}

void ComponentSet::InsertComponent(component::Component* comp)
{
	COMP_BITSET bit = comp->GetBitset();

	// check the component is exist in the set
	if (m_Set.contains(bit) == false) {
		ERROR_QUIT("ERROR!! not this component");
		//exit(1);
	}

	// insert component;
	m_Set[bit].push_back(comp);

}

void ComponentSet::InsertComponentByEntity(Entity* entity)
{
	for (auto& comp : entity->m_Components) 
	{
		comp->ShowYourself();
		InsertComponent(comp);

		// release before comp here
		delete comp;
	}

	entity->m_Components.clear();

	entity->m_Id = m_EntitySize++;
}

void ComponentSet::OnStart(Entity* ent, ECSManager* manager, ResourceManager* rm)
{
	int idx = ent->GetInnerID();

	for (auto& [key, val] : m_Set) {
		component::Component* comp = val.GetData<component::Component>(idx);
		comp->OnStart(ent, manager, rm);
	}

}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// ECS System
/////////////////////////////////////////////////////////////////////////////////////////////////////

ECSManager::ECSManager()
{
	m_TimeLineSystem = new ECSsystem::TimeLineManaging;
	m_ParticleSystem = new ECSsystem::ParticleManaging;
	InsertSystem(m_TimeLineSystem);
	InsertSystem(m_ParticleSystem);
}

ECSManager::~ECSManager()
{
	// todo delete 모두 잘 하는지 얘가 확인 해보자
	for (auto& ent : m_Entities)
		delete ent;

	for (auto& sys : m_Systems)
		delete sys;

	for (auto& sys : m_PreRenderSystem)
		delete sys;

	// component엔 동적할당 하지 않았음
}

void ECSManager::AddEntity(Entity* entity)
{
	// 1. 일단 entity가 어떤 component를 가지고 있는지 확인해야함
	COMP_BITSET bitset(0);
	for (auto& comp : entity->m_Components) 
		bitset |= comp->GetBitset();

	// 여기에 컴포넌트set 맞춰서 넣으면 된다.
	if (m_ComponentSets.contains(bitset) == false) 
	{
		// 없다면 만들어둔다
		m_ComponentSets.emplace(bitset, bitset);
	}

	DebugPrint(std::format("//////////////////////////entity bitset: {}", bitset.to_string()));
	m_ComponentSets[bitset].InsertComponentByEntity(entity);
	entity->m_Bitset = bitset;

	m_Entities.push_back(entity);
}

void ECSManager::AddToRoot(Entity* entity)
{
	m_RootEntities.push_back(entity);
}

void ECSManager::AttachChild(Entity* to, Entity* targetEntity)
{
	// if was parent, erase from root list
	auto iter = std::find(m_RootEntities.begin(), m_RootEntities.end(), targetEntity);
	if (iter != m_RootEntities.end()) m_RootEntities.erase(iter);

	// if was not parent, detach from its parent
	Entity* befParent = targetEntity->GetParent();
	if (befParent != nullptr) {
		befParent->EraseChild(targetEntity);
	}

	// add to child
	to->AddChild(targetEntity);

	// 
	auto selfEntComp = GetComponent<component::SelfEntity>(targetEntity);
	selfEntComp->SetParent(to);
}

void ECSManager::DetachChild(Entity* from, Entity* targetEntity)
{
	// add to root entity list
	auto iter = std::find(m_RootEntities.begin(), m_RootEntities.end(), targetEntity);
	if (iter == m_RootEntities.end()) m_RootEntities.push_back(targetEntity);

	// erase from its entity ch
	from->EraseChild(targetEntity);

	// when detach, set entity to world pos
	auto selfTr = GetComponent<component::Transform>(targetEntity);
	//selfTr->SetRotation(selfTr->GetWorldRotation());
	selfTr->SetPosition(selfTr->GetWorldPosition());
	selfTr->SetParentTransform(Matrix4x4::Identity());
	
	auto selfEntComp = GetComponent<component::SelfEntity>(targetEntity);
	selfEntComp->SetParent(nullptr);
}

void ECSManager::InitSystem()
{
	for (auto& system : m_Systems) {
		system->OnInit(this);
	}

	for (auto& system : m_PreRenderSystem) {
		system->OnInit(this);
	}
}

void ECSManager::UpdateSystem(float deltaTime)
{
	for (auto& system : m_Systems) {
		system->Update(this, deltaTime);
	}
}

void ECSManager::UpdatePreRenderSystem(float deltaTime)
{
	for (auto& system : m_PreRenderSystem) {
		system->Update(this, deltaTime);
	}
}

void ECSManager::OnStart(ResourceManager* rm)
{
	if (m_Started) return;
	m_Started = true;
	
	// for every entities
	for (Entity* ent : m_Entities) {
		// On Start
		COMP_BITSET entBitset = ent->GetBitset();

		m_ComponentSets[entBitset].OnStart(ent, this, rm);
	}

	// todo
	// target을 host일 때와 아닐 때로 구분해서 
	short type = Client::GetInstance().getCharType();

	std::string targetName(Client::GetInstance().GetHostPlayerName());

	// if not host
	if (type != 1)
		targetName = Client::GetInstance().GetHostPlayerName();

	component::PlayerController* ctrl = nullptr;

	std::function<void(component::PlayerController*)> getController = [this, &targetName, &ctrl](component::PlayerController* c) { 
		ctrl = c;
		};
	Execute(getController);

	// set possess
	if (ctrl) {
		ctrl->Possess(this, targetName);
		Entity* curEntity = ctrl->GetControllingPawn()->GetSelfEntity();

		// set inventory modes
		std::function<void(component::Inventory*)> setSubMode = [this](component::Inventory* inv) { inv->SetMainMode(false, this); };
		Execute(setSubMode);

		component::Inventory* inventory = GetComponent<component::Inventory>(curEntity);
		if (inventory) {
			inventory->SetMainMode(true, this);
		}
	}

}

Entity* ECSManager::GetEntity(const std::string& targetName)
{
	Entity* target = nullptr;
	std::function<void(component::Name*, component::SelfEntity*)> getEntity = [&target, &targetName](component::Name* name, component::SelfEntity* selfEnt) {
		if (name->getName().compare(targetName) == 0) target = selfEnt->GetEntity();
		};

	Execute(getEntity);

	return target;
}

Entity* ECSManager::GetEntityInChildren(const std::string& name, Entity* parent)
{
	component::Name* childNameComp = nullptr;

	for (Entity* child : parent->GetChildren()) {
		childNameComp = GetComponent<component::Name>(child);
		if (childNameComp && childNameComp->getName() == name)
			return child;
	}

	return nullptr;
}

Entity* ECSManager::GetEntityFromRoute(const std::string& name, Entity* parent)
{
	Entity* entityTarget = parent;

	size_t start = 0;
	size_t end = name.find('/');

	while (end != std::string::npos) {
		entityTarget = GetEntityInChildren(name.substr(start, end - start), entityTarget);

		if (entityTarget == nullptr)
			ERROR_QUIT("ERROR!!! hierachy error");

		start = end + 1;
		end = name.find('/', start);
	}

	entityTarget = GetEntityInChildren(name.substr(start), entityTarget);
	if (entityTarget == nullptr)
		ERROR_QUIT("ERROR!! no holding hand in current child, inventory component error");

	return entityTarget;
}

void ECSManager::SetEntityState(Entity* entity, bool active)
{
	// renderer
	component::Renderer* rend = GetComponent<component::Renderer>(entity);
	if (rend) rend->SetActive(active);
	
	// dynamic collider
	component::DynamicCollider* dCol = GetComponent<component::DynamicCollider>(entity);
	if (dCol) dCol->SetActive(active);

	// Physics
	component::Physics* phys = GetComponent<component::Physics>(entity);
	if (phys) phys->SetCalculateState(active);

	for (Entity* child : entity->GetChildren())
		SetEntityState(child, active);
}

void ECSManager::InitPartlcie()
{
	ECSsystem::ParticleManaging* pm = m_ParticleSystem;

	std::function<void(component::Particle*)> part = [&pm](component::Particle* par) {
		pm->InitComponent(par->GetType(), par);
		};
	Execute(part);
}

void ECSManager::SyncParticle()
{
	m_ParticleSystem->SyncParticle();
}
