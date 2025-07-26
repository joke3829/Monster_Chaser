
// MainServer.cpp
#include "stdafx.h"
#include "Room.h"
#include "Network.h"
#include <MSWSock.h>



mutex myMutex;
Network g_server;
std::atomic<int> g_client_id = 0;
std::atomic<int>  g_monster_id = 50000;

void do_accept(Network& server) {
	SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	EXP_OVER* accept_over = new EXP_OVER(IO_ACCEPT);
	accept_over->accept_socket = c_socket;

	AcceptEx(server.listen_socket, c_socket, accept_over->buffer, 0,
		sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16,
		NULL, &accept_over->over);
}
void disconnect(int client_id) {
	auto it = g_server.users.find(client_id);
	if (it == g_server.users.end()) return;

	std::shared_ptr<SESSION> session = it->second;
	int room_num = session->player->room_num;

	// �濡�� ����
	if (room_num != -1)
	{
		std::lock_guard<std::mutex> lock(g_server.rooms[room_num].RoomMutex);
		auto& room_ids = g_server.rooms[room_num].id;
		room_ids.erase(std::remove(room_ids.begin(), room_ids.end(), client_id), room_ids.end());
		// �ٸ� Ŭ���̾�Ʈ�鿡�� Leave ��Ŷ ����
		sc_packet_leave leave_pkt;
		leave_pkt.size = sizeof(leave_pkt);
		leave_pkt.type = S2C_P_LEAVE;
		leave_pkt.Local_id = session->player->local_id;

		for (int id : g_server.rooms[room_num].id) {
			g_server.users[id]->do_send(&leave_pkt);
		}
	}


	closesocket(session->socket);
	std::lock_guard<std::mutex> lock(myMutex);
	g_server.users.unsafe_erase(client_id);

	std::cout << "[Ŭ���̾�Ʈ " << client_id << "] ������ ���� ó����." << std::endl;
}

void worker_thread(Network& server) {
	while (true) {
		DWORD bytes;
		ULONG_PTR key;
		WSAOVERLAPPED* over;

		BOOL success = GetQueuedCompletionStatus(server.iocp, &bytes, &key, &over, INFINITE);
		if (over == nullptr) continue; // ���� ��Ȳ ���

		EXP_OVER* exp = reinterpret_cast<EXP_OVER*>(over);

		if (!success || (bytes == 0 && (exp->io_op == IO_RECV || exp->io_op == IO_SEND))) {
			disconnect(static_cast<int>(key));
			if (exp->io_op == IO_SEND) delete exp;
			continue;
		}

		switch (exp->io_op) {
		case IO_ACCEPT:
		{
			int client_id = g_client_id++;
			CreateIoCompletionPort(reinterpret_cast<HANDLE>(exp->accept_socket), server.iocp, client_id, 0);
			server.users[client_id] = std::make_shared<SESSION>(client_id, exp->accept_socket);

			sc_packet_enter ep;
			ep.size = sizeof(ep);
			ep.type = S2C_P_ENTER;

			for (auto& u : server.users)
				u.second->do_send(&ep);

			for (auto& u : server.users) {
				if (u.first != client_id) {
					sc_packet_enter ep2;
					ep2.size = sizeof(ep2);
					ep2.type = S2C_P_ENTER;
					server.users[client_id]->do_send(&ep2);
				}
			}

			std::cout << "[Ŭ���̾�Ʈ " << client_id << " ] �� �����߽��ϴ�." << std::endl;
			do_accept(server);
			break;
		}
		case IO_RECV:
		{
			SESSION* user = server.users[key].get();
			char* ptr = exp->buffer;
			int processed = 0;
			bytes += user->remained;

			while (processed < bytes) {
				int size = ptr[0];
				if (size > bytes - processed)
					break;
				user->process_packet(ptr);
				ptr += size;
				processed += size;
			}

			user->remained = bytes - processed;
			if (user->remained > 0) {
				memcpy(exp->buffer, ptr, user->remained);
			}

			user->do_recv();
			break;
		}
		case IO_SEND:
			delete exp;
			break;
		}
	}
}
int main() {

	g_server.Init();

	do_accept(g_server);
	//netework�� �ִ� �������� ó�� 
	std::vector<std::thread> workers;
	int thread_count = thread::hardware_concurrency() - 2;
	for (int i = 0; i < thread_count; ++i)
		workers.emplace_back(worker_thread, std::ref(g_server));

	for (auto& th : workers) th.join();
	return 0;
}


