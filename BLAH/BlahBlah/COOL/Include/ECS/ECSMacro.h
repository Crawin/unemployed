#pragma once

// todo
// type별로 루프를 여러번 돌고 있다. 개선할 수 있으면 개선하면 좋겠다.

#define INSERT_COLLIDE_EVENT(MANAGER, SELF_COMP, OTHER_COMP, TYPE, FUNC)															\
{																																	\
	EventFunction eventFunc = FUNC;																									\
	std::function<void(SELF_COMP*, component::Collider*)> insertEvent = [&eventFunc](SELF_COMP* , component::Collider* col) {		\
		col->InsertEvent<OTHER_COMP>(eventFunc , TYPE);																				\
		};																															\
	MANAGER->Execute(insertEvent);																									\
}

#define SET_INTERACTION_EVENT(MANAGER, SELF_COMP, FUNC)																				\
{																																	\
	InteractionFuncion eventFunc = FUNC;																							\
	std::function<void(SELF_COMP*, component::Collider*)> insertEvent = [&eventFunc](SELF_COMP* , component::Collider* col) {		\
		col->SetInteractionFunction(eventFunc);																						\
		};																															\
	MANAGER->Execute(insertEvent);																								\
}
