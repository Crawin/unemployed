#include "framework.h"
#include "Timer.h"

Timer::Timer()
{
	QueryPerformanceFrequency((LARGE_INTEGER*)&m_Frequency);
	m_Cycle = 1.0f / m_Frequency;
	Start();
}

Timer::~Timer()
{
}

void Timer::Start()
{
	QueryPerformanceCounter((LARGE_INTEGER*)&m_LastTime);
	m_CurrentTime = m_LastTime;
}

void Timer::Update()
{
	QueryPerformanceCounter((LARGE_INTEGER*)&m_CurrentTime);
	m_deltaTime = (float)(m_CurrentTime - m_LastTime) * m_Cycle;

	//DebugPrint(std::format("FPS: {}", 1.0f / m_deltaTime));

	// fps lock
	/*
	if (m_LockFPS > 0.0f) {
		float goalDt = 1.0f / m_LockFPS;
		while (m_deltaTime < 1.0f / goalDt) {
			QueryPerformanceCounter((LARGE_INTEGER*)&m_CurrentTime);
			m_deltaTime = (float)(m_CurrentTime - m_LastTime) * m_Cycle;
		}
	}
	*/

	m_TotalTime += m_deltaTime;

	m_LastTime = m_CurrentTime;
}

void Timer::Reset()
{
	Start();
	m_LastTime = 0;
	m_deltaTime = 0.0f;
}
