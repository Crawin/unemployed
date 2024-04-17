#pragma once

class Animation;

struct AnimationPlayQueueData {
	std::shared_ptr<Animation> m_Anim = nullptr;
	float m_CurPlayTime = 0.0f;
	float m_MaxTime = 0.0f;
	float m_AnimSpeed = 1.0f;
	bool m_Loop = false;

	void Update(float deltaTime);
};

class AnimationPlayer
{
	friend class ResourceManager;
	std::string m_Name = "no name anim";

	// Animation that this player can play
	std::map <ANIMATION_STATE, std::shared_ptr<Animation>> m_AnimationMap;

	// animation
	AnimationPlayQueueData m_BeforeAnimation;
	AnimationPlayQueueData m_CurrentAnimation;

	// blending weight
	float m_BeforeAnimWeight = 0.0f;

	// animation change complete time
	const float m_AnimChangeSpeed = 0.2f;

	void ChangeToAnimation(std::shared_ptr<Animation> newAnim, float endTime = 0.0f, float startAnimAt = 0.0f, float speed = 1.0f);

public:
	AnimationPlayer();
	~AnimationPlayer();

	const std::string& GetName() const { return m_Name; }

	void Update(float deltaTime);

	void ChangeToAnimation(ANIMATION_STATE animSet);

	void SetAnimationData(ComPtr<ID3D12GraphicsCommandList> commandList, ResourceManager* manager) const;

	// for attach pos
	XMMATRIX& GetAnimatedBone(XMMATRIX& bone, int boneIdx) const;

public:
	float GetCurrentPlayTime() const { return m_CurrentAnimation.m_CurPlayTime; }
	float GetCurrentPlayEndTime() const { return m_CurrentAnimation.m_MaxTime; }

};

