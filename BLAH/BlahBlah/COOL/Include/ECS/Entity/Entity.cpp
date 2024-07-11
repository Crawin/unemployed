#include "framework.h"
#include "Entity.h"

void Entity::AddChild(Entity* entity)
{
	entity->SetParent(this);
	m_Children.push_back(entity);
}

void Entity::EraseChild(Entity* ent)
{
	ent->SetParent(nullptr);

	auto it = std::find(m_Children.begin(), m_Children.end(), ent); 

	if (it != m_Children.end()) 
		m_Children.erase(it);
}
