#include "framework.h"
#include "AnimationPlayer.h"
#include "Animation/Animation.h"
#include "Scene/ResourceManager.h"

void AnimationPlayer::ChangeToNextAnimation()
{
	auto anim = m_NextAnimationQueue.front();

	m_FinishPlaying = false;

	m_BeforeAnimation = m_CurrentAnimation;
	m_BeforeAnimationPlayTime = m_CurrentAnimationPlayTime;
	m_BeforeAnimationSpeed = m_CurrentAnimationSpeed;

	// todo 새로운 정보를 받아옴
	m_BeforeAnimWeight = 1.0f;
	m_CurrentAnimation = anim;
	m_CurrentAnimationPlayTime = 0.0f;
	// todo default speed? 
	m_CurrentAnimationSpeed = 1.0f;

	m_NextAnimationQueue.pop();
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
	if (m_BeforeAnimation->GetLoop() == true || m_FinishPlaying == false) m_CurrentAnimationPlayTime += deltaTime * m_CurrentAnimationSpeed;
	if (m_BeforeAnimation->GetLoop() == true) m_BeforeAnimationPlayTime += deltaTime * m_BeforeAnimationSpeed;

	if (m_CurrentAnimationPlayTime > m_CurrentAnimation->GetEndTime()) {
		
		// loop 일 때
		if (m_CurrentAnimation->GetLoop() == true)
		{
			m_CurrentAnimationPlayTime = 0;
		}
		else 
		{
			m_FinishPlaying = true;
			// change to nextAnimation;
			if (m_NextAnimationQueue.empty() == false)
				ChangeToNextAnimation();
		}

	}
	if (m_BeforeAnimation->GetLoop() == true && m_BeforeAnimationPlayTime > m_BeforeAnimation->GetEndTime()) m_BeforeAnimationPlayTime = 0;

}

void AnimationPlayer::ChangeToAnimation(std::shared_ptr<Animation> newAnim)
{

	if (m_CurrentAnimation && m_CurrentAnimation->GetLoop() == false) {
		m_NextAnimationQueue.push(newAnim);
		return;
	}

	for(int i = 0; i < m_NextAnimationQueue.size(); ++i)
		m_NextAnimationQueue.pop();
	
	if (newAnim->GetLoop()) m_FinishPlaying = false;

	m_BeforeAnimation = m_CurrentAnimation;
	m_BeforeAnimationPlayTime = m_CurrentAnimationPlayTime;
	m_BeforeAnimationSpeed = m_CurrentAnimationSpeed;

	// todo 새로운 정보를 받아옴
	m_BeforeAnimWeight = 1.0f;
	m_CurrentAnimation = newAnim;
	m_CurrentAnimationPlayTime = 0.0f;
	// todo default speed? 
	m_CurrentAnimationSpeed = 1.0f;
}

void AnimationPlayer::ChangeToAnimation(ANIMATION_SET animSet)
{
	if (m_AnimationMap.contains(animSet))
		ChangeToAnimation(m_AnimationMap[animSet]);
	else
		ERROR_QUIT(std::format("NO SUCH ANIMATION DEFINED!!, name: {}", ConvertAnimationSetToString(animSet)));
}

void AnimationPlayer::SetAnimationData(ComPtr<ID3D12GraphicsCommandList> commandList, ResourceManager* manager) const
{
	int firstAnimIdx = m_CurrentAnimation->GetDataIdx();
	int	secondAnimIdx = m_BeforeAnimation->GetDataIdx();
	float weight = m_BeforeAnimWeight;
	float firstAnimPlayTime = m_CurrentAnimationPlayTime;
	float secondAnimPlayTime = m_BeforeAnimationPlayTime;
	int firstAnimFrame = m_CurrentAnimation->GetEndFrame() + 1;
	int secondAnimFrame = m_BeforeAnimation->GetEndFrame() + 1;

	//D3D12_GPU_VIRTUAL_ADDRESS firstAnim = manager->GetResourceDataGPUAddress(RESOURCE_TYPES::SHADER, firstAnimIdx);
	//D3D12_GPU_VIRTUAL_ADDRESS secondAnim = manager->GetResourceDataGPUAddress(RESOURCE_TYPES::SHADER, secondAnimIdx);

	//commandList->SetGraphicsRootShaderResourceView(static_cast<int>(ROOT_SIGNATURE_IDX::ANIMATION_FIRST), firstAnim);
	//commandList->SetGraphicsRootShaderResourceView(static_cast<int>(ROOT_SIGNATURE_IDX::ANIMATION_SECOND), secondAnim);

	commandList->SetGraphicsRoot32BitConstants(static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT), 1, &firstAnimPlayTime, static_cast<int>(ANIM_ROOTCONST::ANI_1_PLAYTIME));
	commandList->SetGraphicsRoot32BitConstants(static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT), 1, &secondAnimPlayTime, static_cast<int>(ANIM_ROOTCONST::ANI_2_PLAYTIME));
	commandList->SetGraphicsRoot32BitConstants(static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT), 1, &weight, static_cast<int>(ANIM_ROOTCONST::ANI_BLEND));
	commandList->SetGraphicsRoot32BitConstants(static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT), 1, &firstAnimFrame, static_cast<int>(ANIM_ROOTCONST::ANI_1_FRAME));
	commandList->SetGraphicsRoot32BitConstants(static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT), 1, &secondAnimFrame, static_cast<int>(ANIM_ROOTCONST::ANI_2_FRAME));
	commandList->SetGraphicsRoot32BitConstants(static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT), 1, &firstAnimIdx, static_cast<int>(ANIM_ROOTCONST::ANI_1_INDEX));
	commandList->SetGraphicsRoot32BitConstants(static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT), 1, &secondAnimIdx, static_cast<int>(ANIM_ROOTCONST::ANI_2_INDEX));

	//DebugPrint(std::format("first anim frame: {}, weight: {}, playTime : {}", firstAnimFrame, weight, firstAnimPlayTime));
	//DebugPrint(std::format("secon anim frame: {}, weight: {}, playTime : {}", secondAnimFrame, weight, secondAnimPlayTime));
	//DebugPrint("");
}
