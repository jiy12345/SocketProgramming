#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<iostream>
#include<winsock2.h>

int main() {
	WSAData wsa_data;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
		return -1;
	}

	SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);

	std::string server_ip;
	int server_port;

	std::cout << "연결할 서버의 ip를 입력해주세요: ";
	std::cin >> server_ip;
	std::cout << "연결할 포트 번호를 입력해주세요: ";
	std::cin >> server_port;

	// 서버의 소켓 주소 설정
	sockaddr_in socket_address;
	socket_address.sin_family = AF_INET;
	socket_address.sin_addr.s_addr = inet_addr(server_ip.c_str());
	socket_address.sin_port = htons(10000);
	int return_value = connect(client_socket, (sockaddr*)&socket_address, sizeof(socket_address));
	if (return_value == SOCKET_ERROR) {
		int error_num = WSAGetLastError();
		std::cout << error_num;
		closesocket(client_socket);
		WSACleanup();
		return error_num;
	}

	// 논 블로킹 소켓으로 변경
	u_long mode = TRUE;
	ioctlsocket(client_socket, FIONBIO, &mode);

	while (true) {
		// 서버로 데이터 보내기
		char msg_to_send[256] = { 0, };
		printf("보낼 데이터: ");
		fgets(msg_to_send, 256, stdin);
		int sended_bytes = send(client_socket, msg_to_send, strlen(msg_to_send), 0);
		// 논 블로킹 소켓이므로 아직 보내지 못했을 때도 SOCKET_ERROR 반환
		if (sended_bytes == SOCKET_ERROR) {
			// 아직 보내지 못했을 때는 항상 에러가 WSAEWOULDBLOCK 값이어야 한다!
			// 그렇지 않으면 에러
			if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
				int error_num = WSAGetLastError();
				printf("%d ", error_num);
				closesocket(client_socket);
				WSACleanup();
				return error_num;

			}
		}

		char msg_to_recv[256] = { 0, };
		int recved_bytes = recv(client_socket, msg_to_recv, 256, 0);
		if (recved_bytes == SOCKET_ERROR) {
			// 아직 받지 못했을 때는 항상 에러가 WSAEWOULDBLOCK 값이어야 한다!
			// 그렇지 않으면 에러
			if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
				int error_num = WSAGetLastError();
				printf("%d ", error_num);
				closesocket(client_socket);
				WSACleanup();
				return error_num;
			}
		}
		else if (recved_bytes == 0) { // 연결이 닫혔을 경우
			closesocket(client_socket);
			WSACleanup();
			return 0;
		}
		else {
			printf("받은 데이터: %s", msg_to_recv);
		}
	}

	closesocket(client_socket);
	WSACleanup();
}