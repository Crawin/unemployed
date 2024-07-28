#include "framework.h"
#include <iostream>
#include <unordered_map>
#include <string>
#include "FmodSound.h"
#include "Network/Client.h"

void ERRCHECK(FMOD_RESULT result)
{
    if (result != FMOD_OK)
    {
        std::cout << "FMOD error! (" << result << ") " << FMOD_ErrorString(result) << std::endl;
    }
}


//int start_fmod(FMOD_INFO& info)
//{
//    FMOD::System* system;
//  /*  FMOD::Sound* duck;
//    FMOD::Channel* channel2 = 0;*/
//
//    SOUND_EFFECT guard_foot;
//    SOUND_EFFECT self_foot;
//
//    FMOD_RESULT result;
//
//    // FMOD 시스템 생성
//    result = FMOD::System_Create(&system);
//    ERRCHECK(result);
//
//    // 시스템 초기화
//    result = system->init(512, FMOD_INIT_NORMAL, 0);
//    ERRCHECK(result);
//
//    result = system->set3DSettings(1.0f, 0.01f, 1.0f);
//    ERRCHECK(result);
//
//    // 사운드 로드 (3D 모드)
//    result = system->createSound("SceneData/Sound/run.wav", FMOD_3D | FMOD_LOOP_NORMAL, 0, &guard_foot.sound);
//    ERRCHECK(result);
//
//    result = guard_foot.sound->set3DMinMaxDistance(100.0f, 1000.0f);
//    ERRCHECK(result);
//
//    //result = system->createSound("SceneData/Sound/ducktest.wav", FMOD_3D, 0, &duck);
//    //ERRCHECK(result);
//
//    //result = system->playSound(duck, 0, false, &channel2);
//    //ERRCHECK(result);
//
//    // 청취자 위치 설정
//    DirectX::XMFLOAT3 player1_pos = info.get_player1_position();
//    DirectX::XMFLOAT3 player1_rot = info.get_player1_rotation();
//    FMOD_VECTOR listenerPos = { player1_pos.x,player1_pos.y,player1_pos.z };
//    FMOD_VECTOR listenerVel = { 0.0f, 0.0f, 0.0f };
//    FMOD_VECTOR listenerForward = { 0.0f, 0.0f, -1.0f };
//    FMOD_VECTOR listenerUp = { 0.0f, 1.0f, 0.0f };
//
//    result = system->set3DListenerAttributes(0, &listenerPos, &listenerVel, &listenerForward, &listenerUp);
//    ERRCHECK(result);
//
//    // 사운드 위치 설정
//    DirectX::XMFLOAT3 guard_pos = info.get_guard_position();
//    FMOD_VECTOR soundPos = { guard_pos.x,guard_pos.y,guard_pos.z };
//    FMOD_VECTOR soundVel = { 0.0f, 0.0f, 0.0f };
//
//    result = guard_foot.channel->set3DAttributes(&soundPos, &soundVel);
//    ERRCHECK(result);
//
//    // 볼륨 설정
//    result = guard_foot.channel->setVolume(0.1);
//    ERRCHECK(result);
//
//    // 사운드 재생
//    //result = system->playSound(guard_foot.sound, 0, false, &guard_foot.channel);
//    //ERRCHECK(result);
//
//    bool self_running = false;
//    bool guard_running = false;
//
//    result = system->createSound("SceneData/Sound/run.wav", FMOD_3D | FMOD_LOOP_NORMAL, 0, &self_foot.sound);
//    ERRCHECK(result);
//    
//    result = self_foot.channel->setVolume(0.1);
//    ERRCHECK(result);
//
//    // 메인 루프
//    while (info.is_client_on())
//    {
//        // 여기서 다른 업데이트 로직 추가 가능
//        // 현재 내 위치 업데이트
//        player1_pos = info.get_player1_position();
//        listenerPos = { player1_pos.x,player1_pos.y,player1_pos.z };
//        player1_rot = info.get_player1_rotation();
//        //listenerForward = {};
//        result = system->set3DListenerAttributes(0, &listenerPos, &listenerVel, &listenerForward, &listenerUp);
//        ERRCHECK(result);
//
//        // 현재 Guard 위치 업데이트
//        guard_pos = info.get_guard_position();
//        soundPos = { guard_pos.x,guard_pos.y,guard_pos.z };
//        result = guard_foot.channel->set3DAttributes(&soundPos, &soundVel);
//        ERRCHECK(result);
//
//        // 가드 발소리
//        if (info.guard_running())  // 클라에서 shift를 눌러 달리는 상태이면
//        {
//            if (guard_running == false)          // 달리지 않다가 달리기가 시작되면
//            {
//                guard_running = true;
//                result = system->playSound(guard_foot.sound, 0, false, &guard_foot.channel);
//                ERRCHECK(result);
//            }
//        }
//        else
//        {                       // 걷고 있는 상태면
//            if (guard_running == true)
//            {
//                guard_running = false;
//                result = guard_foot.channel->stop();
//                ERRCHECK(result);
//            }
//        }
//
//        // 나 자신의 발소리
//        if (info.is_running())  // 클라에서 shift를 눌러 달리는 상태이면
//        {
//            if (self_running == false)          // 달리지 않다가 달리기가 시작되면
//            {
//                self_running = true;
//                result = system->playSound(self_foot.sound, 0, false, &self_foot.channel);
//                ERRCHECK(result);
//            }
//        }
//        else
//        {                       // 걷고 있는 상태면
//            if (self_running == true)
//            {
//                self_running = false;
//                result = self_foot.channel->stop();
//                ERRCHECK(result);
//            }
//        }
//
//        // 시스템 업데이트
//        result = system->update();
//        ERRCHECK(result);
//    }
//
//    // 클린업
//    result = guard_foot.sound->release();
//    ERRCHECK(result);
//
//    result = system->close();
//    ERRCHECK(result);
//
//    result = system->release();
//    ERRCHECK(result);
//
//    return 0;
//}

FMOD_VECTOR FMOD_INFO::get_direction_vector(const float& x, const float& y, const float& z)
{
    //XMVECTOR v = XMVectorSet(x, y, z, 0.0f);
    //float x1 = XMConvertToRadians(m_player1_rotation.x);
    //float y1 = XMConvertToRadians(m_player1_rotation.y);
    //printf("y값: %f\n", y1);
    //XMMATRIX rotX = XMMatrixRotationX(x1);
    //XMMATRIX rotY = XMMatrixRotationY(y1);
    //XMMATRIX rotationMatrix = XMMatrixMultiply(rotY, rotX);
    //XMVECTOR result = XMVector3Transform(v, rotationMatrix);
    //result = XMVector3Normalize(result);
    //XMFLOAT3 resultFloat3;
    //XMStoreFloat3(&resultFloat3, result);
    //FMOD_VECTOR out = { resultFloat3.x,resultFloat3.y,resultFloat3.z };
    //return out;
    XMVECTOR v = XMVectorSet(x, y, z, 0.0f);
    float x1 = XMConvertToRadians(m_player1_rotation.x);
    float y1 = XMConvertToRadians(m_player1_rotation.y);
    printf("y값: %f\n", y1);
    XMMATRIX rotMat = XMMatrixRotationRollPitchYaw(x1, y1, 0.0f);

    XMVECTOR result = XMVector3Transform(v, rotMat);
    result = XMVector3Normalize(result);
    XMFLOAT3 resultFloat3;
    XMStoreFloat3(&resultFloat3, result);
    FMOD_VECTOR out = { resultFloat3.x,resultFloat3.y,resultFloat3.z };
    return out;
}

void FMOD_INFO::begin_fmod()
{
    FMOD_RESULT result;

    // FMOD 시스템 생성
    result = FMOD::System_Create(&m_system);
    ERRCHECK(result);

    // 시스템 초기화
    result = m_system->init(512, FMOD_INIT_NORMAL, 0);
    ERRCHECK(result);

    result = m_system->set3DSettings(1.0f, 0.01f, 1.0f);
    //result = m_system->set3DSettings(1.0f, 1.0f, 1.0f);

    ERRCHECK(result);

    // 사운드 로드 (3D 모드)
    auto a = SOUNDS.try_emplace(SOUND_TYPE::FOOTPRINT);
    if (a.second)
    {
        result = m_system->createSound("SceneData/Sound/walking.wav", FMOD_3D, 0, &SOUNDS[SOUND_TYPE::FOOTPRINT]);
        ERRCHECK(result);
        result = SOUNDS[SOUND_TYPE::FOOTPRINT]->set3DMinMaxDistance(10.0f, 600.0f);
        //result = SOUNDS[SOUND_TYPE::FOOTPRINT]->set3DMinMaxDistance(0.1f, 4.0f);
        ERRCHECK(result);
        result =  SOUNDS[SOUND_TYPE::FOOTPRINT]->setMode(FMOD_3D_LINEARROLLOFF);
        ERRCHECK(result);
    }

    a = SOUNDS.try_emplace(SOUND_TYPE::DUCK);
    if (a.second)
    {
        result = m_system->createSound("SceneData/Sound/ducktest.wav", FMOD_3D, 0, &SOUNDS[SOUND_TYPE::DUCK]);
        ERRCHECK(result);
        result = SOUNDS[SOUND_TYPE::DUCK]->setMode(FMOD_3D_LINEARROLLOFF);
        ERRCHECK(result);
    }

    a = SOUNDS.try_emplace(SOUND_TYPE::CROWBAR_SWING);
    if (a.second)
    {
        result = m_system->createSound("SceneData/Sound/crowbar_swing.wav", FMOD_3D, 0, &SOUNDS[SOUND_TYPE::CROWBAR_SWING]);
        ERRCHECK(result);
        result = SOUNDS[SOUND_TYPE::CROWBAR_SWING]->set3DMinMaxDistance(10.0f, 400.0f);
        ERRCHECK(result);
        result = SOUNDS[SOUND_TYPE::CROWBAR_SWING]->setMode(FMOD_3D_LINEARROLLOFF);
        ERRCHECK(result);
    }

    a = SOUNDS.try_emplace(SOUND_TYPE::CROWBAR_HIT);
    if (a.second)
    {
        result = m_system->createSound("SceneData/Sound/crowbar_hit.wav", FMOD_3D, 0, &SOUNDS[SOUND_TYPE::CROWBAR_HIT]);
        ERRCHECK(result);
        //result = SOUNDS[SOUND_TYPE::CROWBAR_HIT]->set3DMinMaxDistance(10.0f, 100.0f);
        //ERRCHECK(result);
        result = SOUNDS[SOUND_TYPE::CROWBAR_HIT]->set3DMinMaxDistance(10.0f, 1500.0f);
        ERRCHECK(result);
        result = SOUNDS[SOUND_TYPE::CROWBAR_HIT]->setMode(FMOD_3D_LINEARROLLOFF);
        ERRCHECK(result);
    }

    a = SOUNDS.try_emplace(SOUND_TYPE::DOOR_OPEN);
    if (a.second)
    {
        result = m_system->createSound("SceneData/Sound/door_open.wav", FMOD_3D, 0, &SOUNDS[SOUND_TYPE::DOOR_OPEN]);
        ERRCHECK(result);
        result = SOUNDS[SOUND_TYPE::DOOR_OPEN]->set3DMinMaxDistance(10.0f, 300.0f);
        ERRCHECK(result);
        result = SOUNDS[SOUND_TYPE::DOOR_OPEN]->setMode(FMOD_3D_LINEARROLLOFF);
        ERRCHECK(result);
    }

    a = SOUNDS.try_emplace(SOUND_TYPE::DRINK_BUY);
    if (a.second)
    {
        result = m_system->createSound("SceneData/Sound/vending_machine_drop.wav", FMOD_3D, 0, &SOUNDS[SOUND_TYPE::DRINK_BUY]);
        ERRCHECK(result);
        result = SOUNDS[SOUND_TYPE::DRINK_BUY]->set3DMinMaxDistance(10.0f, 1000.0f);
        ERRCHECK(result);
        result = SOUNDS[SOUND_TYPE::DRINK_BUY]->setMode(FMOD_3D_LINEARROLLOFF);
        ERRCHECK(result);
    }

    a = SOUNDS.try_emplace(SOUND_TYPE::DRINK_THROW_HIT);
    if (a.second)
    {
        result = m_system->createSound("SceneData/Sound/can_drop.wav", FMOD_3D, 0, &SOUNDS[SOUND_TYPE::DRINK_THROW_HIT]);
        ERRCHECK(result);
        result = SOUNDS[SOUND_TYPE::DRINK_THROW_HIT]->set3DMinMaxDistance(10.0f, 1000.0f);
        ERRCHECK(result);
        result = SOUNDS[SOUND_TYPE::DRINK_THROW_HIT]->setMode(FMOD_3D_LINEARROLLOFF);
        ERRCHECK(result);
    }

    a = SOUNDS.try_emplace(SOUND_TYPE::DRINK_CONSUME);
    if (a.second)
    {
        result = m_system->createSound("SceneData/Sound/can_drink.wav", FMOD_3D, 0, &SOUNDS[SOUND_TYPE::DRINK_CONSUME]);
        ERRCHECK(result);
        result = SOUNDS[SOUND_TYPE::DRINK_CONSUME]->set3DMinMaxDistance(10.0f, 300.0f);
        ERRCHECK(result);
        result = SOUNDS[SOUND_TYPE::DRINK_CONSUME]->setMode(FMOD_3D_LINEARROLLOFF);
        ERRCHECK(result);
    }

    a = SOUNDS.try_emplace(SOUND_TYPE::KEY_JINGLE);
    if (a.second)
    {
        result = m_system->createSound("SceneData/Sound/key_jingle.wav", FMOD_3D, 0, &SOUNDS[SOUND_TYPE::KEY_JINGLE]);
        ERRCHECK(result);
        result = SOUNDS[SOUND_TYPE::KEY_JINGLE]->set3DMinMaxDistance(10.0f, 500.0f);
        ERRCHECK(result);
        result = SOUNDS[SOUND_TYPE::KEY_JINGLE]->setMode(FMOD_3D_LINEARROLLOFF);
        ERRCHECK(result);
    }


    //// 청취자 위치 설정
    //DirectX::XMFLOAT3 player1_pos = m_player1_position;
    //DirectX::XMFLOAT3 player1_rot = m_player1_rotation;
    //FMOD_VECTOR listenerPos = { player1_pos.x,player1_pos.y,player1_pos.z };
    //FMOD_VECTOR listenerVel = { 0.0f, 0.0f, 0.0f };
    //FMOD_VECTOR listenerForward = { 0.0f, 0.0f, -1.0f };
    //FMOD_VECTOR listenerUp = { 0.0f, 1.0f, 0.0f };

    //result = m_system->set3DListenerAttributes(0, &listenerPos, &listenerVel, &listenerForward, &listenerUp);
    //ERRCHECK(result);

    bool self_running = false;
    bool guard_running = false;
}

void FMOD_INFO::update_fmod()
{
    FMOD_RESULT result;
// 현재 내 위치 업데이트
    FMOD_VECTOR listenerPos = { m_player1_position.x,m_player1_position.y,m_player1_position.z };
    FMOD_VECTOR listenerVel = { 0.0f, 0.0f, 0.0f };
    FMOD_VECTOR listenerForward = get_direction_vector(0, 0, 1);
    FMOD_VECTOR listenerUp = get_direction_vector(0, 1, 0);

    //printf("%f,%f,%f\n", m_player1_position.x, m_player1_position.y, m_player1_position.z);

    result = m_system->set3DListenerAttributes(0, &listenerPos, &listenerVel, &listenerForward, &listenerUp);
    ERRCHECK(result);

    bool isPlaying = false;
    for (auto it = CHANNELS.begin(); it != CHANNELS.end();)
    {
        it->second->isPlaying(&isPlaying);
        if (!isPlaying)
        {
            it = CHANNELS.erase(it);
        }
        else
        {
            //float vol;
            //it->second->getVolume(&vol);
            //std::cout << it->first << "의 볼륨: " << vol << std::endl;
            float min, max;
            it->second->get3DMinMaxDistance(&min, &max);
            std::cout << "min:" << min << ", max: " << max << std::endl;
            ++it;
        }
    }
    // 시스템 업데이트
    result = m_system->update();
    ERRCHECK(result);
}

void FMOD_INFO::end_fmod()
{
    FMOD_RESULT result;
    // 클린업

    for (auto& sound : SOUNDS)
    {
        result = sound.second->release();
        ERRCHECK(result);
    }

    SOUNDS.clear();
    CHANNELS.clear();

    result = m_system->close();
    ERRCHECK(result);

    result = m_system->release();
    ERRCHECK(result);
}

bool FMOD_INFO::play_loop_sound(const DirectX::XMFLOAT3& WorldPos, const SOUND_TYPE& sound, const std::string& channel, float pitch)
{
    FMOD_RESULT result;
    if (CHANNELS.contains(channel) == false) {
        auto b = CHANNELS.try_emplace(channel);
        if (b.second)
        {
            CHANNELS[channel]->setMode(FMOD_LOOP_NORMAL);
            result = m_system->playSound(SOUNDS[sound], 0, false, &CHANNELS[channel]);
            ERRCHECK(result);

            FMOD_VECTOR soundPos = { WorldPos.x, WorldPos.y, WorldPos.z };
            FMOD_VECTOR soundVel = { 0.0f, 0.0f, 0.0f };
            result = CHANNELS[channel]->set3DAttributes(&soundPos, &soundVel);
            ERRCHECK(result);
            return true;
        }
    }
    else
    {
        FMOD_VECTOR soundPos = { WorldPos.x, WorldPos.y, WorldPos.z };
        FMOD_VECTOR soundVel = { 0.0f, 0.0f, 0.0f };
        CHANNELS[channel]->set3DAttributes(&soundPos, &soundVel);
        // 재생 속도
        CHANNELS[channel]->setPitch(pitch);
        return false;
    }
}

bool FMOD_INFO::play_unloop_sound(const DirectX::XMFLOAT3& WorldPos,const SOUND_TYPE& sound,const std::string& channel)
{
    FMOD_RESULT result;
    auto b = CHANNELS.try_emplace(channel);
    if (b.second)
    {
        result = m_system->playSound(SOUNDS[sound], 0, false, &CHANNELS[channel]);
        ERRCHECK(result);

        FMOD_VECTOR soundPos = { WorldPos.x, WorldPos.y, WorldPos.z };
        FMOD_VECTOR soundVel = { 0.0f, 0.0f, 0.0f };
        result = CHANNELS[channel]->set3DAttributes(&soundPos, &soundVel);
        ERRCHECK(result);
        return true;
    }
    else
    {
        return false;
    }
}

bool FMOD_INFO::stop_sound(const std::string& channel)
{
    FMOD_RESULT result;
    auto f = CHANNELS.find(channel);
    if (f != CHANNELS.end())
    {
        f->second->stop();
        CHANNELS.erase(f);
        return true;
    }
    return false;
}
