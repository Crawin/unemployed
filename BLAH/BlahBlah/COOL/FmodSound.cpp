#include "framework.h"
#include "FmodSound.h"

void ERRCHECK(FMOD_RESULT result)
{
    if (result != FMOD_OK)
    {
        std::cout << "FMOD error! (" << result << ") " << FMOD_ErrorString(result) << std::endl;
        exit(-1);
    }
}

int start_fmod()
{
    FMOD::System* system;
    FMOD::Sound* sound;
    FMOD::Sound* duck;
    FMOD::Channel* channel = 0;
    FMOD::Channel* channel2 = 0;
    FMOD_RESULT result;

    // FMOD 시스템 생성
    result = FMOD::System_Create(&system);
    ERRCHECK(result);

    // 시스템 초기화
    result = system->init(512, FMOD_INIT_NORMAL, 0);
    ERRCHECK(result);

    // 사운드 로드 (3D 모드)
    result = system->createSound("SceneData/Sound/run.wav", FMOD_3D, 0, &sound);
    ERRCHECK(result);

    result = system->createSound("SceneData/Sound/ducktest.wav", FMOD_3D, 0, &duck);
    ERRCHECK(result);

    // 사운드 재생
    result = system->playSound(sound, 0, false, &channel);
    ERRCHECK(result);

    result = system->playSound(duck, 0, false, &channel2);
    ERRCHECK(result);

    result = channel2->setMode(FMOD_LOOP_NORMAL);
    ERRCHECK(result);

    // 청취자 위치 설정
    FMOD_VECTOR listenerPos = { 0.0f, 0.0f, 0.0f };
    FMOD_VECTOR listenerVel = { 0.0f, 0.0f, 0.0f };
    FMOD_VECTOR listenerForward = { 0.0f, 0.0f, 1.0f };
    FMOD_VECTOR listenerUp = { 0.0f, 1.0f, 0.0f };

    result = system->set3DListenerAttributes(0, &listenerPos, &listenerVel, &listenerForward, &listenerUp);
    ERRCHECK(result);

    // 사운드 위치 설정
    FMOD_VECTOR soundPos = { 0.0f, 0.0f, 0.0f };
    FMOD_VECTOR soundVel = { 0.0f, 0.0f, 0.0f };

    result = channel->set3DAttributes(&soundPos, &soundVel);
    ERRCHECK(result);

    float radian = 0;
    FMOD_VECTOR soundPos2 = { 1.0f, 0.0f, 0.0f };
    FMOD_VECTOR soundVel2 = { 0.0f, 0.0f, 0.0f };

    result = channel2->set3DAttributes(&soundPos2, &soundVel2);
    ERRCHECK(result);

    // 볼륨 설정
    result = channel->setVolume(0.1);
    ERRCHECK(result);

    // 메인 루프
    while (true)
    {
        // 시스템 업데이트
        result = system->update();
        ERRCHECK(result);

        // 채널 상태 체크
        bool isPlaying = false;
        result = channel->isPlaying(&isPlaying);
        ERRCHECK(result);

        if (!isPlaying)
        {
            break;
        }

        // 여기서 다른 업데이트 로직 추가 가능
        std::cout << "(" << soundPos2.x << ", " << soundPos2.z << ")" << std::endl;
        soundPos2.x = cos(radian);
        soundPos2.z = -sin(radian);
        radian += 0.0174533;
        result = channel2->set3DAttributes(&soundPos2, &soundVel2);
        ERRCHECK(result);
    }

    // 클린업
    result = sound->release();
    ERRCHECK(result);

    result = system->close();
    ERRCHECK(result);

    result = system->release();
    ERRCHECK(result);

    return 0;
}
