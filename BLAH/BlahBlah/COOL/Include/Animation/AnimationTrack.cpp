#include "framework.h"
#include "AnimationTrack.h"

void AnimationTrackBase::ResetAnimationTrack()
{
	// reset
}

AnimationTrackSingle::AnimationTrackSingle(std::shared_ptr<Animation> newAnim)
{
	m_Animation = newAnim;
	m_AnimSpeed = 1.0f;
	m_CurPlayTime = 0.0f;

	m_Loop = ((newAnim != nullptr) ? newAnim->GetLoop() : false);
	m_MaxTime = ((newAnim != nullptr) ? newAnim->GetEndTime() : 0.0f);
}

void AnimationTrackSingle::Update(float deltaTime)
{
	if (m_UpdateFunction != nullptr) {
		DebugPrint("이씨를 영어로 하면?");
		DebugPrint("알 수 없음, 미스터리(mister lee)니까)");
		m_UpdateFunction(deltaTime, nullptr);
	}

	m_CurPlayTime += deltaTime * m_AnimSpeed;

	// if loop && time over max time
	if (m_CurPlayTime > m_MaxTime && m_Loop)
		m_CurPlayTime = 0;
}

void AnimationTrackSingle::ResetAnimationTrack()
{
	m_CurPlayTime = 0;
}

void AnimationTrackSingle::SetAnimationData(ComPtr<ID3D12GraphicsCommandList> commandList, bool isBefore)
{
	float animPlayTime = std::min(m_CurPlayTime, m_MaxTime - 1.0f / 24.0f);
	int animFrame = m_Animation->GetEndFrame() + 1;
	int animIndex = m_Animation->GetDataIdx();

	int playTimeRootIdx = isBefore ? static_cast<int>(ANIM_ROOTCONST::ANI_2_PLAYTIME) : static_cast<int>(ANIM_ROOTCONST::ANI_1_PLAYTIME);
	int frameRootIdx = isBefore ? static_cast<int>(ANIM_ROOTCONST::ANI_2_FRAME) : static_cast<int>(ANIM_ROOTCONST::ANI_1_FRAME);
	int animIndexRootIdx = isBefore ? static_cast<int>(ANIM_ROOTCONST::ANI_2_INDEX) : static_cast<int>(ANIM_ROOTCONST::ANI_1_INDEX);


	commandList->SetGraphicsRoot32BitConstants(static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT), 1, &animPlayTime, playTimeRootIdx);
	commandList->SetGraphicsRoot32BitConstants(static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT), 1, &animFrame, frameRootIdx);
	commandList->SetGraphicsRoot32BitConstants(static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT), 1, &animIndex, animIndexRootIdx);

}

XMMATRIX AnimationTrackSingle::GetAnimatedBoneMat(int boneIdx) const
{
	int endFrame = m_Animation->GetEndFrame();
	float playTime = std::min(m_CurPlayTime, m_MaxTime - 1.0f / 24.0f);;

	int currentFrame = floor(playTime * m_Animation->GetFrame());
	float interpolWeight = ceil(playTime * m_Animation->GetFrame()) - playTime * m_Animation->GetFrame();
	int amimFrame = endFrame + 1;
	int idx = boneIdx * amimFrame + currentFrame;

	//// todo 고쳐야한다......
	XMFLOAT4X4 left =
	{
		-1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1
	};

	XMMATRIX convertLeft = XMLoadFloat4x4(&left);

	XMMATRIX res = interpolWeight * XMLoadFloat4x4(&m_Animation->GetBone(idx)) + (1 - interpolWeight) * XMLoadFloat4x4(&m_Animation->GetBone(idx + 1));

	res = res * convertLeft;
	//printf("frame: %3d, idx1: %d. idx2: %d \t weight: %.1f\n", currentFrame, idx, idx + 1, interpolWeight);
		//lerp(Animation_cur[idx1 + 1], Animation_cur[idx1], interpolWeight));
	return XMMatrixTranspose(res);
}

void AnimationTrackBlendingSpace2D::Update(float deltaTime)
{
	if (m_UpdateFunction == nullptr) 
	{
		DebugPrint("AnimationTrackBlendingSpace2D::m_UpdateFunction missing.");
		DebugPrint("프로가 실수 하는 이유는?");
		DebugPrint("약속(프로\"미스\")했기 때문에");
	}
	else {
		m_UpdateFunction(deltaTime, &m_CurrentPoint);
	}


}

void AnimationTrackBlendingSpace2D::ResetAnimationTrack()
{
	m_CurPlayTime = 0;
}

void AnimationTrackBlendingSpace2D::SetAnimationData(ComPtr<ID3D12GraphicsCommandList> commandList, bool isBefore)
{
}

XMMATRIX AnimationTrackBlendingSpace2D::GetAnimatedBoneMat(int boneIdx) const
{
	return XMMATRIX();
}

void AnimationTrackBlendingSpace2D::InsertAnimation(float atSpeed, float atAngle, std::shared_ptr<Animation> anim)
{
	m_AnimationSpace.emplace_back(Point2D(atSpeed, atAngle), anim);
}
