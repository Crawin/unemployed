#pragma once

class Timer
{
public:
	Timer();
	~Timer();

	void Start();
	void Update();

	void Reset();

	void SetFPSLock(float fps) { m_LockFPS = fps; }

	float GetDeltaTime() const { return m_deltaTime; }
	float GetTotalTime() const { return m_TotalTime; }

private:
	float m_LockFPS = 0.0f;

	float m_deltaTime = 0.0f;
	float m_TotalTime = 0.0f;

	float m_Cycle = 0.0f;

	__int64 m_Frequency;
	__int64 m_LastTime;
	__int64 m_CurrentTime;

};

