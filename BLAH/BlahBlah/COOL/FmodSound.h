#pragma once

#pragma comment(lib,"fmod_vc.lib")
#pragma comment(lib,"fmodL_vc.lib")
#include <../External/Include/FMOD/fmod.hpp>
#include <../External/Include/FMOD/fmod_errors.h>

void ERRCHECK(FMOD_RESULT result);
int start_fmod();