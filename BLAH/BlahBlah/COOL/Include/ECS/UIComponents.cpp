#include "framework.h"
#include "UIComponents.h"
#include "Scene/ResourceManager.h"
#include "json/json.h"


namespace component {
	int UICanvas::m_NextID = 0;


	void UICanvas::Create(Json::Value& v, ResourceManager* rm)
	{
		Json::Value canvas = v["UICanvas"];

		m_On = canvas["On"].asBool();

		m_CanvasID = m_NextID++;
	}

	void UICanvas::ShowYourself() const
	{
	}

	void UITransform::Create(Json::Value& v, ResourceManager* rm)
	{
		Json::Value transform = v["UITransform"];

		m_Center = { transform["Center"][0].asInt(), transform["Center"][1].asInt()};
		m_Size = { transform["Size"][0].asInt(), transform["Size"][1].asInt()};
	}

	void UITransform::ShowYourself() const
	{
	}

	void UIRenderer::Create(Json::Value& v, ResourceManager* rm)
	{
		Json::Value rend = v["UIRenderer"];

		rm->AddLateLoadUI(rend["Material"].asString(), this);

		m_IsSprite = rend["IsSprite"].asBool();

		// todo
		// get sprite position
		if (m_IsSprite) 
		{
			//m_SpriteX = 0;
			//m_SpriteY = 0;
			//m_SpriteNum = 0;
		}
	}

	void UIRenderer::ShowYourself() const
	{

	}

	void Button::Create(Json::Value& v, ResourceManager* rm)
	{
		Json::Value button = v["Button"];

		m_On = button["On"].asBool();
	}

	void Button::ShowYourself() const
	{
	}
}
