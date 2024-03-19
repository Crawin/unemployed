#include "vivoxheaders.h"

int main()
{
    Start_Vivox();
    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void Start_Vivox()
{
    vx_sdk_config_t defaultConfig;
    int status = vx_get_default_config3(&defaultConfig, sizeof(defaultConfig));

    if (status != VxErrorSuccess) {
        printf("vx_sdk_get_default_config3() returned %d: %s\n", status, vx_get_error_string(status));
        return;
    }
    defaultConfig.pf_sdk_message_callback = &OnResponseOrEventFromSdk;
    status = vx_initialize3(&defaultConfig, sizeof(defaultConfig));

    if (status != VxErrorSuccess) {
        printf("vx_initialize3() returned %d : %s\n", status, vx_get_error_string(status));
        return;
    }
    // Vivox Client SDK가 이제 초기화됨
    vx_req_connector_create* req_connector = nullptr;
    CreateConnector(req_connector);
    
    std::string command;
    int sockNum;
    int gameNum;
    while (1)
    {
        std::cin >> command;
        if (command == "login")
        {
            std::cout << "소켓 번호를 입력하시오." << std::endl;
            std::cin >> sockNum;
            LogIn(sockNum);
        }
        else if (command == "addsession")
        {
            std::cout << "방 번호를 입력하시오." << std::endl;
            std::cin >> gameNum;
            RequestParticipate(sockNum,gameNum);
        }
    }
    // 프로그램 종료 전 uninitialize
    status = vx_uninitialize();
}

void MyGamesEventHandler(vx_evt_base_t* evt)
{
    switch (evt->type)
    {
    case evt_account_login_state_change:
        HandleLoginStateChange(reinterpret_cast<vx_evt_account_login_state_change*>(evt));
        break;
    case evt_media_stream_updated:
        HandleMediaStreamUpdatedEvent(reinterpret_cast<vx_evt_media_stream_updated*>(evt));
        break;
    case evt_text_stream_updated:
        HandleTextStreamUpdatedEvent(reinterpret_cast<vx_evt_text_stream_updated*>(evt));
        break;
    case evt_participant_added:
        HandleParticipantAddedEvent(reinterpret_cast<vx_evt_participant_added*>(evt));
        break;
    case evt_participant_removed:
        HandleParticipantRemovedEvent(reinterpret_cast<vx_evt_participant_removed*>(evt));
        break;
    case evt_participant_updated:
        HandleParticipantUpdatedEvent(reinterpret_cast<vx_evt_participant_updated*>(evt));
        break;
    case evt_connection_state_changed:
        HandleConnectionStateChanged(reinterpret_cast<vx_evt_connection_state_changed*>(evt));
        break;
    case evt_sessiongroup_added:
        HandleSessiongroupAdded(reinterpret_cast<vx_evt_sessiongroup_added*>(evt));
        break;
    case evt_session_added:
        HandleSessionAdded(reinterpret_cast<vx_evt_session_added*>(evt));
        break;
    }
}

void MyGamesResponseHandler(vx_resp_base_t* resp)
{
    switch (resp->type)
    {
    case resp_account_anonymous_login:
        HandleLoginResponse(reinterpret_cast<vx_resp_account_anonymous_login*>(resp));
        break;
    case resp_connector_create:
        std::cout << "이제부터 로그인이 가능합니다." << std::endl;
        break;
    case resp_sessiongroup_add_session:
        HandleSessiongroupAddSession(reinterpret_cast<vx_resp_sessiongroup_add_session*>(resp));
        break;
    }
}


//메세지 핸들러
void OnResponseOrEventFromSdk(void* callback_handle) 
{
    int status;
    vx_message_base_t* msg;
    for (;;) {
        status = vx_get_message(&msg);
        if (status == VX_GET_MESSAGE_AVAILABLE) {
            std::cout << "[" << msg->type << "] 콜백 메세지가 들어왔다! [2]:resp, [3]:event" << std::endl;
            if (msg->type == msg_event) {
                MyGamesEventHandler(reinterpret_cast<vx_evt_base_t*>(msg));
            }
            else {
                MyGamesResponseHandler(reinterpret_cast<vx_resp_base_t*>(msg));
            }
            vx_destroy_message(msg);
        }
        else if (status == VX_GET_MESSAGE_FAILURE) {
            // 여기에서 오류를 처리합니다. vx_initialize3()가 아직 호출되지 않는 경우 발생합니다.
        }
        else { /* VX_GET_MESSAGE_NO_MESSAGE */
            break;
        }
    }
}

void CreateConnector(vx_req_connector_create*& req)
{
    //커넥터 핸들 제작
    vx_req_connector_create_create(&req);
    req->connector_handle = vx_strdup("ConnectorHandle");
    req->acct_mgmt_server = vx_strdup(VIVOX_SERVER);
    int request_count;
    int vx_issue_request3_response = vx_issue_request3(&req->base, &request_count);
}

void LogIn(const int& sock)
{
    std::string sock_str = std::to_string(sock);
    std::string UserName = VIVOX_ISSUER;
    UserName += '.' + sock_str;
    std::cout << "User name: " << UserName << std::endl;
    
    vx_req_account_anonymous_login_t* req;
    vx_req_account_anonymous_login_create(&req);
    req->access_token = vx_strdup(vx_debug_generate_token(VIVOX_ISSUER, vx_time_t(-1), "login", 1, NULL, vx_get_user_uri(&UserName[0], VIVOX_DOMAIN, NULL), NULL, (const unsigned char*)VIVOX_KEY, strlen(VIVOX_KEY)));
    req->account_handle = vx_strdup(&std::string("SOCK_" + sock_str)[0]);
    req->connector_handle = vx_strdup("ConnectorHandle");
    req->acct_name = vx_strdup(&std::string('.' + UserName + '.')[0]);
    req->displayname = vx_strdup("Test1");
    vx_issue_request3(&req->base,NULL);
}

void HandleLoginResponse(vx_resp_account_anonymous_login* resp)
{
    if (resp->base.return_code == 1)
    {
        std::cout << "에러 발생: " << resp->base.status_code << ", " << vx_get_error_string(resp->base.status_code) << std::endl;
        return;
    }
    std::cout << "[ account_handle: " << resp->account_handle << " ], 해당 작업 항목의 상태를 진행 중 상태로 변경합니다." << std::endl;
}

void HandleLoginStateChange(vx_evt_account_login_state_change* evt)
{
    if (evt->state == login_state_logged_in)
    {
        printf("[ %s ] is logged in, 해당 작업 항목의 상태를 완료된 상태로 변경합니다.\n", evt->account_handle);
    }
    else if (evt->state == login_state_logged_out)
    {
        if (evt->status_code != 0)
        {
            printf("[ %s ] logged out with status %d:%s\n", evt->status_code, vx_get_error_string(evt->status_code));
        }
        else
        {
            printf("[ %s ] is logged out\n", evt->account_handle);
        }
    }
    else if (evt->state == login_state_logging_in)
    {
        std::cout <<"[ " << evt->account_handle << " ] 로그인 중..." << std::endl;
    }
}

// 로그아웃 할 때는 클라이언트가 vx_req_account_logout 요청을 제출한 후에 클라이언트 애플리케이션이 종료됩니다.

void RequestParticipate(const int& sock, const int& gameNum)
{
    std::string sock_str = std::to_string(sock);

    //char* temp = vx_get_echo_channel_uri("echotest", VIVOX_DOMAIN, VIVOX_ISSUER);       // 에코채널
    char* temp = vx_get_general_channel_uri(&std::to_string(gameNum)[0], VIVOX_DOMAIN, VIVOX_ISSUER);       // 논포지셔널채널

    vx_req_sessiongroup_add_session* req;
    vx_req_sessiongroup_add_session_create(&req);
    req->sessiongroup_handle = vx_strdup(&sock_str[0]);
    req->session_handle = vx_strdup(temp);
    req->uri = vx_strdup(temp);
    req->account_handle = vx_strdup(&std::string("SOCK_" + sock_str)[0]);
    req->connect_audio = 1;
    req->connect_text = 0;
    req->access_token = GenerateToken("join", temp, sock);
    vx_issue_request3(&req->base, NULL);
}
char* GenerateToken(const char* payload, const char* uri,const int& sock)
{
    //int socketnum = 100;
    std::string UserName = VIVOX_ISSUER;
    UserName += '.' + std::to_string(sock);
    char* hi = vx_get_user_uri(&UserName[0], VIVOX_DOMAIN, NULL);

    return vx_strdup(vx_debug_generate_token(VIVOX_ISSUER, vx_time_t(-1), payload, 2, NULL, vx_get_user_uri(&UserName[0], VIVOX_DOMAIN, NULL), uri, (const unsigned char*)VIVOX_KEY, strlen(VIVOX_KEY)));
}

void HandleMediaStreamUpdatedEvent(vx_evt_media_stream_updated* evt)
{
    if (evt->state == session_media_connected)
    {
        printf("Audio Connected to %s\n", evt->session_handle);
    }
    else if (evt->state == session_media_disconnected)
    {
        if (evt->status_code == 0)
        {
            printf("Audio Disconnected from %s\n", evt->session_handle);
        }
        else
        {
            printf("Audio Disconnected from %s, error %d:%s\n", evt->session_handle, evt->status_code, vx_get_error_string(evt->status_code));
        }
    }
    else if (evt->state == session_media_connecting)
    {
        std::cout << "[" << evt->session_handle << "] 연결중..." << std::endl;
    }
}
void HandleTextStreamUpdatedEvent(vx_evt_text_stream_updated* evt)
{
    if (evt->state == session_text_connected)
    {
        printf("Text Connected to %s\n", evt->session_handle);
    }
    else if (evt->state == session_text_disconnected)
    {
        if (evt->status_code == 0)
        {
            printf("Text Disconnected from %s\n", evt->session_handle);
        }
        else
        {
            printf("Text Disconnected from %s, error %d:%s\n", evt->session_handle, evt->status_code, vx_get_error_string(evt->status_code));
        }
    }
}

void HandleParticipantAddedEvent(vx_evt_participant_added* evt)
{
    printf("User %s joined %s\n", evt->encoded_uri_with_tag, evt->session_handle);
}

void HandleParticipantRemovedEvent(vx_evt_participant_removed* evt)
{
    printf("User %s left %s\n", evt->encoded_uri_with_tag, evt->session_handle);
}

void HandleParticipantUpdatedEvent(vx_evt_participant_updated* evt)
{
    //printf("User %s %s speaking to %s\n", evt->encoded_uri_with_tag, evt->is_speaking ? "is" : "is not", evt->session_handle);
}

void HandleConnectionStateChanged(vx_evt_connection_state_changed* evt)
{
    std::cout << evt->account_handle << "이 " << evt->connection_state << "로 상태가 변경되었습니다." << std::endl;
}

void HandleSessiongroupAddSession(vx_resp_sessiongroup_add_session* evt)
{
    if (evt->base.return_code == 1)
    {
        std::cout << "에러 발생: " << evt->base.status_code << ", " << vx_get_error_string(evt->base.status_code) << std::endl;
        return;
    }
    std::cout <<"["<< evt->session_handle << "] SessiongroupAddSession" << std::endl;
}

void HandleSessiongroupAdded(vx_evt_sessiongroup_added* evt)
{
    std::cout <<"["<< evt->account_handle << "] Sessiongroup Added" << std::endl;
}

void HandleSessionAdded(vx_evt_session_added* evt)
{
    std::cout << "[" << evt->session_handle << "] SessionAdded" << std::endl;
}