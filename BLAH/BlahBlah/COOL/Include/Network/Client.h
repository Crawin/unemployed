#pragma once
#define _CRT_SECURE_NO_WARNINGS // 구형 C 함수 사용 시 경고 끄기
#define _WINSOCK_DEPRECATED_NO_WARNINGS // 구형 소켓 API 사용 시 경고 끄기

#include <winsock2.h> // 윈속2 메인 헤더
#include <ws2tcpip.h> // 윈속2 확장 헤더

#include <tchar.h> // _T(), ...
#include <stdio.h> // printf(), ...
#include <stdlib.h> // exit(), ...
#include <string.h> // strncpy(), ...

#pragma comment(lib, "ws2_32") // ws2_32.lib 링크
#include <iostream>
#include <thread>
#include <vector>
#include <DirectXMath.h>
#include <string.h>
#include <unordered_map>

#define BUFSIZE    512

void print_error(const char* msg, int err_no)
{
	WCHAR* msg_buf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPWSTR>(&msg_buf), 0, NULL);
	std::cout << msg;
	std::wcout << L" : 에러 : " << msg_buf;
	//while (true);
	LocalFree(msg_buf);
}

/*
	PACKCET
	-----------------------------
	size
	type
	playernum;
	xmfloat3 pos
	xmfloat3 rot
*/


enum PACKET_TYPE
{
	POSITION,
	LOGIN
};

// SendType, XMFLOAT3
struct SendPosition {
	PACKET_TYPE type;
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 rot;
};

class sc_packet_position
{
	int size;
	PACKET_TYPE type;
	SOCKET player;
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 rotation;
public:
	sc_packet_position() {};
	sc_packet_position(const sc_packet_position& t) :size(t.size), type(t.type), player(t.player), position(t.position), rotation(t.rotation) {}
	const DirectX::XMFLOAT3 getPos() { return position; }
	const DirectX::XMFLOAT3 getRot() { return rotation; }
};

class cs_packet_position
{
private:
	int size;
	PACKET_TYPE type;
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 rotation;
public:
	cs_packet_position(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& rot) :position(pos), rotation(rot)
	{
		size = sizeof(int) + sizeof(PACKET_TYPE) + sizeof(SOCKET) + (sizeof(DirectX::XMFLOAT3) * 2);
		type = POSITION;
	};
};

class Client
{
public:
	static Client& GetInstance() {
		static Client inst;
		return inst;
	}
private:
	Client();
	~Client();
	char* m_cpServerIP;
	SOCKET m_sServer;
	WSABUF wsabuf[1];
	WSAOVERLAPPED wsaover;
	char buf[BUFSIZE];
public:
	std::vector<sc_packet_position> m_vPosition_Queue;
	std::list<char> over_buf;

	void Recv_Start();
	char* Get_Buf();
	void Send_Pos(const DirectX::XMFLOAT3& , const DirectX::XMFLOAT3&);
	void Connect_Server();
};

void CALLBACK recv_callback(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void CALLBACK send_callback(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);