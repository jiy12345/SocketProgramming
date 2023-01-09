#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<iostream>
#include<winsock2.h>

DWORD WINAPI ServerThread(LPVOID lpThreadParameter)
{
	SOCKET client_socket = (SOCKET)lpThreadParameter;
	while (true)
	{
		char msg_to_recv[256] = { 0, };
		int recved_bytes = recv(client_socket, msg_to_recv, 256, 0);
		if (recved_bytes == 0)
		{
			std::cout << "클라이언트 정상 종료" << '\n';
			closesocket(client_socket);
			break;
		}
		if (recved_bytes == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
				int error_num = WSAGetLastError();
				std::cout << "클라이언트 비정상 종료! 에러 코드: " << error_num << '\n';
				closesocket(client_socket);
				return error_num;
			}
		}
		else
		{
			printf("%s\n", msg_to_recv);
		}

		if (recved_bytes > 0)
		{
			int iSendBytes = send(client_socket, msg_to_recv, strlen(msg_to_recv), 0);
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

		// 연결된 클라이언트마다 스레드 만들기
		DWORD thread_id;
		HANDLE hClient = CreateThread(0, 0, ServerThread,
			(LPVOID)client_socket, 0, &thread_id);     
	}

	closesocket(listen_socket);
	WSACleanup();
}