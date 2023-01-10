#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<iostream>
#include<winsock2.h>
#include<list>

#include"Protocol.h"

struct User
{
	SOCKET client_socket;
	SOCKADDR_IN socket_address;
	char client_name[9] = { 0, };
	char recved_msg[255] = { 0, };
	int total_recved_bytes = 0;
};

std::list<User> userlist;

void MakePacket(PACKET& packet, SOCKET client_socket, char* msg, short packet_type) {
    ZeroMemory(&packet, sizeof(PACKET));
    if (msg == NULL) {
        packet.ph.len = PACKET_HEADER_SIZE;
    }
    else {
        packet.ph.len = strlen(msg) + PACKET_HEADER_SIZE;
        memcpy(packet.msg, msg, strlen(msg));
    }
    packet.ph.type = packet_type;
}

int SendMsg(SOCKET client_socket, char* msg, short packet_type)
{
	PACKET packet;
	MakePacket(packet, client_socket, msg, packet_type);

	char* msgSend = (char*)&packet;

	int iSendBytes = send(client_socket, msgSend, packet.ph.len, 0);
	if (iSendBytes == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			int error_num = WSAGetLastError();
			std::cout << "에러 발생! 에러 코드: " << error_num << '\n';
			closesocket(client_socket);
			return error_num;
		}
	}
	return 0;
}


DWORD WINAPI ServerThread(LPVOID lpThreadParameter)
{
    while (1)
    {
        // 모든 접속한 유저에 대해 반복
        for (auto iter_user = userlist.begin(); userlist.end() != iter_user;)
        {
            // 패킷 헤더 받아오기
            int recved_bytes = recv(iter_user->client_socket, iter_user->recved_msg,
                PACKET_HEADER_SIZE - iter_user->total_recved_bytes, 0);
            if (recved_bytes == 0)
            {
                printf("클라이언트 접속 종료 : IP:%s, PORT:%d\n",
                    inet_ntoa(iter_user->socket_address.sin_addr), ntohs(iter_user->socket_address.sin_port));
                closesocket(iter_user->client_socket);
                iter_user = userlist.erase(iter_user);
                continue;
            }

            if (recved_bytes == SOCKET_ERROR)
            {
                DWORD last_error = WSAGetLastError();
                if (last_error != WSAEWOULDBLOCK)
                {
                    //WSAEWOULDBLOCK 아니라면 오류!
                    closesocket(iter_user->client_socket);
                    iter_user = userlist.erase(iter_user);
                }
                else {
                    iter_user++;
                }
                continue;
            }

            PACKET packet;
            ZeroMemory(&packet, sizeof(PACKET));

            iter_user->total_recved_bytes += recved_bytes;
            if (iter_user->total_recved_bytes == PACKET_HEADER_SIZE)
            {
                memcpy(&packet.ph, iter_user->recved_msg, PACKET_HEADER_SIZE);

                char* msg = (char*)&packet;
                int total_recved_packet_data_bytes = 0;
                // 데이터 읽기
                while((packet.ph.len - PACKET_HEADER_SIZE) > total_recved_packet_data_bytes) {
                    int recved_bytes = recv(iter_user->client_socket,
                        &packet.msg[total_recved_packet_data_bytes],
                        packet.ph.len - PACKET_HEADER_SIZE - total_recved_packet_data_bytes, 0);

                    if (recved_bytes == 0)
                    {
                        printf("연결 정상 종료\n");
                        break;
                    }
                    if (recved_bytes == SOCKET_ERROR)
                    {
                        if (WSAGetLastError() != WSAEWOULDBLOCK)
                        {
                            printf("연결 비정상 종료\n");
                            closesocket(iter_user->client_socket);
                            WSACleanup();
                            return 1;
                        }
                        continue;
                    }
                    total_recved_packet_data_bytes += recved_bytes;
                }
            }

            // 패킷 헤더에 따라 다른 처리
            switch (packet.ph.type)
            {
            case PACKET_CHAR_MSG:
            {
                printf("[%s]%s\n", iter_user->client_name,
                    packet.msg);
                packet.ph.len += strlen(iter_user->client_name) + 2;
                std::string msg_to_broadcast = "[";
                msg_to_broadcast += iter_user->client_name;
                msg_to_broadcast += "]";
                msg_to_broadcast += packet.msg;
                ZeroMemory(packet.msg, 2048);
                memcpy(packet.msg, msg_to_broadcast.c_str(), msg_to_broadcast.size());
            }break;
            case PACKET_NAME_REQ:
            {
                memcpy(iter_user->client_name,
                    packet.msg, strlen(packet.msg));
                packet.ph.type = PACKET_JOIN_USER;
                SendMsg(iter_user->client_socket, nullptr, PACKET_NAME_ACK);
            }break;
            }

            // 사용자가 입력한 내용 모든 연결된 클라이언트에 전파
            for (auto iter_user_broadcast = userlist.begin(); userlist.end() != iter_user_broadcast;)
            {
                if (packet.ph.type == PACKET_JOIN_USER)
                {
                    // 입장 내용은 자신한테는 보이지 않도록 하기
                    if (iter_user == iter_user_broadcast)
                    {
                        iter_user_broadcast++;
                        continue;
                    }
                }
                int iSendBytes = send(iter_user_broadcast->client_socket, (char*)&packet,
                    packet.ph.len, 0);

                if (iSendBytes == SOCKET_ERROR)
                {
                    if (WSAGetLastError() != WSAEWOULDBLOCK)
                    {
                        //WSAEWOULDBLOCK 아니라면 오류!
                        printf("클라이언트 접속 비정상 종료 : IP:%s, PORT:%d\n",
                            inet_ntoa(iter_user_broadcast->socket_address.sin_addr), ntohs(iter_user_broadcast->socket_address.sin_port));
                        closesocket(iter_user_broadcast->client_socket);
                        iter_user_broadcast = userlist.erase(iter_user_broadcast);
                        continue;
                    }
                }
                
                iter_user_broadcast++;
            }

            ZeroMemory(&packet, sizeof(PACKET));
            iter_user->total_recved_bytes = 0;

            iter_user++;
        }
    }

	return 0;
};

int main() {
	WSAData wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		return -1;
	}

	// 연결되어야 하는 통신 방법이 정해져 있으므로, 0으로 적어 기본 값을 설정하도록 하면된다!
	SOCKET listen_socket = socket(AF_INET, SOCK_STREAM, 0);

	// 서버측의 소켓 생성
	// 어떤 주소에서든 접근할 수 있도록 모든 주소와 연결 가능하도록 설정
	SOCKADDR_IN sa;
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	sa.sin_port = htons(10000);
	int return_value = bind(listen_socket, (sockaddr*)&sa, sizeof(sa));
	if (return_value == SOCKET_ERROR)
	{
		int error_num = WSAGetLastError();
		std::cout << "에러 발생! 에러 코드: " << error_num << '\n';
		closesocket(listen_socket);
		WSACleanup();
		return error_num;
	}
	return_value = listen(listen_socket, SOMAXCONN);
	if (return_value == SOCKET_ERROR)
	{
		int error_num = WSAGetLastError();
		std::cout << "에러 발생! 에러 코드: " << error_num << '\n';
		closesocket(listen_socket);
		WSACleanup();
		return error_num;
	}

    DWORD thread_id;
    HANDLE client_handle = CreateThread(0, 0, ServerThread, 0, 0, &thread_id);

	while (true) {
		SOCKADDR_IN client_addr;
		int addr_length = sizeof(client_addr);
		// 접속을 시도하는 클라이언트 받아들이기
		SOCKET client_socket = accept(listen_socket, (sockaddr*)&client_addr, &addr_length);
		if (client_socket == SOCKET_ERROR)
		{
			int error_num = WSAGetLastError();
			std::cout << "에러 발생! 에러 코드: " << error_num << '\n';
			closesocket(listen_socket);
			WSACleanup();
			return error_num;
		}
		printf("클라이언트 접속 : IP:%s, PORT:%d\n",
			inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        u_long iMode = TRUE;
        ioctlsocket(client_socket, FIONBIO, &iMode);
        
        // 접속한 클라이언트를 나타낼 User 객체 생성 및 등록
        User user;
        user.client_socket = client_socket;
        user.socket_address = client_addr;
        userlist.push_back(user);

        // 이름 요청 보내기
        SendMsg(client_socket, nullptr, PACKET_CHATNAME_REQ);
	}

	closesocket(listen_socket);
	WSACleanup();
}