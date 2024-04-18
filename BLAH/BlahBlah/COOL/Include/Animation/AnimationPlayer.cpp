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
	float firstAnimPlayTime = min(m_CurrentAnimation.m_CurPlayTime, m_CurrentAnimation.m_MaxTime - 1.0f / 24.0f);
	float secondAnimPlayTime = min(m_BeforeAnimation.m_CurPlayTime, m_BeforeAnimation.m_MaxTime - 1.0f / 24.0f);
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

XMMATRIX& AnimationPlayer::GetAnimatedBone(XMMATRIX& bone, int boneIdx) const
{
	XMMATRIX animRes;

	// get current anim frame
	auto& anim1 = m_CurrentAnimation.m_Anim;

	float anim1PlayTime = m_CurrentAnimation.m_CurPlayTime;// min(m_CurrentAnimation.m_CurPlayTime, m_CurrentAnimation.m_MaxTime);// -1.0f / anim1->GetFrame());
	int anim1EndFrame = anim1->GetEndFrame();
	int anim1CurFrame = floor(anim1PlayTime * anim1->GetFrame());

	if (anim1PlayTime > m_CurrentAnimation.m_MaxTime) {
		anim1PlayTime = m_CurrentAnimation.m_MaxTime;
		anim1CurFrame = anim1->GetEndFrame() - 1;
	}


	XMMATRIX boneInv = XMMatrixInverse(nullptr, bone);

	// get current actual bone idx
	int idx1 = anim1->GetEndFrame() * boneIdx + anim1CurFrame;
	int idx2 = idx1 + 1;

	// if over
	if (anim1CurFrame >= anim1EndFrame - 1) {
		if (m_CurrentAnimation.m_Loop) 
			idx2 = anim1->GetEndFrame() * boneIdx;
		else idx2 = idx1;
	}

	float anim1InterpolWegith = ceil(anim1PlayTime * anim1->GetFrame()) - anim1PlayTime * anim1->GetFrame();

	//if (anim1CurFrame == 28)
	//DebugPrint(std::format("frame: {}, idx1: {} idx2: {}\t weight: {}", anim1CurFrame, idx1, idx2, anim1InterpolWegith));
	printf("frame: %3d, idx1: %d. idx2: %d \t weight: %.1f\n", anim1CurFrame, idx1, idx2, anim1InterpolWegith);

	animRes = XMMatrixTranspose((1 - anim1InterpolWegith) * XMLoadFloat4x4(&(anim1->GetBone(idx2))) + anim1InterpolWegith * XMLoadFloat4x4(&(anim1->GetBone(idx1))));// *bone;
	animRes = XMMatrixTranspose(XMLoadFloat4x4(&(anim1->GetBone(idx1))));

	// blending
	if (m_BeforeAnimWeight > 0) {
		auto& anim2 = m_BeforeAnimation.m_Anim;

		float anim2PlayTime = min(m_BeforeAnimation.m_CurPlayTime, m_BeforeAnimation.m_MaxTime - 1.0f / anim2->GetFrame());
		int anim2EndFrame = anim2->GetEndFrame();
		int anim2CurFrame = floor(anim2PlayTime * anim2->GetFrame());

		int idx3 = anim2->GetEndFrame() * boneIdx + anim2CurFrame;
		int idx4 = idx3 + 1;

		// if over
		if (anim2CurFrame + 1 >= anim2EndFrame) {
			if (m_BeforeAnimation.m_Loop) idx4 = anim2->GetEndFrame() * boneIdx;
			else idx4 = idx3;
		}

		float anim2InterpolWegith = ceil(anim1PlayTime * anim1->GetFrame()) - anim1PlayTime * anim1->GetFrame();

		XMMATRIX animRes2 = XMMatrixTranspose((1 - anim2InterpolWegith) * XMLoadFloat4x4(&(anim2->GetBone(idx4))) + anim2InterpolWegith * XMLoadFloat4x4(&(anim2->GetBone(idx3))));// *bone;

		animRes = (1 - m_BeforeAnimWeight) * animRes + m_BeforeAnimWeight * animRes2;
	}

	//float firstAnimPlayTime = min(m_CurrentAnimation.m_CurPlayTime, m_CurrentAnimation.m_MaxTime - 1.0f / 24.0f);
	//int firstAnimFrame = m_CurrentAnimation.m_Anim->GetEndFrame();
	//int idx1 = boneIdx * firstAnimFrame + floor(firstAnimPlayTime * m_CurrentAnimation.m_Anim->GetFrame());
	//float anim1InterpolWegith = ceil(firstAnimPlayTime * m_CurrentAnimation.m_Anim->GetFrame()) - firstAnimPlayTime * m_CurrentAnimation.m_Anim->GetFrame();
	//auto curAnim = m_CurrentAnimation.m_Anim;

	//XMMATRIX boneInv = bone;
	//return bone;
	////XMMATRIX boneInv = XMMatrixInverse(nullptr, bone);

	//XMMATRIX anim1 = XMMatrixTranspose(anim1InterpolWegith * XMLoadFloat4x4(&(curAnim->GetBone(idx1 + 1))) + (1 - anim1InterpolWegith) * XMLoadFloat4x4(&(curAnim->GetBone(idx1)))) * boneInv;

	//if (m_BeforeAnimWeight > 0) {
	//	float secondAnimPlayTime = min(m_BeforeAnimation.m_CurPlayTime, m_BeforeAnimation.m_MaxTime - 1.0f / 24.0f);
	//	int secondAnimFrame = m_BeforeAnimation.m_Anim->GetEndFrame();
	//	int idx2 = boneIdx * secondAnimFrame + floor(secondAnimPlayTime * m_BeforeAnimation.m_Anim->GetFrame());
	//	float anim2InterpolWegith = ceil(secondAnimPlayTime * m_BeforeAnimation.m_Anim->GetFrame()) - secondAnimPlayTime * m_BeforeAnimation.m_Anim->GetFrame();
	//	auto befAnim = m_BeforeAnimation.m_Anim;

	//	XMMATRIX anim2 = XMMatrixTranspose(anim2InterpolWegith * XMLoadFloat4x4(&(befAnim->GetBone(idx2 + 1))) + (1 - anim2InterpolWegith) * XMLoadFloat4x4(&(befAnim->GetBone(idx2)))) * boneInv;

	//	anim1 =
	//		m_BeforeAnimWeight *		anim2 +
	//		(1 - m_BeforeAnimWeight) *	anim1;
	//}


	return animRes;
}
