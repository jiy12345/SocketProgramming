#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<iostream>
#include<winsock2.h>

#include "Protocol.h"

void MakePacket(PACKET& packet, SOCKET client_socket, char* msg, short packet_type) {
	ZeroMemory(&packet, sizeof(PACKET));
	if(msg == NULL) {
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

// WINAPI 스레드를 위한 함수
// 스레드로의 진입점 함수가 되기 위해 반드시 WINAPI로 선언되어야
DWORD WINAPI SendThread(LPVOID socket)
{
	SOCKET client_socket = (SOCKET)socket;
	while (true)
	{
		char msg_to_send[256] = { 0, };
		fgets(msg_to_send, 256, stdin);
		msg_to_send[strlen(msg_to_send) - 1] = 0;
		
		// 종료 요청
		if (strcmp(msg_to_send, "exit") == 0)
		{
			break;
		}
		// 에러 발생
		if (SendMsg(client_socket, msg_to_send, PACKET_CHAR_MSG) != 0)
		{
			break;
		}
	}
	closesocket(client_socket);

	return 0;
};

int main() {
	WSAData wsa_data;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
		return -1;
	}

	SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);

	std::string server_ip = "192.168.0.174";
	int server_port = 10000;

	//std::cout << "연결할 서버의 ip를 입력해주세요: ";
	//std::cin >> server_ip;
	//std::cout << "연결할 포트 번호를 입력해주세요: ";
	//std::cin >> server_port;

	// 서버의 소켓 주소 설정
	sockaddr_in socket_address;
	socket_address.sin_family = AF_INET;
	socket_address.sin_addr.s_addr = inet_addr(server_ip.c_str());
	socket_address.sin_port = htons(10000);
	int return_value = connect(client_socket, (sockaddr*)&socket_address, sizeof(socket_address));
	if (return_value == SOCKET_ERROR) {
		int error_num = WSAGetLastError();
		std::cout << "에러 발생! 에러 코드: " << error_num << '\n';
		closesocket(client_socket);
		WSACleanup();
		return error_num;
	}

	// 논 블로킹 소켓으로 변경
	u_long mode = TRUE;
	ioctlsocket(client_socket, FIONBIO, &mode);

	// WINAPI 스레드 생성
	DWORD thread_id;
	// CREATE_SUSPENDED옵션 사용하여 사용자가 이름을 입력 한 후 Resumethread 하여 
	// 이름을 입력할 때까지는 사용자의 입력이 서버로 메시지로 보내지지 않도록 하기
	HANDLE thread_handle = CreateThread(0, 0, SendThread, (LPVOID)client_socket, CREATE_SUSPENDED, &thread_id);

	int cur_packet_header_bytes = 0;
	char packet_header[PACKET_HEADER_SIZE];
	while (true) {

		// 패킷 헤더 읽기
		char msg_to_recv[256] = { 0, };
		int recved_bytes = recv(client_socket, msg_to_recv, PACKET_HEADER_SIZE - cur_packet_header_bytes, 0);
		if (recved_bytes == 0) {
			std::cout << "서버 정상 종료\n";
			break;
		}
		if (recved_bytes == SOCKET_ERROR) {
			// 아직 받지 못했을 때는 항상 에러가 WSAEWOULDBLOCK 값이어야 한다!
			// 그렇지 않으면 에러
			if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
				int error_num = WSAGetLastError();
				std::cout << "에러 발생! 에러 코드: " << error_num << '\n';
				closesocket(client_socket);
				WSACleanup();
				return error_num;
			}
			continue;
		}
		memcpy(packet_header + cur_packet_header_bytes, msg_to_recv, sizeof(char) * recved_bytes);
		cur_packet_header_bytes += recved_bytes;
		
		// 패킷 헤더를 모두 읽었을 경우 => 실제 데이터 읽어오기
		if (cur_packet_header_bytes == PACKET_HEADER_SIZE)
		{
			// 패킷 헤더 생성 및 헤더 입력하기
			PACKET packet;
			ZeroMemory(&packet, sizeof(PACKET));
			memcpy(&packet.ph, packet_header, PACKET_HEADER_SIZE);

			// 실제 데이터 읽기
			int total_recved_packet_data_bytes = 0;
			while ((packet.ph.len - PACKET_HEADER_SIZE) > total_recved_packet_data_bytes) {
				int recved_bytes = recv(client_socket,
					&packet.msg[total_recved_packet_data_bytes],
					packet.ph.len - PACKET_HEADER_SIZE - total_recved_packet_data_bytes, 0);

				if (recved_bytes == 0)
				{
					printf("서버 정상 종료\n");
					break;
				}
				if (recved_bytes == SOCKET_ERROR)
				{
					if (WSAGetLastError() != WSAEWOULDBLOCK)
					{
						printf("서버 비정상 종료\n");
						closesocket(client_socket);
						WSACleanup();
						return 1;
					}
					continue;
				}
				total_recved_packet_data_bytes += recved_bytes;
			} 

			// 패킷 타입에 따라 다른 처리 하기
			switch (packet.ph.type)
			{
			case PACKET_CHAR_MSG:
			{
				printf("%s\n", packet.msg);
			} break;
			case PACKET_CHATNAME_REQ:
			{
				printf("이름을 입력하시오 : ");
				char szName[256] = { 0, };
				fgets(szName, 256, stdin);
				szName[strlen(szName) - 1] = 0;
				SendMsg(client_socket, szName, PACKET_NAME_REQ);
				ResumeThread(thread_handle);
			} break;
			case PACKET_JOIN_USER:
			{
				printf("%s %s\n", packet.msg, "님이 입장하였습니다.");
			}break;
			case PACKET_NAME_ACK:
			{
				printf("대화명 사용 승인\n");
			}break;
			}

			cur_packet_header_bytes = 0;
		}
	}

	CloseHandle(thread_handle);
	closesocket(client_socket);
	WSACleanup();
}