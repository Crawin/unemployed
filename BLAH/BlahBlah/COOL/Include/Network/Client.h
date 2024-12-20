﻿#pragma once
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
#include <chrono>
#include "Packets.h"

#define BUFSIZE    2048
constexpr int PoliceID = 1;
constexpr int StudentID = 2;

void print_error(const char* msg, int err_no);

class GameCharacters
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 rotation;
	DirectX::XMFLOAT3 speed;
	bool updated;
public:
	GameCharacters() 
	{
		position = DirectX::XMFLOAT3(0, 0, 0);
		rotation = DirectX::XMFLOAT3(0, 0, 0);
		speed = DirectX::XMFLOAT3(0, 0, 0);
		updated = false;
	};
	void setPosRotSpeed(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3 rot, const DirectX::XMFLOAT3 sp) 
	{
		position = pos; rotation = rot; speed = sp;
		updated = true;
	}
	const DirectX::XMFLOAT3 getPos() { return position; }
	const DirectX::XMFLOAT3 getRot() { return rotation; }
	const DirectX::XMFLOAT3 getSpeed() { return speed; }
	bool IsUpdated() const { return updated; }
	void SetUpdate(bool bo) { updated = bo; }
};

struct EXP_OVER
{
	WSAOVERLAPPED over;
	WSABUF wsabuf[1];
	char buf[BUFSIZE];
	EXP_OVER()
	{
		wsabuf[0].buf = buf;
	}
};

class SceneManager;

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
	//char* m_cpServerIP;
	std::string m_ServerIP;
	SOCKET m_sServer;
	EXP_OVER EXPover;
	WSABUF wsabuf[1];
	WSAOVERLAPPED wsaover;
	char buf[BUFSIZE];
	SOCKET playerSock[2];
	unsigned int roomNum = NULL;
	short characterType = NULL;
	short prev_packet_size = 0;

	bool m_UpdatePosition = true;

	float m_SendTimeElapsed = 0.0f;
	// 1초에 24번 보냄
	const float m_SendFrame = 1.0f / 60.0f;

	char store_buf[BUFSIZE];
	SceneManager* m_SceneManager;

private:
	std::string m_HostPlayerName = "Player1";
	std::string m_GuestPlayerName = "Player2";

public:
	std::unordered_map<int, GameCharacters> characters;
	std::atomic_bool is_talking = false;
	float mic_lv = 0.4;
	DirectX::XMFLOAT3 self_position = { 0,0,0 };
	VIVOX_STATE* vivox_state;

	void Recv_Start();
	char* Get_Buf();
	char* Get_Store_Buf() { return store_buf + prev_packet_size; }
	void Send_Pos(const DirectX::XMFLOAT3&, const DirectX::XMFLOAT3&, const DirectX::XMFLOAT3&, float deltaTime);
	void Send_Room(const PACKET_TYPE&, const unsigned int&);
	void Connect_Server();
	void setPSock(const SOCKET&);
	void swapPSock();
	const SOCKET* getPSock() { return playerSock; }
	void setRoomNum(const unsigned int n);
	const unsigned int getRoomNum() { return roomNum; }
	const short getCharType() { return characterType; }
	void setCharType(const short& type) { characterType = type; }
	void set_prev_packet_size(const short& size) { prev_packet_size = size; }
	void pull_packet(const int& current_size);
	void logout_opponent();
	short get_prev_packet_size() { return prev_packet_size; }
	void setSceneManager(SceneManager* scenemanager);
	SceneManager* getSceneManager() { return m_SceneManager; }

	void send_packet(void*);

	void SetPositionRecv(bool state) { m_UpdatePosition = state; }
	bool GetPositionRecv() const { return m_UpdatePosition; }

	const std::string& GetHostPlayerName() const { return m_HostPlayerName; }
	const std::string& GetGuestPlayerName() const { return m_GuestPlayerName; }

	void Reset();
};

void CALLBACK recv_callback(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void CALLBACK send_callback(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);

void process_packet(packet_base*&);