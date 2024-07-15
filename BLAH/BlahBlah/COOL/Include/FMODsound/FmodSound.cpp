#include "framework.h"
#include <atomic>
#include "FmodSound.h"

void ERRCHECK(FMOD_RESULT result)
{
    if (result != FMOD_OK)
    {
        std::cout << "FMOD error! (" << result << ") " << FMOD_ErrorString(result) << std::endl;
        exit(-1);
    }
}


int start_fmod(FMOD_INFO& info)
{
    FMOD::System* system;
  /*  FMOD::Sound* duck;
    FMOD::Channel* channel2 = 0;*/

    SOUND_EFFECT guard_foot;
    SOUND_EFFECT self_foot;

    FMOD_RESULT result;

    // FMOD 시스템 생성
    result = FMOD::System_Create(&system);
    ERRCHECK(result);

    // 시스템 초기화
    result = system->init(512, FMOD_INIT_NORMAL, 0);
    ERRCHECK(result);

    result = system->set3DSettings(1.0f, 0.01f, 1.0f);
    ERRCHECK(result);

    // 사운드 로드 (3D 모드)
    result = system->createSound("SceneData/Sound/run.wav", FMOD_3D | FMOD_LOOP_NORMAL, 0, &guard_foot.sound);
    ERRCHECK(result);

    result = guard_foot.sound->set3DMinMaxDistance(100.0f, 1000.0f);
    ERRCHECK(result);

    //result = system->createSound("SceneData/Sound/ducktest.wav", FMOD_3D, 0, &duck);
    //ERRCHECK(result);

    //result = system->playSound(duck, 0, false, &channel2);
    //ERRCHECK(result);

    // 청취자 위치 설정
    DirectX::XMFLOAT3 player1_pos = info.get_player1_position();
    DirectX::XMFLOAT3 player1_rot = info.get_player1_rotation();
    FMOD_VECTOR listenerPos = { player1_pos.x,player1_pos.y,player1_pos.z };
    FMOD_VECTOR listenerVel = { 0.0f, 0.0f, 0.0f };
    FMOD_VECTOR listenerForward = { 0.0f, 0.0f, -1.0f };
    FMOD_VECTOR listenerUp = { 0.0f, 1.0f, 0.0f };

    result = system->set3DListenerAttributes(0, &listenerPos, &listenerVel, &listenerForward, &listenerUp);
    ERRCHECK(result);

    // 사운드 위치 설정
    DirectX::XMFLOAT3 guard_pos = info.get_guard_position();
    FMOD_VECTOR soundPos = { guard_pos.x,guard_pos.y,guard_pos.z };
    FMOD_VECTOR soundVel = { 0.0f, 0.0f, 0.0f };

    result = guard_foot.channel->set3DAttributes(&soundPos, &soundVel);
    ERRCHECK(result);

    // 볼륨 설정
    result = guard_foot.channel->setVolume(0.1);
    ERRCHECK(result);

    // 사운드 재생
    result = system->playSound(guard_foot.sound, 0, false, &guard_foot.channel);
    ERRCHECK(result);

    bool self_running = false;

    result = system->createSound("SceneData/Sound/run.wav", FMOD_3D | FMOD_LOOP_NORMAL, 0, &self_foot.sound);
    ERRCHECK(result);
    
    result = self_foot.channel->setVolume(0.1);
    ERRCHECK(result);

    // 메인 루프
    while (info.is_client_on())
    {
        // 여기서 다른 업데이트 로직 추가 가능
        // 현재 내 위치 업데이트
        player1_pos = info.get_player1_position();
        listenerPos = { player1_pos.x,player1_pos.y,player1_pos.z };
        player1_rot = info.get_player1_rotation();
        //listenerForward = {};
        result = system->set3DListenerAttributes(0, &listenerPos, &listenerVel, &listenerForward, &listenerUp);
        ERRCHECK(result);

        // 현재 Guard 위치 업데이트
        guard_pos = info.get_guard_position();
        soundPos = { guard_pos.x,guard_pos.y,guard_pos.z };
        result = guard_foot.channel->set3DAttributes(&soundPos, &soundVel);
        ERRCHECK(result);


        // 나 자신의 발소리
        if (info.is_running())  // 클라에서 shift를 눌러 달리는 상태이면
        {
            if (self_running == false)          // 달리지 않다가 달리기가 시작되면
            {
                self_running = true;
                result = system->playSound(self_foot.sound, 0, false, &self_foot.channel);
                ERRCHECK(result);
            }
        }
        else
        {                       // 걷고 있는 상태면
            if (self_running == true)
            {
                self_running = false;
                result = self_foot.channel->stop();
            }
        }

        // 시스템 업데이트
        result = system->update();
        ERRCHECK(result);
    }

    // 클린업
    result = guard_foot.sound->release();
    ERRCHECK(result);

    result = system->close();
    ERRCHECK(result);

    result = system->release();
    ERRCHECK(result);

    return 0;
}

void FMOD_INFO::set_guard_position(const DirectX::XMFLOAT3& position)
{
    m_guard_position = position;
}
