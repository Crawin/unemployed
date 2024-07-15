﻿#pragma once

#pragma comment(lib,"fmod_vc.lib")
#pragma comment(lib,"fmodL_vc.lib")
#include <../External/Include/FMOD/fmod.hpp>
#include <../External/Include/FMOD/fmod_errors.h>

struct SOUND_EFFECT
{
    FMOD::Sound* sound;
    FMOD::Channel* channel = 0;
};

class FMOD_INFO
{
public:
    static FMOD_INFO& GetInstance() {
        static FMOD_INFO inst;
        return inst;
    }
private:
    bool client_on = true;
    std::atomic_bool m_self_running;
    DirectX::XMFLOAT3 m_guard_position;
    DirectX::XMFLOAT3 m_player1_position;
    DirectX::XMFLOAT3 m_player1_rotation;
public:
    inline DirectX::XMFLOAT3 get_player1_position() { return m_player1_position; }
    inline DirectX::XMFLOAT3 get_player1_rotation() { return m_player1_rotation; }
    inline DirectX::XMFLOAT3 get_guard_position() { return m_guard_position; }
    
    void set_guard_position(const DirectX::XMFLOAT3& position);
    void set_running(const bool& b) { m_self_running = b; }
    inline bool is_running() { return m_self_running; }

    void set_client_on(const bool& b) { client_on = b; }
    inline bool is_client_on() { return client_on; }
};

void ERRCHECK(FMOD_RESULT result);
int start_fmod(FMOD_INFO& info);