#pragma once
#include <SDK/include/vivox-config.h>
#include <SDK/include/Vxc.h>
#include <SDK/include/VxcErrors.h>
#include <SDK/include/VxcEvents.h>
#include <SDK/include/VxcExports.h>
#include <SDK/include/VxcRequests.h>
#include <SDK/include/VxcResponses.h>
#include <SDK/include/VxcTypes.h>

#define VIVOX_SERVER "https://unity.vivox.com/appconfig/f7bfe-vivox-31834-udash"
#define VIVOX_DOMAIN "mtu1xp.vivox.com"
#define VIVOX_ISSUER "f7bfe-vivox-31834-udash"
#define VIVOX_KEY "l9ca3trZCZm5ISZ7BGWWWswBcBv5NjCC"

#include <stdio.h>
#include <iostream>
#include <string>
#include <thread>

void Start_Vivox();
void MyGamesEventHandler(vx_evt_base_t*);
void MyGamesResponseHandler(vx_resp_base_t*);
void OnResponseOrEventFromSdk(void*);
void CreateConnector(vx_req_connector_create*&);
void LogIn(const int&);
void HandleLoginResponse(vx_resp_account_anonymous_login*);
void HandleLoginStateChange(vx_evt_account_login_state_change*);
void RequestParticipate(const int&);
char* GenerateToken(const char*, const char*);
void HandleMediaStreamUpdatedEvent(vx_evt_media_stream_updated*);
void HandleTextStreamUpdatedEvent(vx_evt_text_stream_updated*);
void HandleParticipantAddedEvent(vx_evt_participant_added*);
void HandleParticipantRemovedEvent(vx_evt_participant_removed*);
void HandleParticipantUpdatedEvent(vx_evt_participant_updated*);
void HandleConnectionStateChanged(vx_evt_connection_state_changed*);
void HandleSessiongroupAddSession(vx_resp_sessiongroup_add_session*);
void HandleSessiongroupAdded(vx_evt_sessiongroup_added*);
void HandleSessionAdded(vx_evt_session_added*);
