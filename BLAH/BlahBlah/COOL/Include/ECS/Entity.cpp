#include "framework.h"
#include "Entity.h"

void Entity::EraseChild(Entity* ent)
{
	auto it = std::find(m_Children.begin(), m_Children.end(), ent); 

	if (it != m_Children.end()) 
		m_Children.erase(it);
}
