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
			std::cout << "Ŭ���̾�Ʈ ���� ����" << '\n';
			closesocket(client_socket);
			break;
		}
		if (recved_bytes == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
				int error_num = WSAGetLastError();
				std::cout << "Ŭ���̾�Ʈ ������ ����! ���� �ڵ�: " << error_num << '\n';
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
					std::cout << "���� �߻�! ���� �ڵ�: " << error_num << '\n';
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
		std::cout << "���� �߻�! ���� �ڵ�: " << error_num << '\n';
		closesocket(listen_socket);
		WSACleanup();
		return error_num;
	}
	return_value = listen(listen_socket, SOMAXCONN);
	if (return_value == SOCKET_ERROR)
	{
		int error_num = WSAGetLastError();
		std::cout << "���� �߻�! ���� �ڵ�: " << error_num << '\n';
		closesocket(listen_socket);
		WSACleanup();
		return error_num;
	}

	while (true) {
		SOCKADDR_IN client_addr;
		int addr_length = sizeof(client_addr);
		// ������ �õ��ϴ� Ŭ���̾�Ʈ �޾Ƶ��̱�
		SOCKET client_socket = accept(listen_socket, (sockaddr*)&client_addr, &addr_length);
		if (client_socket == SOCKET_ERROR)
		{
			int error_num = WSAGetLastError();
			std::cout << "���� �߻�! ���� �ڵ�: " << error_num << '\n';
			closesocket(listen_socket);
			WSACleanup();
			return error_num;
		}

		printf("Ŭ���̾�Ʈ ���� : IP:%s, PORT:%d\n",
			inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

		// ����� Ŭ���̾�Ʈ���� ������ �����
		DWORD thread_id;
		HANDLE hClient = CreateThread(0, 0, ServerThread,
			(LPVOID)client_socket, 0, &thread_id);     
	}

	closesocket(listen_socket);
	WSACleanup();
}