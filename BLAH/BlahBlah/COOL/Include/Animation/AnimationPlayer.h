#pragma once

class Animation;
class AnimationTrackBase;

class AnimationPlayer
{
	friend class ResourceManager;
	std::string m_Name = "no name anim";

	// Animation that this player can play
	std::map <ANIMATION_STATE, AnimationTrackBase*> m_AnimationMap;

	// animation
	AnimationTrackBase* m_BeforeAnimation = nullptr;
	AnimationTrackBase* m_CurrentAnimation = nullptr;

	// blending weight
	float m_BeforeAnimWeight = 0.0f;

	// animation change complete time
	const float m_AnimChangeSpeed = 0.2f;

public:
	AnimationPlayer();
	~AnimationPlayer();

	const std::string& GetName() const { return m_Name; }

	void Update(float deltaTime);

	void ChangeToAnimation(ANIMATION_STATE animSet);

	void SetAnimationData(ComPtr<ID3D12GraphicsCommandList> commandList, ResourceManager* manager) const;

	// for attach pos
	XMMATRIX& GetAnimatedBone(int boneIdx) const;

public:
	float GetCurrentPlayTime() const;
	float GetCurrentPlayEndTime() const;

};

