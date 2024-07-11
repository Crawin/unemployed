#pragma once



template <typename T>
struct KeyFrame {
	T m_Value;
	float m_Time;
};

class ITimeLine {
protected:
	float m_CurrentTime = 0;
	bool m_Playing = true;
	bool m_PlayBack = false;

public:
	virtual void Update(float deltaTime) = 0;
	virtual void SyncData() = 0;
	void Play(float startFrom = 0) { m_Playing = true; m_CurrentTime = startFrom; }
	void Stop() { m_Playing = false; };

	float GetTime() { return m_CurrentTime; }
	bool IsPlaying() { return m_Playing; }
	bool IsBack() { return m_PlayBack; }
	void SetMoveFront() { m_PlayBack = false; }
	void SetMoveBack() { m_PlayBack = true; }
};

template <typename T>
class TimeLine : public ITimeLine {
	std::vector<KeyFrame<T>> m_KeyFrames;
	T m_CurrentValue;
	T* m_TargetValue = nullptr;

	//std::function<T(const T&, const T&, float)> m_LerpFunction;
public:

	TimeLine(T* target) : m_TargetValue{ target } {};

	void AddKeyFrame(T value, float time) {
		m_KeyFrames.emplace_back(value, time);
		std::sort(m_KeyFrames.begin(), m_KeyFrames.end(), [](const KeyFrame<T>& a, const KeyFrame<T>& b) { return a.m_Time < b.m_Time; });
	}

	virtual void Update(float deltaTime) {
		if (!m_Playing || m_KeyFrames.empty()) return;

		m_CurrentTime += deltaTime;

		if (m_CurrentTime >= m_KeyFrames.back().m_Time) {
			Stop();
			m_CurrentValue = m_KeyFrames.back().m_Value;
			return;
		}

		for (size_t i = 0; i < m_KeyFrames.size() - 1; ++i) {
			if (m_CurrentTime >= m_KeyFrames[i].m_Time && m_CurrentTime <= m_KeyFrames[i + 1].m_Time) {
				float t = (m_CurrentTime - m_KeyFrames[i].m_Time) / (m_KeyFrames[i + 1].m_Time - m_KeyFrames[i].m_Time);
				m_CurrentValue = LerpValue(m_KeyFrames[i].m_Value, m_KeyFrames[i + 1].m_Value, t);
				break;
			}
		}
	}
	virtual void SyncData() { (*m_TargetValue) = m_CurrentValue; }
};



