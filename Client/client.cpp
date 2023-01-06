#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<iostream>
#include<winsock2.h>

// WINAPI 스레드를 위한 함수
// 스레드로의 진입점 함수가 되기 위해 반드시 WINAPI로 선언되어야
DWORD WINAPI SendThread(LPVOID socket)
{
	SOCKET client_socket = (SOCKET)socket;
	while (true)
	{
		char msg_to_send[256] = { 0, };
		fgets(msg_to_send, 256, stdin);
		int sended_bytes = send(client_socket, msg_to_send, strlen(msg_to_send), 0);
		if (sended_bytes == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
				int error_num = WSAGetLastError();
				std::cout << "에러 발생! 에러 코드: " << error_num;
				closesocket(client_socket);
				WSACleanup();
				return error_num;
			}
		}
	}
};

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
		std::cout << "에러 발생! 에러 코드: " << error_num;
		closesocket(client_socket);
		WSACleanup();
		return error_num;
	}

	// 논 블로킹 소켓으로 변경
	u_long mode = TRUE;
	ioctlsocket(client_socket, FIONBIO, &mode);

	// WINAPI 스레드 생성
	DWORD dwThreadID;
	HANDLE hClient = CreateThread(0, 0, SendThread, (LPVOID)client_socket, 0, &dwThreadID);

	while (true) {
		char msg_to_recv[256] = { 0, };
		int recved_bytes = recv(client_socket, msg_to_recv, 256, 0);
		if (recved_bytes == SOCKET_ERROR) {
			// 아직 받지 못했을 때는 항상 에러가 WSAEWOULDBLOCK 값이어야 한다!
			// 그렇지 않으면 에러
			if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
				int error_num = WSAGetLastError();
				std::cout << "에러 발생! 에러 코드: " << error_num;
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