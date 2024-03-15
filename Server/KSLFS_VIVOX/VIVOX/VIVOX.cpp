#include "vivoxheaders.h"
void Start_Vivox();
void OnResponseOrEventFromSdk(void* callback_handle);
void CreateConnector(vx_req_connector_create*);
void LogIn();
void HandleLoginResponse(vx_resp_account_anonymous_login* resp);
void HandleLoginStateChange(vx_evt_account_login_state_change* evt);
int main()
{
    Start_Vivox();
    return 0;
}

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

    std::string temp;
    std::cin >> temp;

    while (1)
    {
        LogIn();
        std::cin >> temp;
    }
    // 프로그램 종료 전 uninitialize
    status = vx_uninitialize();
}

//메세지 핸들러
void OnResponseOrEventFromSdk(void* callback_handle) {
    int status;
    vx_message_base_t* msg;
    for (;;) {
        status = vx_get_message(&msg);
        if (status == VX_GET_MESSAGE_AVAILABLE) {
            std::cout << "콜백 메세지가 들어왔다!" << std::endl;
            if (msg->type == msg_event) {
                HandleLoginStateChange(reinterpret_cast<vx_evt_account_login_state_change*>(msg));
            }
            else {
                HandleLoginResponse(reinterpret_cast<vx_resp_account_anonymous_login*>(msg));
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

void CreateConnector(vx_req_connector_create* req)
{
    //커넥터 핸들 제작
    vx_req_connector_create_create(&req);
    req->connector_handle = vx_strdup("connectorhandle");
    req->acct_mgmt_server = vx_strdup(VIVOX_SERVER);
    int request_count;
    int vx_issue_request3_response = vx_issue_request3(&req->base, &request_count);
}

void LogIn()
{
    int socketnum = 802;
    std::string UserName = VIVOX_ISSUER;
    UserName += '.' + std::to_string(socketnum);
    std::cout << "User name: " << UserName << std::endl;
    
    vx_req_account_anonymous_login_t* req;
    vx_req_account_anonymous_login_create(&req);
    req->access_token = vx_strdup(vx_debug_generate_token(VIVOX_ISSUER, vx_time_t(-1), "login", 1, NULL, vx_get_user_uri(&UserName[0], VIVOX_DOMAIN, NULL), NULL, (const unsigned char*)VIVOX_KEY, strlen(VIVOX_KEY)));
    req->account_handle = vx_strdup("testhandle");
    req->connector_handle = vx_strdup("connectorhandle");
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
    printf("login succeeded for account %s\n", resp->account_handle);
}

void HandleLoginStateChange(vx_evt_account_login_state_change* evt)
{
    if (evt->state == login_state_logged_in)
    {
        printf("%s is logged in\n", evt->account_handle);
    }
    else if (evt->state == login_state_logged_out)
    {
        if (evt->status_code != 0)
        {
            printf("%s logged out with status %d:%s\n", evt->status_code, vx_get_error_string(evt->status_code));
        }
        else
        {
            printf("%s is logged out\n", evt->account_handle);
        }
    }
    else if (evt->state == login_state_logging_in)
    {
        std::cout << "로그인 중..." << std::endl;
    }
}