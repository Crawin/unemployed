#pragma once

#pragma comment(lib,"fmod_vc.lib")
#pragma comment(lib,"fmodL_vc.lib")
#include <../External/Include/FMOD/fmod.hpp>
#include <../External/Include/FMOD/fmod_errors.h>


struct SOUND_EFFECT
{
    FMOD::Sound* sound;
    FMOD::Channel* channel = 0;
};

// 서버에서도 사용하기에 packets로 이동
enum class SOUND_TYPE;

class FMOD_INFO
{
public:
    static FMOD_INFO& GetInstance() {
        static FMOD_INFO inst;
        return inst;
    }
private:
    bool client_on = true;

    DirectX::XMFLOAT3 m_guard_position;

    DirectX::XMFLOAT3 m_player1_position;
    DirectX::XMFLOAT3 m_player1_rotation;

    bool m_before_self_running;
    bool m_before_guard_running;

    float m_self_speed;
    float m_guard_speed;

private:
    FMOD::System* m_system;

    std::unordered_map<SOUND_TYPE, FMOD::Sound*> SOUNDS;
    std::unordered_map<std::string, FMOD::Channel*> CHANNELS;
public:
    void set_self_position(const DirectX::XMFLOAT3& position) { m_player1_position = position; }
    void set_guard_position(const DirectX::XMFLOAT3& position) { m_guard_position = position; }

    void set_client_on(const bool& b) { client_on = b; }
    inline bool is_client_on() { return client_on; }
    
    void set_self_speed(const float& s) { m_self_speed = s; }
    void set_guard_speed(const float& s) { m_guard_speed = s; }

    void set_player1_rotation_x(const float& x) { m_player1_rotation.x = x; }
    void set_player1_rotation_y(const float& y) { m_player1_rotation.y = y; }
    void set_player1_rotation_z(const float& z) { m_player1_rotation.z = z; }

    FMOD_VECTOR get_direction_vector(const float& x, const float& y, const float& z);
public:
    void begin_fmod();
    void update_fmod();
    void end_fmod();
    bool play_loop_sound(const DirectX::XMFLOAT3& WorldPos, const SOUND_TYPE& sound, const std::string& channel, float pitch = 1.0f);
    bool play_unloop_sound(const DirectX::XMFLOAT3& WorldPos, const SOUND_TYPE& sound, const std::string& channel);
    bool stop_sound(const std::string& channel);
};

void ERRCHECK(FMOD_RESULT result);