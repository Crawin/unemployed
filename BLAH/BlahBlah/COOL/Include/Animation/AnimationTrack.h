#pragma once
#include "Animation.h"

/////////////////////////////////////////////////////////
// Animation Track Base
//
class AnimationTrackBase
{
protected:
	std::function<void(float, void*)> m_UpdateFunction;

	bool m_AffectOnlyUpper = false;

	int m_Mode = -1;

	float m_CurPlayTime = 0.0f;
	float m_MaxTime = 0.0f;
	float m_AnimSpeed = 1.0f;
	bool m_Loop = false;

public:
	void SetUpdateFunction(const std::function<void(float, void*)>& func) { m_UpdateFunction = func; }

	float GetCurrentPlayTime() const { return m_CurPlayTime; }
	float GetMaxPlaytime() const { return m_MaxTime; }

	virtual void Update(float deltaTime) = 0;
	virtual void ResetAnimationTrack() = 0;
	virtual void SetAnimationData(ComPtr<ID3D12GraphicsCommandList> commandList, bool isCurrent) = 0;
	virtual XMMATRIX GetAnimatedBoneMat(int boneIdx) const = 0;

	bool GetAffectUpperState() const { return m_AffectOnlyUpper; }
	void SetAffactUpper(bool state) { m_AffectOnlyUpper = state; }
};

/////////////////////////////////////////////////////////
// Animation Track Single
// single animation
class AnimationTrackSingle : public AnimationTrackBase {
	std::shared_ptr<Animation> m_Animation;

	int m_AffectIndexTo = 48;

public:
	AnimationTrackSingle(std::shared_ptr<Animation> newAnim);

	virtual void Update(float deltaTime);
	virtual void ResetAnimationTrack();
	virtual void SetAnimationData(ComPtr<ID3D12GraphicsCommandList> commandList, bool isCurrent);
	virtual XMMATRIX GetAnimatedBoneMat(int boneIdx) const;
};

struct Point2D {
	float m_Angle;
	float m_Speed;

	Point2D(float angle, float speed) : m_Angle{ angle }, m_Speed{ speed } {}
};

/////////////////////////////////////////////////////////
// Animation Track Blending Space
// multiple animations, blend anims
class AnimationTrackBlendingSpace2D : public AnimationTrackBase {
	Point2D m_CurrentPoint = { 0,0 };
	std::vector<std::pair<Point2D, std::shared_ptr<Animation>>> m_AnimationSpace;
	int m_ClosePoints[4] = { 0,0,0,0 };			// left right bottom top
	XMFLOAT4 m_BlendingWeights{ 1,0,0,0 };

	int m_CurrentSuperiorPoint = 0;

	XMMATRIX EvaluateFromAnimation(int boneIdx, int point) const;


	int GetMaxIndex(const XMFLOAT4 vec4);

public:
	AnimationTrackBlendingSpace2D();

	void UpdateTime(float deltaTime);

	virtual void Update(float deltaTime);
	virtual void ResetAnimationTrack();
	virtual void SetAnimationData(ComPtr<ID3D12GraphicsCommandList> commandList, bool isCurrent);
	virtual XMMATRIX GetAnimatedBoneMat(int boneIdx) const;


	void InsertAnimation(float atAngle, float atSpeed, std::shared_ptr<Animation> anim);

};

