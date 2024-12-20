#include "IOCP_Common.h"
#include "Mesh.h"
#include "aStar.h"
#include "IOCP.h"
#include <random>
#include <set>
std::vector<Mesh*> m_vMeshes;
std::unordered_map<int, NODE*> g_um_graph;
std::unordered_map<unsigned int, SESSION> login_players;
std::priority_queue<npc_info> g_npc_timer;
std::mutex g_mutex_npc_timer;
std::mutex g_mutex_login_players;

std::random_device rd;
std::default_random_engine dre(rd());
std::uniform_int_distribution <> uid(0, 88);

extern std::set<short> StairNodeNums;

void IOCP_SERVER_MANAGER::start()
{
	LoadResources();

	std::wcout.imbue(std::locale("korean"));
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);

	detail.m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	SOCKET server_s = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server_a;
	server_a.sin_family = AF_INET;
	server_a.sin_port = htons(PORT);
	server_a.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	bind(server_s, reinterpret_cast<sockaddr*>(&server_a), sizeof(server_a));
	listen(server_s, SOMAXCONN);
	int addr_size = sizeof(server_a);


	SOCKET client_s = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
	int option = TRUE;
	setsockopt(client_s, IPPROTO_TCP, TCP_NODELAY, (const char*)&option, sizeof(option));
	EXP_OVER accept_over;
	ZeroMemory(&accept_over.over, sizeof(accept_over.over));
	accept_over.c_op = C_ACCEPT;
	accept_over.sock = client_s;
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(server_s), detail.m_hIOCP, -1, 0);
	AcceptEx(server_s, client_s, accept_over.buf, 0, addr_size + 16, addr_size + 16, nullptr, &accept_over.over);

	std::thread command(&IOCP_SERVER_MANAGER::command_thread,this);

	detail.m_bServerState = true;
	std::cout << "서버 실행" << std::endl;
	//lobby = new Lobby;
	//worker(server_s);
	
	std::thread ai_thread{ &IOCP_SERVER_MANAGER::ai_timer,this };

	// 멀티쓰레드
	int num_threads = std::thread::hardware_concurrency();
	std::vector<std::thread> worker_threads;
	for (int i = 0; i < num_threads; ++i)
	{
		worker_threads.emplace_back(&IOCP_SERVER_MANAGER::worker, this, server_s);
	}

	for (auto& w : worker_threads)
		w.join();


	//// 싱글쓰레드 디버그용
	//worker(server_s);

	//delete lobby;
	closesocket(server_s);
	WSACleanup();
	command.join();
	ai_thread.join();

	for (auto& mesh : m_vMeshes)
		delete mesh;
	for (auto& navi : g_um_graph)
		delete navi.second;
}

void IOCP_SERVER_MANAGER::worker(SOCKET server_s)
{
	while (detail.m_bServerState)
	{
		DWORD rw_byte;
		ULONG_PTR key;
		WSAOVERLAPPED* over;
		BOOL ret = GetQueuedCompletionStatus(detail.m_hIOCP, &rw_byte, &key, &over, INFINITE);
		unsigned int my_id = static_cast<unsigned int>(key);

		if (FALSE == ret) {
			if (0 == rw_byte) {
				unsigned int gameNum = login_players[my_id].getGameNum();
				if (Games.find(gameNum) == Games.end())
				{
					if (gameNum != 0)
					{
						std::cout << "Error!!" << gameNum << "방이 존재하지 않습니다." << std::endl;
					}
				}
				else
				{
					if (Games[gameNum].erasePlayer(my_id))
					{
						std::cout << gameNum << "방의 " << my_id << " 플레이어를 삭제하였습니다." << std::endl;
						auto players = Games[gameNum].getPlayers();
						if (players[0].id == NULL && players[1].id == NULL)
						{
							std::cout << gameNum << "에 플레이어가 존재하지 않습니다. 방을 삭제합니다." << std::endl;
							while (true)
							{
								bool before = true;
								bool after = false;
								if (Games[gameNum].CAS_state(before, after))
									break;
							}
						}
					}
					else
					{
						//std::cout << "Error!!" << gameNum << "방의 " << my_id << " 플레이어가 존재하지 않아 삭제하지 못했습니다." << std::endl;
						continue;
					}
				}
				g_mutex_login_players.lock();
				login_players.erase(my_id);
				g_mutex_login_players.unlock();
				std::cout << "login_players에서 " << my_id << "를 삭제하였습니다." << std::endl;
				continue;
			}
			print_error("GQCS", WSAGetLastError());
		}

		EXP_OVER* e_over = reinterpret_cast<EXP_OVER*>(over);
		switch (e_over->c_op)
		{
		case C_ACCEPT:
		{
			SOCKET client_s = e_over->sock;
			CreateIoCompletionPort(reinterpret_cast<HANDLE>(client_s), detail.m_hIOCP, detail.id, 0);
			g_mutex_login_players.lock();
			login_players.try_emplace(detail.id, detail.id, client_s, PS_LOBBY);
			g_mutex_login_players.unlock();
			login_players[detail.id].do_recv();

			sc_packet_login login(client_s);
			login_players[detail.id++].send_packet(reinterpret_cast<packet_base*>(&login));
			std::cout << "[" << detail.id - 1 << "," << client_s << "] Login" << std::endl;

			client_s = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
			e_over->sock = client_s;
			ZeroMemory(&e_over->over, sizeof(e_over->over));
			int addr_size = sizeof(SOCKADDR_IN);
			AcceptEx(server_s, client_s, e_over->buf, 0, addr_size + 16, addr_size + 16, nullptr, &e_over->over);
			break;
		}
		case C_RECV:
		{
			int current_size = 0;
			int total_recv_packet_size = rw_byte + login_players[my_id].get_prev_packet_size();
			while (current_size < total_recv_packet_size)
			{
				auto base = reinterpret_cast<packet_base*>(&e_over->buf[current_size]);
				char packet_size = base->getSize();
				if (packet_size + current_size > total_recv_packet_size)
				{
					login_players[my_id].set_prev_packet_size(total_recv_packet_size - current_size);
					login_players[my_id].pull_recv_buf(current_size);
					login_players[my_id].do_recv();
					continue;
				}
				else
				{
					process_packet(my_id, e_over);
					current_size += packet_size;
				}
			}
			login_players[my_id].set_prev_packet_size(0);
			login_players[my_id].do_recv();
			break;
		}
		case C_SEND:
		{
			delete e_over;
			break;
		}
		case C_SHUTDOWN:
			std::cout << "서버 종료 명령으로 인한 쓰레드 종료" << std::endl;
			delete e_over;
			break;
		case C_TIMER:
			using namespace std::chrono;
			auto start = steady_clock::now();
			if (Games.find(rw_byte) == Games.end()) {
				delete e_over;
				break;
			}
			Games[rw_byte].update(detail.m_bNPC, my_id);
			Games[rw_byte].compare_time();

			if (Games.find(rw_byte) != Games.end())
			{
				bool b = true;
				if (Games[rw_byte].CAS_state(b, b)) // 게임이 아직 돌아가고 있는 상태면
				{
					using namespace std::chrono;
					g_mutex_npc_timer.lock();
					//g_npc_timer.emplace(rw_byte, my_id, duration_cast<milliseconds>(start + 1s - steady_clock::now()));
					if (my_id == 1)	// 경비 npc
					{
						bool f = false;
						if (std::atomic_compare_exchange_strong(&Games[rw_byte].day, &f, false))	// 밤시간이면 업데이트 추가
						{
							//if (Games[rw_byte].getGuardGameOver() == false)
							//{
								g_npc_timer.emplace(rw_byte, my_id, milliseconds(16));
								//printf("가드 업데이트 추가\n");
							//}
						}
					}
					else if (my_id < STUDENT_SIZE + 2)	// 학생 npc
					{
						bool t = true;
						if (std::atomic_compare_exchange_strong(&Games[rw_byte].day, &t, true))		// 낮시간이면 업데이트 추가
						{
							g_npc_timer.emplace(rw_byte, my_id, milliseconds(64));
						}
					}
					g_mutex_npc_timer.unlock();
					//std::cout << "NPC[" << my_id << "] 0.008초 후 업데이트 추가" << std::endl;
				}
				else
				{
					bool f = false;
					if (std::atomic_compare_exchange_strong(&Games[rw_byte].erasing, &f, true))
					{
						Games[rw_byte].erase_start();
						Games.erase(rw_byte);
					}
				}
			}
			delete e_over;
			break;
		}
	}
}

void IOCP_SERVER_MANAGER::process_packet(const unsigned int& id, EXP_OVER*& over)
{
	packet_base* base = reinterpret_cast<packet_base*>(over->buf);
	switch (base->getType())
	{
		case pPOSITION:										//		pPOSITION,
		{
			cs_packet_position* position = reinterpret_cast<cs_packet_position*>(base);
			auto ping = std::chrono::high_resolution_clock::now() - position->sendTime;
			if (login_players[id].getPlayerState() == PlayerState::PS_LOBBY) break;

			if (Games.find(login_players[id].getGameNum()) == Games.end())break;
			auto& gameRoom = Games[login_players[id].getGameNum()];
			
			// 충돌체크 하지 않고 위치 전달
			gameRoom.setPlayerPR(id, position);

			//short floor = floor_collision(position);
			float floor = floor_collision(id, gameRoom);
			if (floor < 6)
			{
				if (detail.m_bLog)
					std::cout << id << "가 " << floor << "층으로 이동" << std::endl;
				gameRoom.setFloor(id, floor);
			}

			sc_packet_position after_pos(position->getID(), position->getPosition(), position->getRotation(), position->getSpeed());

			// 게임 방 내의 플레이어들 모두에게 패킷 전송
			Player* players = gameRoom.getPlayers();
			for (int i = 0; i < 2; ++i)
			{
				if (players[i].id && players[i].id != id)
				{
					login_players[players[i].id].send_packet(reinterpret_cast<packet_base*>(&after_pos));
				}
			}

			DirectX::XMFLOAT3 pos = position->getPosition();
			DirectX::XMFLOAT3 rot = position->getRotation();

			if(detail.m_bLog)
				std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(ping)<<" [" << id << ", " << login_players[id].getSock() << "] : ( " << pos.x << ", " << pos.y << ", " << pos.z << "), (" << rot.x << ", " << rot.y << ", " << rot.z << ")" << std::endl;
			
		}
			break;
		case pLOGIN:										//		pLOGIN,
			std::cout << "오류 발생 ["<<id<<" , "<<login_players[id].getSock()<<"] 로 부터 pLOGIN 형태의 패킷 수신" << std::endl;
			break;
		case pMAKEROOM:										//		pMAKEROOM
		{
			if (Games.try_emplace(currentRoom, currentRoom).second == false)
				break;
			Games[currentRoom].init(id, login_players[id].getSock());
			login_players[id].setState(PS_GAME);
			login_players[id].setGameNum(currentRoom);
			std::cout << "[" << id << ", " << login_players[id].getSock() << "] 이용자가 " << currentRoom << "방을 생성하였습니다." << std::endl;

			sc_packet_make_room make(currentRoom);
			login_players[id].send_packet(reinterpret_cast<packet_base*>(&make));

			g_mutex_npc_timer.lock();
			//g_npc_timer.emplace(currentRoom, 1, std::chrono::milliseconds(0));
			for (int npc_id = 2; npc_id < STUDENT_SIZE + 2; ++npc_id)//STUDENT_SIZE + 2
			{
				g_npc_timer.emplace(currentRoom, npc_id, std::chrono::milliseconds(0));
				printf("%d 추가 완료\n", npc_id);
			}
			g_mutex_npc_timer.unlock();

			++currentRoom;
			break;
		}
		case pENTERROOM:										//		pENTERROOM
		{
			cs_packet_enter_room* cs_enter = reinterpret_cast<cs_packet_enter_room*>(base);
			const unsigned n = cs_enter->getRoomNum();
			auto f = Games.find(n);
			if (f != Games.end()&& f->second.hasEmpty())
			{
				Games[n].init(id, login_players[id].getSock());
				Player* players = Games[n].getPlayers();
				login_players[id].setState(PS_GAME);
				login_players[id].setGameNum(n);
				// 방 입장 알림 보내기 및  방 게스트에게 호스트 소켓번호 보내주기
				sc_packet_enter_room sc_enter(n, true, players[0].sock);
				login_players[id].send_packet(reinterpret_cast<packet_base*>(&sc_enter));

				std::cout << "[" << id << ", " << login_players[id].getSock() << "] 이용자가 " << n << "방에 입장하였습니다." << std::endl;
				
				// 방 호스트에게 게스트 소켓번호 보내주기
				sc_packet_room_player htog(players[1].sock);
				login_players[players[0].id].send_packet(reinterpret_cast<packet_base*>(&htog));
				
				Games[n].setBeiginTime();
			}
			else
			{
				sc_packet_enter_room sc_enter(n, false, NULL);
				login_players[id].send_packet(reinterpret_cast<packet_base*>(&sc_enter));
			}
		}
			break;
		case pRoomPlayer:
			std::cout << "오류: 게임에 들어가있는 멤버들을 전송하는 패킷이 서버로 들어왔습니다." << std::endl;
			break;
		case pLogout:
			break;
		case pAttack:
			break;
		case pAnimation:
		{
			auto packet = reinterpret_cast<cs_packet_anim_type*>(base);
			auto sent_player = login_players.find(id);
			//printf("[%d] Sent Animation Change\n", id);
			if (Games.find(sent_player->second.getGameNum()) == Games.end())break;
			auto InGamePlayers = Games[sent_player->second.getGameNum()].getPlayers();
			if (InGamePlayers[0].id == id)
			{
				if (InGamePlayers[1].id)
				{
					sc_packet_anim_type anim(InGamePlayers[0].sock, packet->getAnimType());
					login_players[InGamePlayers[1].id].send_packet(reinterpret_cast<packet_base*>(&anim));
				}
			}
			else if (InGamePlayers[1].id == id)
			{
				if (InGamePlayers[0].id)
				{
					sc_packet_anim_type anim(InGamePlayers[1].sock, packet->getAnimType());
					login_players[InGamePlayers[0].id].send_packet(reinterpret_cast<packet_base*>(&anim));
				}
			}
			else
			{
				//std::cout << "오류: 게임에 " << id << "에 해당하는 플레이어가 존재하지 않습니다." << std::endl;
			}
			break;
		}
		case pOpenDoor:
		{
			auto sent_player = login_players.find(id);
			if (Games.find(sent_player->second.getGameNum()) == Games.end())break;
			auto InGamePlayers = Games[sent_player->second.getGameNum()].getPlayers();
			printf("%d로부터 오픈패킷 수신\n", id);
			for (int i = 0; i < 2; ++i)
			{
				if (InGamePlayers[i].id)
				{
					login_players[InGamePlayers[i].id].send_packet(base);
					printf("%d -> %d 오픈패킷 송신\n", id, InGamePlayers[i].id);
				}
				else
				{
					//std::cout << "오류: 게임에 " << id << "에 해당하는 플레이어가 존재하지 않습니다." << std::endl;
				}
			}
			break;
		}
		case pUnlockDoor:
		{
			auto sent_player = login_players.find(id);
			if (Games.find(sent_player->second.getGameNum()) == Games.end())break;
			auto InGamePlayers = Games[sent_player->second.getGameNum()].getPlayers();
			printf("%d로부터 언락 수신\n", id);
			if (InGamePlayers[0].id == id)
			{
				if (InGamePlayers[1].id)
				{
					login_players[InGamePlayers[1].id].send_packet(base);
				}
			}
			else if (InGamePlayers[1].id == id)
			{
				if (InGamePlayers[0].id)
				{
					login_players[InGamePlayers[0].id].send_packet(base);
				}
			}
			else
			{
				std::cout << "오류: 게임에 " << id << "에 해당하는 플레이어가 존재하지 않습니다." << std::endl;
			}
			break;
		}
		case pGetItem:
		{
			auto packet = reinterpret_cast<cs_packet_get_item*>(base);
			auto sent_player = login_players.find(id);
			if (Games.find(sent_player->second.getGameNum()) == Games.end())break;
			auto InGamePlayers = Games[sent_player->second.getGameNum()].getPlayers();
			if (InGamePlayers[0].id == id)
			{
				if (InGamePlayers[1].id)
				{
					sc_packet_get_item item(InGamePlayers[0].sock, packet->getItemID(), packet->getSlotID());
					login_players[InGamePlayers[1].id].send_packet(reinterpret_cast<packet_base*>(&item));
				}
			}
			else if (InGamePlayers[1].id == id)
			{
				if (InGamePlayers[0].id)
				{
					sc_packet_get_item item(InGamePlayers[1].sock, packet->getItemID(), packet->getSlotID());
					login_players[InGamePlayers[0].id].send_packet(reinterpret_cast<packet_base*>(&item));
				}
			}
			else
			{
				std::cout << "오류: 게임에 " << id << "에 해당하는 플레이어가 존재하지 않습니다." << std::endl;
			}
		}
			break;
		case pKeyInput:
		{
			auto packet = reinterpret_cast<cs_packet_key_input*>(base);
			auto sent_player = login_players.find(id);
			if (Games.find(sent_player->second.getGameNum()) == Games.end())break;
			auto InGamePlayers = Games[sent_player->second.getGameNum()].getPlayers();
			if (InGamePlayers[0].id == id)
			{
				if (InGamePlayers[1].id)
				{
					sc_packet_key_input input(InGamePlayers[0].sock, packet->getKeyState(), packet->getGameInput());
					login_players[InGamePlayers[1].id].send_packet(reinterpret_cast<packet_base*>(&input));
					//printf("[%d] -> [%d] Sent KeyInput\n", id, InGamePlayers[1].id);
				}
			}
			else if (InGamePlayers[1].id == id)
			{
				if (InGamePlayers[0].id)
				{
					sc_packet_key_input input(InGamePlayers[1].sock, packet->getKeyState(), packet->getGameInput());
					login_players[InGamePlayers[0].id].send_packet(reinterpret_cast<packet_base*>(&input));
					//printf("[%d] -> [%d] Sent KeyInput\n", id, InGamePlayers[0].id);
				}
			}
			else
			{
				std::cout << "오류: 게임에 " << id << "에 해당하는 플레이어가 존재하지 않습니다." << std::endl;
			}
			break;
		}
		case pSound:
		{
			auto packet = reinterpret_cast<cs_packet_sound_start*>(base);
			auto sent_player = login_players.find(id);
			auto gameNum = sent_player->second.getGameNum();
			if (Games.find(gameNum) == Games.end())
				break;
			auto InGamePlayers = Games[gameNum].getPlayers();
			
			if (detail.m_bNPC)
				Games[gameNum].can_hear(packet->getPosition());

			if (packet->getType() > SOUND_TYPE::FOOTPRINT)
			{
				if (InGamePlayers[0].id == id)
				{
					if (InGamePlayers[1].id)
					{
						login_players[InGamePlayers[1].id].send_packet(base);
					}
				}
				else if (InGamePlayers[1].id == id)
				{
					if (InGamePlayers[0].id)
					{
						login_players[InGamePlayers[0].id].send_packet(base);
					}
				}
				else
				{
					std::cout << "오류: 게임에 " << id << "에 해당하는 플레이어가 존재하지 않습니다." << std::endl;
				}
			}
			break;
		}
		case pEnding:
		{
			auto packet = reinterpret_cast<sc_packet_ending*>(base);
			auto sent_player = login_players.find(id);
			auto gameNum = sent_player->second.getGameNum();
			if (Games.find(gameNum) == Games.end())
				break;
			auto InGamePlayers = Games[gameNum].getPlayers();
			printf("%d가 보낸 엔딩패킷 수신\n", id);
			for (int i = 0; i < 2; ++i)
			{
				if (InGamePlayers[i].id)
				{
					login_players[InGamePlayers[i].id].send_packet(base);
					printf("%d -> %d 엔딩패킷 전송\n", id, InGamePlayers[i].id);
					login_players[InGamePlayers[i].id].setGameNum(0);
					Games[gameNum].reset_player(InGamePlayers[i].id);
				}
				else
				{
					std::cout << "오류: 게임에 " << id << "에 해당하는 플레이어가 존재하지 않습니다." << std::endl;
				}
			}
			Games[gameNum].setGuardGameOver(true);
			//Games.erase(gameNum);
			break;
		}
		default:
			std::cout << "정의되지 않은 패킷 타입" << std::endl;
			break;
	}
}

void IOCP_SERVER_MANAGER::LoadResources()
{
	std::cout << "장애물 로드 시작" << std::endl;
	for (const auto& file : std::filesystem::directory_iterator("Resource/"))
	{
		const auto fileName = file.path();
		std::ifstream meshFile(fileName, std::ios::binary);
		if (meshFile.is_open() == false) {
			std::cout << "Failed to open mesh file!! fileName: " << fileName << std::endl;
		}
		Mesh* temp = new Mesh;
		temp->LoadMeshData(meshFile);
		m_vMeshes.emplace_back(temp);
		std::cout << fileName << "로드 완료" << std::endl;
	}
	std::cout << "장애물 로드 완료" << std::endl;
	//------------------------------------------------------------------------------------------
	// astar 노드 npc마다 적용하기 위해 노드를 멤버변수로 할당
	//std::cout << "A* NODE 로드 시작" << std::endl;
	//MakeGraph();
	////for (auto& node : g_um_graph)
	////{
	////	std::cout << node.first << " 노드 생성 완료" << std::endl;
	////}
	//std::cout << "A* NODE 로드 완료" << std::endl;
}

void IOCP_SERVER_MANAGER::command_thread()
{
	std::unordered_map<std::string, std::function<void()>> commands = {		// 이곳에 추가하고 싶은 명령어 기입 { 명령어 , 람다 }
		{"/STOP",[this]() {
			detail.m_bServerState = false;
			EXP_OVER* over = new EXP_OVER;
			over->c_op = C_SHUTDOWN;
			int num_threads = std::thread::hardware_concurrency();
			for (int i = 0; i < num_threads; ++i)
				PostQueuedCompletionStatus(detail.m_hIOCP, 1, -1, reinterpret_cast<OVERLAPPED*>(&over));
		}}
		,
		{"/LOG",[this]() {
			detail.m_bLog = !detail.m_bLog;
			std::cout << "LOG 출력 : " << detail.m_bLog << std::endl;
		}}
		,
		{"/NPC",[this]() {
			detail.m_bNPC = !detail.m_bNPC;
			std::cout << "NPC 플레이어 추적 : " << detail.m_bNPC << std::endl;
		}}
		,
		{"/HELP",[this]() {
			std::cout << "-------------------------------------------------------------------" << std::endl;
			std::cout << "/STOP : 서버 종료\n/LOG: 로그 출력\n/NPC: NPC가 플레이어 추적" << std::endl;
			std::cout << "-------------------------------------------------------------------" << std::endl;
		}}
		,
		{"/STATE",[this]() {
			g_mutex_login_players.lock();
			for (auto& player : login_players)
			{
				std::cout << "[" << player.first << "]" << std::endl;
			}
			g_mutex_login_players.unlock();
		}},
		{"/TIME",[this]()
		{
			for (auto& game : Games)
			{
				game.second.setBeiginTime();
				game.second.addBeiginTime(std::chrono::minutes(3));
			}
		}},
		{"/GUARD",[this]()
		{
			for (auto& game : Games)
			{
				game.second.respawn_guard();
				printf("가드 리스폰");
			}
		}},
		{"/DEST",[this]()
		{
			int input;
			printf("목표 층을 설정하세요: ");
			std::cin >> input;
			for (auto& game : Games)
			{
				game.second.set_guard_destination(input);
			}
		}},
		{"/REBOOT",[this]()
		{
			detail.id = 1;
			currentRoom = 10000;
			g_mutex_login_players.lock();
			login_players.clear();
			g_mutex_login_players.unlock();
			Games.clear();
			printf("서버 재시작 완료\n");
		}}

	};

	while (detail.m_bServerState)
	{
		std::string input;
		std::cin >> input;

		if (commands.find(input) == commands.end())
		{
			std::cout << "해당 명령어가 존재하지 않습니다." << std::endl;
		}
		else commands[input]();
	}
}

void IOCP_SERVER_MANAGER::ai_thread()
{
	using namespace std::chrono;
	while (detail.m_bServerState)
	{
		auto start_t = system_clock::now();
		EXP_OVER* over = new EXP_OVER;
		over->c_op = C_TIMER;
		PostQueuedCompletionStatus(detail.m_hIOCP, 1, 0, &over->over);
		auto end_t = system_clock::now();
		auto hb_time = end_t - start_t;
		if (hb_time < 0.008s)
			std::this_thread::sleep_for(0.008s - hb_time);
		//if (hb_time < 3s)
		//	std::this_thread::sleep_for(3s - hb_time);
	}
}

void IOCP_SERVER_MANAGER::ai_timer()
{
	using namespace std::chrono;
	while (detail.m_bServerState)
	{
		if (g_npc_timer.empty())
		{
			//std::this_thread::sleep_for(0.008s);
			continue;
		}
		else
		{
			g_mutex_npc_timer.lock();
			auto q = g_npc_timer.top();
			if (q.start_time < steady_clock::now())
			{
				g_npc_timer.pop();
				g_mutex_npc_timer.unlock();
				EXP_OVER* over = new EXP_OVER;
				over->c_op = C_TIMER;
				PostQueuedCompletionStatus(detail.m_hIOCP, q.gameNum, q.id, &over->over);
				/*g_npc_timer.emplace(q.gameNum, q.id, steady_clock::now() + 5s);*/				// GQCS작업이 완료되면 타이머를 추가
			}
			else
			{
				g_mutex_npc_timer.unlock();
			}
		}
	}
}

unsigned short IOCP_SERVER_MANAGER::floor_collision(cs_packet_position*& packet)
{
	DirectX::XMFLOAT3 pivot = packet->getPosition();
	DirectX::XMFLOAT3 temp_extents = { 40,50,40 };
	DirectX::XMFLOAT4 temp_quarta = { 0,0,0,1 };
	DirectX::BoundingOrientedBox player(pivot, temp_extents, temp_quarta);
	
	DirectX::XMFLOAT3 trash;
	short floor;
	m_vMeshes[0]->m_Childs[0].m_Childs[0].floor_collision(player, &trash, &trash, &floor);
	return floor;
}

float IOCP_SERVER_MANAGER::floor_collision(const unsigned int& id, Game& gameRoom)
{
	DirectX::BoundingOrientedBox player_obb;
	gameRoom.getPlayerOBB(player_obb, id);
	
	DirectX::XMFLOAT3 trash;
	float now_floor = 0;
	auto& map_floors = m_vMeshes[0]->m_Childs[0].m_Childs[0].m_Childs;
	for (auto& floor : map_floors)
	{
		if (floor.floor_collision(player_obb, now_floor))
			break;
	}
	return now_floor;
}

void Game::init(const unsigned int& i, const SOCKET& s)
{
	if (player[0].id == NULL)
	{
		player[0].id = i;
		player[0].sock = s;
		player[0].make_obb();
	}
	else
	{
		player[1].id = i;
		player[1].sock = s;
		player[1].make_obb();
	}
}

const DirectX::XMFLOAT3 Game::getPlayerPos(const unsigned int& id)
{
	for (auto& p : player)
	{
		if (p.id == id)
		{
			return p.position;
		}
	}
}

void Game::setPlayerPR(const unsigned int& id, cs_packet_position*& packet)
{
	for (auto& p : player)
	{
		if (p.id == id)
		{
			if (packet->getID() == p.sock)
			{
				p.position = packet->getPosition();
				p.rotation = packet->getRotation();
				p.speed = packet->getSpeed();
			}
			break;
		}
	}
}

void Game::setPlayerPR_v2(const unsigned int& id, cs_packet_position*& packet, const DirectX::XMFLOAT3& newSpeed)
{
	for (auto& p : player)
	{
		if (p.id == id)
		{
			p.position = packet->getPosition();
			if (p.position.y < 0) p.position.y = 0;
			p.rotation = packet->getRotation();
			p.speed = newSpeed;
			break;
		}
	}
}

void Game::setPlayerPR_v3(const unsigned int& id, const DirectX::XMFLOAT3& newPosition, const DirectX::XMFLOAT3& newSpeed, const DirectX::XMFLOAT3& rot, const unsigned short& floor)
{
	for (auto& p : player)
	{
		if (p.id == id)
		{
			p.position = newPosition;
			p.rotation = rot;
			p.speed = newSpeed;
			p.m_floor = floor;
			break;
		}
	}
}

void Game::setPlayerRot(const unsigned int& id, cs_packet_position*& packet)
{
	for (auto& p : player)
	{
		if (p.id == id)
		{
			p.rotation = packet->getRotation();
			p.speed = DirectX::XMFLOAT3(0, 0, 0);
			break;
		}
	}
}

void Game::setPlayerRotSpeed(const unsigned int& id, cs_packet_position*& packet, DirectX::XMFLOAT3& newSpeed)
{
	for (auto& p : player)
	{
		if (p.id == id)
		{
			p.rotation = packet->getRotation();
			p.speed = newSpeed;
			DirectX::XMFLOAT3 pos = packet->getPosition();
			if (newSpeed.x == 0)
			{
				p.position.y = pos.y;
				p.position.z = pos.z;
			}
			else if (newSpeed.y == 0)
			{
				p.position.x = pos.x;
				p.position.z = pos.z;
			}
			else if (newSpeed.z == 0)
			{
				p.position.x = pos.x;
				p.position.y = pos.y;
			}
			if (p.position.y < 0) p.position.y = 0;
			break;
		}
	}
}

const DirectX::XMFLOAT3 Game::getPlayerRot(const unsigned int& id)
{
	for (auto& p : player)
	{
		if (p.id == id)
		{
			return p.rotation;
		}
	}
}

const DirectX::XMFLOAT3 Game::getPlayerSp(const unsigned int& id)
{
	for (auto& player : player)
	{
		if (player.id == id)
		{
			return player.speed;
		}
	}
}

void Game::update(const bool& npc_state)
{	// 아무리 생각해도 npc 별로 타이머를 써서 update하는게 더 나아보인단 말이지
	//bool f = false;
	//if (std::atomic_compare_exchange_strong(&guard.updating, &f, true))
	//{
	//	guard.guard_state_machine(player, npc_state);
	//}
	////std::cout << "npc 회전: " << guard.rotation.y << std::endl;
	//for (auto& student : students)
	//{
	//	if(std::atomic_compare_exchange_strong(&student.updating,&f,true))	// 업데이트 중인데 다른 쓰레드에서 update함수가 불려서 2중 업데이트를 하지 못하게 방지
	//		student.student_state_machine(player);
	//}
	//sc_packet_position guard_position(guard.id, guard.position, guard.rotation, guard.speed);
	//for (auto& p : player)
	//{
	//	if (p.sock != NULL)
	//	{
	//		if (login_players.end() != login_players.find(p.id))
	//		{
	//			login_players[p.id].send_packet(reinterpret_cast<packet_base*>(&guard_position));
	//			for (auto& student : students)
	//			{
	//				sc_packet_position student_position(student.id, student.position, student.rotation, student.speed);
	//				login_players[p.id].send_packet(reinterpret_cast<packet_base*>(&student_position));
	//			}
	//		}
	//		else
	//			p.sock = NULL;
	//	}
	//}
}

bool Game::erasePlayer(const unsigned int& id)
{
	for (int i = 0; i < 2; ++i)
	{
		if (player[i].id == id)
		{
			if (player[1 - i].id == 0) {
				player[i].reset();
				return true;
			}
			sc_packet_logout logout(player[i].sock);

			login_players[player[1 - i].id].send_packet(reinterpret_cast<packet_base*>(&logout));
			player[i].reset();
			return true;
		}
	}
	return false;
}

void Game::setFloor(const unsigned int& id, const float &floor)
{
	for (auto& p : player)
	{
		if (p.id == id)
		{
			p.m_floor = floor;
			return;
		}
	}
}

bool Game::hasEmpty()
{
	for (int i = 0; i < 2; ++i)
	{
		if (player[i].id == NULL)
			return true;
	}
	return false;
}

bool Game::CAS_state(bool& before, bool& after)
{
	return std::atomic_compare_exchange_strong(&state, &before, after);
}

bool Game::getPlayerOBB(DirectX::BoundingOrientedBox& out, const unsigned int& id)
{
	for (auto& p : player)
	{
		if (p.id == id)
		{
			float pitch = DirectX::XMConvertToRadians(p.rotation.x); // x축을 기준으로 회전
			float yaw = DirectX::XMConvertToRadians(p.rotation.y);   // y축을 기준으로 회전
			float roll = DirectX::XMConvertToRadians(p.rotation.z);  // z축을 기준으로 회전
			DirectX::XMVECTOR player_rotation = DirectX::XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);
			DirectX::XMVECTOR player_location = DirectX::XMLoadFloat3(&p.position);
			p.obb.Transform(out, 1, player_rotation, player_location);
			return true;
		}
	}
	return false;
}

void Game::update(const bool& npc_state, const unsigned int& npc_id)
{
	using namespace std::chrono;
	if (npc_id == 1)		// guard
	{
		guard.guard_state_machine(player, npc_state);
		if (guard.gameover == true) {	// 체포되어 gameover 가 되면
			bool t = true;
			while (true) {
				if (std::atomic_compare_exchange_strong(&this->state, &t, false))	//Game의 state를 0으로 바꿔
					break;
			}
			return;
		}
		sc_packet_position guard_position(guard.id, guard.position, guard.rotation, guard.speed);
		for (auto& p : player)
		{
			if (p.sock != NULL)
			{
				if (login_players.end() != login_players.find(p.id))
				{
					login_players[p.id].send_packet(reinterpret_cast<packet_base*>(&guard_position));
				}
				else
					p.sock = NULL;
			}
		}
	}
	else if (npc_id < STUDENT_SIZE + 2)	// student
	{
		auto& student = students[npc_id - 2];
		if (student.student_state_machine(player))
		{
			sc_packet_position student_position(student.id, student.position, student.rotation, student.speed);
			for (auto& p : player)
			{
				if (p.sock != NULL)
				{
					if (login_players.end() != login_players.find(p.id))
					{
						login_players[p.id].send_packet(reinterpret_cast<packet_base*>(&student_position));
					}
					else
						p.sock = NULL;
				}
			}
		}
	}
	else
	{
		std::cout << "잘못된 NPC아이디 입니다." << std::endl;
	}
}

void Game::setBeiginTime()
{
	using namespace std::chrono;
	begin_time = steady_clock::now();
}
void Game::addBeiginTime(std::chrono::steady_clock::duration time)
{
	using namespace std::chrono;
	begin_time -= time;
}

void Game::can_hear(const DirectX::XMFLOAT3& sound_position)
{
	if (this->day == false)
	{
		if (guard.can_hear(sound_position))
		{
			//std::cout << sound_position.x << "," << sound_position.y << "," << sound_position.z << " 들어버렸다..." << std::endl;
		}
		else
		{
			//std::cout << "멀어서 안들령~" << std::endl;
		}
	}
}

void Game::respawn_guard()
{
	guard.reset_npc();
	//guard.position = { 3160, 0, -400 };
	guard.position = DirectX::XMFLOAT3(-427, 1769.8, -1249.1);
	guard.rotation = { 0, 0, 0 };
	guard.speed = { 0,0,0 };
	guard.destination = { 0,0,0 };
	guard.movement_speed = 300;
	guard.state = 0;
	guard.aggro_type = 2;
}

void Game::set_guard_destination(const int& floor)
{
	guard.set_manual_destination(floor);
}

bool Game::isDay()
{
	using namespace std::chrono;
	if (this->begin_time.time_since_epoch() < nanoseconds(1))
		return true;
	return begin_time + minutes(3) > steady_clock::now();		// 낮 시간 3분
	//return begin_time + seconds(30) > steady_clock::now();		// 낮 시간 20초
}

void Game::reset_player(const int& id)
{
	for (int i = 0; i < 2; ++i)
	{
		if (player[i].id == id)
		{
			player[i].reset();
		}
	}
}

void Game::compare_time()
{
	using namespace std::chrono;
	bool t = true;
	if (std::atomic_compare_exchange_strong(&day, &t, true))		// 낮시간이였는데
	{
		if (this->begin_time.time_since_epoch() > nanoseconds(1) && !isDay())	// 측정해보니 밤이 되었다면?
		{
			bool f = false;
			if (std::atomic_compare_exchange_strong(&changing_time, &f, true))	// 젤 먼저 들어온 애가 패킷 1회만 전송하고 npc 죽일거야! 나머진 나가!
			{
				sc_packet_change_day_or_night change(22);
				for (auto& p : player)
				{
					if (p.sock != NULL)
					{
						if (login_players.end() != login_players.find(p.id))
						{
							login_players[p.id].send_packet(reinterpret_cast<packet_base*>(&change));
						}
						else
							p.sock = NULL;
					}
				}
				begin_time = steady_clock::now() + minutes(5);			// 타임오버 시간으로 수정해야함. 밤 시간 5분
				//begin_time = steady_clock::now() + seconds(20);			// 타임오버 시간으로 수정해야함. 밤 시간 5분
				std::atomic_compare_exchange_strong(&day, &t, false);
				for (auto& bot : students)
				{
					while (true)
					{
						if (std::atomic_compare_exchange_strong(&bot.running, &f, true))		// 모든 봇이 run하고 있지 않다가 내가 run 시켜두면 다른곳에서 run 할 일이 없지
						{
							std::atomic_compare_exchange_strong(&bot.gameover, &f, true);		// 거기다가 student의 gameover를 true로 해서 위에서 running으로 잡아두고, gameover까지 모두 true 면 student는 죽은 목숨
							break;
						}
					}
				}
				g_npc_timer.emplace(this->GameNum, 1, std::chrono::milliseconds(0));
				std::cout << "학생 죽이고, 가드 살리기 완료" << std::endl;
			}
		}
	}
	else
	{
		if (begin_time < steady_clock::now())	// 밤시간에 타임오버 시간 측정
		{
			bool bt = true;
			if (std::atomic_compare_exchange_strong(&state, &bt, false))
			{
				//std::cout << "타임아웃이다 이놈아" << std::endl;
				sc_packet_ending ending(1);
				for (auto& p : player)
				{
					if (p.sock != NULL)
					{
						if (login_players.end() != login_players.find(p.id))
						{
							login_players[p.id].send_packet(reinterpret_cast<packet_base*>(&ending));
							login_players[p.id].setGameOver();
						}
						p.reset();
					}
				}
			}
		}
	}
}

void Game::erase_start()
{
	for (auto& bot : students)
	{
		while (true) {
			if (bot.gameover && bot.running)		//학생의 running과 gameover 둘 다 true라는 뜻은 학생은 더 이상 업데이트가 이루어지지 않는 죽은 목숨
				break;
		}
	}
	//학생들은 모두 죽은 목숨이고
	// 가드의 행동이 끝났는지 확인할 차례
	bool f = false;
	while (true)
	{
		if (guard.gameover && !guard.running)		//가드는 잡는 즉시 게임이 끝이기에 가드가 돌아가고 있을 확률은 0
			break;
	}

}

NPC::NPC()
{
	DirectX::XMFLOAT3 basic_obb_position = { 0,0,0 };	//0, 85, 0
	DirectX::XMFLOAT3 basic_obb_extents = basic_extents;
	DirectX::XMFLOAT4 basic_obb_orients = { 0,0,0,1 };
	obb = DirectX::BoundingOrientedBox(basic_obb_position, basic_obb_extents, basic_obb_orients);
	movement_speed = 1;
	MakeGraph(astar_graph);
}

void NPC::guard_state_machine(Player* p,const bool& npc_state)
{
	bool f = false;
	if (std::atomic_compare_exchange_strong(&running, &f, true))
	{
		std::chrono::duration<double> delta;
		if (lastTime.time_since_epoch().count() == 0)		// 시작
		{
			lastTime = std::chrono::steady_clock::now();
		}
		auto currentTime = std::chrono::steady_clock::now();
		delta = currentTime - lastTime;
		lastTime = currentTime;

		DirectX::BoundingOrientedBox now_obb;

		float pitch = DirectX::XMConvertToRadians(this->rotation.x); // x축을 기준으로 회전
		float yaw = DirectX::XMConvertToRadians(this->rotation.y);   // y축을 기준으로 회전
		float roll = DirectX::XMConvertToRadians(this->rotation.z);  // z축을 기준으로 회전
		DirectX::XMVECTOR guard_rotation = DirectX::XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);

		DirectX::XMVECTOR guard_location = DirectX::XMLoadFloat3(&this->position);
		this->obb.Transform(now_obb, 1, guard_rotation, guard_location);
		DirectX::XMFLOAT3 temp;

		float now_floor = 0;
		auto& map_floors = m_vMeshes[0]->m_Childs[0].m_Childs[0].m_Childs;

		bool b_stair = false;
		auto map_floors_cnt = map_floors.size();

		auto& stairs = map_floors[map_floors_cnt - 1].m_Childs;

		for (int i = 0; i < 4; ++i)
		{	// 계단의 방향 +Z, -Z, +X, -X
			if (stairs[i].stair_collision(now_obb, this->position.y, i))
			{
				b_stair = true;
				break;
			}

		}

		if (b_stair == false)		// 계단과 충돌하지 못했다면 바닥과 충돌중인지 확인
		{
			for (int i = 0; i < map_floors_cnt - 1; ++i)
			{
				auto& floor = map_floors[i];
				if (floor.floor_collision(now_obb, now_floor, this->position.y))
					break;
			}
			//std::cout << "npc: " << now_floor << std::endl;
			if (now_floor <= 5.5)
			{
				//if (m_floor != now_floor)
				//	std::cout << "npc가 " << m_floor << " -> " << now_floor << "로 이동" << std::endl;
				m_floor = now_floor;
			}
		}

		if (this->state == 0)				// 두리번 두리번 상태
		{
			if (set_destination(p, npc_state))			// 두리번 거리다가 뭔갈 발견했으면, 목적지 조정하고 이동
			{
				this->state = 1;
			}
			else
			{	// 위치를 못찾았으면
				int duribun_duribun = 5;
				auto duration = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - arrive_time).count();
				if (duration > duribun_duribun)		// 5초 넘게 두리번 거렸는데 위치를 못찾았으면
				{
					this->state = 1;
					// 가드가 사용할 astar 그래프 생성
					//std::unordered_map<int, NODE*> guard_graph;
					//MakeGraph(guard_graph);
					while (true)
					{
						reset_graph();
						//int des = rand() % astar_graph.size();
						int des = uid(dre);
						//std::cout << des << "로 목적지 설정" << std::endl;
						this->destination = astar_graph[des]->pos; // 랜덤한 목적지 설정
						this->path = aStarSearch(position, destination, astar_graph);
						if (this->path != nullptr)
							break;
					}
					this->destination = path->pos;
					this->aggro_type = 2;
				}
			}
		}
		else if (this->state == 1)	// 목적지로 이동하는 중
		{
			set_destination(p, npc_state);		// 이동 하면서도 눈으로 보이는지, 소리가 들리는지 확인
		}

		if (state == 1)
			move(delta.count());

		// 다른 캐릭터와 충돌했나?
		bool hit = false;
		DirectX::BoundingOrientedBox player_obb;
		float player_pitch;
		float player_yaw;
		float player_roll;
		DirectX::XMVECTOR player_rotation;
		DirectX::XMVECTOR player_location;
		for (int i = 0; i < 2; ++i)
		{
			if (p[i].sock == NULL)
				continue;
			player_pitch = DirectX::XMConvertToRadians(p[i].rotation.x);
			player_yaw = DirectX::XMConvertToRadians(p[i].rotation.y);
			player_roll = DirectX::XMConvertToRadians(p[i].rotation.z);
			player_rotation = DirectX::XMQuaternionRotationRollPitchYaw(player_pitch, player_yaw, player_roll);
			player_location = DirectX::XMLoadFloat3(&p[i].position);
			p[i].obb.Transform(player_obb, 1, player_rotation, player_location);
			hit = now_obb.Intersects(player_obb);
			if (hit)
			{
				sc_packet_ending busted(0);
				for (int i = 0; i < 2; ++i)
				{
					if (login_players[p[i].id].getGameNum() != 0)
					{
						login_players[p[i].id].send_packet(reinterpret_cast<packet_base*>(&busted));
						printf("%d 에게 엔딩 전송", p[i].id);
						login_players[p[i].id].setGameOver();
					}
				}
				bool f = false;
				while (true)
				{
					if (std::atomic_compare_exchange_strong(&this->gameover, &f, true))	// npc의 gameover가 true가 될때까지 루프
						break;
				}
				//std::cout << i << " 와 충돌" << std::endl;
				break;
			}
		}
		bool t = true;
		std::atomic_compare_exchange_strong(&running, &t, false);
	}
}

bool NPC::student_state_machine(Player* p)
{
	bool f = false;
	if (std::atomic_compare_exchange_strong(&running, &f, true)) {
		std::chrono::duration<double> delta;
		if (lastTime.time_since_epoch().count() == 0)		// 시작
		{
			lastTime = std::chrono::steady_clock::now();
		}
		auto currentTime = std::chrono::steady_clock::now();
		delta = currentTime - lastTime;
		lastTime = currentTime;

		DirectX::BoundingOrientedBox now_obb;

		float pitch = DirectX::XMConvertToRadians(this->rotation.x); // x축을 기준으로 회전
		float yaw = DirectX::XMConvertToRadians(this->rotation.y);   // y축을 기준으로 회전
		float roll = DirectX::XMConvertToRadians(this->rotation.z);  // z축을 기준으로 회전
		DirectX::XMVECTOR student_rotation = DirectX::XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);

		DirectX::XMVECTOR student_location = DirectX::XMLoadFloat3(&this->position);
		this->obb.Transform(now_obb, 1, student_rotation, student_location);

		auto& map_floors = m_vMeshes[0]->m_Childs[0].m_Childs[0].m_Childs;
		float now_floor = 0;
		auto map_floors_cnt = map_floors.size();
		auto& stairs = map_floors[map_floors_cnt - 1].m_Childs;
		bool b_stair = false;
		for (int i = 0; i < 4; ++i)
		{	// 계단의 방향 +Z, -Z, +X, -X
			if (stairs[i].stair_collision(now_obb, this->position.y, i))
			{
				b_stair = true;
				break;
			}

		}

		if (b_stair == false)		// 계단과 충돌하지 못했다면 바닥과 충돌중인지 확인
		{
			for (int i = 0; i < map_floors_cnt - 1; ++i)
			{
				auto& floor = map_floors[i];
				if (floor.floor_collision(now_obb, now_floor, this->position.y))
					break;
			}
			//std::cout << "npc: " << now_floor << std::endl;
			if (now_floor <= 5.5)
			{
				//if (m_floor != now_floor)
				//	std::cout << "npc가 " << m_floor << " -> " << now_floor << "로 이동" << std::endl;
				m_floor = now_floor;
			}
		}

		// 다른 캐릭터와 충돌했나?
		bool hit = false;
		DirectX::BoundingOrientedBox player_obb;
		float player_pitch;
		float player_yaw;
		float player_roll;
		DirectX::XMVECTOR player_rotation;
		DirectX::XMVECTOR player_location;
		for (int i = 0; i < 2; ++i)
		{
			if (p[i].sock == NULL)
				continue;
			player_pitch = DirectX::XMConvertToRadians(p[i].rotation.x);
			player_yaw = DirectX::XMConvertToRadians(p[i].rotation.y);
			player_roll = DirectX::XMConvertToRadians(p[i].rotation.z);
			player_rotation = DirectX::XMQuaternionRotationRollPitchYaw(player_pitch, player_yaw, player_roll);
			player_location = DirectX::XMLoadFloat3(&p[i].position);
			p[i].obb.Transform(player_obb, 1, player_rotation, player_location);
			hit = now_obb.Intersects(player_obb);
			if (hit && state != 2)
			{
				std::cout << i << " 와 충돌" << std::endl;
				break;
			}
		}

		if (hit)
		{
			if (state != 2)				// 충돌 판정이 일어났을때, 최초의 충돌이면
			{
				state = 2;
				//std::cout << "최초 충돌이 발생했다@" << std::endl;
				this->speed = DirectX::XMFLOAT3(0, 0, 0);
				using namespace std::chrono;
				attacked_time = steady_clock::now();				// 충돌한 시간을 저장
				arrive_time = steady_clock::now();
				sc_packet_npc_attack attack_student(this->id);
				for (int i = 0; i < 2; ++i)
				{
					if (p[i].sock != NULL)
					{
						login_players[p[i].id].send_packet(reinterpret_cast<packet_base*>(&attack_student));
						//std::cout << "충돌 패킷 전송 완료" << std::endl;
					}
				}
			}
			else
			{		//  충돌 되어있는 상태에서 3초가 지났다면
				using namespace std::chrono;
				if (attacked_time + 3s < steady_clock::now())		// 충돌 이후 3초가 지났으면
				{
					//std::cout << "충돌 애니메이션이 재생된지 3초가 지나 state를 목적지 도착 상태로 변경했다!" << std::endl;
					state = 0;
				}
			}
		}
		else
		{
			if (this->state == 0)			// 목적지 도착한 상태
			{
				// 2초동안 대기
				int stop = 2;
				auto duration = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - arrive_time).count();
				if (duration > stop)			// 2초 대기 이후
				{
					this->state = 1;
					// 새로운 목표 위치 선정
					// 학생이 사용할 astar 그래프 깊은 복사
					while (true)
					{
						reset_graph();
						//int des = rand() % astar_graph.size();
						int des = uid(dre);
						std::cout << "student[" << this->id << "] " << des << "로 목적지 설정" << std::endl;
						this->destination = astar_graph[des]->pos; // 랜덤한 목적지 설정
						this->path = aStarSearch(position, destination, astar_graph);
						if (this->path != nullptr)
							break;
					}
					this->destination = path->pos;
				}
			}
			else if (this->state == 1)		// 목적지로 이동하는 중
			{
				if (compare_position(this->destination))		// 목적지에 도달하였으면
				{
					if (this->path == nullptr || this->path->next == nullptr)			// 더 이상의 목적지가 없으면 두리번 상태로 변경
					{
						if (this->state != 0)
						{
							arrive_time = std::chrono::high_resolution_clock::now();	// 도달한 시각 기록
							this->state = 0;
							this->speed = DirectX::XMFLOAT3(0, 0, 0);
							std::cout << "NPC[" << id << "] 두리번 상태로 변경" << std::endl;
						}
					}
					else
					{
						path = path->next;
						destination = path->pos;
					}
				}
				else
				{
					move(delta.count());		// 목표 위치로 이동
				}
			}
			else if (this->state == 2)		// 충돌 애니메이션이 진행 중
			{
				using namespace std::chrono;
				if (attacked_time + 3s < steady_clock::now())		// 충돌 이후 3초가 지났으면
				{
					//std::cout << "충돌 애니메이션이 재생된지 3초가 지나 state를 목적지 도착 상태로 변경했다!" << std::endl;
					state = 0;
				}
			}
		}
		bool t = true;
		std::atomic_compare_exchange_strong(&running, &t, false);
		return true;
	}
	return false;
}

bool NPC::can_see(Player& p, bool& floor_gap)
{
	float npc_floor = m_floor;
	if (p.m_floor != m_floor)
	{
		//if (p.m_floor - m_floor == 0.5)			// 플레이어가 .5층에 있고 npc가 온전한 층에 있을때
		//{
		//	floor_gap = true;
		//}
		//else if (m_floor - p.m_floor == 0.5)	// 플레이어가 온전한 층에 있고, npc가 .5층에 있을때
		//{
		//	npc_floor = p.m_floor;
		//	floor_gap = true;
		//}
		//else {
		//	return false;
		//}
		return false;
	}
	else if ((m_floor - (int)m_floor) == 0.5)			// 계단 사이 .5층에 존재
	{
		floor_gap = false;
		return true;
	}


	DirectX::XMFLOAT3 playerPos = p.position;
	playerPos.y += 50; //127
	DirectX::XMFLOAT3 npcPos = position;
	npcPos.y += 50;
	for (auto& mesh : m_vMeshes[0]->m_Childs[0].m_Childs[npc_floor + 2].m_Childs)
	{
		if (mesh.sight_block(playerPos, npcPos))
		{
			// 하나의 벽이라도 막혀있다면 시야에 보이지 않는 것.
			return false;
		}
		//if (mesh->can_see(playerPos, npcPos, m_floor))		// npc -> 플레이어로의 광선과 장애물들을 ray 충돌하여 npc가 플레이어를 볼 수 있는지 확인
		//{
		//	std::cout << "보인다 보여.." << std::endl;
		//	return true;
		//}
	}
	// 모든 벽을 다 돌아봤는데 막힌게 없으면 시야에 보이는 것.
	return true;
}

bool NPC::can_hear(const DirectX::XMFLOAT3& sound_pos)
{
	const unsigned short can_hear_distance = 2500;
	
	if (distance(sound_pos) < can_hear_distance * can_hear_distance && this->aggro_type > 1)		// 플레이어가 소리를 내고 있으며, 플레이어와의 거리가 들을 수 있는 거리 이내이면
	{
		while (true)
		{
		reset_path();
		reset_graph();
		this->path = aStarSearch(position, sound_pos, astar_graph);
		if (this->path != nullptr)
			break;
		}
		compare_length_next_path(path, this->position, sound_pos);
		if (nullptr == path)
			state = 0;
		else
		{
			state = 1;
			this->destination = this->path->pos;
		}
		this->aggro_type = 1;
		return true;
	}
	return false;
}

float NPC::distance(const DirectX::XMFLOAT3& sound_pos)
{
	return (position.x - sound_pos.x) * (position.x - sound_pos.x) + (position.y - sound_pos.y) * (position.y - sound_pos.y) + (position.z - sound_pos.z) * (position.z - sound_pos.z);
}

bool NPC::compare_position(DirectX::XMFLOAT3& pos)
{
	float error_value = 10;
	if (this->position.x >= pos.x - error_value && this->position.x <= pos.x + error_value)
		if (this->position.y >= pos.y - error_value*5 && this->position.y <= pos.y + error_value*5)
			if (this->position.z >= pos.z - error_value && this->position.z <= pos.z + error_value)
			{
				if (state == 1)
				{
					//std::cout <<"NPC["<<this->id<<"] 목적지 도착" << std::endl;
					return true;
				}
			}
	return false;
}

bool NPC::set_destination(Player*& p, const bool& npc_state)
{
	short n = find_near_player(p);
	if (npc_state) {
		//for (int i = 0; i < 2; ++i)
		//{
		//	if (p[n].sock == NULL)
		//	{
		//		n = 1 - n;
		//		continue;
		//	}
			bool floor_gap = false;
			if (can_see(p[n], floor_gap))
			{	// 보이면 그냥 돌진
				std::cout << n << "P 발견" << std::endl;
				if (floor_gap)			// 계단을 거쳐 가야함으로 astar 사용
				{
					while (true)
					{
						reset_path();
						reset_graph();
						this->path = aStarSearch(position, p[n].position, astar_graph);
						if (path != nullptr)
							break;
					}
					//compare_length_next_path(path, this->position, p[n].position);
					if (this->path == nullptr)
						state = 0;
					else
					{
						this->destination = this->path->pos;
					}
				}
				else
				{
					while (true)
					{
						reset_path();
						reset_graph();
						this->path = aStarSearch(position, p[n].position, astar_graph);
						if (path != nullptr)
							break;
					}
					//compare_length_next_path(path, this->position, p[n].position);
					if (this->path == nullptr)
						state = 0;
					else
					{
						//this->destination = this->path->pos;
						this->destination = p[n].position;
					}
					//reset_path();
					//this->destination = p[n].position;
				}
				this->aggro_type = 0;
				return true;
			}
		//	n = 1 - n;
		//}
	}

	// 플레이어를 찾지도, 듣지도 못했다면
	if (compare_position(this->destination))		// 목적지에 도달하였으면
	{
		if (this->path == nullptr || this->path->next == nullptr)			// 더 이상의 목적지가 없으면 두리번 상태로 변경
		{
			if (this->state != 0)
			{
				arrive_time = std::chrono::high_resolution_clock::now();	// 도달한 시각 기록
				this->state = 0;
				this->speed = DirectX::XMFLOAT3(0, 0, 0);
				std::cout << "두리번 상태로 변경" << std::endl;
				this->aggro_type = 2;
			}
		}
		else
		{
			PATH* next = path->next;
			delete path;
			path = next;
			destination = path->pos;
		}
	}
	return false;
}

void NPC::move(const double& time)
{
	using namespace DirectX;
	XMVECTOR ActorPos = XMLoadFloat3(&position);
	XMVECTOR DestPos = XMLoadFloat3(&destination);
	//printf("DEST: %f,%f,%f\n", destination.x, destination.y, destination.z);
	XMVECTOR direction = XMVectorSubtract(DestPos, ActorPos);
	direction = XMVectorSetY(direction, 0);
	direction = XMVector3Normalize(direction);
	XMVECTOR movement = XMVectorScale(direction, movement_speed * time);
	XMStoreFloat3(&position, XMVectorAdd(ActorPos, movement));
	
	XMFLOAT3 basic_head(0, 0, 1);
	XMVECTOR basic = XMLoadFloat3(&basic_head);
	XMMATRIX rotationMatrix = XMMatrixRotationY(XMConvertToRadians(rotation.y));
	XMVECTOR dir = XMVector3Transform(basic, rotationMatrix);

	float costheta = XMVectorGetX(XMVector3Dot(dir, direction));
	float radian = acos(costheta);
	float degree = XMConvertToDegrees(radian);

	XMVECTOR cross = XMVector3Cross(dir, direction);


	float wise = XMVectorGetY(cross);
	if (wise < 0)
	{
		degree = -degree;
	}
	
	if(degree*degree > 0.25) rotation.y += degree / 10;
	movement /= time;
	DirectX::XMStoreFloat3(&speed, movement);
	/*if (this->id == 2)
		printf("%f,%f,%f\n", speed.x, speed.y, speed.z);*/
}

const short NPC::find_near_player(Player*& players)
{
	return 0;

	float distance[2] = { 1E+37,1E+37 };
	for (int i = 0; i < 2; ++i)
	{
		if (players[i].sock)
		{
			distance[i] = (this->position.x - players[i].position.x) * (this->position.x - players[i].position.x)
				+ (this->position.y - players[i].position.y) * (this->position.y - players[i].position.y)
				+ (this->position.z - players[i].position.z) * (this->position.z - players[i].position.z);
		}
	}

	if (distance[0] <= distance[1]) return 0;
	else return 1;

}

void NPC::reset_graph()
{
	graph_lock.lock();
	for (auto& graph : astar_graph)
	{
		graph.second->init();
	}
	graph_lock.unlock();
}

void NPC::reset_path()
{
	path_lock.lock();
	PATH* next = this->path;
	while (this->path != nullptr)
	{
		next = next->next;
		delete this->path;
		this->path = next;
	}
	path_lock.unlock();
}

void NPC::reset_npc()
{
	reset_graph();
	reset_path();
	goalNode = nullptr;
	state = 0;
	m_floor = 1;
}

void NPC::set_manual_destination(const int& floor)
{
	this->state = 1;
	while (true)
	{
		reset_path();
		reset_graph();
		int des;
		switch (floor)
		{
		case 1:
			des = 10;
			break;
		case 2:
			des = 18;
			break;
		case 3:
			des = 39;
			break;
		case 4:
			des = 56;
			break;
		case 5:
			des = 67;
			break;
		}
		this->destination = astar_graph[des]->pos;
		this->path = aStarSearch(position, destination, astar_graph);
		if (this->path != nullptr)
			break;
	}
	this->destination = path->pos;
}

void SESSION::setGameOver()
{
	setGameNum(0);
	setState(PS_LOBBY);
}
