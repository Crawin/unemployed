#include "framework.h"
#include "AnimationPlayer.h"
#include "AnimationTrack.h"
#include "Animation.h"
#include "Scene/ResourceManager.h"

AnimationPlayer::AnimationPlayer()
{
}

AnimationPlayer::~AnimationPlayer()
{
	for (auto& animTrack : m_AnimationMap)
		delete animTrack.second;
}

void AnimationPlayer::Update(float deltaTime)
{
	// anim blending time
	if (m_BeforeAnimWeight > 0) {
		m_BeforeAnimWeight -= deltaTime / m_AnimChangeSpeed;
		if (m_BeforeAnimWeight <= 0) m_BeforeAnimWeight = 0;
	}

	// anim play time
	m_CurrentAnimation->Update(deltaTime);
	if (m_BeforeAnimWeight > 0 || m_CurrentAnimation->GetAffectUpperState() == true) m_BeforeAnimation->Update(deltaTime);
}

void AnimationPlayer::ChangeToAnimation(ANIMATION_STATE animSet)
{
	if (m_AnimationMap.contains(animSet)) {
		if (m_BeforeAnimation)
			m_BeforeAnimation->ResetAnimationTrack();
		AnimationTrackBase* nextTrack = m_AnimationMap[animSet];

		// cur and next are affect upper
		if (nextTrack->GetAffectUpperState() && m_CurrentAnimation->GetAffectUpperState()) {
			m_CurrentAnimation = m_AnimationMap[animSet];
		}
		else {
			m_BeforeAnimation = m_CurrentAnimation;
			m_CurrentAnimation = m_AnimationMap[animSet];
		}
		m_BeforeAnimWeight = 1.0f;
	}
	else
		ERROR_QUIT(std::format("NO SUCH ANIMATION DEFINED!!, name: {}", ConvertAnimationStateToString(animSet)));
}

void AnimationPlayer::SetAnimationData(ComPtr<ID3D12GraphicsCommandList> commandList, ResourceManager* manager) const
{
	m_CurrentAnimation->SetAnimationData(commandList, true);
	m_BeforeAnimation->SetAnimationData(commandList, false);
	
	commandList->SetGraphicsRoot32BitConstants(static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT), 
		1, &m_BeforeAnimWeight, static_cast<int>(ANIM_ROOTCONST::BEFORE_ANIM_BLEND_WEIGHT));
}

XMMATRIX& AnimationPlayer::GetAnimatedBone(int boneIdx) const
{
	XMMATRIX animRes;

	animRes = m_CurrentAnimation->GetAnimatedBoneMat(boneIdx);

	// blending
	if (m_BeforeAnimWeight > 0 && m_CurrentAnimation != m_BeforeAnimation)
		animRes = (1 - m_BeforeAnimWeight) * animRes + m_BeforeAnimWeight * m_BeforeAnimation->GetAnimatedBoneMat(boneIdx);

	return animRes;
}

void AnimationPlayer::SetUpdateFunctionTo(ANIMATION_STATE state, std::function<void(float, void*)> func)
{
	m_AnimationMap[state]->SetUpdateFunction(func);
}

float AnimationPlayer::GetCurrentPlayTime() const
{
	return m_CurrentAnimation->GetCurrentPlayTime();
}

float AnimationPlayer::GetCurrentPlayEndTime() const
{
	return m_CurrentAnimation->GetMaxPlaytime();
}
