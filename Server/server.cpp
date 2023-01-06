#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<iostream>
#include<winsock2.h>

int main() {
	WSAData wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		return -1;
	}

	// ����Ǿ�� �ϴ� ��� ����� ������ �����Ƿ�, 0���� ���� �⺻ ���� �����ϵ��� �ϸ�ȴ�!
	SOCKET listen_socket = socket(AF_INET, SOCK_STREAM, 0);

	// �������� ���� ����
	// � �ּҿ����� ������ �� �ֵ��� ��� �ּҿ� ���� �����ϵ��� ����
	SOCKADDR_IN sa;
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	sa.sin_port = htons(10000);
	int return_value = bind(listen_socket, (sockaddr*)&sa, sizeof(sa));
	if (return_value == SOCKET_ERROR)
	{
		int error_num = WSAGetLastError();
		std::cout << "���� �߻�! ���� �ڵ�: " << error_num;
		closesocket(listen_socket);
		WSACleanup();
		return error_num;
	}
	return_value = listen(listen_socket, SOMAXCONN);
	if (return_value == SOCKET_ERROR)
	{
		int error_num = WSAGetLastError();
		std::cout << "���� �߻�! ���� �ڵ�: " << error_num;
		closesocket(listen_socket);
		WSACleanup();
		return error_num;
	}

	// ������ �õ��ϴ� Ŭ���̾�Ʈ �޾Ƶ��̱�
	SOCKADDR_IN clientaddr;
	int length = sizeof(clientaddr);
	SOCKET client_socket = accept(listen_socket, (sockaddr*)&clientaddr, &length);
	printf("Ŭ���̾�Ʈ ���� : IP:%s, PORT:%d\n",
		inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

	while (true) {
		char msg_to_recv[256] = { 0, };
		int recved_bytes = recv(client_socket, msg_to_recv, 256, 0);
		if (recved_bytes == SOCKET_ERROR) {
			int error_num = WSAGetLastError();
			std::cout << "���� �߻�! ���� �ڵ�: " << error_num;
			closesocket(client_socket);
			closesocket(listen_socket);
			WSACleanup();
		}
		printf("���� ������: %s\n", msg_to_recv);

		int sended_bytes = send(client_socket, msg_to_recv, strlen(msg_to_recv), 0);
		// �� ���ŷ �����̹Ƿ� ���� ������ ������ ���� SOCKET_ERROR ��ȯ
		if (sended_bytes == SOCKET_ERROR) {
			// ���� ������ ������ ���� �׻� ������ WSAEWOULDBLOCK ���̾�� �Ѵ�!
			// �׷��� ������ ����
			if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
				int error_num = WSAGetLastError();
				std::cout << "���� �߻�! ���� �ڵ�: " << error_num;
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