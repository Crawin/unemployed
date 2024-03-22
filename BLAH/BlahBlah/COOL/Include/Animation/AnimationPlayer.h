#pragma once

class Animation;

class AnimationPlayer
{
	friend class ResourceManager;
	std::string m_Name = "no name anim";

	// Animation that this player can play
	std::map <std::string, std::shared_ptr<Animation>> m_AnimationMap;

	// animation
	std::shared_ptr<Animation> m_CurrentAnimation;
	std::shared_ptr<Animation> m_BeforeAnimation;

	// animation play time
	float m_CurrentAnimationPlayTime = 0.0f;
	float m_BeforeAnimationPlayTime = 0.0f;

	// animation play speed
	float m_CurrentAnimationSpeed = 1.0f;
	float m_BeforeAnimationSpeed = 1.0f;

	// animation blending weight/ one to zero
	float m_BeforeAnimWeight = 0.0f;

	// animation change complete time
	const float m_AnimChangeSpeed = 0.4f;

public:
	AnimationPlayer();
	~AnimationPlayer();

	const std::string& GetName() const { return m_Name; }

	void Update(float deltaTime);

	void ChangeToAnimation(std::shared_ptr<Animation> newAnim);
	void ChangeToAnimation(const std::string& animName);

	void SetAnimationData(ComPtr<ID3D12GraphicsCommandList> commandList, ResourceManager* manager) const;
};

