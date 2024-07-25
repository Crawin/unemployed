#include "vivoxheaders.h"
std::string UserName;
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
    defaultConfig.pf_on_audio_unit_before_recv_audio_mixed = &OnBeforeReceivedAudioMixed;
    //defaultConfig.pf_on_audio_unit_before_recv_audio_rendered = &OnAudioBeforeRecvAudioRendered;
    status = vx_initialize3(&defaultConfig, sizeof(defaultConfig));

    if (status != VxErrorSuccess) {
        printf("vx_initialize3() returned %d : %s\n", status, vx_get_error_string(status));
        return;
    }
    // Vivox Client SDK�� ���� �ʱ�ȭ��
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
            std::cout << "���� ��ȣ�� �Է��Ͻÿ�." << std::endl;
            std::cin >> sockNum;
            LogIn(sockNum);
        }
        else if (command == "addsession")
        {
            std::cout << "�� ��ȣ�� �Է��Ͻÿ�." << std::endl;
            std::cin >> gameNum;
            RequestParticipate(sockNum,gameNum);
        }
        else if (command == "exit")
        {
            ExitChannel(gameNum);
        }
        else if (command == "logout")
        {
            LogOut(sockNum);
            break;
        }
    }
    // ���α׷� ���� �� uninitialize
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
    default:
        std::cout << "������� ���� �̺�Ʈ �ݹ�" << std::endl;
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
        std::cout << "�������� �α����� �����մϴ�." << std::endl;
        break;
    case resp_sessiongroup_add_session:
        HandleSessiongroupAddSession(reinterpret_cast<vx_resp_sessiongroup_add_session*>(resp));
        break;
    case resp_sessiongroup_remove_session:
        break;
    default:
        std::cout << "������� ���� �������� �ݹ�" << std::endl;
        break;
    }
}


//�޼��� �ڵ鷯
void OnResponseOrEventFromSdk(void* callback_handle)
{
    int status;
    vx_message_base_t* msg;
    for (;;) {
        status = vx_get_message(&msg);
        if (status == VX_GET_MESSAGE_AVAILABLE) {
            std::cout << "[" << msg->type << "] �ݹ� �޼����� ���Դ�! [2]:resp, [3]:event" << std::endl;
            if (msg->type == msg_event) {
                MyGamesEventHandler(reinterpret_cast<vx_evt_base_t*>(msg));
            }
            else {
                MyGamesResponseHandler(reinterpret_cast<vx_resp_base_t*>(msg));
            }
            vx_destroy_message(msg);
        }
        else if (status == VX_GET_MESSAGE_FAILURE) {
            // ���⿡�� ������ ó���մϴ�. vx_initialize3()�� ���� ȣ����� �ʴ� ��� �߻��մϴ�.
        }
        else { /* VX_GET_MESSAGE_NO_MESSAGE */
            break;
        }
    }
}

void OnAudioBeforeRecvAudioRendered(
    void* callback_handle,
    const char* session_group_handle,
    const char* initial_target_uri,
    short* pcm_frames,      // ������ ����
    int pcm_frame_count,    // ���� �� ä�� ��
    int audio_frame_rate,   // ���� ����Ʈ, ��Ʈ�� ���� ��ȯ ����
    int channels_per_frame, // �����Ӵ� ä��
    int is_silence)          // ���� ���� ����� �����Ͱ� ������ 0
{
    std::cout << "����� �ݹ� ����" << std::endl;
}

void OnBeforeReceivedAudioMixed(
    void* callback_handle,
    const char* session_group_handle,
    const char* initial_target_uri,
    vx_before_recv_audio_mixed_participant_data_t* participants_data,
    size_t num_participants)
{
    //std::cout << "����� �ݹ� ����" << std::endl;

    //std::string example_uri = "example_uri";
    //// �ټ��� ����� ��Ʈ���� ���� �ݺ�
    //for (unsigned int i = 0; i < num_participants; i++) {
    //    auto& data = participants_data[i];

    //    int channels_per_frame = data.channels_per_frame;
    //    std::string participant_uri = data.participant_uri;

    //    //if (participant_uri == example_uri) {
    //        //for (unsigned int f = 0; f < data.pcm_frame_count; f++) {
    //        //    for (unsigned int c = 0; c < channels_per_frame; c++) {
    //        //        // ����� ������ ���/����. ���⼭�� �ܼ��� ������ 2�� �����ϴ�.
    //        //        data.pcm_frames[(f * channels_per_frame) + c] /= 10;
    //        //    }
    //        //}
    //    //}
    //    auto tempdata = data.pcm_frames;    // ������
    //    for (unsigned int f = 0; f < data.pcm_frame_count/4; ++f)
    //    {
    //        data.pcm_frames[f] = (tempdata[f*4]+tempdata[f*4+1]+ tempdata[f * 4+2] + tempdata[f * 4 + 3]) / 4;
    //    }
    //}
}

void CreateConnector(vx_req_connector_create*& req)
{
    //Ŀ���� �ڵ� ����
    vx_req_connector_create_create(&req);
    req->connector_handle = vx_strdup("ConnectorHandle");
    req->acct_mgmt_server = vx_strdup(VIVOX_SERVER);
    int request_count;
    int vx_issue_request3_response = vx_issue_request3(&req->base, &request_count);
}

void LogIn(const int& sock)
{
    std::string sock_str = std::to_string(sock);
    UserName = VIVOX_ISSUER;
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
        std::cout << "���� �߻�: " << resp->base.status_code << ", " << vx_get_error_string(resp->base.status_code) << std::endl;
        return;
    }
    std::cout << "[ account_handle: " << resp->account_handle << " ], �ش� �۾� �׸��� ���¸� ���� �� ���·� �����մϴ�." << std::endl;
}

void HandleLoginStateChange(vx_evt_account_login_state_change* evt)
{
    if (evt->state == login_state_logged_in)
    {
        printf("[ %s ] is logged in, �ش� �۾� �׸��� ���¸� �Ϸ�� ���·� �����մϴ�.\n", evt->account_handle);
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
        std::cout <<"[ " << evt->account_handle << " ] �α��� ��..." << std::endl;
    }
}

// �α׾ƿ� �� ���� Ŭ���̾�Ʈ�� vx_req_account_logout ��û�� ������ �Ŀ� Ŭ���̾�Ʈ ���ø����̼��� ����˴ϴ�.

void RequestParticipate(const int& sock, const int& gameNum)
{
    std::string sock_str = std::to_string(sock);
    std::string gameNum_str = std::to_string(gameNum);
    //char* temp = vx_get_echo_channel_uri(&gameNum_str[0], VIVOX_DOMAIN, VIVOX_ISSUER);       // ����ä��
    char* temp = vx_get_general_channel_uri(&std::to_string(gameNum)[0], VIVOX_DOMAIN, VIVOX_ISSUER);       // �������ų�ä��

    vx_req_sessiongroup_add_session* req;
    vx_req_sessiongroup_add_session_create(&req);
    req->sessiongroup_handle = vx_strdup(&gameNum_str[0]);
    req->session_handle = vx_strdup(&std::string("Game" + gameNum_str)[0]);
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
    //char* hi = vx_get_user_uri(&UserName[0], VIVOX_DOMAIN, NULL);

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
        std::cout << "[" << evt->session_handle << "] ������..." << std::endl;
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
    std::cout <<
        evt->is_current_user << std::endl;
    //printf("User %s %s speaking to %s\n", evt->encoded_uri_with_tag, evt->is_speaking ? "is" : "is not", evt->session_handle);
}

void HandleConnectionStateChanged(vx_evt_connection_state_changed* evt)
{
    std::cout << evt->account_handle << "�� " << evt->connection_state << "�� ���°� ����Ǿ����ϴ�." << std::endl;
}

void HandleSessiongroupAddSession(vx_resp_sessiongroup_add_session* evt)
{
    if (evt->base.return_code == 1)
    {
        std::cout << "���� �߻�: " << evt->base.status_code << ", " << vx_get_error_string(evt->base.status_code) << std::endl;
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

void ExitChannel(const int& gameNum)
{
    std::string gameNum_str = std::to_string(gameNum);
    vx_req_sessiongroup_remove_session* req;
    vx_req_sessiongroup_remove_session_create(&req);
    req->sessiongroup_handle = vx_strdup(&gameNum_str[0]);
    req->session_handle = vx_strdup(&std::string("Game" + gameNum_str)[0]);
    vx_issue_request3(&req->base, NULL);
}

void LogOut(const int& sock)
{
    std::string sock_str = std::to_string(sock);
    vx_req_account_logout* req;
    vx_req_account_logout_create(&req);
    req->account_handle = vx_strdup(&std::string("SOCK_" + sock_str)[0]);
    vx_issue_request3(&req->base, NULL);
}