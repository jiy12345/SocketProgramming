#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<iostream>
#include<winsock2.h>

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
		std::cout << "에러 발생! 에러 코드: " << error_num;
		closesocket(listen_socket);
		WSACleanup();
		return error_num;
	}
	return_value = listen(listen_socket, SOMAXCONN);
	if (return_value == SOCKET_ERROR)
	{
		int error_num = WSAGetLastError();
		std::cout << "에러 발생! 에러 코드: " << error_num;
		closesocket(listen_socket);
		WSACleanup();
		return error_num;
	}

	// 접속을 시도하는 클라이언트 받아들이기
	SOCKADDR_IN clientaddr;
	int length = sizeof(clientaddr);
	SOCKET client_socket = accept(listen_socket, (sockaddr*)&clientaddr, &length);
	printf("클라이언트 접속 : IP:%s, PORT:%d\n",
		inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

	while (true) {
		char msg_to_recv[256] = { 0, };
		int recved_bytes = recv(client_socket, msg_to_recv, 256, 0);
		if (recved_bytes == SOCKET_ERROR) {
			int error_num = WSAGetLastError();
			std::cout << "에러 발생! 에러 코드: " << error_num;
			closesocket(client_socket);
			closesocket(listen_socket);
			WSACleanup();
		}
		printf("받은 데이터: %s\n", msg_to_recv);

		int sended_bytes = send(client_socket, msg_to_recv, strlen(msg_to_recv), 0);
		// 논 블로킹 소켓이므로 아직 보내지 못했을 때도 SOCKET_ERROR 반환
		if (sended_bytes == SOCKET_ERROR) {
			// 아직 보내지 못했을 때는 항상 에러가 WSAEWOULDBLOCK 값이어야 한다!
			// 그렇지 않으면 에러
			if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
				int error_num = WSAGetLastError();
				std::cout << "에러 발생! 에러 코드: " << error_num;
				closesocket(client_socket);
				closesocket(listen_socket);
				WSACleanup();
				return error_num;
			}
		}

	}

	closesocket(listen_socket);
	WSACleanup();
}