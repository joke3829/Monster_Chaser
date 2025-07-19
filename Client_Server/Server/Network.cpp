#include "stdafx.h"
#include "Network.h"


extern Network g_server;
extern mutex myMutex;

EXP_OVER::EXP_OVER(IO_OP op) : io_op(op) {
    ZeroMemory(&over, sizeof(over));
    wsabuf[0].buf = buffer;
    wsabuf[0].len = BUF_SIZE;
    accept_socket = INVALID_SOCKET;
}

SESSION::SESSION(int Num, SOCKET s) : m_uniqueNo(Num), socket(s) {
    recv_over = std::make_unique<EXP_OVER>(IO_RECV);
    player = std::make_shared<Player>(-1, "", -1); // 기본 생성
    g_server.playerManager.AddPlayer(m_uniqueNo, player); 
    do_recv();
}

SESSION::~SESSION() {
    g_server.playerManager.RemovePlayer(m_uniqueNo);
    closesocket(socket);
}

void SESSION::do_recv() {
    DWORD flags = 0;
    recv_over->wsabuf[0].buf = recv_over->buffer + remained;
    recv_over->wsabuf[0].len = BUF_SIZE - remained;
    WSARecv(socket, recv_over->wsabuf, 1, nullptr, &flags, &recv_over->over, nullptr);
}

void SESSION::do_send(void* packet) {
    EXP_OVER* over = new EXP_OVER(IO_SEND);
    int len = reinterpret_cast<unsigned char*>(packet)[0];
    memcpy(over->buffer, packet, len);
    over->wsabuf[0].len = len;
    WSASend(socket, over->wsabuf, 1, nullptr, 0, &over->over, nullptr);
}

void SESSION::process_packet(char* p) {
    char type = p[1];
    switch (type) {
    case C2S_P_LOGIN: {
        break;
    }
    case C2S_P_ENTER_ROOM: {
        cs_packet_enter_room* pkt = reinterpret_cast<cs_packet_enter_room*>(p);
        int room_Num = static_cast<int>(pkt->room_number);
        lock_guard<mutex> lock(g_server.rooms[room_Num].RoomMutex);

        if (!g_server.rooms[room_Num].IsAddPlayer()) break;

        player->local_id = g_server.rooms[room_Num].GetPlayerCount();
        player->room_num = room_Num;
        g_server.rooms[room_Num].AddPlayer(m_uniqueNo);

        for (auto& id : g_server.rooms[room_Num].id) {
            sc_packet_select_room sp;
            sp.size = sizeof(sp);
            sp.type = S2C_P_SELECT_ROOM;
            sp.Local_id = player->local_id;
            sp.room_number = static_cast<char>(room_Num);
            sp.is_self = (id == m_uniqueNo);
            g_server.users[id]->do_send(&sp);
        }

        for (int existing_id : g_server.rooms[room_Num].id) {
            if (existing_id == m_uniqueNo) continue;
            sc_packet_select_room sp_existing;
            sp_existing.size = sizeof(sp_existing);
            sp_existing.type = S2C_P_SELECT_ROOM;
            sp_existing.Local_id = g_server.users[existing_id]->player->local_id;
            sp_existing.room_number = static_cast<char>(room_Num);
            sp_existing.is_self = false;
            g_server.users[m_uniqueNo]->do_send(&sp_existing);
        }

        for (const auto& [existing_id, char_type] : g_server.rooms[room_Num].selected_characters) {
            sc_packet_pickcharacter cp;
            cp.size = sizeof(cp);
            cp.type = S2C_P_PICKCHARACTER;
            cp.C_type = char_type;
            cp.Local_id = g_server.users[existing_id]->player->local_id;
            g_server.users[m_uniqueNo]->do_send(&cp);
        }

        sc_packet_room_info rp;
        rp.size = sizeof(rp);
        rp.type = S2C_P_UPDATEROOM;
        for (int i = 0; i < g_server.rooms.size(); ++i)
            rp.room_info[i] = g_server.rooms[i].GetPlayerCount();

        for (auto& player : g_server.users)
            player.second->do_send(&rp);

        std::cout << "[로컬아이디 " << player->local_id << "을 가진 클라이언트 " << m_uniqueNo
            << "]이 " << (int)room_Num << "번 방에 입장했습니다." << std::endl;
        break;
    }
    case C2S_P_PICKCHARACTER: {
        cs_packet_pickcharacter* pkt = reinterpret_cast<cs_packet_pickcharacter*>(p);
        short Character_type = pkt->C_type;
        int room_num = static_cast<int>(pkt->room_number);

        g_server.rooms[room_num].selected_characters[m_uniqueNo] = Character_type;

        sc_packet_pickcharacter cp;
        cp.size = sizeof(cp);
        cp.type = S2C_P_PICKCHARACTER;
        cp.Local_id = player->local_id;
        cp.C_type = Character_type;

        for (auto& id : g_server.rooms[room_num].id)
            g_server.users[id]->do_send(&cp);
        break;
    }
    case C2S_P_GETREADY: {
        cs_packet_getready* pkt = reinterpret_cast<cs_packet_getready*>(p);
        int room_num = player->room_num;
        player->isReady = pkt->isReady;

        if (player->isReady)
            g_server.rooms[room_num].setReadyUser(1);
        else
            g_server.rooms[room_num].setReadyUser(-1);

        sc_packet_set_ready rp;
        rp.size = sizeof(rp);
        rp.type = S2C_P_SETREADY;
        rp.Local_id = player->local_id;
        rp.room_number = static_cast<char>(room_num);
        rp.is_ready = player->isReady;

        for (int id : g_server.rooms[room_num].id)
            if (id != m_uniqueNo)
                g_server.users[id]->do_send(&rp);

        if (g_server.rooms[room_num].GetReadyUser() == g_server.rooms[room_num].id.size()) {
            sc_packet_Ingame_start sp;
            sp.size = sizeof(sp);
            sp.type = S2C_P_ALLREADY;
            sp.room_number = static_cast<char>(room_num);

			g_server.rooms[room_num].SpawnMonsters();       //Monster Spawn
			g_server.rooms[room_num].StartGame();           //Start Game


            for (int id : g_server.rooms[room_num].id)
                g_server.users[id]->do_send(&sp);
        }
        break;
    }
    case C2S_P_ROOM_UPDATE: {
        sc_packet_room_info pkt;
        pkt.size = sizeof(pkt);
        pkt.type = S2C_P_UPDATEROOM;
        for (int i = 0; i < g_server.rooms.size(); ++i)
            pkt.room_info[i] = g_server.rooms[i].GetPlayerCount();

        for (auto& player : g_server.users)
            player.second->do_send(&pkt);
        break;
    }
    case C2S_P_MOVE: {
        cs_packet_move* pkt = reinterpret_cast<cs_packet_move*>(p);

        g_server.playerManager.SetPosition(m_uniqueNo, pkt->pos);

        sc_packet_move mp;
        mp.size = sizeof(mp);
        mp.type = S2C_P_MOVE;
        mp.Local_id = player->local_id;
        mp.pos = pkt->pos;
        mp.time = pkt->time;
        mp.state = pkt->state;

        auto duration = 3; // 간단 예시 (ping 시간 계산은 생략)
        mp.pingTime = static_cast<UINT>(duration);

        for (int id : g_server.rooms[player->room_num].id)
            g_server.users[id]->do_send(&mp);
        break;
    }

    case C2S_P_PLAYERATTACK: {
        cs_packet_player_attack* pkt = reinterpret_cast<cs_packet_player_attack*>(p);
        int monster_id = pkt->target_monster_id;

        Room& room = g_server.rooms[player->room_num];
        auto it = room.monsters.find(monster_id);
        if (it == room.monsters.end()) break;

        auto& monster = it->second;
        bool isDead = monster->TakeDamage(10); // ✅ 데미지만 주고 결과만 받아옴

        // 모두에게 히트 패킷 전송
        sc_packet_monster_hit hit{};
        hit.size = sizeof(hit);
        hit.type = S2C_P_MONSTER_HIT;
        hit.monster_id = monster_id;
        hit.current_hp = std::max(0, monster->GetHP()); // 새로 만들면 좋음
      
        for (int pid : room.id)
            g_server.users[pid]->do_send(&hit);

        if (isDead) {
            sc_packet_monster_die die{};
            die.size = sizeof(die);
            die.type = S2C_P_MONSTER_DIE;
            die.monster_id = monster_id;
            die.gold = monster->GetGold(); // 몬스터가 죽었을 때 골드 전송
            for (int pid : room.id)
                g_server.users[pid]->do_send(&die);
        }

        break;
    }

    case C2S_P_MONSTER_HIT: {

        auto* pkt = reinterpret_cast<cs_packet_monster_hit*>(p);
        
        Room& room = g_server.rooms[player->room_num];
        auto monster = room.monsters[pkt->attacker_id];
        auto target = g_server.playerManager.GetPlayer(pkt->target_player_id);


		//target->GetHP() -= pkt->damage; // 플레이어의 HP 감소

        if (monster && target) {
           // bool dead = target->TakeDamage(10); // 예시: 10 데미지

            // 클라에 피격 정보 전송
            sc_packet_player_hit hpkt;
            hpkt.size = sizeof(hpkt);
            hpkt.type = S2C_P_PLAYER_HIT;
            hpkt.target_id = pkt->target_player_id;
            hpkt.current_hp = target->GetHP();

            for (int pid : room.id)
                g_server.users[pid]->do_send(&hpkt);

            //if (dead) {
            //    // 죽었을 경우 처리 추가 가능
            //}
        }
        break;
    }
                        

    }
}

void SESSION::BroadCasting_position(const int& size) {
    for (int i = 0; i < size; ++i) {
        int num = g_server.rooms[player->room_num].getID(i);
        auto& pos = g_server.users[num]->player->GetPosition();
        std::cout << "=== [객체" << num << " position] === x: "
            << pos._41 << " y: " << pos._42 << " z: " << pos._43 << std::endl;
    }
}

Network::Network() : rooms(10) {}

void Network::Init() {
    WSAData wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    listen_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
    SOCKADDR_IN addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(listen_socket, (sockaddr*)&addr, sizeof(addr));
    listen(listen_socket, SOMAXCONN);

    iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    CreateIoCompletionPort(reinterpret_cast<HANDLE>(listen_socket), iocp, 0, 0);

    std::cout << "[서버 초기화 완료]" << std::endl;
}
