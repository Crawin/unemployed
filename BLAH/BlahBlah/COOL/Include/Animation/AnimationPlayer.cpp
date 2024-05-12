#include "framework.h"
#include "AnimationPlayer.h"
#include "Animation/Animation.h"
#include "Scene/ResourceManager.h"


void AnimationPlayQueueData::Update(float deltaTime)
{
	m_CurPlayTime += deltaTime * m_AnimSpeed;

	// if loop && time over max time
	if (m_CurPlayTime > m_MaxTime && m_Loop)
		m_CurPlayTime = 0;
}

XMMATRIX AnimationPlayQueueData::GetAnimatedBoneMat(int boneIdx) const
{

	int endFrame = m_Anim->GetEndFrame();
	float playTime = std::min(m_CurPlayTime, m_MaxTime - 1.0f / 24.0f);;
	
	int currentFrame = floor(playTime * m_Anim->GetFrame());
	float interpolWeight = ceil(playTime * m_Anim->GetFrame()) - playTime * m_Anim->GetFrame();
	int amimFrame = endFrame + 1;
	int idx = boneIdx * amimFrame + currentFrame;

	XMFLOAT4X4 left =
	{
		-1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1
	};

	XMMATRIX convertLeft = XMLoadFloat4x4(&left);

	XMMATRIX res = interpolWeight * XMLoadFloat4x4(&m_Anim->GetBone(idx)) + (1 - interpolWeight) * XMLoadFloat4x4(&m_Anim->GetBone(idx + 1));
	
	res = res * convertLeft;
	//printf("frame: %3d, idx1: %d. idx2: %d \t weight: %.1f\n", currentFrame, idx, idx + 1, interpolWeight);
		//lerp(Animation_cur[idx1 + 1], Animation_cur[idx1], interpolWeight));
	return XMMatrixTranspose(res);
}

AnimationPlayer::AnimationPlayer()
{
}

AnimationPlayer::~AnimationPlayer()
{
}

void AnimationPlayer::Update(float deltaTime)
{
	// anim blending time
	if (m_BeforeAnimWeight > 0) {
		m_BeforeAnimWeight -= deltaTime / m_AnimChangeSpeed;
		if (m_BeforeAnimWeight <= 0) m_BeforeAnimWeight = 0;
	}

	// anim play time
	m_CurrentAnimation.Update(deltaTime);
	m_BeforeAnimation.Update(deltaTime);
}

void AnimationPlayer::ChangeToAnimation(std::shared_ptr<Animation> newAnim, float endTime, float startAnimAt, float speed)
{
	AnimationPlayQueueData newData;
	newData.m_Anim = newAnim;
	newData.m_AnimSpeed = speed;
	newData.m_CurPlayTime = startAnimAt;
	newData.m_Loop = ((newAnim != nullptr) ? newAnim->GetLoop() : false);
	newData.m_MaxTime = ((newAnim != nullptr) ? newAnim->GetEndTime() : endTime);

	m_BeforeAnimWeight = 1.0f;

	m_BeforeAnimation = m_CurrentAnimation;
	m_CurrentAnimation = newData;
}

void AnimationPlayer::ChangeToAnimation(ANIMATION_STATE animSet)
{
	if (m_AnimationMap.contains(animSet)) {
		ChangeToAnimation(m_AnimationMap[animSet]);
	}
	else
		ERROR_QUIT(std::format("NO SUCH ANIMATION DEFINED!!, name: {}", ConvertAnimationStateToString(animSet)));
}

void AnimationPlayer::SetAnimationData(ComPtr<ID3D12GraphicsCommandList> commandList, ResourceManager* manager) const
{
	int firstAnimIdx = m_CurrentAnimation.m_Anim->GetDataIdx();
	int	secondAnimIdx = m_BeforeAnimation.m_Anim->GetDataIdx();
	float weight = m_BeforeAnimWeight;
	float firstAnimPlayTime = std::min(m_CurrentAnimation.m_CurPlayTime, m_CurrentAnimation.m_MaxTime - 1.0f / 24.0f);
	float secondAnimPlayTime = std::min(m_BeforeAnimation.m_CurPlayTime, m_BeforeAnimation.m_MaxTime - 1.0f / 24.0f);
	int firstAnimFrame = m_CurrentAnimation.m_Anim->GetEndFrame() + 1;
	int secondAnimFrame = m_BeforeAnimation.m_Anim->GetEndFrame() + 1;

	commandList->SetGraphicsRoot32BitConstants(static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT), 1, &firstAnimPlayTime, static_cast<int>(ANIM_ROOTCONST::ANI_1_PLAYTIME));
	commandList->SetGraphicsRoot32BitConstants(static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT), 1, &secondAnimPlayTime, static_cast<int>(ANIM_ROOTCONST::ANI_2_PLAYTIME));
	commandList->SetGraphicsRoot32BitConstants(static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT), 1, &weight, static_cast<int>(ANIM_ROOTCONST::ANI_BLEND));
	commandList->SetGraphicsRoot32BitConstants(static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT), 1, &firstAnimFrame, static_cast<int>(ANIM_ROOTCONST::ANI_1_FRAME));
	commandList->SetGraphicsRoot32BitConstants(static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT), 1, &secondAnimFrame, static_cast<int>(ANIM_ROOTCONST::ANI_2_FRAME));
	commandList->SetGraphicsRoot32BitConstants(static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT), 1, &firstAnimIdx, static_cast<int>(ANIM_ROOTCONST::ANI_1_INDEX));
	commandList->SetGraphicsRoot32BitConstants(static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT), 1, &secondAnimIdx, static_cast<int>(ANIM_ROOTCONST::ANI_2_INDEX));
}

XMMATRIX& AnimationPlayer::GetAnimatedBone(int boneIdx) const
{
	XMMATRIX animRes;

	animRes = m_CurrentAnimation.GetAnimatedBoneMat(boneIdx);

	// blending
	if (m_BeforeAnimWeight > 0) 
		animRes = (1 - m_BeforeAnimWeight) * animRes + m_BeforeAnimWeight * m_BeforeAnimation.GetAnimatedBoneMat(boneIdx);

	return animRes;
}
