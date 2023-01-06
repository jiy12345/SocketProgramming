#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<iostream>
#include<winsock2.h>

// WINAPI �����带 ���� �Լ�
// ��������� ������ �Լ��� �Ǳ� ���� �ݵ�� WINAPI�� ����Ǿ��
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
				std::cout << "���� �߻�! ���� �ڵ�: " << error_num;
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

	std::cout << "������ ������ ip�� �Է����ּ���: ";
	std::cin >> server_ip;
	std::cout << "������ ��Ʈ ��ȣ�� �Է����ּ���: ";
	std::cin >> server_port;

	// ������ ���� �ּ� ����
	sockaddr_in socket_address;
	socket_address.sin_family = AF_INET;
	socket_address.sin_addr.s_addr = inet_addr(server_ip.c_str());
	socket_address.sin_port = htons(10000);
	int return_value = connect(client_socket, (sockaddr*)&socket_address, sizeof(socket_address));
	if (return_value == SOCKET_ERROR) {
		int error_num = WSAGetLastError();
		std::cout << "���� �߻�! ���� �ڵ�: " << error_num;
		closesocket(client_socket);
		WSACleanup();
		return error_num;
	}

	// �� ���ŷ �������� ����
	u_long mode = TRUE;
	ioctlsocket(client_socket, FIONBIO, &mode);

	// WINAPI ������ ����
	DWORD dwThreadID;
	HANDLE hClient = CreateThread(0, 0, SendThread, (LPVOID)client_socket, 0, &dwThreadID);

	while (true) {
		char msg_to_recv[256] = { 0, };
		int recved_bytes = recv(client_socket, msg_to_recv, 256, 0);
		if (recved_bytes == SOCKET_ERROR) {
			// ���� ���� ������ ���� �׻� ������ WSAEWOULDBLOCK ���̾�� �Ѵ�!
			// �׷��� ������ ����
			if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
				int error_num = WSAGetLastError();
				std::cout << "���� �߻�! ���� �ڵ�: " << error_num;
				closesocket(client_socket);
				WSACleanup();
				return error_num;
			}
		}
		else if (recved_bytes == 0) { // ������ ������ ���
			closesocket(client_socket);
			WSACleanup();
			return 0;
		}
		else {
			printf("���� ������: %s", msg_to_recv);
		}
	}

	closesocket(client_socket);
	WSACleanup();
}