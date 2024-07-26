#include "framework.h"
#include "AnimationTrack.h"

void AnimationTrackBase::ResetAnimationTrack()
{
	// reset
}

AnimationTrackSingle::AnimationTrackSingle(std::shared_ptr<Animation> newAnim)
{
	m_Mode = 0;

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

void AnimationTrackSingle::SetAnimationData(ComPtr<ID3D12GraphicsCommandList> commandList, bool isCurrent)
{
	float animPlayTime = m_CurPlayTime;// std::min(m_CurPlayTime, m_MaxTime - 1.0f / 24.0f);
	if (animPlayTime >= m_MaxTime) animPlayTime = m_MaxTime - 1.0f / 24.0f;

	int animFrame = m_Animation->GetEndFrame() + 1;
	int animIndex = m_Animation->GetDataIdx();

	int targetParameter = isCurrent ? static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT) : static_cast<int>(ROOT_SIGNATURE_IDX::ANIMATION_EXTRA);

	float dummy = 1;

	commandList->SetGraphicsRoot32BitConstants(targetParameter, 1, &dummy, static_cast<int>(ANIM_ROOTCONST::SPACE_BLEND_WEIGHTS));
	commandList->SetGraphicsRoot32BitConstants(targetParameter, 1, &animFrame, static_cast<int>(ANIM_ROOTCONST::FRAMES));
	commandList->SetGraphicsRoot32BitConstants(targetParameter, 1, &animIndex, static_cast<int>(ANIM_ROOTCONST::INDICES));
	commandList->SetGraphicsRoot32BitConstants(targetParameter, 1, &animPlayTime, static_cast<int>(ANIM_ROOTCONST::PLAYTIME));
	commandList->SetGraphicsRoot32BitConstants(targetParameter, 1, &m_Mode, static_cast<int>(ANIM_ROOTCONST::MODE));

	int extra = static_cast<int>(ROOT_SIGNATURE_IDX::ANIMATION_EXTRA);

	int zero = 0;

	commandList->SetGraphicsRoot32BitConstants(extra, 1, &zero, static_cast<int>(ANIM_ROOTCONST::GO_AFFECT_UPPER));
	commandList->SetGraphicsRoot32BitConstants(extra, 1, &zero, static_cast<int>(ANIM_ROOTCONST::AFFECT_INDEX));

	if (isCurrent) {
		int goTrue = 0;

		if (m_AffectOnlyUpper) {
			goTrue = m_AffectIndexTo;
			//DebugPrint("GGG");
		}

		commandList->SetGraphicsRoot32BitConstants(extra, 1, &goTrue, static_cast<int>(ANIM_ROOTCONST::GO_AFFECT_UPPER));
		commandList->SetGraphicsRoot32BitConstants(extra, 1, &goTrue, static_cast<int>(ANIM_ROOTCONST::AFFECT_INDEX));


	}
}

XMMATRIX AnimationTrackSingle::GetAnimatedBoneMat(int boneIdx) const
{
	int endFrame = m_Animation->GetEndFrame();
	int animFrameRate = m_Animation->GetFrame();
	int animStride = endFrame + 1;

	float animPlayTime = m_CurPlayTime;// std::min(m_CurPlayTime, m_MaxTime - 1.0f / 24.0f);
	if (animPlayTime >= m_MaxTime) animPlayTime = m_MaxTime - 1.0f / 24.0f;

	int frameFloor = (int)floor(animPlayTime * animFrameRate) % animStride;
	int frameCeil = (frameFloor + 1) % animStride;

	float weightInterpol = ceil(animPlayTime * animFrameRate) - (animPlayTime * animFrameRate);

	int idx1 = boneIdx * animStride + frameFloor;
	int idx2 = boneIdx * animStride + frameCeil;

	XMFLOAT4X4 left =
	{
		-1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1
	};

	XMMATRIX convertLeft = XMLoadFloat4x4(&left);
	XMMATRIX anim1 = XMLoadFloat4x4(&m_Animation->GetBone(idx1));
	XMMATRIX anim2 = XMLoadFloat4x4(&m_Animation->GetBone(idx2));

	XMMATRIX result = anim1 * weightInterpol + anim2 * (1.0f - weightInterpol);

	result = result * convertLeft;
	//printf("frame: %3d, idx1: %d. idx2: %d \t weight: %.1f\n", currentFrame, idx, idx + 1, interpolWeight);
		//lerp(Animation_cur[idx1 + 1], Animation_cur[idx1], interpolWeight));
	return XMMatrixTranspose(result);
}

XMMATRIX AnimationTrackBlendingSpace2D::EvaluateFromAnimation(int boneIdx, int point) const
{
	auto& animation = m_AnimationSpace[point].second;
	int endFrame = animation->GetEndFrame();
	int animFrameRate = animation->GetFrame();
	int animStride = endFrame + 1;

	//float playTime = m_CurPlayTime;
	//playTime /= m_AnimationSpace[m_CurrentSuperiorPoint].second->GetEndTime();
	//playTime *= m_AnimationSpace[point].second->GetEndTime();

	int frameFloor = (int)floor(m_CurPlayTime * animFrameRate) % animStride;
	int frameCeil = (frameFloor + 1) % animStride;

	float weightInterpol = ceil(m_CurPlayTime * animFrameRate) - (m_CurPlayTime * animFrameRate);

	int idx1 = boneIdx * animStride + frameFloor;
	int idx2 = boneIdx * animStride + frameCeil;

	XMFLOAT4X4 left =
	{
		-1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1
	};

	XMMATRIX convertLeft = XMLoadFloat4x4(&left);
	XMMATRIX anim1 = XMLoadFloat4x4(&animation->GetBone(idx1));
	XMMATRIX anim2 = XMLoadFloat4x4(&animation->GetBone(idx2));

	XMMATRIX result = anim1 * weightInterpol + anim2 * (1.0f - weightInterpol);

	result = result * convertLeft;
	//printf("frame: %3d, idx1: %d. idx2: %d \t weight: %.1f\n", currentFrame, idx, idx + 1, interpolWeight);
		//lerp(Animation_cur[idx1 + 1], Animation_cur[idx1], interpolWeight));
	return XMMatrixTranspose(result);
}

int AnimationTrackBlendingSpace2D::GetMaxIndex(const XMFLOAT4 vec4)
{
	float maxWeight = vec4.x;
	int maxIndex = 0;

	if (vec4.y > maxWeight) {
		maxWeight = vec4.y;
		maxIndex = 1;
	}
	if (vec4.z > maxWeight) {
		maxWeight = vec4.z;
		maxIndex = 2;
	}
	if (vec4.w > maxWeight) {
		maxWeight = vec4.w;
		maxIndex = 3;
	}

	return maxIndex;
}

AnimationTrackBlendingSpace2D::AnimationTrackBlendingSpace2D()
{
	m_Mode = 2;
}

void AnimationTrackBlendingSpace2D::UpdateTime(float deltaTime)
{
	// todo
	// 1. blend weight가 가장 큰 animation을 찾는다.
	// 2. 바뀔 animation에 맞게 시간(m_CurPlayTime)을 수정한다.
	// 3. beforeAnim = 1번의 animation으로 바꾼다
	// 시간 ++
	m_CurPlayTime += deltaTime * m_AnimSpeed;// / 2.0f;

	int animIdx = m_ClosePoints[m_CurrentSuperiorPoint];
	float endTime = m_AnimationSpace[animIdx].second->GetEndTime();
	//if (m_CurPlayTime >= endTime) m_CurPlayTime = 0.0f;

	int maxIdx = GetMaxIndex(m_BlendingWeights);

	if (maxIdx != m_CurrentSuperiorPoint) {
		//m_CurPlayTime = 0;
		//DebugPrint(std::format("idx: {}/beftime: {}", m_CurrentSuperiorPoint, m_CurPlayTime));
		//m_CurPlayTime /= m_AnimationSpace[m_CurrentSuperiorPoint].second->GetEndTime();
		//m_CurPlayTime *= m_AnimationSpace[maxIdx].second->GetEndTime();
		//m_CurrentSuperiorPoint = maxIdx;
		//DebugPrint(std::format("idx: {}/aftTime: {}\n", m_CurrentSuperiorPoint, m_CurPlayTime));
	}
	

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

	if (m_CurrentPoint.m_Angle >= 360.0f) m_CurrentPoint.m_Angle -= 360.0f;



	// find animations by
	// changed x,y
	Point2D leftBottom = { -FLT_MAX, -FLT_MAX };
	Point2D leftTop = { -FLT_MAX, FLT_MAX };
	Point2D rightBottom = { FLT_MAX, -FLT_MAX };
	Point2D rightTop = { FLT_MAX, FLT_MAX };
	int startIdx = 0;

	m_ClosePoints[0] = -1;
	m_ClosePoints[1] = -1;
	m_ClosePoints[2] = -1;
	m_ClosePoints[3] = -1;

	// left bottom
	for (int i = m_AnimationSpace.size() - 1; i >= 0; --i) {
		const auto& point = m_AnimationSpace[i].first;

		startIdx = i;
		m_ClosePoints[0] = i;
		leftBottom = point;
		if (m_CurrentPoint.m_Angle >= point.m_Angle &&
			m_CurrentPoint.m_Speed >= point.m_Speed)
			break;
	}

	// left top(go up)
	const auto& point1 = m_AnimationSpace[m_ClosePoints[0] + 1].first;

	if (point1.m_Angle > leftBottom.m_Angle) {
		// its on high speed
		m_ClosePoints[1] = m_ClosePoints[0];
		leftTop = leftBottom;
	}
	else {
		// can step
		m_ClosePoints[1] = m_ClosePoints[0] + 1;
		leftTop = point1;
	}

	// right top
	for (int i = 0; i < m_AnimationSpace.size(); ++i) {
		const auto& point = m_AnimationSpace[i].first;

		m_ClosePoints[3] = i;
		rightTop = point;
		if (leftTop.m_Angle < point.m_Angle &&
			leftTop.m_Speed <= point.m_Speed)
			break;
	}

	// right bottom(go down)
	const auto& point2 = m_AnimationSpace[m_ClosePoints[3] - 1].first;

	if (point2.m_Angle < rightTop.m_Angle || leftBottom.m_Speed > point2.m_Speed) {
		// its on low speed
		m_ClosePoints[2] = m_ClosePoints[3];
		rightBottom = rightTop;
	}
	else {
		// can step
		m_ClosePoints[2] = m_ClosePoints[3] - 1;
		rightBottom = point2;
	}


	float xBlend = 1.0f - std::clamp((rightBottom.m_Angle - m_CurrentPoint.m_Angle) / (rightBottom.m_Angle - leftBottom.m_Angle), 0.0f, 1.0f);
	float yBlend = 1.0f - std::clamp((leftTop.m_Speed - m_CurrentPoint.m_Speed) / (leftTop.m_Speed - leftBottom.m_Speed), 0.0f, 1.0f);

	m_BlendingWeights = {
		(1.0f - xBlend) * (1.0f - yBlend),
		(1.0f - xBlend) * (yBlend),
		(xBlend) * (1.0f - yBlend),
		(xBlend) * (yBlend)
	};

	//if (m_CurrentPoint.m_Angle != 0 || m_CurrentPoint.m_Speed > 0.5f) {
	//	DebugPrint(std::format("Angle: {}, Speed: {}", m_CurrentPoint.m_Angle, m_CurrentPoint.m_Speed));
	//	printf("%d, %d, %d, %d\n", m_ClosePoints[0], m_ClosePoints[1], m_ClosePoints[2], m_ClosePoints[3]);
	//	printf("%.2f, %.2f, %.2f, %.2f\n", m_BlendingWeights.x, m_BlendingWeights.y, m_BlendingWeights.z, m_BlendingWeights.w);
	//	printf("\n");
	//}

	UpdateTime(deltaTime);
}

void AnimationTrackBlendingSpace2D::ResetAnimationTrack()
{
	m_CurPlayTime = 0;
}

void AnimationTrackBlendingSpace2D::SetAnimationData(ComPtr<ID3D12GraphicsCommandList> commandList, bool isCurrent)
{
	//float animPlayTime = m_CurPlayTime;// std::min(m_CurPlayTime, m_MaxTime - 1.0f / 24.0f);
	
	XMUINT4 frames{
		static_cast<uint32_t>(m_AnimationSpace[m_ClosePoints[0]].second->GetEndFrame() + 1),
		static_cast<uint32_t>(m_AnimationSpace[m_ClosePoints[1]].second->GetEndFrame() + 1),
		static_cast<uint32_t>(m_AnimationSpace[m_ClosePoints[2]].second->GetEndFrame() + 1),
		static_cast<uint32_t>(m_AnimationSpace[m_ClosePoints[3]].second->GetEndFrame() + 1)
	};

	XMUINT4 indices{
		static_cast<uint32_t>(m_AnimationSpace[m_ClosePoints[0]].second->GetDataIdx()),
		static_cast<uint32_t>(m_AnimationSpace[m_ClosePoints[1]].second->GetDataIdx()),
		static_cast<uint32_t>(m_AnimationSpace[m_ClosePoints[2]].second->GetDataIdx()),
		static_cast<uint32_t>(m_AnimationSpace[m_ClosePoints[3]].second->GetDataIdx())
	};

	int targetParameter = isCurrent ? static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT) : static_cast<int>(ROOT_SIGNATURE_IDX::ANIMATION_EXTRA);

	commandList->SetGraphicsRoot32BitConstants(targetParameter, 4, &m_BlendingWeights, static_cast<int>(ANIM_ROOTCONST::SPACE_BLEND_WEIGHTS));
	commandList->SetGraphicsRoot32BitConstants(targetParameter, 4, &frames, static_cast<int>(ANIM_ROOTCONST::FRAMES));
	commandList->SetGraphicsRoot32BitConstants(targetParameter, 4, &indices, static_cast<int>(ANIM_ROOTCONST::INDICES));
	commandList->SetGraphicsRoot32BitConstants(targetParameter, 1, &m_CurPlayTime, static_cast<int>(ANIM_ROOTCONST::PLAYTIME));
	commandList->SetGraphicsRoot32BitConstants(targetParameter, 1, &m_Mode, static_cast<int>(ANIM_ROOTCONST::MODE));
}

XMMATRIX AnimationTrackBlendingSpace2D::GetAnimatedBoneMat(int boneIdx) const
{
	XMMATRIX res{
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	};

	res += EvaluateFromAnimation(boneIdx, m_ClosePoints[0]) * m_BlendingWeights.x;
	res += EvaluateFromAnimation(boneIdx, m_ClosePoints[1]) * m_BlendingWeights.y;
	res += EvaluateFromAnimation(boneIdx, m_ClosePoints[2]) * m_BlendingWeights.z;
	res += EvaluateFromAnimation(boneIdx, m_ClosePoints[3]) * m_BlendingWeights.w;

	return res;
}

void AnimationTrackBlendingSpace2D::InsertAnimation(float atAngle, float atSpeed, std::shared_ptr<Animation> anim)
{
	m_AnimationSpace.emplace_back(Point2D(atAngle, atSpeed), anim);

	// todo 추후에 나중에 정렬하게 하자
	std::sort(m_AnimationSpace.begin(), m_AnimationSpace.end(), [](const auto& a, const auto& b){
		if (a.first.m_Angle == b.first.m_Angle)
			return a.first.m_Speed < b.first.m_Speed;

		return a.first.m_Angle < b.first.m_Angle;
		});

}
