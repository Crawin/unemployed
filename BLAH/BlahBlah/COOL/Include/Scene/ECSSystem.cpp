#include "framework.h"
#include "ECSSystem.h"
#include "Object/Entity.h"
#include "Object/Component.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Component Container
/////////////////////////////////////////////////////////////////////////////////////////////////////

ComponentContainer::ComponentContainer(size_t stride)
	: m_Stride{ stride }
{
}

template<class T>
inline void ComponentContainer::push_back(const T* data)
{
	++m_Elements;
	m_Data.resize(m_Data.size() + m_Stride);

	memcpy(&m_Data.front(), data, m_Stride);
}

template<class T>
T* ComponentContainer::operator[](size_t index)
{
	auto realIdx = index * m_Stride;

	// 포인터로 넘겨줘야 할듯하다
	return static_cast<T*>(&m_Data[realIdx]);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Component Set
/////////////////////////////////////////////////////////////////////////////////////////////////////

template<std::size_t N>
inline ComponentSet<N>::ComponentSet(std::bitset<N> bit)
{
	// 미리 해당 bitset에 맞춰 componentcontainer를 만들어둔다.
	for (int i = 0; i < N; ++i) {
		std::bitset<N> tempBit(1);
		tempBit <<= i;

		if (bit & tempBit) {
			// add
			size_t componentSize = GET_COMP_SIZE(i);


			m_Set.emplace(m_Set[tempBit], componentSize);
			
			//std::bitset bitset = Components::m_BIT;

		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// ECS System
/////////////////////////////////////////////////////////////////////////////////////////////////////

template<std::size_t N>
inline void ECSSystem<N>::AddEntity(Entity* entity)
{
	// 1. 일단 entity가 어떤 component를 가지고 있는지 확인해야함
	std::bitset<N> bitset(0);
	for (auto& comp : entity->m_Components) {
		std::bitset<N> compBit(1);
		compBit <<= comp->GetGID();

		bitset |= compBit;
	}

	// 여기에 컴포넌트set 맞춰서 넣으면 된다.

	DebugPrint(std::format("entity bitset: {}", bitset.to_string()));

}

