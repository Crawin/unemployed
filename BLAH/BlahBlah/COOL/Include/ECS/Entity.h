#pragma once

class Component;

class Entity
{
	std::unordered_map<int, Component*> m_Components;

public:

	template <class T>
	void AddComponent() {
		int compID = T::GetIndex();
		if (m_Components.contains(compID)) {
			// 이미 있다
			return;
		}


		T* component = new T;

	}
};
